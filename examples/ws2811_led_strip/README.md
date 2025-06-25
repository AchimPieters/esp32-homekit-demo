# Example for `HomeKit RGB Light`

## What it does

This example implements a full-featured RGB WS2812/WS2812B/SK6812 (NeoPixel) smart light accessory for Apple HomeKit using an ESP32. It supports RGB color control, brightness adjustment, and white temperature tuning (from warm to cool white) using both an addressable LED strip and two separate white LEDs controlled by PWM.

## Features

- RGB color control (Hue, Saturation)
- Brightness adjustment
- Color temperature control (Warm White ↔ Cold White)
- WiFi auto-connect and error handling
- HomeKit accessory identification (blinking LED effect)
- Compatible with Apple Home

## Hardware Setup

| Name                     | Description                        | Default |
|--------------------------|------------------------------------|---------|
| `CONFIG_ESP_LED_GPIO`    | GPIO connected to RGBW LED strip   | `18`    |
| `CONFIG_ESP_STRIP_LENGTH`| Number of LEDs in the strip        | `8`     |
| `GPIO_WARM_WHITE`        | GPIO for warm white LED (PWM)      | `18`    |
| `GPIO_COLD_WHITE`        | GPIO for cold white LED (PWM)      | `19`    |

This setup uses both a digital RGBW LED strip (like **SK6812**) and **two PWM-controlled white LEDs** (warm & cold white) to simulate tunable white lighting via color temperature.

## Color Temperature Control

HomeKit controls color temperature using **Mirek** units (1,000,000 / Kelvin). This example converts between Kelvin ↔ Mirek automatically.

Supported range:

- Warm White: ~2700K
- Neutral: ~4000K
- Cool White: ~6500K

## Scheme

![HomeKit RGBW Light](https://www.studiopieters.nl/wp-content/uploads/2025/03/homekit_rgbw_light_temp.png)

## Requirements

- **idf version:** `>=5.0`
- **espressif/mdns version:** `1.8.0`
- **wolfssl/wolfssl version:** `5.7.6`
- **achimpieters/esp32-homekit version:** `1.0.0`
- **achimpieters/esp32-color-converter** `1.0.1`

## Configuration

- Set your Wi-Fi credentials under `menuconfig → StudioPieters`
- Set LED GPIO pins and strip length
- Set HomeKit Setup Code and Setup ID (optional, if you want a custom pairing code)

---

This example is ideal for creating a **HomeKit Tunable RGB Light**, **color-adjustable LED ceiling fixture**, or **DIY smart lighting** setup.

Control everything from your iPhone, iPad, or via Siri.
