# Requirements traceability

| Capability | Implementation evidence | Verification evidence |
|---|---|---|
| Arduino Uno controller | `smart_irrigation.ino`, `diagram.json` | Arduino Uno compile |
| Soil moisture input | A0 and `moisturePercentFromAdc()` | Dry/wet and hysteresis states |
| LDR input | A1 and `lightCategory()` | Bright-state model evidence |
| Temperature/humidity | D2 and DHT library | DHT update state |
| LCD output | A4/A5 and `LiquidCrystal_PCF8574` | LCD/I2C startup and state text |
| Rain interlock | D3, active LOW | Rain-stop states |
| Tank interlock | D4, active LOW | Tank fault and runtime shutdown |
| Relay/pump control | D6 transistor driver | Irrigating and safe OFF states |
| Buzzer warning | D7 | Tank-empty event |
| Status LEDs | D8/D9 and relay load LED | Circuit-only state screenshots |
| Hysteresis | 35% start / 60% stop | Wet-threshold stop state |
| UART telemetry | 9600 baud reporting | Periodic report state |
