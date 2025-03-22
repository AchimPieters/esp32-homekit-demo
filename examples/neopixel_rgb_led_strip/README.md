# Example for `HomeKit RGB Light`

## What it does

This code is for an ESP32-based HomeKit-compatible RGB(W) LED strip. It connects the ESP32 to WiFi and allows users to control the power (on/off), brightness, hue, and saturation of an addressable RGB LED strip (e.g., WS2812) via Apple HomeKit.

## Key Functions:
- **WiFi Management:** Handles connection, reconnection, and IP assignment.
- **LED Strip Control:** Uses HSI to RGBW conversion for smooth color blending and brightness scaling.
- **HomeKit Integration:** Enables control over power, brightness, hue, and saturation.
- **Accessory Identification:** Implements a flashing light pattern to help identify the device.

## Wiring

| Name | Description | Defaults |
|------|-------------|----------|
| `CONFIG_ESP_LED_GPIO` | GPIO number for `LED Strip Data` pin | "2" Default |
| `CONFIG_ESP_STRIP_LENGTH` | Number of LEDs in the strip | "3" Default |

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
