# ESP32 - HomeKit Demo

<b>REPRODUCTION STEPS</b>

Open a terminal window on your mac.

```
docker pull espressif/idf:latest
```
- At this point idf (ESP-IDF v5.4-dev-78-gd4cd437ede)
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
- Select `Serial flasher config` and then `Flash size (2MB)` set to `4MB`
- Select `Partition table` and then `Partition Table(Single factory app, no OTA)` Set to `Custom partition table CSV`
- Select `StudioPieters` and then `(mysid) WIFI SSID` and fill in your Wi-Fi Network name, then Select `(mypassword) WiFI Password` and fill in your Wi-Fi Network password.
- Then press `ESC` until you are asked `Save Configuration?` and select `(Y)es`
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
- Replace `/dev/tty.usbserial-01FD1166` with your USB port.
```
screen /dev/tty.usbserial-01FD1166 115200
```
- Replace `/dev/tty.usbserial-01FD1166` with your USB port.

<br>


|                  | <sub>ESP32 Series</sub> | <sub>ESP32-C2 Series</sub> | <sub>ESP32-C3 Series</sub> | <sub>ESP32-C5 Series</sub>| <sub>ESP32-C6 Series</sub> | <sub>ESP32-S2 Series</sub> | <sub>ESP32-S3 Series</sub> | <sub>ESP32-H2 Series</sub>| <sub>ESP32-P2 Series</sub> |
|------------------|--------------|-----------------|-----------------|----------------|-----------------|-----------------|-----------------|-----------------|-----------------|
| <sub>LED</sub>              |<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/NO.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/NO.svg" width="20">|
| <sub>LOCK</sub>           |<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/NO.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/NO.svg" width="20">|
| <sub>PROGRAMMABLE SWITCH</sub> |<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/NO.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/NO.svg" width="20">|
| <sub>SWITCH</sub> |<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/NO.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/NO.svg" width="20">|
| <sub>SECURITY SYSTEM</sub> |<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/NO.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/NO.svg" width="20">|
| <sub>TEMPERATURE SENSOR</sub> |<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/NO.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/NO.svg" width="20">|
| <sub>THERMOSTAT</sub> |<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/NO.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/NO.svg" width="20">|
| <sub>OUTLET</sub> |<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/NO.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/NO.svg" width="20">|
| <sub>LIGHT</sub> |<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/NO.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/NO.svg" width="20">|
| <sub>LIGHT STRIP RGB</sub> |<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/NO.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/NO.svg" width="20">|
| <sub>LIGHT STRIP RGBW</sub> |<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/NO.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/NO.svg" width="20">|
| <sub>WINDOW COVERING</sub> |<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/NO.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/NO.svg" width="20">|
| <sub>FAN</sub> |<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/NO.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/NO.svg" width="20">|
<sub>GARAGE DOOR OPENER</sub> |<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/NO.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/NO.svg" width="20">|
| <sub>LIGHT SENSOR</sub> |<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/NO.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/NO.svg" width="20">|
<sub>MOTION SENSOR</sub> |<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/YES.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/NO.svg" width="20">|<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/NO.svg" width="20">|
<br>

<img  style="float: right;" src="https://github.com/AchimPieters/ESP32-SmartPlug/blob/main/images/works-with-apple-home.svg" width="150"> <img  style="float: right;" src="https://github.com/AchimPieters/ESP32-SmartPlug/blob/main/images/MIT%7C%20SOFTWARE%20WHITE.svg" width="150">

