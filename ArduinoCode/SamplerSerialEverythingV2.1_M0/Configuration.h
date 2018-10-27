#pragma once

#include <stdint.h>

#include "Definitions.h"

enum class Mode {DAILY, PERIODIC};

// Values are stored in a struct within the Configuration class to make them easy to copy to/from flash
struct config_flash_t {
  uint8_t sampleAlarmMinute; // Minute to take a sample
  uint8_t sampleAlarmHour;   // Hour   to take a sample
  uint16_t periodicAlarmMinutes; // Time (in minutes) between samples in Periodic mode
  unsigned long flushDurationMs; // How long to perform a flush of main line (in milliseconds)
  unsigned long bagFlushDurationMs; // How long to perform a flush of individual bag (in milliseconds)
  unsigned long bagDrawDurationMs; // How long to perform a draw of individual bag (in milliseconds)
  unsigned long sampleDurationMs; // How long to draw a sample (in milliseconds)
  unsigned long sampleVolumeMl; // How big is the bag (in mL)
  uint8_t valveNumber; // Current number of valves sampled, 0=reset/default
  uint8_t flushValveNumber; // Number for flush valve
  bool written; // Has flash been written to?
  Mode mode; // Daily or Periodic sample timer
  char SMSNumbers[numSMSRecipients][phoneNumberLength + 1]; // Phone numbers for SMS status update recipients
  byte        checksum;                 // Value is changed when flash memory is written to.
//  uint8_t     instance_number;          // Default 0, should be set on startup from a patch
//  char        packet_header_string[80]; // String of expected packet header (dynamically formed based on config.h)
};

/**
   Stores OPEnSampler's configuration values
   Storing configuration in this class (as opposed to global struct like in earlier iterations) ensures these
   values are accessed/updated/validated consistently.
*/
class configClass
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
    void setBagFlushDuration(unsigned long milliseconds);
    uint8_t setSampleHour(unsigned int hour);
    uint8_t setSampleMinute(unsigned int minute);
    void setSMSNumber(int index, char *buffer);
    void setValveNumber(unsigned int valveNumber);

    Mode getMode();
    bool getWritten();
    unsigned long getFlushDuration();
    unsigned long getBagFlushDuration();
    unsigned long getSampleDuration();
    uint8_t getSampleHour();
    uint8_t getSampleMinute();
    char * getSMSNumber(int index);
    uint16_t getPeriodicAlarmLength();
    uint8_t getValveNumber();

    void read_non_volatile();
    void write_non_volatile();

  private:
    // This is stored as a struct so it can be easily written to flash.
    config_flash_t configData;
};
