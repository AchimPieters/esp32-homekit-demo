# Example for `HomeKit White LED Strip`

## What it does

This code is for an ESP32-based HomeKit-compatible White LED strip. It connects the ESP32 to WiFi and allows users to control power (on/off) and brightness of a warm-white (WW) and cool-white (CW) LED strip via Apple HomeKit.

## Key Functions:
- **WiFi Management:** Handles connection, reconnection, and IP assignment.
- **White LED Control:** Uses PWM (Pulse Width Modulation) to adjust brightness smoothly.
- **HomeKit Integration:** Enables control over power and brightness levels.
- **Accessory Identification:** Implements a blinking pattern to help identify the device.

## Wiring

| Name | Description | Defaults |
|------|-------------|----------|
| `CONFIG_ESP_WW_LED_GPIO` | GPIO number for `Warm White LED` pin | "25" Default |
| `CONFIG_ESP_CW_LED_GPIO` | GPIO number for `Cool White LED` pin | "26" Default |

## Scheme

![HomeKit LED](https://www.studiopieters.nl/wp-content/uploads/2025/03/homekit_white_led_strip.png)

## Requirements

- **idf version:** `>=5.0`
- **espressif/mdns version:** `1.8.0`
- **wolfssl/wolfssl version:** `5.7.6`
- **achimpieters/esp32-homekit version:** `1.0.0`

## Notes

- Choose your GPIO number under `StudioPieters` in `menuconfig`. The default is `25` and `26` (On an ESP32 WROOM 32D).
- Set your `WiFi SSID` and `WiFi Password` under `StudioPieters` in `menuconfig`.
- **Optional:** You can change `HomeKit Setup Code` and `HomeKit Setup ID` under `StudioPieters` in `menuconfig`. _(Note: you need to generate a new QR code to make it work.)_
