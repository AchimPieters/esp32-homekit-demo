# Example for `Aubess power monitor switch`

## What it does

This code is for an ESP32-based HomeKit-compatible power monitoring switch. It integrates a relay switch, an LED indicator, and a BL0942 energy meter to measure current (A), voltage (V), and power (W), all accessible through Apple HomeKit.

# Code state: Alfa; This code is still being worked on!

## Key Functions:
- WiFi Management: Ensures network connectivity.
- Power Monitoring: Reads current, voltage, and power consumption from the BL0942 energy meter via UART.
- Relay Control: Turns an electrical device on or off.
- HomeKit Integration: Exposes switch control and real-time power data (A/V/W).
- LED Indicator & Identification: Provides status feedback and device identification.

## Wiring

Connect `Aubess Power Monitor Switch` pin to the following pin:

| Name | Description | Defaults |
|------|-------------|----------|
| `CONFIG_ESP_LED_GPIO` | GPIO number for `LED` pin | "7" Default |
| `CONFIG_ESP_Button_GPIO` | GPIO number for `BUTTON` pin | "10" Default |
| `CONFIG_ESP_942RX_GPIO` | GPIO number for `942RX` pin | "RXD" Default |
| `CONFIG_ESP_942TX_GPIO` | GPIO number for `942TX` pin | "TXD" Default |
| `CONFIG_ESP_Wall_switch_GPIO` | GPIO number for `Wall switch` pin | "5" Default |
| `CONFIG_ESP_Relay_GPIO` | GPIO number for `Relay` pin | "4" Default |

## Scheme

![HomeKit LED](https://raw.githubusercontent.com/AchimPieters/esp32-homekit-demo/refs/heads/main/examples/Aubess_power_monitor_switch/scheme.png)

## Requirements

- **idf version:** `>=5.0`
- **espressif/mdns version:** `1.8.0`
- **wolfssl/wolfssl version:** `5.7.6`
- **achimpieters/esp32-homekit version:** `1.0.0`

## Notes

- Choose your GPIO number under `StudioPieters` in `menuconfig`. The default is `2` (On an ESP32 WROOM 32D).
- Set your `WiFi SSID` and `WiFi Password` under `StudioPieters` in `menuconfig`.
- **Optional:** You can change `HomeKit Setup Code` and `HomeKit Setup ID` under `StudioPieters` in `menuconfig`. _(Note: you need to make a new QR-CODE to make it work.)_
