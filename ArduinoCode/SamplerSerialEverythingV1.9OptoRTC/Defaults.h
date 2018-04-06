#pragma once

#include <stdint.h>

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

// Volume in milliliters of each sample container
// (This is currently unused, but could be used in the future with safety calculations.)
const uint8_t SampleVolMlDef = 250;

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
const uint8_t numSMSRecipients = 3;

// Maximum number of digits in a phone number
const uint8_t phoneNumberLength = 13;

/* -----------------------
 * OPEnSampler Information
 * -----------------------
 */
const int numValves = 24; // Amount of valves in each module
const int numModules = 1; // XXX: Unused. 1 master module for now. Will be used to calculate number of shifts to TPICs for expansion modules

/* --------------------
 * OPEnSampler Features
 * --------------------
 */
// TODO
const bool enableBluetooth = true;
const bool enableSMS       = true;
