#include <RTClibExtended.h> // Library to configure DS3231
#include "EEPROMAnything.h" // For reading/writing to EEPROM
#include "Configuration.h"

extern RTC_DS3231 RTC;

/**
 * Overwrite config with default values.
 */
void Configuration::setDefaults()
{
  configData.sampleAlarmMinute = SampleAlarmMinDef;
  configData.sampleAlarmHour   = SampleAlarmHrDef;
  configData.periodicAlarmMinutes = SampleAlarmPerDef;
  configData.flushDurationMs = FlushDurMsDef;
  configData.sampleDurationMs = SampleDurMsDef;
  configData.sampleVolumeMl = SampleVolMlDef;
  configData.valveNumber = SampleValveNumDef;
  configData.written = false;
  configData.mode = Mode::DAILY;

  // DEBUG
  // memset(configData.SMSNumbers, 0, sizeof(char) * numSMSRecipients * (phoneNumberLength + 1));
  memset(configData.SMSNumbers, 'A', sizeof(char) * numSMSRecipients * (phoneNumberLength + 1));
}

/**
 * Fills the data byte array with the configuration struct.
 */
void Configuration::getConfigData(uint8_t data[])
{
  memcpy(data, &configData, sizeof(configData));
}

/**
 * Set daily sample alarm in configuration and refresh alarm in RTC.
 */
void Configuration::setDailyAlarm(unsigned int hour, unsigned int minute)
{
  setSampleHour(hour);
  setSampleMinute(minute);
  configData.mode = Mode::DAILY;

  RTC.setAlarm(ALM1_MATCH_HOURS, getSampleMinute(), getSampleHour(), 0);

  Serial.println(F("Daily alarm set to sample once a day. Taking samples at "));
  Serial.print(getSampleHour());
  Serial.print(F(":"));
  Serial.println(getSampleMinute());
}

/**
 * Refresh the next sample time (for periodic mode) based on the current time.
 *
 * Calculates the next sample time based on the current time and the configured
 * periodic alarm length, then updates the configuration with these values.
 * Finally, it updates the RTC's alarm.
 */
void Configuration::refreshPeriodicAlarm()
{
  DateTime now = RTC.now();
  int periodLength = getPeriodicAlarmLength();

  int hour   = (now.hour() + (now.minute() + periodLength) / 60) % 24;
  int minute = (now.minute() + periodLength) % 60;

  hour   = setSampleHour(hour);
  minute = setSampleMinute(minute);

  RTC.setAlarm(ALM1_MATCH_HOURS, minute, hour, 0);

  Serial.print(F("Periodic sample alarm length (in minutes) is: "));
  Serial.print(minute);
}

/**
 * Set the hour for the next sample alarm.
 * Returns the stored sample hour.
 */
uint8_t Configuration::setSampleHour(unsigned int hour)
{
  if (hour < 24)
    configData.sampleAlarmHour = hour;
  else
  {
    Serial.print(F("ERROR: Invalid sample alarm hour specified: "));
    Serial.println(hour);
  }

  return configData.sampleAlarmHour;
}

/**
 * Set the minute for the next sample alarm.
 * Returns the stored sample minute.
 */
uint8_t Configuration::setSampleMinute(unsigned int minute)
{
  if (minute < 60)
    configData.sampleAlarmMinute = minute;
  else
  {
    Serial.print(F("ERROR: Invalid sample alarm minute specified: "));
    Serial.println(minute);
  }

  return configData.sampleAlarmMinute;
}

/**
 * Set the valve number of the next valve/bag to sample into.
 */
void Configuration::setValveNumber(unsigned int valveNumber)
{
  configData.valveNumber = valveNumber;
  Serial.print(F("The next valve to sample to is: "));
  // The value stored is 1 less than the next to actually be sampled (see loop())
  Serial.println(configData.valveNumber + 1);
}

/**
 * Set length between periodic sample alarms (in minutes).
 *
 * The alarm period must be longer than the flush and sample durations or the
 * operation will fail. If passed a valid period length, periodic alarm mode
 * will be enabled automatically.
 */
