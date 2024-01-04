# Example for `Window Covering`

## What it does

It's a motorized window coverings or shades - examples include shutters, blinds, awnings etc.example for the HomeKit Demo.

## Wiring

Connect `Window Covering` pin to the following pin:

| Name | Description | Defaults |
|------|-------------|----------|
| `CONFIG_ESP_LED_GPIO` | GPIO number for `LED` pin | "2" Default |
| `ESP_MOTOR_UP_GPIO` | GPIO number for `MOTOR UP` pin | "17" Default |
| `ESP_MOTOR_DOWN_GPIO` | GPIO number for `MOTOR DOWN` pin | "18" Default |



## Notes

- Choose your LED GPIO number under `StudioPieters` in `menuconfig`. The default is `2` (On an ESP32 WROOM 32D).
- Choose your MOTOR UP GPIO number under `StudioPieters` in `menuconfig`. The default is `17` (On an ESP32 WROOM 32D).
- Choose your MOTOR DOWN GPIO number under `StudioPieters` in `menuconfig`. The default is `18` (On an ESP32 WROOM 32D).
- Set your `WiFi SSID` and `WiFi Password` under `StudioPieters` in `menuconfig`.
- Optional: You can change `HomeKit Setup Code` and `HomeKit Setup ID` under `StudioPieters` in `menuconfig`. (Note:  you need to make a new QR-CODE To make it work)
