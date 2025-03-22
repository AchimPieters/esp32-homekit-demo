# Example for `HomeKit Security System`

## What it does

This code is for an ESP32-based HomeKit-compatible security system. It connects to WiFi and allows users to control and monitor the state of the security system (Disarmed, Stay Arm, Away Arm, Night Arm, and Triggered) directly from the Apple Home app.

## Key Functions:
- **WiFi Management:** Manages connection and reconnection to the network with error handling.
- **HomeKit Integration:** Exposes a full-featured HomeKit security system service with current and target state handling.
- **State Synchronization:** Updates current state when the target state changes, and notifies HomeKit accordingly.
- **Accessory Identification:** Uses an onboard LED to visually blink for identification.

## Wiring

| Name | Description | Defaults |
|------|-------------|----------|
| `CONFIG_ESP_LED_GPIO` | GPIO for the `Status LED` | `"2"` Default |

## Scheme

![HomeKit Security System](https://www.studiopieters.nl/wp-content/uploads/2025/03/homekit_security_system.png)

## Requirements

- **idf version:** `>=5.0`
- **espressif/mdns version:** `1.8.0`
- **wolfssl/wolfssl version:** `5.7.6`
- **achimpieters/esp32-homekit version:** `1.0.0`

## Notes

- Set your `WiFi SSID` and `WiFi Password` under `StudioPieters` in `menuconfig`.
- Choose the GPIO for your LED under `StudioPieters` in `menuconfig` (default is GPIO 2).
- **Optional:** Change your `HomeKit Setup Code` and `Setup ID` under `StudioPieters` in `menuconfig`. _(If changed, generate a new QR code.)_

- Supported security system states:
  - **0 – Stay Arm**
  - **1 – Away Arm**
  - **2 – Night Arm**
  - **3 – Disarmed**
  - **4 – Alarm Triggered**

---

This is ideal for building your own HomeKit-compatible **alarm panel**, **security controller**, or integrating physical switches and sensors for HomeKit automation.
