# Example for `Heating fan`

## What it does

This code is for an ESP32-based HomeKit-compatible smart fan with LED control and temperature/humidity monitoring. It connects to WiFi, controls a fan and LED strip, reads temperature and humidity data from an SHT3X sensor, and integrates with Apple HomeKit for remote control.

# Code state: Alfa; This code is still being worked on!

## Key Functions:
- WiFi Management: Ensures stable network connectivity.
- Fan Control: Uses PWM to adjust fan speed based on HomeKit commands or automatic temperature-based control.
- LED Strip Control: Uses an RGB LED to indicate fan status and manual override.
- Temperature & Humidity Monitoring: Uses an SHT3X sensor via I2C and updates HomeKit.
- HomeKit Integration: Enables remote fan speed adjustment, power control, and environmental monitoring.
- Physical Button Control: Allows manual override for fan speed cycling.

## Wiring

| Name | Description | Defaults |
|------|-------------|----------|
| `CONFIG_ESP_LED_GPIO` | GPIO number for `LED` pin | "2" Default |
| `CONFIG_ESP_FAN_GPIO`      | GPIO number for `Fan` control          | 33        |
| `CONFIG_I2C_MASTER_SDA`    | GPIO number for I2C `SDA` pin          | 21        |
| `CONFIG_I2C_MASTER_SCL`    | GPIO number for I2C `SCL` pin          | 22        |
| `CONFIG_SHT3X_I2C_ADDRESS` | I2C address of the `SHT3X` sensor      | 0x44      |
| `CONFIG_ESP_BUTTON_GPIO`   | GPIO number for control button         | 0         |

## Scheme

![HomeKit LED](https://raw.githubusercontent.com/AchimPieters/esp32-homekit-demo/refs/heads/main/examples/heating_fan/scheme.png)

## Requirements

- **idf version:** `>=5.0`
- **espressif/mdns version:** `1.8.0`
- **wolfssl/wolfssl version:** `5.7.6`
- **achimpieters/esp32-homekit version:** `1.0.0`
- **achimpieters/esp32-sht3x version:** `^1.0.5`

## Notes

- Choose your GPIO number under `StudioPieters` in `menuconfig`. The default is `2` (On an ESP32 WROOM 32D).
- Set your `WiFi SSID` and `WiFi Password` under `StudioPieters` in `menuconfig`.
- **Optional:** You can change `HomeKit Setup Code` and `HomeKit Setup ID` under `StudioPieters` in `menuconfig`. _(Note: you need to make a new QR-CODE to make it work.)_
