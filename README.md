# OPEnSampler (Capstone Fork)

The OPEnSampler is an openly published water sampler designed with a focus on
customizability. The device was designed in Autodesk Fusion 360, a free CAD software for
any student or startup with low profits, specified by their terms for the license.

The main features include an Arduino-controlled array of solenoid valves to manage the
sample water flow and a 12VDC peristaltic pump, an extruded aluminum frame and 3D printed
components for attaching the valves, bags, pump, and electronics to the device.
The OPEnSampler fits in a Pelican 80QT Rolling Cooler and is powered from a 12VDC battery.

Check out our blog for updates and more information!

**Blog:** http://www.open-sensing.org/watersamplerblog/

**Fusion 360:** https://www.autodesk.com/products/fusion-360/students-teachers-educators

**OPEnS Lab Website:** http://www.open-sensing.org/

**OPEnS Lab Facebook:** https://www.facebook.com/open.sensing/

## Capstone

This folder is a fork of the OPEnSampler project built on by members of an Oregon State
University CS Capstone group, working in collaboration with OPEnS Lab.
This project enhances the OPEnSampler by adding support for sending text messages (SMS) to
send information to users, and by developing an Android app that can configure the sampler
over Bluetooth. 

## Dependencies

The following dependencies are available in `ArduinoCode/Libraries`.
To install them, follow the [instructions for installing libraries in Arduino
IDE](https://www.arduino.cc/en/guide/libraries#toc4).

* [Adafruit FONA](https://github.com/adafruit/Adafruit_FONA.git) for sending SMS
  status updates.
* [Adafruit nRF8001](https://github.com/adafruit/Adafruit_nRF8001.git) for
  communicating with the OPEnSampler Companion App over Bluetooth Low Energy.
* [EEPROMWriteAnything](http://playground.arduino.cc/Code/EEPROMWriteAnything)
  for writing the OPEnSampler's configuration to EEPROM. (This is included
  directly within the Skectchbook.)
* [Low-Power](https://github.com/rocketscream/Low-Power) for sleeping in
  between samples.
* [RTClibExtended](https://github.com/FabioCuomo/FabioCuomo-DS3231) for
  interrupt alarms that wake the OPEnSampler from sleeping.
