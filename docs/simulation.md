# Simulation guide

## Browser run

The project uses Velxio as the Linux-friendly visual simulator. The circuit
definition is [`../diagram.json`](../diagram.json), and the project metadata is
[`../velxio.toml`](../velxio.toml).

1. Open <https://velxio.dev/editor/>.
2. Open the project containing `diagram.json`, `velxio.toml`, and
   `smart_irrigation.ino`.
3. Select **Arduino Uno**.
4. Add or confirm the exact libraries from [`../libraries.txt`](../libraries.txt):
   - `DHT sensor library` 1.4.7
   - `Adafruit Unified Sensor` 1.1.15
   - `LiquidCrystal_PCF8574` 2.3.0
5. Compile, then run.
6. Open Serial at 9600 baud for the textual telemetry stream.

## State exercises

- Turn the soil potentiometer toward the dry side to request irrigation.
- Release the tank-available control to verify tank-empty shutdown.
- Press the rain control to verify the rain safety stop.
- Move the soil value above the wet threshold to verify hysteresis stop.
- Change DHT22 temperature and humidity to verify environmental telemetry.
- Change the photoresistor illumination to inspect the light classification.

## Visual evidence

The screenshots in [`evidence/`](evidence/) are circuit-only views: the entire
wiring and component set is visible on the dark simulator canvas while editor,
code, Serial, Scope, and Output panels are hidden. UART evidence is recorded
separately in the verification notes.
