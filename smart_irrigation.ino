#include <DHT.h>
#include <LiquidCrystal_PCF8574.h>
#include <Wire.h>
#include <math.h>


#ifndef SMART_IRRIGATION_DHT_TYPE
#define SMART_IRRIGATION_DHT_TYPE DHT22
#endif

#ifndef SMART_IRRIGATION_RELAY_ACTIVE_HIGH
#define SMART_IRRIGATION_RELAY_ACTIVE_HIGH 1
#endif

namespace irrigation {

enum class StopReason : uint8_t {
  None,
  SoilWet,
  Rain,
  TankEmpty,
  InvalidConfiguration,
};

struct DecisionInput {
  uint8_t moisturePercent;
  bool rainDetected;
  bool waterAvailable;
  bool pumpOn;
};

struct DecisionOutput {
  bool pumpOn;
  bool tankFault;
  StopReason reason;
};

uint8_t moisturePercentFromAdc(int rawAdc, int dryAdc, int wetAdc) {
  if (dryAdc == wetAdc) {
    return 0;
  }

  long percent = 0;
  if (dryAdc > wetAdc) {
    percent = static_cast<long>(dryAdc - rawAdc) * 100L /
              static_cast<long>(dryAdc - wetAdc);
  } else {
    percent = static_cast<long>(rawAdc - dryAdc) * 100L /
              static_cast<long>(wetAdc - dryAdc);
  }

  if (percent < 0) {
    percent = 0;
  }
  if (percent > 100) {
    percent = 100;
  }
  return static_cast<uint8_t>(percent);
}

DecisionOutput decidePump(const DecisionInput& input,
                          uint8_t startThreshold,
                          uint8_t stopThreshold) {
  if (startThreshold >= stopThreshold) {
    return {false, true, StopReason::InvalidConfiguration};
  }

  if (!input.waterAvailable) {
    return {false, true, StopReason::TankEmpty};
  }

  if (input.rainDetected) {
    return {false, false, StopReason::Rain};
  }

  if (input.pumpOn && input.moisturePercent >= stopThreshold) {
    return {false, false, StopReason::SoilWet};
  }

  if (!input.pumpOn && input.moisturePercent <= startThreshold) {
    return {true, false, StopReason::None};
  }

  return {input.pumpOn, false, StopReason::None};
}

}  

