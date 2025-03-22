# Example for `HomeKit Programmable Switch`

## What it does

This code is for an ESP32-based HomeKit-compatible programmable switch. It connects the ESP32 to WiFi and allows users to trigger programmable switch events (single press, double press, long press) using Apple HomeKit. These events can be used to automate other HomeKit devices or scenes.

## Key Functions:
- **WiFi Management:** Handles connection, reconnection, and IP assignment.
- **Programmable Button Support:** Sends HomeKit events for single, double, and long presses.
- **HomeKit Integration:** Implements a stateless programmable switch service.
- **Accessory Identification:** Flashes onboard LED to visually identify the accessory.

## Wiring

| Name | Description | Defaults |
|------|-------------|----------|
| `CONFIG_ESP_BUTTON_GPIO` | GPIO for the `Button` | `"0"` Default |
| `CONFIG_ESP_LED_GPIO` | GPIO for the `Status LED` | `"2"` Default |

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
