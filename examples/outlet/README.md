# Example for `HomeKit Triple Outlet`

## What it does

This code is for an ESP32-based HomeKit-compatible triple outlet. It connects the ESP32 to WiFi and exposes three individually controllable outlets to Apple HomeKit. Each outlet can be controlled through the Home app or via physical buttons. An onboard LED can be used for identification and status feedback.

## Key Functions:
- **WiFi Management:** Handles connection, reconnection, and IP assignment.
- **Relay Control:** Three relays allow individual outlet switching.
- **HomeKit Integration:** Exposes each outlet as a separate HomeKit service with state control and monitoring.
- **Button Support:** Each outlet has a dedicated button with single press functionality (toggle).
- **Accessory Identification:** LED blinks in a pattern to visually identify the accessory.

## Wiring

| Name | Description | Defaults |
|------|-------------|----------|
| `CONFIG_ESP_BUTTON_1_GPIO` | GPIO for `Button 1` (Outlet 1 control) | `"0"` Default |
| `CONFIG_ESP_BUTTON_2_GPIO` | GPIO for `Button 2` (Outlet 2 control) | `"4"` Default |
| `CONFIG_ESP_BUTTON_3_GPIO` | GPIO for `Button 3` (Outlet 3 control) | `"5"` Default |
| `CONFIG_ESP_RELAY_1_GPIO` | GPIO for `Relay 1` (Outlet 1 power) | `"12"` Default |
| `CONFIG_ESP_RELAY_2_GPIO` | GPIO for `Relay 2` (Outlet 2 power) | `"13"` Default |
| `CONFIG_ESP_RELAY_3_GPIO` | GPIO for `Relay 3` (Outlet 3 power) | `"14"` Default |
| `CONFIG_ESP_LED_GPIO` | GPIO for `Status LED` | `"2"` Default |

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
