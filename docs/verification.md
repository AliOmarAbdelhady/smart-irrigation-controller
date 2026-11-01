# Verification summary

## Completed checks

- Arduino Uno compilation: PASS
- `diagram.json` parsing: PASS
- Pinned library declarations: PASS
- LCD/I2C startup status: PASS
- Safe relay/load initialization: PASS
- Tank-empty fault and buzzer event: PASS
- Dry soil with water available starts irrigation: PASS
- Rain safety stop: PASS
- Wet-soil hysteresis stop: PASS
- Tank becoming empty during irrigation shuts the pump down: PASS
- DHT temperature/humidity update: PASS
- Repeating UART reports at 9600 baud: PASS
- Circuit-only visual evidence for eleven operating states: PASS

## Build footprint

The verified Arduino Uno build used 12,574 bytes of flash and 996 bytes of
SRAM. The exact library versions are pinned in [`../libraries.txt`](../libraries.txt).

## Known boundary

The hosted Velxio photoresistor model exposed only a low raw ADC range during
the verified browser run. Values remained below the firmware's midpoint
threshold of 512, so the live model demonstrated the `BRIGHT` classification
but did not produce a valid `DARK` transition. This is a simulator-model
limitation; the physical LDR divider can be calibrated across the full Arduino
ADC range.
