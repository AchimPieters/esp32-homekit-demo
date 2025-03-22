# Example for `Garage door opener`

## What it does

This code is for an ESP32-based HomeKit-compatible smart garage door opener. It connects the ESP32 to WiFi and allows users to remotely open or close their garage door via Apple HomeKit while monitoring its status.

## Key Functions:
- WiFi Management: Ensures a stable connection.
- Garage Door Control: Uses relays to open and close the door.
- Status Monitoring: Detects if the door is open, closed, or obstructed using reed switches.
- HomeKit Integration: Enables remote operation and real-time status updates.
- Accessory Identification: Implements an LED blinking pattern for identification.

## Wiring

| Name | Description | Defaults |
|------|-------------|----------|
| `CONFIG_ESP_LED_GPIO` | GPIO number for `LED` pin | "2" Default |
| `CONFIG_ESP_RELAY_UP_GPIO` | GPIO number for `REALY UP` pin | "17" Default |
| `CONFIG_ESP_RELAY_DOWN_GPIO` | GPIO number for `RELAY DOWN` pin | "18" Default |
| `CONFIG_ESP_REED_OPEN_GPIO` | GPIO number for `REED OPEN` pin | "22" Default |
| `CONFIG_ESP_REED_CLOSE_GPIO` | GPIO number for `REED CLOSE` pin | "23" Default |
| `CONFIG_ESP_DELAY` | Delay for `MAX_DOOR_OPERATION_TIME` | "5000" Default |

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
