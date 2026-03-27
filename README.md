# ORS ESP32 — Open River Sensor

An ESP32-based river monitoring system that measures water distance and temperature, transmits data via MQTT over a cellular (SIM7000G) connection, and supports wireless configuration through Bluetooth Low Energy (BLE).

## Overview

The ORS (Open River Sensor) is a low-power, deep-sleep-capable sensor node built on the **LilyGO SIM7000G ESP32** board. It periodically wakes up, reads sensor data, publishes it to an MQTT broker, and returns to deep sleep. On a fresh power-on, a 60-second BLE maintenance window allows field configuration from a smartphone or tablet using the included Web BLE interface.

## Features

- **Water distance measurement** — serial ultrasonic sensor (e.g. JSN-SR04T) or pulse-based echo sensor
- **Water temperature** — Dallas DS18B20 via OneWire
- **Internal ESP32 chip temperature** monitoring
- **Cellular uplink** — SIM7000G modem with auto-APN detection for German carriers (Telekom, Vodafone, O2, Congstar, MEDIONmobil)
- **MQTT publishing** — JSON payload with distance, temperature, battery voltage, firmware version, and sensor ID
- **Deep sleep** — configurable interval (default 60 min) for low power consumption
- **BLE configuration** — on power-on, the device advertises for 60 seconds, allowing a connected client to read/write:
  - Sensor ID
  - MQTT username & password
  - Network APN
  - Sleep time (1–360 minutes)
- **Web BLE companion app** — single-page HTML/JS interface using the Web Bluetooth API for zero-install mobile configuration
- **Persistent configuration** — all settings stored in ESP32 flash via the Preferences library
- **Conditional compilation** — enable/disable MQTT, serial debug output, AT command logging, and sensor type via `#define` flags

## Build Instructions (German)

A complete step-by-step assembly and configuration guide (in German) is available in [Anleitung_ORS_Komplett.md](Anleitung_ORS_Komplett.md).

## Hardware

| Component | Description |
|---|---|
| MCU / Modem | LilyGO SIM7000G ESP32 |
| Distance sensor | Serial ultrasonic (RX/TX) **or** pulse-based (TRIG/ECHO) |
| Temperature sensor | Dallas DS18B20 on OneWire bus (GPIO 21) |
| LED | Onboard indicator (GPIO 12) |

### Pin Assignment

| Pin | Function |
|---|---|
| 26 / 27 | Modem RX / TX |
| 25 | Modem DTR |
| 4 | Modem PWRKEY |
| 21 | OneWire (DS18B20) |
| 33 / 32 | Sensor RX / TX (serial mode) **or** ECHO / TRIG (pulse mode) |
| 15 | Sensor power enable (serial mode) |
| 12 | Board LED |

## Project Structure

```
ORS_05/
├── ORS_05.ino             Main sketch — setup, deep sleep, BLE maintenance window
├── Constants.h            Compile-time flags, pin definitions, network constants
├── Globals.h              Shared data structures, extern declarations
├── BLE_Service.h/.cpp     BLE server setup, characteristic callbacks, flash storage
├── ModemService.h/.cpp    SIM7000G power control, GPRS connection, MQTT publish
├── 04_SensorService.h     Distance and temperature sensor reading (header-only class)
├── DebugUtils.h/.cpp      Conditional serial and BLE debug output
indexBLE03_0.9.1.html      Web BLE companion app for mobile configuration
```

## Configuration

Compile-time options in [ORS_05/Constants.h](ORS_05/Constants.h):

| Define | Purpose |
|---|---|
| `SENSOR_SERIAL` | Use serial distance sensor. Comment out for pulse sensor. |
| `WITH_MQTT` | Enable MQTT data transmission |
| `DEBUG_MODE_AKTIV` | Enable serial debug output |
| `DUMP_AT_COMMANDS` | Log raw AT modem communication |

Runtime settings (configurable via BLE or stored in flash):

| Parameter | Default | Description |
|---|---|---|
| Sensor ID | `sensor_default` | Unique identifier, used as MQTT topic suffix (`ors/<sensor_id>`) |
| MQTT User | `ors-test` | MQTT broker username |
| MQTT Password | `testpw` | MQTT broker password |
| APN | auto-detected | Cellular APN; auto-detected from carrier name if not set |
| Sleep Time | 60 min | Deep sleep interval between measurements |

## MQTT Payload

Published to topic `ors/<sensor_id>`:

```json
{
  "waterdist_mm": 1234,
  "watertemp_dc": 185,
  "Batt_mV": 4120,
  "version": "0.9.1",
  "sensor_id": "Loge004"
}
```

Temperatures are in deci-degrees Celsius (÷ 10 for °C). Distance is in millimeters.

## Web BLE Companion App

Open [indexBLE03_0.9.1.html](indexBLE03_0.9.1.html) in a Web Bluetooth-capable browser (Chrome on Android/Desktop). The app connects to the ESP32 during the maintenance window and allows reading and writing all configuration parameters. A live status console displays log messages pushed from the device via BLE notifications.

## Flashing Firmware

A pre-built firmware binary is included in this repository at [`ORS_05/build/ORS_05.ino.merged.bin`](ORS_05/build/ORS_05.ino.merged.bin). This single file contains the bootloader, partition table, and application firmware — no Arduino IDE required.

### Using a Web Flasher (e.g. ESPTool.spacehuhn.com)

1. Open **ESPTool.spacehuhn.com** in **Chrome**.
2. Connect the ESP32 board via USB and make sure the **power switch** is on.
3. Click **Connect** and select the serial port in the browser popup.
4. Select `ORS_05.ino.merged.bin` as the firmware file.
5. Set the flash address to **`0x0`**.
6. Start the flash process and wait for completion.
7. Press the **Reset** button to boot into the new firmware.

> **Troubleshooting — "Couldn't sync to ESP":** Check the power switch is on. Try unplugging USB, cycling the power switch, and reconnecting. Press the **Reset** button while the flasher is trying to connect. Also verify the USB cable supports data (not charge-only) and that a COM port appears in Device Manager (install CP210x or CH340 driver if not).

## Dependencies

Arduino libraries (install via Library Manager or PlatformIO):

- [TinyGSM](https://github.com/vshymanskyy/TinyGSM)
- [PubSubClient](https://github.com/knolleary/pubsubclient)
- [OneWire](https://github.com/PaulStoffregen/OneWire)
- [DallasTemperature](https://github.com/milesburton/Arduino-Temperature-Control-Library)
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
- ESP32 BLE Arduino (included with ESP32 board package)

## Building

1. Install the [ESP32 board package](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html) in Arduino IDE or PlatformIO.
2. Install the libraries listed above.
3. Open `ORS_05/ORS_05.ino`.
4. Adjust the `#define` flags in [ORS_05/Constants.h](ORS_05/Constants.h) as needed.
5. Select board **ESP32 Dev Module** (or the specific LilyGO variant) and upload.

## License

This project is licensed under the **GNU General Public License v3.0 (GPLv3)**. See [LICENSE](LICENSE) for the full license text.

## Firmware Version

Current version: **0.9.1**
