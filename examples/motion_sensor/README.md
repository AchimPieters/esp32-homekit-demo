# Example for `HomeKit Motion Sensor`

## What it does

This code is for an ESP32-based HomeKit-compatible motion sensor. It connects the ESP32 to WiFi and detects motion using a PIR sensor. When motion is detected, it notifies Apple HomeKit.

## Key Functions:
- **WiFi Management:** Handles connection, reconnection, and IP assignment.
- **Motion Detection:** Uses GPIO to monitor a PIR motion sensor.
- **HomeKit Integration:** Sends motion detection updates to Apple HomeKit.
- **Accessory Identification:** Implements a blinking pattern to help identify the device.

## Wiring

| Name | Description | Defaults |
|------|-------------|----------|
| `CONFIG_ESP_MOTION_SENSOR_GPIO` | GPIO number for the `Motion Sensor` | "4" Default |
| `CONFIG_ESP_LED_GPIO` | GPIO number for the `Status LED` | "2" Default |

## Scheme

![HomeKit LED](https://raw.githubusercontent.com/AchimPieters/esp32-homekit-demo/refs/heads/main/examples/motion_sensor/scheme.png)

## Requirements

- **idf version:** `>=5.0`
- **espressif/mdns version:** `1.8.0`
- **wolfssl/wolfssl version:** `5.7.6`
- **achimpieters/esp32-homekit version:** `1.0.0`

## Notes

- Choose your GPIO number under `StudioPieters` in `menuconfig`. The default is `2` (On an ESP32 WROOM 32D).
- Set your `WiFi SSID` and `WiFi Password` under `StudioPieters` in `menuconfig`.
- **Optional:** You can change `HomeKit Setup Code` and `HomeKit Setup ID` under `StudioPieters` in `menuconfig`. _(Note: you need to make a new QR-CODE to make it work.)_