namespace {

constexpr uint8_t SOIL_MOISTURE_PIN = A0;
constexpr uint8_t LDR_PIN = A1;
constexpr uint8_t DHT_PIN = 2;
constexpr uint8_t RAIN_PIN = 3;
constexpr uint8_t TANK_PIN = 4;
constexpr uint8_t RELAY_PIN = 6;
constexpr uint8_t BUZZER_PIN = 7;
constexpr uint8_t GREEN_LED_PIN = 8;
constexpr uint8_t RED_LED_PIN = 9;

constexpr uint8_t LCD_ADDRESS = 0x27;
constexpr uint8_t LCD_COLUMNS = 20;
constexpr uint8_t LCD_ROWS = 4;

constexpr int MOISTURE_DRY_ADC = 850;
constexpr int MOISTURE_WET_ADC = 350;
constexpr uint8_t START_WATERING_PERCENT = 35;
constexpr uint8_t STOP_WATERING_PERCENT = 60;
constexpr int LIGHT_BRIGHT_THRESHOLD = 512;

constexpr unsigned long SENSOR_POLL_INTERVAL_MS = 100;
constexpr unsigned long DHT_READ_INTERVAL_MS = 2500;
constexpr unsigned long LCD_UPDATE_INTERVAL_MS = 250;
constexpr unsigned long UART_REPORT_INTERVAL_MS = 1000;
constexpr unsigned long BUZZER_PERIOD_MS = 1000;
constexpr unsigned long BUZZER_ON_MS = 250;

constexpr uint8_t RELAY_ON_LEVEL =
    SMART_IRRIGATION_RELAY_ACTIVE_HIGH ? HIGH : LOW;
constexpr uint8_t RELAY_OFF_LEVEL =
    SMART_IRRIGATION_RELAY_ACTIVE_HIGH ? LOW : HIGH;

LiquidCrystal_PCF8574 lcd(LCD_ADDRESS);
DHT dht(DHT_PIN, SMART_IRRIGATION_DHT_TYPE);

struct SensorData {
  int soilRaw = 0;
  uint8_t soilPercent = 0;
  int ldrRaw = 0;
  bool rainDetected = false;
  bool waterAvailable = false;
  bool dhtValid = false;
  float temperatureC = NAN;
  float humidityPercent = NAN;
  bool pumpOn = false;
  bool tankFault = false;
  irrigation::StopReason stopReason = irrigation::StopReason::None;
};

SensorData data;
unsigned long lastSensorPollAt = 0;
unsigned long lastDhtReadAt = 0;
unsigned long lastLcdUpdateAt = 0;
unsigned long lastUartReportAt = 0;
unsigned long lastBuzzerAt = 0;
unsigned long startupDisplayUntil = 0;
bool hasDhtRead = false;
bool buzzerIsOn = false;

void writeLcdLine(uint8_t row, const char* text) {
  lcd.setCursor(0, row);
  uint8_t column = 0;
  while (text[column] != '\0' && column < LCD_COLUMNS) {
    lcd.print(text[column]);
    ++column;
  }
  while (column < LCD_COLUMNS) {
    lcd.print(' ');
    ++column;
  }
}

const char* lightCategory() {
  return data.ldrRaw < LIGHT_BRIGHT_THRESHOLD ? "BRIGHT" : "DARK";
}

const char* statusText() {
  if (data.tankFault) {
    return "TANK EMPTY";
  }
  if (data.rainDetected) {
    return "RAIN STOP";
  }
  return data.pumpOn ? "IRRIGATING" : "NORMAL";
}

const char* reasonText(irrigation::StopReason reason) {
  switch (reason) {
    case irrigation::StopReason::SoilWet:
      return "SOIL_WET";
    case irrigation::StopReason::Rain:
      return "RAIN";
    case irrigation::StopReason::TankEmpty:
      return "TANK_EMPTY";
    case irrigation::StopReason::InvalidConfiguration:
      return "INVALID_CONFIG";
    case irrigation::StopReason::None:
    default:
      return "NONE";
  }
}

void initializeSafeOutputs() {

  digitalWrite(RELAY_PIN, RELAY_OFF_LEVEL);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  noTone(BUZZER_PIN);
}

void readDhtIfDue(unsigned long now) {
  if (hasDhtRead && now - lastDhtReadAt < DHT_READ_INTERVAL_MS) {
    return;
  }

  lastDhtReadAt = now;
  hasDhtRead = true;
  const float temperature = dht.readTemperature();
  const float humidity = dht.readHumidity();
  data.dhtValid = !isnan(temperature) && !isnan(humidity);
  if (data.dhtValid) {
    data.temperatureC = temperature;
    data.humidityPercent = humidity;
  } else {
    data.temperatureC = NAN;
    data.humidityPercent = NAN;
  }
}

void readSensors(unsigned long now) {
  data.soilRaw = analogRead(SOIL_MOISTURE_PIN);
  data.soilPercent = irrigation::moisturePercentFromAdc(
      data.soilRaw, MOISTURE_DRY_ADC, MOISTURE_WET_ADC);
  data.ldrRaw = analogRead(LDR_PIN);

  // Both simulation buttons and physical switches pull the input to ground.
  data.rainDetected = digitalRead(RAIN_PIN) == LOW;
  data.waterAvailable = digitalRead(TANK_PIN) == LOW;
  readDhtIfDue(now);
}

void applyPumpOutputs() {
  digitalWrite(RELAY_PIN, data.pumpOn ? RELAY_ON_LEVEL : RELAY_OFF_LEVEL);
  digitalWrite(GREEN_LED_PIN, data.pumpOn ? HIGH : LOW);
  digitalWrite(RED_LED_PIN, data.tankFault ? HIGH : LOW);
}

void updatePumpDecision() {
  const irrigation::DecisionInput input{
      data.soilPercent, data.rainDetected, data.waterAvailable, data.pumpOn};
  const irrigation::DecisionOutput decision = irrigation::decidePump(
      input, START_WATERING_PERCENT, STOP_WATERING_PERCENT);

  if (decision.pumpOn != data.pumpOn) {
    Serial.print("EVENT pump=");
    Serial.print(decision.pumpOn ? "ON" : "OFF");
    Serial.print(" reason=");
    Serial.println(reasonText(decision.reason));
  }

  data.pumpOn = decision.pumpOn;
  data.tankFault = decision.tankFault;
  data.stopReason = decision.reason;
  applyPumpOutputs();
}

void updateBuzzer(unsigned long now) {
  if (!data.tankFault) {
    noTone(BUZZER_PIN);
    buzzerIsOn = false;
    return;
  }

  if (!buzzerIsOn && now - lastBuzzerAt >= BUZZER_PERIOD_MS) {
    tone(BUZZER_PIN, 2000, BUZZER_ON_MS);
    Serial.println("EVENT buzzer=ON reason=TANK_EMPTY");
    buzzerIsOn = true;
    lastBuzzerAt = now;
  } else if (buzzerIsOn && now - lastBuzzerAt >= BUZZER_ON_MS) {
    noTone(BUZZER_PIN);
    buzzerIsOn = false;
  }
}

void updateLcd(unsigned long now) {
  if (now - lastLcdUpdateAt < LCD_UPDATE_INTERVAL_MS ||
      now < startupDisplayUntil) {
    return;
  }
  lastLcdUpdateAt = now;

  char line[21];
  snprintf(line, sizeof(line), "STATUS: %s", statusText());
  writeLcdLine(0, line);

  snprintf(line, sizeof(line), "Soil:%3u%% %s", data.soilPercent,
           data.soilPercent <= START_WATERING_PERCENT ? "DRY" : "WET");
  writeLcdLine(1, line);

  lcd.setCursor(0, 2);
  lcd.print("T:");
  if (data.dhtValid) {
    lcd.print(data.temperatureC, 1);
  } else {
    lcd.print("NA");
  }
  lcd.print("C H:");
  if (data.dhtValid) {
    lcd.print(data.humidityPercent, 0);
  } else {
    lcd.print("NA");
  }
  lcd.print('%');
  lcd.print("                ");

  snprintf(line, sizeof(line), "L:%s R:%s T:%s", lightCategory(),
           data.rainDetected ? "YES" : "NO",
           data.waterAvailable ? "OK" : "EMP");
  writeLcdLine(3, line);
}

void printOptionalFloat(float value) {
  if (isnan(value)) {
    Serial.print("NA");
  } else {
    Serial.print(value, 1);
  }
}

void printUartReport(unsigned long now) {
  if (now - lastUartReportAt < UART_REPORT_INTERVAL_MS) {
    return;
  }
  lastUartReportAt = now;

  Serial.print("REPORT soil_raw=");
  Serial.print(data.soilRaw);
  Serial.print(" soil_pct=");
  Serial.print(data.soilPercent);
  Serial.print(" temp_c=");
  printOptionalFloat(data.temperatureC);
  Serial.print(" humidity_pct=");
  printOptionalFloat(data.humidityPercent);
  Serial.print(" light=");
  Serial.print(lightCategory());
  Serial.print(" light_raw=");
  Serial.print(data.ldrRaw);
  Serial.print(" rain=");
  Serial.print(data.rainDetected ? "YES" : "NO");
  Serial.print(" tank=");
  Serial.print(data.waterAvailable ? "AVAILABLE" : "EMPTY");
  Serial.print(" pump=");
  Serial.print(data.pumpOn ? "ON" : "OFF");
  Serial.print(" status=");
  Serial.println(statusText());
}

}  // namespace

