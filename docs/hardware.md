# Hardware reference

## Components

- Arduino Uno R3
- 20x4 I2C LCD at address `0x27`
- Soil-moisture analog sensor/potentiometer
- LDR/photoresistor analog sensor
- DHT22 temperature/humidity sensor
- Rain pushbutton, active LOW
- Tank-available pushbutton, active LOW
- Relay coil driven by a 2N2222 low-side transistor
- Buzzer
- Green pump-status LED
- Red tank-fault LED
- Blue pump-load LED connected through the relay's normally-open contact
- 1 kΩ transistor base resistor and 220 Ω LED resistors

## Pin map

| Signal | Pin | Direction | Notes |
|---|---:|---|---|
| Soil moisture | A0 | Analog input | Calibrated from dry ADC 850 to wet ADC 350 |
| LDR | A1 | Analog input | `<512` is reported as `BRIGHT` |
| DHT22 data | D2 | Digital input | 2.5 s minimum read interval |
| Rain | D3 | Digital input | LOW means rain detected |
| Tank available | D4 | Digital input | LOW means water available |
| Relay driver | D6 | Digital output | Active HIGH in the simulator profile |
| Buzzer | D7 | Digital output | Periodic tank-empty warning |
| Pump status | D8 | Digital output | Green LED |
| Tank fault | D9 | Digital output | Red LED |
| LCD SDA | A4 | I2C | Shared I2C bus |
| LCD SCL | A5 | I2C | Shared I2C bus |

## Control policy

The controller applies safety checks before watering:

1. Invalid threshold configuration forces the pump OFF and reports a fault.
2. An unavailable tank forces the pump OFF and raises the tank fault.
3. Rain forces the pump OFF.
4. If watering is active, soil at or above 60% stops the pump.
5. If watering is inactive, soil at or below 35% starts the pump.
6. Intermediate moisture keeps the previous pump state, implementing
   hysteresis.

The relay output is initialized to its OFF level before the pin is configured
as an output, preventing an unintended startup pulse.
