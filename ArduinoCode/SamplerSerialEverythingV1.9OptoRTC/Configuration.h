// TODO: Move documentation comments to header

#pragma once

#include <stdint.h>
#include <Arduino.h>
#include <RTClibExtended.h> // Library to configure DS3231
#include "Defaults.h"

enum class Mode {DAILY, PERIODIC};

struct config_t {
  Mode mode; // Mode of sample alarm
  bool written; // Has EEPROM been written to?
  uint8_t sampleAlarmHour;   // Hour   to take a sample
  uint8_t sampleAlarmMinute; // Minute to take a sample
  uint16_t periodicAlarmMinutes; // Time (in minutes) between samples in Periodic mode
  unsigned long flushDurationMs; // How long to perform a flush (in milliseconds)
  unsigned long sampleDurationMs; // How long to draw a sample (in milliseconds)
  uint16_t sampleVolumeMl; // Sample volume in ml (currently unused)
  uint8_t valveNumber; // Current number of valves sampled, 0=reset/default
  char SMSNumbers[numSMSRecipients][phoneNumberLength+1]; // Phone numbers for SMS status update recipients
};

// Struct for saving Sampler params in EEPROM, see http://playground.arduino.cc/Code/EEPROMWriteAnything
class Configuration
{
public:
  void setDefaults();

  void getConfigData(uint8_t data[]);

  void setDailyAlarm(unsigned int hour, unsigned int minute);

  void setPeriodicAlarm(unsigned int minutes);
  void refreshPeriodicAlarm();

  void setMode(Mode mode);
  void setWritten(bool written);
  void setSampleDuration(unsigned long milliseconds);
  void setFlushDuration(unsigned long milliseconds);
  uint8_t setSampleHour(unsigned int hour);
  uint8_t setSampleMinute(unsigned int minute);
  void setSMSNumber(int index, char *buffer);
  void setValveNumber(unsigned int valveNumber);

  Mode getMode();
  bool getWritten();
  unsigned long getFlushDuration();
  unsigned long getSampleDuration();
  uint8_t getSampleHour();
  uint8_t getSampleMinute();

  /**
   * Get the phone number of an SMS status update recipient.
   *
   * @return a pointer to an ASCII phone number, or null if the phone number at
   * index is undefined. The number is stored without punctuation or whitespace
   * (e.g. 123456789012345).
   */
  char * getSMSNumber(int index); // TODO: const char *?

  uint16_t getPeriodicAlarmLength();
  uint8_t getValveNumber();

  void readFromEEPROM();
  void writeToEEPROM();

private:
  // This is stored as a struct so it can be easily written to EEPROM.
  config_t configData;
};
