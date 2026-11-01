# Smart Irrigation Controller

An Arduino Uno smart-irrigation controller with moisture-based watering,
rain and tank safety interlocks, hysteresis, LCD telemetry, environmental
sensing, status indicators, and UART reporting.

The project is designed to be understandable in hardware, reproducible in a
Linux-friendly simulator, and easy to extend to a physical Arduino Uno.

## Highlights

- Starts with relay, pump indicator, buzzer, and fault indicators safely OFF.
- Starts watering when soil moisture is at or below 35%.
- Stops watering at or above 60%, providing a 25-point hysteresis band.
- Blocks watering during rain or when the tank is unavailable.
- Reports soil, light, temperature, humidity, rain, tank, pump, and status data
  on a 20x4 I2C LCD and over UART at 9600 baud.
- Uses `LiquidCrystal_PCF8574` to avoid ambiguous `LiquidCrystal_I2C`
  library-selection conflicts.

## Project layout

```text
.
├── smart_irrigation.ino       # Standalone Arduino sketch
├── diagram.json               # Complete Velxio circuit and connections
├── velxio.toml                # Simulator project metadata
├── libraries.txt              # Pinned library declarations
├── docs/
│   ├── hardware.md            # Hardware and pin/wiring reference
│   ├── simulation.md          # Reproduce the browser simulation
│   ├── verification.md        # Verification summary and boundaries
│   ├── requirements-traceability.md
│   ├── mcp-setup.md           # Optional local MCP integration
│   └── evidence/               # Circuit-only state screenshots
└── .github/workflows/
    └── arduino-build.yml      # Automated Arduino Uno compile check
```

## Hardware

| Function | Arduino Uno pin |
|---|---:|
| Soil moisture analog input | A0 |
| LDR analog input | A1 |
| DHT22 data | D2 |
| Rain input, active LOW | D3 |
| Tank-available input, active LOW | D4 |
| Relay driver | D6 |
| Buzzer | D7 |
| Green pump LED | D8 |
| Red tank-fault LED | D9 |
| LCD SDA / SCL | A4 / A5 |

See [`docs/hardware.md`](docs/hardware.md) for the complete connection model,
component notes, and safety behavior.

## Run the simulation

1. Open <https://velxio.dev/editor/>.
2. Choose **File → Open project** and open this project directory or package.
3. Confirm the active board is **Arduino Uno**.
4. Confirm the three libraries in [`libraries.txt`](libraries.txt) are
   available in the project.
5. Compile and run the circuit.
6. Use the soil potentiometer, rain/tank controls, DHT22, and photoresistor to
   exercise the state transitions.

The complete circuit is defined in [`diagram.json`](diagram.json). The
standalone source file is [`smart_irrigation.ino`](smart_irrigation.ino).

## Verification

The sketch has been checked with an Arduino Uno build, JSON parsing, the
hosted Velxio browser simulation, LCD/I2C startup, safe output initialization,
irrigation, rain stop, tank-empty shutdown, hysteresis, DHT updates, and UART
reporting. The hosted photoresistor model stayed below the firmware's ADC
midpoint during the verified run, so the live model only demonstrated the
bright classification; this is documented rather than hidden.

See [`docs/verification.md`](docs/verification.md) and the eleven
circuit-only screenshots under [`docs/evidence/`](docs/evidence/).

## Optional MCP integration

The browser editor is the easiest visual simulation surface. An optional local
Velxio MCP setup is documented in [`docs/mcp-setup.md`](docs/mcp-setup.md).
No credentials are stored in this repository.

## License

Released under the MIT License. See [`LICENSE`](LICENSE).
