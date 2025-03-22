# Example for `Light`

## What it does

This code is for an ESP32-based HomeKit-compatible smart light. It connects the ESP32 to WiFi, initializes an LED with PWM brightness control, and integrates with Apple's HomeKit ecosystem, allowing users to control the light's power state and brightness via HomeKit.

## Key Functions:
- WiFi Management: Handles connection, reconnection, and IP assignment.
- LED Control: Uses PWM to adjust brightness and on/off state.
- HomeKit Integration: Defines HomeKit characteristics (on/off, brightness) and initializes the HomeKit server.
- Accessory Identification: Implements a blinking pattern for identifying the device.

## Wiring

Connect `Led` pin to the following pin:

| Name | Description | Defaults |
|------|-------------|----------|
| `CONFIG_ESP_LED_GPIO` | GPIO number for `LED` pin | "2" Default |

## Scheme

![HomeKit Light](https://raw.githubusercontent.com/AchimPieters/esp32-homekit-demo/refs/heads/main/examples/led/scheme.png)

## Requirements

- **idf version:** `>=5.0`
- **espressif/mdns version:** `1.8.0`
- **wolfssl/wolfssl version:** `5.7.6`
- **achimpieters/esp32-homekit version:** `1.0.0`

## Notes

- Choose your GPIO number under `StudioPieters` in `menuconfig`. The default is `2` (On an ESP32 WROOM 32D).
- Set your `WiFi SSID` and `WiFi Password` under `StudioPieters` in `menuconfig`.
- **Optional:** You can change `HomeKit Setup Code` and `HomeKit Setup ID` under `StudioPieters` in `menuconfig`. _(Note: you need to make a new QR-CODE to make it work.)_
