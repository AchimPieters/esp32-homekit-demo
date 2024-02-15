# Example for `LED STRIP`

## What it does

Based on a NEO-Pixel SK6812, RGBW LED strip. ON/OFF, Brightness, HUE, Color temperature and Saturation.

## Wiring

Connect `LED STRIP` pin to the following pin:

| Name | Description | Defaults |
|------|-------------|----------|
| `CONFIG_ESP_LED_GPIO` | GPIO number for `LED` pin | "2" Default |



## Notes

- Choose your GPIO number under `StudioPieters` in `menuconfig`. The default is `2` (On an ESP32 WROOM 32D).
- Choose your strip length under `StudioPieters` in `menuconfig`. The default is `3`.
- Set your `WiFi SSID` and `WiFi Password` under `StudioPieters` in `menuconfig`.
- Optional: You can change `HomeKit Setup Code` and `HomeKit Setup ID` under `StudioPieters` in `menuconfig`. (Note:  you need to make a new QR-CODE To make it work)
