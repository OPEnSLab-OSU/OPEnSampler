  /* --------------
 * Global Objects
 * --------------
 * This header declares global objects defined in SamplerSerialEverything.
 * It can be included as a convenience (instead of declaring externs everywhere
 * they're needed). */

#pragma once

#include "Adafruit_BLE_UART.h" // Library for nRF8001 (BLE breakout)
#include "Adafruit_FONA.h"     // Library for FONA 808 (GSM/SMS breakout)
#include <SoftwareSerial.h>    // Needed for FONA 808
#include <RTClibExtended.h>    // Library for DS3231 Real Time Clock (RTC)

#include "Configuration.h"

extern Configuration config; // Configuration data (stored persistently)
extern Adafruit_BLE_UART BLESerial; // For Bluetooth Low Energy (BLE) comms
extern Adafruit_FONA fona; // For GSM/SMS comms (text messages)
extern RTC_DS3231 RTC; 
