# Example for `IP Camera`

## What it does

This code is for an ESP32-based HomeKit-compatible IP camera. It connects the ESP32 to WiFi, initializes a camera module, encodes video using H.264, and streams it via HomeKit RTP streaming.

# Code state: Alfa; This code is still being worked on!

## Key Functions:
- WiFi Management: Handles connection, reconnection, and IP assignment.
- LED Control: Uses a GPIO pin to indicate device status.
- Camera Initialization: Sets up the ESP32 camera module and H.264 encoding.
- HomeKit Integration: Implements video, audio, and RTP streaming configurations.
- Streaming Task: Captures frames, encodes them, and prepares them for RTP transmission.

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
