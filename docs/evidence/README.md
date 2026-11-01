# Circuit-only evidence

These eleven PNGs show the complete simulator canvas for each verified
operating state. The editor, code, Serial, Scope, and Output panels are hidden
so the Arduino, LCD, sensors, controls, wiring, relay, buzzer, transistor, and
status indicators remain visible together.

| State | Screenshot |
|---|---|
| Power-up / safe OFF | `01_power_up_safe_off.png` |
| Tank empty fault | `02_tank_empty_fault.png` |
| Dry soil irrigation | `03_dry_soil_irrigating.png` |
| Wet soil normal stop | `04_wet_soil_normal_stop.png` |
| Rain stop with dry soil | `05_rain_stop_dry_soil.png` |
| Wet soil plus rain | `06_wet_soil_rain_stop.png` |
| DHT update | `07_dht_temperature_humidity_update.png` |
| Hysteresis stop | `08_hysteresis_wet_threshold_stop.png` |
| UART report state | `09_uart_periodic_report.png` |
| LDR bright state | `10_ldr_bright_model_output.png` |
| Tank-empty shutdown | `11_tank_empty_shutdown.png` |

Exact UART lines and verification boundaries are documented in
[`../verification.md`](../verification.md).
