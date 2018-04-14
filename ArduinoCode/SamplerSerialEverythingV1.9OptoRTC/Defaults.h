#pragma once

/* ----------------------
 * Configuration Defaults
 * ----------------------
 * Alter the values below to change the OPEnSampler's default configuration.
 */

// Minute to start a sample on in daily mode
const uint8_t SampleAlarmMinDef = 00;

// Hour (in 24-hour time) to start a sample on in daily mode
const uint8_t SampleAlarmHrDef = 8;

// Number of minutes in between samples in periodic mode
const uint16_t SampleAlarmPerDef = 2880; // 2 days

// Number of milliseconds to run pumps for when sampling.
// (Takes 60 seconds for 250ml, so don't overfill.)
const uint16_t SampleDurMsDef = 30000; // 30 seconds

// Number of milliseconds to run pumps for when flushing.
const uint16_t FlushDurMsDef  = 30000; // 30 seconds

// The current active valve number.
// This value + 1 will be the next valve to draw a sample.
const bool SampleValveNumDef = 0;


/* --------------------
 * Phone Number Storage
 * --------------------
 * WARNING: Raising this too high might may cause instability/crashing if too
 * little dynamic memory is available, or if the configuration struct becomes too
 * large for EEPROM.
 *
 * bytes used = numSMSRecipients * (phoneNumberLength + 1)
 */

// Quantity of phone numbers to store in EEPROM that will recieve SMS status updates.
const uint8_t numSMSRecipients = 2;

// Maximum number of digits in a phone number
const uint8_t phoneNumberLength = 11;


/* -----------------------
 * OPEnSampler Information
 * -----------------------
 */
const int numValves = 24; // Amount of valves in each module (currently only supports 1 module)


/* --------------------
 * OPEnSampler Features
 * --------------------
 */
// TODO: Decide how to handle value location
// Maybe make enableSMS const and use another value for tracking it?
const bool enableBluetooth = true;
extern bool enableSMS; // Whether to send SMS status updates (set in SamplerSerialEverything)


/* --------------------
 * Miscellaneous
 * --------------------
 */
const uint32_t baud = 115200; // BAUD rate for USB serial (when debugging ensure serial monitor matches)
