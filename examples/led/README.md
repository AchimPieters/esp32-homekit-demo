# Example for `Led`

## What it does

It's a "Hello World" example for the HomeKit Demo. This code is for an ESP32-based HomeKit-compatible smart LED. It connects the ESP32 to WiFi and allows users to control an LED’s on/off state via Apple HomeKit.

## Key Functions:
- WiFi Management: Handles connection, reconnection, and IP assignment.
- LED Control: Uses a GPIO pin to turn an LED on or off.
- HomeKit Integration: Defines HomeKit characteristics for power state control.
- Accessory Identification: Implements a blinking pattern for device identification.

## Wiring

Connect `LED` pin to the following pin:

| Name | Description | Defaults |
|------|-------------|----------|
| `CONFIG_ESP_LED_GPIO` | GPIO number for `LED` pin | "2" Default |

## Scheme

![HomeKit LED](https://raw.githubusercontent.com/AchimPieters/esp32-homekit-demo/refs/heads/main/examples/led/scheme.png)

## Requirements

- **idf version:** `>=5.0`
- **espressif/mdns version:** `1.8.0`
- **wolfssl/wolfssl version:** `5.7.6`
- **achimpieters/esp32-homekit version:** `1.0.0`

## Notes

- Choose your GPIO number under `StudioPieters` in `menuconfig`. The default is `2` (On an ESP32 WROOM 32D).
- Set your `WiFi SSID` and `WiFi Password` under `StudioPieters` in `menuconfig`.
- **Optional:** You can change `HomeKit Setup Code` and `HomeKit Setup ID` under `StudioPieters` in `menuconfig`. _(Note: you need to make a new QR-CODE to make it work.)_
