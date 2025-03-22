# Example for `HomeKit Thermostat`

## What it does

This code is for an ESP32-based HomeKit-compatible thermostat. It connects to WiFi, reads temperature and humidity from a DHT sensor, and controls heating, cooling, and fan relays. It allows users to monitor and control the climate system through Apple HomeKit.

## Key Functions:
- **WiFi Management:** Handles connection, reconnection, and IP assignment.
- **DHT Sensor Integration:** Reads temperature and humidity from a supported DHT sensor (DHT11, AM2301, or SI7021).
- **HomeKit Integration:** Provides a full-featured thermostat service, supporting heating/cooling modes, thresholds, and current sensor readings.
- **Relay Control:** Drives GPIO outputs to control heater, cooler, and fan based on the desired setpoint.
- **Accessory Identification:** Blinks an LED for identification in the Apple Home app.

## Wiring

| Name | Description | Defaults |
|------|-------------|----------|
| `CONFIG_ESP_TEMP_SENSOR_GPIO` | GPIO for `DHT Sensor Data` | `"4"` Default |
| `CONFIG_ESP_LED_GPIO`         | GPIO for `Status LED`      | `"2"` Default |
| `CONFIG_ESP_FAN_GPIO`         | GPIO for `Fan Relay`       | `"5"` Default |
| `CONFIG_ESP_COOLER_GPIO`      | GPIO for `Cooler Relay`    | `"12"` Default |
| `CONFIG_ESP_HEATER_GPIO`      | GPIO for `Heater Relay`    | `"14"` Default |

## Scheme

![HomeKit Thermostat](https://www.studiopieters.nl/wp-content/uploads/2025/03/homekit_thermostat.png)

## Requirements

- **idf version:** `>=5.0`
- **espressif/mdns version:** `1.8.0`
- **wolfssl/wolfssl version:** `5.7.6`
- **achimpieters/esp32-homekit version:** `1.0.0`
- **achimpieters/esp32-dht version:** `1.0.2`

## Notes

- Set your `WiFi SSID` and `WiFi Password` under `StudioPieters` in `menuconfig`.
- Choose your DHT sensor type (`DHT11`, `AM2301`, or `SI7021`) and select it in `menuconfig` under `Example Configuration`.
- Configure GPIOs for your relays and LED under `StudioPieters` in `menuconfig`.
- **Optional:** You can change the `HomeKit Setup Code` and `Setup ID`. _(You'll need a new QR code if changed.)_

### Supported Modes:
- `OFF` – All relays are turned off.
- `HEAT` – Activates heater if temperature is below target or threshold.
- `COOL` – Activates cooler if temperature is above target or threshold.
- `AUTO` – Automatically chooses between heat and cool.

---

Perfect for creating a DIY **smart HVAC controller**, **room thermostat**, or **environmental automation** system in your HomeKit setup. 🌡️❄️🔥