void setup() {
  initializeSafeOutputs();

  pinMode(RAIN_PIN, INPUT_PULLUP);
  pinMode(TANK_PIN, INPUT_PULLUP);
  pinMode(SOIL_MOISTURE_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);

  Serial.begin(9600);
  dht.begin();
  lcd.begin(LCD_COLUMNS, LCD_ROWS, Wire);
  lcd.setBacklight(255);
  Wire.beginTransmission(LCD_ADDRESS);
  const uint8_t lcdI2cStatus = Wire.endTransmission();
  Serial.print("LCD I2C status=");
  Serial.println(lcdI2cStatus);
  writeLcdLine(0, "SMART IRRIGATION");
  writeLcdLine(1, "Starting safely...");
  writeLcdLine(2, "Pump: OFF");
  writeLcdLine(3, "Sensors initializing");
  startupDisplayUntil = millis() + 1500;

  Serial.println("SMART IRRIGATION READY");
  Serial.println("Relay/load initialized OFF");
}

void loop() {
  const unsigned long now = millis();
  if (now - lastSensorPollAt >= SENSOR_POLL_INTERVAL_MS) {
    lastSensorPollAt = now;
    readSensors(now);
    updatePumpDecision();
  }

  updateBuzzer(now);
  updateLcd(now);
  printUartReport(now);
}
