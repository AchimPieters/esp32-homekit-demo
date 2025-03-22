# Example for `Light RGB strip`

## What it does

This code is for an ESP32-based HomeKit-compatible RGB LED strip. It connects the ESP32 to WiFi and allows users to control the power (on/off), brightness, hue, and saturation of an RGB LED strip via Apple HomeKit.

## Key Functions:
- WiFi Management: Handles connection, reconnection, and IP assignment.
- RGB LED Control: Uses PWM (Pulse Width Modulation) to adjust LED colors smoothly.
- HSI to RGB Conversion: Converts Hue, Saturation, and Intensity (HSI) values to RGB.
- HomeKit Integration: Enables control over power, brightness, hue, and saturation.
- Accessory Identification: Implements a blinking pattern to help identify the device.

## Wiring

| Name | Description | Defaults |
|------|-------------|----------|
| `CONFIG_ESP_RED_LED_GPIO` | GPIO number for `RED LED'S` pin | "25" Default |
| `CONFIG_ESP_GREEN_LED_GPIO` | GPIO number for `GREEN LED'S` pin | "26" Default |
| `CONFIG_ESP_BLUE_LED_GPIO` | GPIO number for `BLUE LED'S` pin | "27" Default |

## Scheme

![HomeKit LED](https://raw.githubusercontent.com/AchimPieters/esp32-homekit-demo/refs/heads/main/examples/light_RGB_strip/scheme.png)

## Requirements

- **idf version:** `>=5.0`
- **espressif/mdns version:** `1.8.0`
- **wolfssl/wolfssl version:** `5.7.6`
- **achimpieters/esp32-homekit version:** `1.0.0`

## Notes

- Choose your GPIO number under `StudioPieters` in `menuconfig`. The default is `2` (On an ESP32 WROOM 32D).
- Set your `WiFi SSID` and `WiFi Password` under `StudioPieters` in `menuconfig`.
- **Optional:** You can change `HomeKit Setup Code` and `HomeKit Setup ID` under `StudioPieters` in `menuconfig`. _(Note: you need to make a new QR-CODE to make it work.)_