void Configuration::setPeriodicAlarm(unsigned int minutes)
{
  unsigned long milliseconds = (unsigned long) minutes * 60 * 1000;
  unsigned long minimum = getFlushDuration() + getSampleDuration();

  if(milliseconds <= minimum)
  {
    Serial.print(F("ERROR: Failed to set sample alarm period to "));
    Serial.print(minutes);
    Serial.print(F(" minutes ("));
    Serial.print(milliseconds);
    Serial.print(F(" ms). Must be greater than flush and sample durations combined: "));
    Serial.print(minimum);
    Serial.println(" ms");
    return;
  }

  configData.periodicAlarmMinutes = minutes;
  configData.mode = Mode::PERIODIC;

  // Update next sample time with new alarm period length
  refreshPeriodicAlarm();
}

/**
 * Set the duration of a sample in milliseconds.
 *
 * Returns false and fails if the value is not positive.
 */
void Configuration::setSampleDuration(unsigned long milliseconds)
{
  if (milliseconds == 0)
  {
    Serial.println(F("ERROR: Sample duration must be a positive value."));
    return;
  }

  configData.sampleDurationMs = milliseconds;

  Serial.print(F("Sample duration set to "));
  Serial.print(getSampleDuration());
  Serial.println(F(" milliseconds."));
}

/**
 * Set the phone number of an SMS status update recipient.
 *
 * Sets a null-terminated string containing a phone number at the specified
 * index to be stored persistently in configuration. Phone numbers can be up to
 * as long as phoneNumberLength specifies.
 * The number should be stored without punctuation or whitespace.
 * (e.g. 123456789012345).
 */
void Configuration::setSMSNumber(int index, char *buffer)
{
  if (index >= numSMSRecipients)
    return;

  strncpy(configData.SMSNumbers[index], buffer, phoneNumberLength);
  configData.SMSNumbers[index][phoneNumberLength] = '\0';

  return;
}

/**
 * Set the duration of a flush in milliseconds.
 */
void Configuration::setFlushDuration(unsigned long milliseconds)
{
  if (milliseconds == 0)
  {
    Serial.println(F("ERROR: Flush duration must be a positive value."));
    return;
  }

  configData.flushDurationMs = milliseconds;

  Serial.print(F("Flush duration set to "));
  Serial.print(getFlushDuration());
  Serial.println(F(" milliseconds."));
}

void Configuration::setWritten(bool written)
{
  configData.written = written;
}

void Configuration::setMode(Mode mode)
{
  configData.mode = mode;
}

Mode Configuration::getMode()
{
  return configData.mode;
}

bool Configuration::getWritten()
{
  return configData.written;
}

unsigned long Configuration::getFlushDuration()
{
    return configData.flushDurationMs;
}

unsigned long Configuration::getSampleDuration()
{
    return configData.sampleDurationMs;
}

uint8_t Configuration::getSampleHour()
{
    return configData.sampleAlarmHour;
}

uint8_t Configuration::getSampleMinute()
{
    return configData.sampleAlarmMinute;
}

/**
 * Get the phone number of an SMS status update recipient.
 *
 * Returns a pointer to an ASCII phone number, or null if the phone number at
 * index is undefined. The number is stored without punctuation or whitespace
 * (e.g. 123456789012345).
 */
char * Configuration::getSMSNumber(int index)
{
  if (index >= numSMSRecipients)
    return NULL;

  if (strlen(configData.SMSNumbers[index]) == 0)
    return NULL;

  return configData.SMSNumbers[index];
}

uint16_t Configuration::getPeriodicAlarmLength()
{
  return configData.periodicAlarmMinutes;
}

uint8_t Configuration::getValveNumber()
{
  return configData.valveNumber;
}

/**
 * Copy the configuration data stored in EEPROM to the Configuration class's storage
 */
void Configuration::readFromEEPROM()
{
  EEPROM_readAnything(0, configData);
}

/**
 * Write the configuration data from the Configuration class's storage to EEPROM
 */
void Configuration::writeToEEPROM()
{
  EEPROM_writeAnything(0, configData);
  setWritten(true);
}
