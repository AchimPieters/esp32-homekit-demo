# ESP32 - HomeKit Demo

This repository provides a set of HomeKit-enabled example projects for the ESP32 platform. Using this demo, you can quickly get up and running with Apple Home-compatible accessories powered by the ESP32.

---

## Requirements

- **ESP32 board** (DevKit, NodeMCU, or similar)
- **ESP-IDF v5.4** via Docker
- Compatible USB-to-Serial connection
- Wi-Fi 2.4GHz network

---

## Reproduction Steps

Follow these steps to clone, build, and flash the example on your ESP32 device:

### 1. Clone the Repository

```bash
git clone --recursive https://github.com/AchimPieters/esp32-homekit-demo.git
```

### 2. Pull the ESP-IDF Docker Image

```bash
docker pull espressif/idf:v5.4
```

> At this point, you'll be using **ESP-IDF v5.4 Stable**

### 3. Run Docker and Enter the Project

```bash
docker run -it -v ~/esp32-homekit-demo:/project -w /project espressif/idf:v5.4
```

### 4. Navigate to an Example

```bash
cd examples/led
idf.py set-target esp32
idf.py menuconfig
```

### 5. Configure Wi-Fi

- Go to `StudioPieters` → Set `(mysid)` to your Wi-Fi SSID.
- Set `(mypassword)` to your Wi-Fi Password.
- Press `ESC` and choose `Yes` when prompted to save the config.

### 6. Build the Firmware

```bash
idf.py build
```

### 7. Flash the Firmware

In a **new terminal window** (on your Mac):

```bash
cd esp32-homekit-demo/examples/led
esptool.py erase_flash
esptool.py -p /dev/tty.usbserial-01FD1166 -b 460800 --before default_reset --after hard_reset --chip esp32 \
  write_flash --flash_mode dio --flash_size detect --flash_freq 40m \
  0x1000 build/bootloader/bootloader.bin \
  0x8000 build/partition_table/partition-table.bin \
  0x10000 build/main.bin
```

> Replace `/dev/tty.usbserial-01FD1166` with your actual USB port.

### 8. Open Serial Monitor

```bash
screen /dev/tty.usbserial-01FD1166 74880
```
---

Made with by [StudioPieters®](https://www.studiopieters.nl)

---

## MIT License

Copyright © 2025 StudioPieters© | Achim Pieters

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

<img src="https://raw.githubusercontent.com/AchimPieters/esp32-homekit-demo/refs/heads/main/images/MIT| SOFTWARE WHITE.svg" width="150">

---

## Works with Apple Home

This demo works natively with **Apple Home** and **Siri**. Once flashed and connected to Wi-Fi, the accessory will be discoverable in the Home app via the pairing QR code or setup code.

<img src="https://raw.githubusercontent.com/AchimPieters/esp32-homekit-demo/refs/heads/main/images/works-with-apple-home.svg" width="150">

> If you plan to manufacture and distribute HomeKit devices, your company must be enrolled in the **[Apple MFi Program](https://developer.apple.com/programs/mfi/)**.

---

## Example Projects Included

| Example        | Description                                  |
|----------------|----------------------------------------------|
| `led`          | A simple LED light you can toggle via HomeKit |
| `switch`       | GPIO relay control through HomeKit           |
| `thermostat`   | Temperature + humidity monitoring (DHT)      |
| `rgb-light`    | RGBW LED strip with HomeKit control          |
| `security`     | Simulated security system (stay/away/disarm) |
| `button`       | Stateless programmable switch accessory      |

---

## Safety Disclaimer

> **Use at your own risk!**

Always take necessary precautions when working with electronics. Power tools, mains electricity, and some sensors may be dangerous if mishandled. StudioPieters® assumes no responsibility for damage, injury, or violation of laws resulting from misuse of this codebase or hardware.

Some projects described may not work, may become outdated, or may be inconsistent with current laws or user agreements. Always verify your tools, take safety precautions, and check the [official StudioPieters.nl website](https://www.studiopieters.nl) for any updates or notices.

---

## About `wolfSSL`

This project uses [wolfSSL](https://www.wolfssl.com/) for secure communication under GPLv2 or commercial license.

- **License**: GPLv2 (or later) or Commercial
- **Support & Licensing**:
  - [View licensing details](https://www.wolfssl.com/license/)
  - Email: `licensing@wolfssl.com`
  - Phone: `+1 425 245 8247`

<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/wolfssl_logo.svg" width="100">

> Commercial support and maintenance available at [wolfssl.com/products](https://www.wolfssl.com/products/support-and-maintenance)

---

## Apple HomeKit Protocol

This demo implements Apple's **HomeKit Accessory Protocol (HAP)**, supporting:

- IP-based transport
- Accessories like lights, switches, thermostats, sensors, and more
- Apple Home App and Siri integration

> Based on the **HomeKit Accessory Protocol Specification (Non-Commercial Version)** — [Download here](https://developer.apple.com/homekit/)

<sub align="left"> <img src="https://raw.githubusercontent.com/AchimPieters/esp32-homekit-demo/refs/heads/main/images/apple_logo.png" width="10"> Copyright © 2019 Apple Inc. All rights reserved.</sub>
