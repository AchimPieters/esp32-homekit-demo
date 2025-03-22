# Example for `HomeKit Window Covering`

## What it does

This code is for an ESP32-based HomeKit-compatible motorized window covering (e.g., blinds, curtains, or shades). It connects to WiFi and allows users to control the position of a motor via Apple HomeKit, supporting opening, closing, and intermediate positions.

## Key Functions:
- **WiFi Management:** Handles connection, reconnection, and IP assignment with robust error handling.
- **HomeKit Integration:** Implements the `Window Covering` service with support for `Target Position`, `Current Position`, and `Position State`.
- **Motor Control:** Uses two GPIOs to control motor direction for raising or lowering the covering.
- **Accessory Identification:** Uses an onboard LED to blink when identified in the Home app.

## Wiring

| Name | Description | Defaults |
|------|-------------|----------|
| `CONFIG_ESP_MOTOR_UP_GPIO` | GPIO for `Motor Up` Direction | `"25"` Default |
| `CONFIG_ESP_MOTOR_DOWN_GPIO` | GPIO for `Motor Down` Direction | `"26"` Default |
| `CONFIG_ESP_LED_GPIO` | GPIO for `Status LED` | `"2"` Default |

## Scheme

![HomeKit Window Covering](https://www.studiopieters.nl/wp-content/uploads/2025/03/homekit_window_covering.png)

## Requirements

- **idf version:** `>=5.0`
- **espressif/mdns version:** `1.8.0`
- **wolfssl/wolfssl version:** `5.7.6`
- **achimpieters/esp32-homekit version:** `1.0.0`

## Notes

- Set your `WiFi SSID` and `WiFi Password` under `StudioPieters` in `menuconfig`.
- Configure GPIOs for motor directions and LED under `StudioPieters` in `menuconfig`.
- **Optional:** You can change the `HomeKit Setup Code` and `Setup ID`. _(Remember to regenerate the QR code if changed.)_

### Position State Values:
- `0` – Increasing (opening)
- `1` – Decreasing (closing)
- `2` – Stopped (fully open/closed or no movement)

---

Ideal for smart DIY **blinds**, **roller shutters**, **curtains**, or **vertical window motors**, giving you full **HomeKit automation and voice control** with Siri. 🪟📱⚙️
