# Example for `HomeKit Temperature & Humidity Sensor`

## What it does

This project reads temperature and humidity data from a DHT sensor (DHT11, AM2301, or SI7021) and exposes the data to Apple HomeKit as two separate sensors: one for temperature and one for humidity. It also includes a status LED for accessory identification and error feedback.

## Key Features

- **WiFi Connectivity**: Automatically connects to your configured network and reconnects if disconnected.
- **Sensor Support**: Reads real-time data from supported DHT sensors.
- **Apple HomeKit**: Integrates directly with HomeKit, allowing you to monitor temperature and humidity from any iOS device.
- **Smart Updates**: Notifies HomeKit only when values change significantly or at regular intervals.
- **LED Feedback**: Blinks the onboard LED during identification.

---

## Supported Sensors

Select your sensor type in `menuconfig` under **Example Configuration**:
- `DHT11`
- `AM2301`
- `SI7021`

## Wiring

| Pin             | Description             | Default GPIO |
|----------------|-------------------------|--------------|
| `TEMP_SENSOR`  | DHT Data Pin            | `GPIO4`      |
| `LED_GPIO`     | Onboard / External LED  | `GPIO2`      |

> Configure these pins in `menuconfig` under **StudioPieters Configuration**.

---

## Update Behavior

- Updates are triggered:
  - When temperature changes ≥ **0.5°C**
  - When humidity changes ≥ **1.0%**
  - Or every **30 minutes** (as a fallback)

---

## Requirements

- **idf version:** `>=5.0`
- **espressif/mdns version:** `1.8.0`
- **wolfssl/wolfssl version:** `5.7.6`
- **achimpieters/esp32-homekit version:** `1.0.0`
- **achimpieters/esp32-dht version:** `1.0.2`
---

## HomeKit Accessories

This firmware registers as **1 HomeKit accessory** with **2 services**:

- **Temperature Sensor**
- **Humidity Sensor**

You can monitor both in the Apple Home app.

---

## Example Use Cases

- Indoor climate monitoring
- Greenhouse temperature tracking
- Smart home environmental automation

---

## Screenshot

![HomeKit Temperature & Humidity Sensor](https://www.studiopieters.nl/wp-content/uploads/2025/03/homekit_temp_humidity.png)

---

## Notes

- Configure your **WiFi SSID**, **Password**, and **HomeKit Setup Code** via `menuconfig`.
- Optionally adjust GPIOs, sensor type, and LED behavior.

---

Perfect for building your own **smart indoor climate sensor** or integrating basic **environmental data** into your HomeKit ecosystem!
