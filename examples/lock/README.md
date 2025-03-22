# Example for `HomeKit Smart Lock`

## What it does

This code is for an ESP32-based HomeKit-compatible smart lock. It connects the ESP32 to WiFi and allows users to control the lock (locked/unlocked state) via Apple HomeKit. It also includes physical button control for unlocking.

## Key Functions:
- **WiFi Management:** Handles connection, reconnection, and IP assignment.
- **Lock Control:** Manages locking and unlocking mechanisms with a relay.
- **HomeKit Integration:** Enables control over lock state and provides status updates.
- **Button Support:** Supports single press, double press, and long press for different actions.
- **Accessory Identification:** Implements a blinking pattern to help identify the device.

## Wiring

| Name | Description | Defaults |
|------|-------------|----------|
| `CONFIG_ESP_BUTTON_GPIO` | GPIO number for the `Button` | "0" Default |
| `CONFIG_ESP_LED_GPIO` | GPIO number for the `Status LED` | "2" Default |
| `CONFIG_ESP_RELAY_GPIO` | GPIO number for the `Lock Relay` | "5" Default |

## Scheme

![HomeKit LED](https://raw.githubusercontent.com/AchimPieters/esp32-homekit-demo/refs/heads/main/examples/lock/scheme.png)

## Requirements

- **idf version:** `>=5.0`
- **espressif/mdns version:** `1.8.0`
- **wolfssl/wolfssl version:** `5.7.6`
- **achimpieters/esp32-homekit version:** `1.0.0`

## Notes

- Choose your GPIO number under `StudioPieters` in `menuconfig`. The default is `2` (On an ESP32 WROOM 32D).
- Set your `WiFi SSID` and `WiFi Password` under `StudioPieters` in `menuconfig`.
- **Optional:** You can change `HomeKit Setup Code` and `HomeKit Setup ID` under `StudioPieters` in `menuconfig`. _(Note: you need to make a new QR-CODE to make it work.)_
