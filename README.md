# ESP32 - HomeKit Demo

1. Install docker
2. Install IDF V5.0 - `docker pull espressif/idf:v5.0`
3. Clone repository - `git clone --recursive https://github.com/AchimPieters/esp32-homekit-demo.git`
4. start IDF - ` docker run -it -v ~/ESP32-HOMEKIT-DEMO:/project -w /project espressif/idf:v5.0`
5. cd `examples` - cd `led`
6. `idf.py build`

<br>

|                  | <sub>ESP32 Series</sub> | <sub>ESP32-C6 Series</sub> | <sub>ESP32-C3 Series</sub> | <sub>ESP32-C Series</sub> | <sub>ESP32-S3 Series</sub> | <sub>ESP32-S2 Series</sub> | <sub>ESP32-H2 Series</sub> |
|------------------|--------------|-----------------|-----------------|----------------|-----------------|-----------------|-----------------|
| <sub>LED</sub>              |              |                 |                 |                |                 |                 |                 |
| <sub>BUTTON</sub>           |              |                 |                 |                |                 |                 |                 |
| <sub>DYNAMIC SERVICES</sub> |              |                 |                 |                |                 |                 |                 |



<br>

<img  style="float: right;" src="https://github.com/AchimPieters/ESP32-SmartPlug/blob/main/images/works-with-apple-home.svg" width="150"> <img  style="float: right;" src="https://github.com/AchimPieters/ESP32-SmartPlug/blob/main/images/MIT%7C%20SOFTWARE%20WHITE.svg" width="150"> 

<br>
<sub><sup>-------------------------------------------------------------------------------------------------------------------------------------</sup></sub>
<br>

**<sub>SAFETY</sub>**

<sub><sup>Technology and the laws and limitations imposed by manufacturers and content owners are constantly changing. Thus, some of the projects described may not work, may be inconsistent with current laws or user agreements, or may damage or adversely affect some equipment.
Your safety is your own responsibility, including proper use of equipment and safety gear, and determining whether you have adequate skill and experience. Power tools, electricity, and other resources used for these projects are dangerous, unless used properly and with adequate precautions, including safety gear. Some illustrative photos do not depict safety precautions or equipment, in order to show the project steps more clearly. These projects are not intended for use by children. Use of the instructions, kits, projects and suggestions on StudioPieters.nl is at your own risk. StudioPieters® disclaims all responsibility for any resulting damage, injury, or expense. It is your responsibility to make sure that your activities comply with applicable laws, including copyright.Always check the webpage associated with each project before you get started. There may be important updates or corrections! The United States Fire Administration (USFA) has a guide and many simple steps you can take to prevent the loss of life and property resulting from electrical fires. Click here to read them! StudioPieters® is not responsible for your product function or certification if you choose to use our products in your design.</sup></sub>

<br>
<sub><sup>-------------------------------------------------------------------------------------------------------------------------------------</sup></sub>
<br>

**<sub>ORIGINAL PROJECT</sub>**

<sub><sup>MIT LICENCE</sup></sub>

<sub><sup>Copyright © 2017 [Maxim Kulkin](https://github.com/maximkulkin/esp-homekit-demo)</sup></sub>

<sub><sup>Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:</sup></sub>

*<sub><sup>The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software</sup></sub>*

<sub><sup>THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.</sup></sub>

<br>
<sub><sup>-------------------------------------------------------------------------------------------------------------------------------------</sup></sub>
<br>

**<sub>APPLE HOME</sub>**

<img  style="float: right;" src="https://github.com/AchimPieters/ESP32-SmartPlug/blob/main/images/apple_logo.png" width="10"> <sub><sup>HomeKit Accessory Protocol (HAP) is Apple’s proprietary protocol that enables third-party accessories in the home (e.g., lights, thermostats and door locks) and Apple products to communicate with each other. HAP supports two transports, IP and Bluetooth LE. The information provided in the HomeKit Accessory Protocol Specification (Non-Commercial Version) describes how to implement HAP in an accessory that you create for non-commercial use and that will not be distributed or sold.</sup></sub>

<sub><sup> The HomeKit Accessory Protocol Specification (Non-Commercial Version) can be downloaded from the HomeKit Apple Developer page.</sup></sub>

<sub><sup> Copyright © 2019 Apple Inc. All rights reserved. </sup></sub>
