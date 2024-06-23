# Example for `FAN`

## What it does

The fan example let's you turn ON, OFF and set Speed of an fan connected to an ESP Module.

## Wiring

Connect `FAN` pin to the following pin:

| Name | Description | Defaults |
|------|-------------|----------|
| `CONFIG_ESP_LED_GPIO` | GPIO number for `LED` pin | "2" Default |
| `CONFIG_ESP_FAN_GPIO` | GPIO number for `FAN` pin | "33" Default |

## Scheme

![alt text](./scheme.png)

## Notes

- Choose your GPIO number under `StudioPieters` in `menuconfig`. The default is `2` for the LED and `33` for the fan (On an ESP32 WROOM 32D).
- Set your `WiFi SSID` and `WiFi Password` under `StudioPieters` in `menuconfig`.
- Optional: You can change `HomeKit Setup Code` and `HomeKit Setup ID` under `StudioPieters` in `menuconfig`. (Note:  you need to make a new QR-CODE To make it work)
