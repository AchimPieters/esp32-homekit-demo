# Example for `MOTION SENSOR`

## What it does

It shows temperature and humidity in a loop.

## Wiring

Connect `DATA` pin to the following pin:

| Name | Description | Defaults |
|------|-------------|----------|
| `CONFIG_EXAMPLE_DATA_GPIO` | GPIO number for `DATA` pin | "4" for `esp8266` and `esp32c3`, "17" for `esp32`, `esp32s2`, and `esp32s3` |

## Notes

- Choose your GPIO number under `StudioPieters` in `menuconfig`. The default is `2` (On an ESP32 WROOM 32D).
- Set your `WiFi SSID` and `WiFi Password` under `StudioPieters` in `menuconfig`.
- Optional: You can change `HomeKit Setup Code` and `HomeKit Setup ID` under `StudioPieters` in `menuconfig`. (Note:  you need to make a new QR-CODE To make it work)
