# Example for `HomeKit Switch Button`

## What it does

This code is for an ESP32-based HomeKit-compatible smart switch with built-in custom logic for handling single, double, and long presses—without relying on external button libraries. It connects to WiFi and allows users to control a physical relay (e.g., to switch lights or devices) via Apple HomeKit or a physical push button.

## Key Functions:
- **WiFi Management:** Handles connection, reconnection, and IP assignment.
- **HomeKit Integration:** Provides on/off control and state synchronization with HomeKit.
- **Relay Control:** Drives a relay GPIO to toggle power to external devices.
- **LED Feedback:** Optional status LED controlled alongside the relay.
- **Button Event Handling:** Implements custom single, double, and long press detection via GPIO interrupts and timers.
- **Accessory Identification:** Flashes LED for visual identification in the Home app.

## Wiring

| Name | Description | Defaults |
|------|-------------|----------|
| `CONFIG_ESP_BUTTON_GPIO` | GPIO for the `Push Button` | `"0"` Default |
| `CONFIG_ESP_LED_GPIO`    | GPIO for the `Status LED`  | `"2"` Default |
| `CONFIG_ESP_RELAY_GPIO`  | GPIO for the `Relay`       | `"5"` Default |

## Scheme

![HomeKit Switch](https://www.studiopieters.nl/wp-content/uploads/2025/03/homekit_switch_custom_button.png)

## Requirements

- **idf version:** `>=5.0`
- **espressif/mdns version:** `1.8.0`
- **wolfssl/wolfssl version:** `5.7.6`
- **achimpieters/esp32-homekit version:** `1.0.0`

## Notes

- Set your `WiFi SSID` and `WiFi Password` under `StudioPieters` in `menuconfig`.
- Configure GPIOs for button, LED, and relay under `StudioPieters` in `menuconfig`.
- **Optional:** You can change the `HomeKit Setup Code` and `Setup ID` under `StudioPieters` in `menuconfig`. _(Changing these requires generating a new QR code.)_
- Button actions:
  - **Single Press:** Toggle relay state (on/off)
  - **Double Press:** Reserved for future functionality
  - **Long Press:** Reserved for future functionality

---

This project is ideal for **retrofit smart switches**, **relay-based control modules**, or DIY **HomeKit-compatible wall switches**.
