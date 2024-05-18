# Example for `LIGHT RGB STRIP`

## What it does

It's a Non-Addressable White LED Strip example for the HomeKit Demo. It is a code to turn ON and OFF or Set a warm or cold color an the LED Strip connected to an ESP32 Module.

## Wiring

Connect `COLD LED'S` and `WARM LED'S` pin to the following pin:

| Name | Description | Defaults |
|------|-------------|----------|
| `CONFIG_ESP_CW_LED_GPIO` | GPIO number for `COLD LED'S` pin | "25" Default |
| `CONFIG_ESP_WW_LED_GPIO` | GPIO number for `WARM LED'S` pin | "26" Default |
|

## Notes

- Choose your GPIO's numbers under `StudioPieters` in `menuconfig`. The default is `25` for the COLD LED'S and `26` for the WARM LED'S (On an ESP32 WROOM 32D).
- Set your `WiFi SSID` and `WiFi Password` under `StudioPieters` in `menuconfig`.
- Optional: You can change `HomeKit Setup Code` and `HomeKit Setup ID` under `StudioPieters` in `menuconfig`. (Note:  you need to make a new QR-CODE To make it work)
