# Example for `GARAGE DOOR OPENER`

## What it does

It's a "Hello World" example for the HomeKit Demo. It is a code to turn ON and OFF an LED connected to an ESP Module.

## Wiring

Connect `LED` pin to the following pin:

| Name | Description | Defaults |
|------|-------------|----------|
| `CONFIG_ESP_LED_GPIO` | GPIO number for `LED` pin | "2" Default |
| `CONFIG_ESP_RELAY_UP_GPIO` | GPIO number for `REALY UP` pin | "17" Default |
| `CONFIG_ESP_RELAY_DOWN_GPIO` | GPIO number for `RELAY DOWN` pin | "18" Default |
| `CONFIG_ESP_REED_OPEN_GPIO` | GPIO number for `REED OPEN` pin | "22" Default |
| `CONFIG_ESP_REED_CLOSE_GPIO` | GPIO number for `REED CLOSE` pin | "23" Default |
| `CONFIG_ESP_DELAY` | Delay for `MAX_DOOR_OPERATION_TIME` | "5000" Default |



## Notes

- Choose your GPIO number under `StudioPieters` in `menuconfig`. The default is `2` (On an ESP32 WROOM 32D).
- Set your `WiFi SSID` and `WiFi Password` under `StudioPieters` in `menuconfig`.
- Optional: You can change `HomeKit Setup Code` and `HomeKit Setup ID` under `StudioPieters` in `menuconfig`. (Note:  you need to make a new QR-CODE To make it work)
