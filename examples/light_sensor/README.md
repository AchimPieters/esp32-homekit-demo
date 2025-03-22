# Example for `Light sensor`

## What it does

This code is for an ESP32-based HomeKit-compatible light sensor. It connects the ESP32 to WiFi and integrates a BH1750 ambient light sensor, allowing users to monitor lux levels (light intensity) via Apple HomeKit.

## Key Functions:

- WiFi Management: Ensures a stable network connection.
- Light Sensing: Uses an I2C-based BH1750 sensor to measure ambient light levels.
- HomeKit Integration: Exposes real-time light intensity (lux) as a HomeKit characteristic.
- Accessory Identification: Implements an LED blinking pattern for device recognition.

## Wiring

| Name | Description | Defaults |
|------|-------------|----------|
| `CONFIG_EXAMPLE_I2C_MASTER_SCL` | GPIO number for `SCL` | "5" for `esp8266`, "6" for `esp32c3`, "19" for `esp32`, `esp32s2`, and `esp32s3` |
| `CONFIG_EXAMPLE_I2C_MASTER_SDA` | GPIO number for `SDA` | "4" for `esp8266`, "5" for `esp32c3`, "18" for `esp32`, `esp32s2`, and `esp32s3` |

## Scheme

![HomeKit LED](https://raw.githubusercontent.com/AchimPieters/esp32-homekit-demo/refs/heads/main/examples/light_sensor/scheme.png)

## Requirements

- **idf version:** `>=5.0`
- **espressif/mdns version:** `1.8.0`
- **wolfssl/wolfssl version:** `5.7.6`
- **achimpieters/esp32-homekit version:** `1.0.0`

## Notes

- Choose your GPIO number under `StudioPieters` in `menuconfig`. The default is `2` (On an ESP32 WROOM 32D).
- Set your `WiFi SSID` and `WiFi Password` under `StudioPieters` in `menuconfig`.
- **Optional:** You can change `HomeKit Setup Code` and `HomeKit Setup ID` under `StudioPieters` in `menuconfig`. _(Note: you need to make a new QR-CODE to make it work.)_