| <sup><b>HOMEKIT ACCESSORY CATEGORY</b></sup> | <sup><b>NUMBER</b></sup> |
|----------------------------|--------|
| <sup>Other</sup>                      | <sup>1</sup>      |
| <sup>Bridges</sup>                    | <sup>2</sup>      |
| <sup>Fans</sup>                       | <sup>3</sup>      |
| <sup>Garage door openers</sup>        | <sup>4</sup>      |
| <sup>Lighting</sup>                   | <sup>5</sup>      |
| <sup>Locks</sup>                      | <sup>6</sup>      |
| <sup>Outlets</sup>                    | <sup>7</sup>      |
| <sup>Switches</sup>                   | <sup>8</sup>      |
| <sup>Thermostats</sup>                | <sup>9</sup>      |
| <sup>Sensors</sup>                    | <sup>10</sup>     |
| <sup>Security systems</sup>           | <sup>11</sup>     |
| <sup>Doors</sup>                      | <sup>12</sup>     |
| <sup>Windows</sup>                    | <sup>13</sup>     |
| <sup>Window coverings</sup>           | <sup>14</sup>     |
| <sup>Programmable switches</sup>      | <sup>15</sup>     |
| <sup>Range extenders</sup>            | <sup>16</sup>     |
| <sup>IP cameras</sup>                 | <sup>17</sup>     |
| <sup>Video door bells</sup>           | <sup>18</sup>     |
| <sup>Air purifiers</sup>              | <sup>19</sup>     |
| <sup>Heaters</sup>                    | <sup>20</sup>     |
| <sup>Air conditioners</sup>           | <sup>21</sup>     |
| <sup>Humidifiers</sup>                | <sup>22</sup>     |
| <sup>Dehumidifiers</sup>              | <sup>23</sup>     |
| <sup>Apple tv</sup>                   | <sup>24</sup>     |
| <sup>Speakers</sup>                   | <sup>26</sup>     |
| <sup>Airport</sup>                    | <sup>27</sup>     |
| <sup>Sprinklers</sup>                 | <sup>28</sup>     |
| <sup>Faucets</sup>                    | <sup>29</sup>     |
| <sup>Shower heads</sup>               | <sup>30</sup>     |
| <sup>Televisions</sup>                | <sup>31</sup>     |
| <sup>Target remotes</sup>             | <sup>32</sup>     |
<br>
<br>
<sub><sup>-------------------------------------------------------------------------------------------------------------------------------------</sup></sub>
<br>
<b>WORKS WITH APPLE HOME BADGE</b>

<sup>The Works with Apple Home badge can be used to visually communicate that your accessory is compatible with the Apple Home and Siri on Apple devices. If you plan to develop or manufacture a HomeKit accessory that will be distributed or sold, your company needs to be enrolled in the MFi Program.</sup>

<img  style="float: right;" src="https://github.com/AchimPieters/ESP32-SmartPlug/blob/main/images/works-with-apple-home.svg" width="150">

<br>
<sub><sup>-------------------------------------------------------------------------------------------------------------------------------------</sup></sub>
<br>
<b>SAFETY</b>

<sub>Technology and the laws and limitations imposed by manufacturers and content owners are constantly changing. Thus, some of the projects described may not work, may be inconsistent with current laws or user agreements, or may damage or adversely affect some equipment.
Your safety is your own responsibility, including proper use of equipment and safety gear, and determining whether you have adequate skill and experience. Power tools, electricity, and other resources used for these projects are dangerous, unless used properly and with adequate precautions, including safety gear. Some illustrative photos do not depict safety precautions or equipment, in order to show the project steps more clearly. These projects are not intended for use by children. Use of the instructions, kits, projects and suggestions on StudioPieters.nl is at your own risk. StudioPieters® disclaims all responsibility for any resulting damage, injury, or expense. It is your responsibility to make sure that your activities comply with applicable laws, including copyright.Always check the webpage associated with each project before you get started. There may be important updates or corrections! The United States Fire Administration (USFA) has a guide and many simple steps you can take to prevent the loss of life and property resulting from electrical fires. StudioPieters® is not responsible for your product function or certification if you choose to use our products in your design.</sub>

<br>
<sub><sup>-------------------------------------------------------------------------------------------------------------------------------------</sup></sub>
<br>

<b>WOLFSSL</b>

<img  style="float: right;" src="https://github.com/AchimPieters/esp32-homekit-demo/blob/main/images/wolfssl_logo.svg" width="100">

<b><sup>GPLv2</sup></b>

<sub>wolfSSL (formerly known as CyaSSL) and wolfCrypt are either licensed for use under the GPLv2 (or at your option any later version) or a standard commercial license. For our users who cannot use wolfSSL under GPLv2 (or any later version), a commercial license to wolfSSL and wolfCrypt is available.</sub>

<sub>See the LICENSE.txt, visit wolfssl.com/license, contact us at licensing@wolfssl.com or call +1 425 245 8247</sub>

<sub>View Commercial Support Options: wolfssl.com/products/support-and-maintenance</sub>

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
