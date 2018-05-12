# OPEnSampler Command API

The OPEnSampler exposes an API for running commands that can update
configuration and control the device. This API is exposed in two ways:

1. Via USB (can be used with Arduino IDE's serial monitor)
2. Via Bluetooth Low Energy (for use with the OPEnSampler Companion App for
   Android)

This document provides a reference for available commands and their syntax.
The format of these commands vary slightly between USB serial and Bluetooth. In
the long term it would be better to refactor the codebase to make the API
consistent, and parse commands & arguments correctly regardless of their source.

## Command Syntax

All commands begin with a command identifier which consists of a string of
capital letters or a single capital letter. This identifier is then followed by
a series of arguments (when needed). The specific syntax varies depending on
whether the command is parsed from USB serial or from Bluetooth.

### USB Serial Command Syntax
TODO

### Bluetooth Command Syntax
TODO

## List of Commands

 | Identifier (via USB) | Identifier (via Bluetooth) | Arguments                      | Description
 | -------------------- | -------------------------- | ------------------------------ | -----------
 | CLK                  | C                          | day, month, year, hour, minute | Sets the real-time clock (RTC).
 | FD                   | F                          | milliseconds                   | Sets the flush duration in milliseconds.
 | FD                   |                            |                                | Prints the flush duration (in milliseconds) to Serial.
 | M                    | M                          | pump state                     | Sets the pump's state depending on the argument's value: off (0), on (1), reverse (-1)
 | PR                   | G                          | index                          | Prints an SMS status update recipient's phone number stored at the specified index to USB serial.
 | PW                   | W                          | index, phone number            | Sets an SMS status update recipient's phone number at the specified index.
 | RST                  | R                          |                                | Resets configuration stored in EEPROM
 | SAD                  | D                          | hour, minute                   | Sets daily sample alarm to sample once per day at the specified time and switch to daily sampling mode.
 | SAP                  | P                          | minutes                        | Sets periodic sample alarm to sample at fixed intervals (in minutes), starting from the current time.
 | SAP                  |                            |                                | Prints the periodic sample alarm length (in minutes) to serial.
 | SD                   | S                          | milliseconds                   | Sets the sample duration in milliseconds.
 | SD                   |                            |                                | Prints the sample duration (in milliseconds) to Serial.
 | VN                   | V                          | valve number                   | Sets the next valve/bag to sample into (they're numbered, starting from 1).
 | VS                   | U                          | valve number, valve state      | Sets the specificed valve's state: open (1), closed (0)
 |                      | B                          |                                | Sends the raw configuration data over Bluetooth.
