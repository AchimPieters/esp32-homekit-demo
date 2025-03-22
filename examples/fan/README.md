# Example for `Fan`

## What it does

This code is for an ESP32-based HomeKit-compatible smart fan. It connects the ESP32 to WiFi and allows users to control a fan's power (on/off) and speed via Apple HomeKit.

## Key Functions:
- WiFi Management: Connects to the network and ensures stability.
- Fan Control: Uses PWM (Pulse Width Modulation) to adjust fan speed.
- LED Indicator: Provides feedback for device identification.
- HomeKit Integration: Enables remote control of the fan’s power and speed.
- Accessory Identification: Implements a blinking LED pattern for device recognition.

## Wiring

| Name | Description | Defaults |
|------|-------------|----------|
| `CONFIG_ESP_LED_GPIO` | GPIO number for `LED` pin | "2" Default |
| `CONFIG_ESP_FAN_GPIO` | GPIO number for `FAN` pin | "33" Default |

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
