# ESP32 - HomeKit Demo

<b>REPRODUCTION STEPS</b>

Open a terminal window on your mac.

```
docker pull espressif/idf:latest
```
```
git clone --recursive https://github.com/AchimPieters/esp32-homekit-demo.git
```
```
docker run -it -v ~/esp32-homekit-demo:/project -w /project espressif/idf:latest
```
```
cd examples/led
```
```
idf.py set-target esp32
```
```
idf.py menuconfig
```
- Select 'Serial falsher config' and then 'Flash size (2MB)' set to '4MB'
- Select 'Partition table' and then 'Partition Table(Single factory app, no OTA)' Set to 'Custom partition table CSV'
- Select 'StudioPieters' and then '(mysid) WIFI SSID' and fill in your Wi-Fi Network name, then Select '(mypassword) WiFI Password' and fill in your Wi-Fi Network password.
- Then press 'ESC' until you are asked 'Save Configuration?' and select '(Y)es'
```
idf.py build
```
Open a new terminal window on your mac.
```
cd esp32-homekit-demo/examples/led
```
```
esptool.py erase_flash
```
```
esptool.py -p /dev/tty.usbserial-01FD1166 -b 460800 --before default_reset --after hard_reset --chip esp32  write_flash --flash_mode dio --flash_size detect --flash_freq 40m 0x1000 build/bootloader/bootloader.bin 0x8000 build/partition_table/partition-table.bin 0x10000 build/main.bin
```
- Replace '/dev/tty.usbserial-01FD1166' with your USB port.
```
screen /dev/tty.usbserial-01FD1166 115200
```
- Replace '/dev/tty.usbserial-01FD1166' with your USB port.

<br>

|                  | <sub>ESP32 Series</sub> | <sub>ESP32-C6 Series</sub> | <sub>ESP32-C3 Series</sub> | <sub>ESP32-C Series</sub> | <sub>ESP32-S3 Series</sub> | <sub>ESP32-S2 Series</sub> | <sub>ESP32-H2 Series</sub> |
|------------------|--------------|-----------------|-----------------|----------------|-----------------|-----------------|-----------------|
| <sub>LED</sub>              |<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/MAYBE.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/MAYBE.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/MAYBE.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/MAYBE.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/MAYBE.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/NO.svg" width="20">|
| <sub>BUTTON</sub>           |<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/MAYBE.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/MAYBE.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/MAYBE.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/MAYBE.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/MAYBE.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/MAYBE.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/NO.svg" width="20">|
| <sub>DYNAMIC SERVICES</sub> |<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/MAYBE.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/MAYBE.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/MAYBE.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/MAYBE.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/MAYBE.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/MAYBE.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/NO.svg" width="20">|



<br>

<img  style="float: right;" src="https://github.com/AchimPieters/ESP32-SmartPlug/blob/main/images/works-with-apple-home.svg" width="150"> <img  style="float: right;" src="https://github.com/AchimPieters/ESP32-SmartPlug/blob/main/images/MIT%7C%20SOFTWARE%20WHITE.svg" width="150">

<br>
<sub><sup>-------------------------------------------------------------------------------------------------------------------------------------</sup></sub>
<br>
<b>SAFETY</b>

<sub>Technology and the laws and limitations imposed by manufacturers and content owners are constantly changing. Thus, some of the projects described may not work, may be inconsistent with current laws or user agreements, or may damage or adversely affect some equipment.
Your safety is your own responsibility, including proper use of equipment and safety gear, and determining whether you have adequate skill and experience. Power tools, electricity, and other resources used for these projects are dangerous, unless used properly and with adequate precautions, including safety gear. Some illustrative photos do not depict safety precautions or equipment, in order to show the project steps more clearly. These projects are not intended for use by children. Use of the instructions, kits, projects and suggestions on StudioPieters.nl is at your own risk. StudioPieters® disclaims all responsibility for any resulting damage, injury, or expense. It is your responsibility to make sure that your activities comply with applicable laws, including copyright.Always check the webpage associated with each project before you get started. There may be important updates or corrections! The United States Fire Administration (USFA) has a guide and many simple steps you can take to prevent the loss of life and property resulting from electrical fires. StudioPieters® is not responsible for your product function or certification if you choose to use our products in your design.</sub>

<br>
<sub><sup>-------------------------------------------------------------------------------------------------------------------------------------</sup></sub>
<br>

<b>ORIGINAL PROJECT</b>

<b><sup>MIT LICENCE</sup></b>

<sub>Copyright © 2017 [Maxim Kulkin | ESP-Homekit-demo](https://github.com/maximkulkin/esp-homekit-demo)</sub>

<sub>Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:</sub>

<sub>The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software</sub>

<sub>THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.</sub>

<br>
<sub><sup>-------------------------------------------------------------------------------------------------------------------------------------</sup></sub>
<br>

<b>APPLE HOME</b>

<img  style="float: right;" src="https://github.com/AchimPieters/ESP32-SmartPlug/blob/main/images/apple_logo.png" width="10"><br><sub>HomeKit Accessory Protocol (HAP) is Apple’s proprietary protocol that enables third-party accessories in the home (e.g., lights, thermostats and door locks) and Apple products to communicate with each other. HAP supports two transports, IP and Bluetooth LE. The information provided in the HomeKit Accessory Protocol Specification (Non-Commercial Version) describes how to implement HAP in an accessory that you create for non-commercial use and that will not be distributed or sold.</sub>

<sub>The HomeKit Accessory Protocol Specification (Non-Commercial Version) can be downloaded from the HomeKit Apple Developer page.</sub>

<sub>Copyright © 2019 Apple Inc. All rights reserved.</sub>
