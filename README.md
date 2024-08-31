# chickenguard-2019-upgrade

![architecture](./docs/R1.1/architecture.drawio.png)

Open-source replacement for the electronics of the [ChickenGuard](https://www.chickenguard.be/), an automatic chicken door opener.

## Added features, not present in original ChickenGuard
* Open source, so you can easily repair it yourself.  No epoxy covered PCBs.
* Will stop the door when blocked (e.g. by a chicken) (this feature is only present in most recent versions of the ChickenGuard).
* Automatic configuration of the door position.
* Web interface for configuration.
  * Time and date set automatically correctly from the smartphone.
  * Easy setup of fixed times for opening and closing the door.
  * Automatic setup of sunrise and sunset times, based on geolocation.
  * To save costs, the LCD display can be removed, and the device can be configured via the smartphone.
  * Future firmware upgrade might allow integration with home automation systems.
* USB-C port for power and programming.
* RoHS compliant (no CdS light sensor).

## [Use cases](./use-cases.md)

## Documentation

* [Product Requirement Document](./docs/PRD.md)

## Future work
The initial goal was to send the geolocation of the smartphone to the ESP32, so it can calculate the sunrise and sunset times.  That didn't work, because the ESP32 captive portal is not using HTTPS with a valid certificate.  The smartphone refuses to send the geolocation to an insecure website.

Instead of running a webserver on the ESP32.  Web Bluetooth (example [here](https://randomnerdtutorials.com/esp32-web-bluetooth/)) could be used.  The smartphone downloads a web page from the internet.  Using this info, the smartphone can connect to the ESP32 using Web Bluetooth.  Maybe that will allow the smartphone to send the geolocation to the ESP32. 

## Revision history
### Revision 1.02
  * [Schematic design](https://personal-viewer.365.altium.com/client/index.html?feature=embed&source=66EF0726-05D2-429F-9004-3D691D80F956&activeView=SCH)
  * Patched PCB of 1.01.  No new PCB layout done


### Revision 1.01 : Using an external RTC
  * PCB cost reduction : layer count reduced from 4 to 2.
  * [Concept](./docs/R1.1/concept.md)
  * [Technical study](./docs/R1.1/technical-study_1.1.ipynb)
  * [Hardware design](https://365.altium.com/files/C9F51258-D859-4807-ACB1-E5928658F052) : for users with a free [AltiumLive account](https://365.altium.com/signin?ReturnUrl=https%3A%2F%2Fwww.altium.com%2Fviewer%2F).
  * [Hardware test report](./docs/R1.1/HardwareTest.ipynb)

### Revision 1.00 : Using the internal RTC of the ESP32
  * PCB design created, but never manufactured
  * [Concept](./docs/R1.0/concept.md)
  * [Technical study](./docs/R1.0/technical-study_1.0.ipynb)

## Prior Art
* [Peno64 ChickenGuard](https://github.com/peno64/ChickenGuard/blob/master/ChickenGuard.ino)
* [JP_chickenDoor](https://github.com/f2knpw/JP_chickenDoor/blob/master/JP_ESP32_ChickenDoor_wifi_IRsensor_Arduino.ino)
* [ChickenGuard](https://www.chickenguard.be/)
  * Uses an 8bit MCU (PIC18F14K22), an external RTC with 32.768kHz crystal and an SOIC8 motor driver.
  * The PCB was covered in epoxy, which has been removed using a hot air gun.  Some components got damaged in the process.  Aceton might also work (it should certainly work in case of hot glue).

---

Original version of this repository is at [LieBtrau/chickenguard-2019-upgrade](https://github.com/LieBtrau/chickenguard-2019-upgrade).

