# Example for `LIGHT RGB STRIP`

## What it does

It's a Non-Addressable RGB LED Strip example for the HomeKit Demo. It is a code to turn ON and OFF or Set any color an RGB LED Strip connected to an ESP32 Module.

## Wiring

Connect `RED LED'S`, `GREEN LED'S` and `BLUE LED'S` pin to the following pin:

| Name | Description | Defaults |
|------|-------------|----------|
| `CONFIG_ESP_RED_LED_GPIO` | GPIO number for `RED LED'S` pin | "25" Default |
| `CONFIG_ESP_GREEN_LED_GPIO` | GPIO number for `GREEN LED'S` pin | "26" Default |
| `CONFIG_ESP_BLUE_LED_GPIO` | GPIO number for `BLUE LED'S` pin | "27" Default |

## Notes

- Choose your GPIO's numbers under `StudioPieters` in `menuconfig`. The default is `25` for the RED LED'S,`26` for the GREEN LED'S adn `27` for the RED LED'S (On an ESP32 WROOM 32D).
- Set your `WiFi SSID` and `WiFi Password` under `StudioPieters` in `menuconfig`.
- Optional: You can change `HomeKit Setup Code` and `HomeKit Setup ID` under `StudioPieters` in `menuconfig`. (Note:  you need to make a new QR-CODE To make it work)
