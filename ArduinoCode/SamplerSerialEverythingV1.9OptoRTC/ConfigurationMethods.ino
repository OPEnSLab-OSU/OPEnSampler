/**
 * This file contains methods for manipulating the configuration struct and
 * OPEnSampler state. The struct is declared in SamplerSerialEverythingv1.90ptoRTC.ino
 */

bool puppetValveState(unsigned int valveNumber, bool valveState)
{
  if(valveNumber < 0 || valveNumber > NUM_VALVES)
  {
    Serial.print(F("ERROR: Specified valve does not exist: "));
    Serial.println(valveNumber);
    return false;
  }

  configuration.VNum = valveNumber;

  Serial.print(F("Toggling valve: "));
  Serial.println(valveNumber);
  Serial.print(F("Valve is now "));

  // Valve 0 is the flush valve, and is handled uniquely from the others
  if(valveNumber == 0)
  {
    if(valveState) // then turn flush on or off depending on valveState
    {
      flushON();   // Flush bit on, Preserves valve bits
      Serial.println(F("open."));
    }
    else
    {
      flushOFF();
      Serial.println(F("closed."));
    }
    return true;
  }

  if(valveState)
  {
    Serial.println(F("open."));
    // Configure TPIC buffers according to current valve, strobe out to SPI
    setValveBits(); // Assumes you only want one valve on at a time, auto closes other valves
  }
  else
  {
    Serial.println(F("closed."));
    clearValveBits(); // CLEAR valve bits, preserve flush bit
  }
  return true;
}

void setAlarmPeriod()
{
  DateTime now = RTC.now(); // Check the current time

  // Calculate new time
  configuration.SAMin = (now.minute()+configuration.SAPer)%60;  // wrap-around using modulo every 60 sec
  configuration.SAHr  = (now.hour()+((now.minute()+configuration.SAPer)/60))%24; // quotient of now.min+periodMin added to now.hr, wraparound every 24hrs

  Serial.print(F("Resetting Alarm 1 for: ")); Serial.print(configuration.SAHr); Serial.print(F(":"));Serial.println(configuration.SAMin);

   // Set alarm1
   RTC.setAlarm(ALM1_MATCH_HOURS, configuration.SAMin, configuration.SAHr, 0);
}

/**
 * Set a daily alarm to sample at the specified hour and minute of the day.
 *
 * Automatically switches to daily sampling mode and resets the sample alarm.
 */
void setDailyAlarm(unsigned int hour, unsigned int minute)
{
  configuration.Is_Daily = true;

  configuration.SAHr = hour;
  configuration.SAMin = minute;

  RTC.setAlarm(ALM1_MATCH_HOURS, configuration.SAMin, configuration.SAHr, 0);
}

/**
 * Set the duration of a flush in milliseconds.
 *
 * Returns false and fails if the value is not positive.
 */
bool setFlushDuration(unsigned int milliseconds)
{
  if (milliseconds == 0)
  {
    Serial.println(F("ERROR: Flush duration must be a positive value."));
    return false;
  }

  configuration.FDMs = milliseconds;
  return true;
}

/**
 * Set the valve number of the next valve/bag to sample into.
 *
 * Returns false and fails if the next valve is < 1.
 */
bool setNextValve(unsigned int valveNumber)
{
  if (valveNumber == 0)
  {
    Serial.println(F("ERROR: Next valve number must be a positive value."));
    return false;
  }

  // The value is decremented before storing because it will be incremented
  // automatically in the main loop.
  configuration.VNum = valveNumber - 1;
  return true;
}

/**
 * Set a periodic alarm which samples repeatedly after the specified interval.
 *
 * The alarm period must be longer than the flush and sample durations or the
 * operation will fail. If passed a valid period length, periodic alarm mode
 * will be enabled automatically and the current sample alarm will be reset.
 *
 * Returns true if the periodic alarm is updated successfully.
 */
bool setPeriodicAlarm(unsigned int minutes)
{
  if((minutes * 60 * 1000) <= (configuration.FDMs+configuration.SDMs))
  {
    Serial.print(F("ERROR: Sample period of "));
    Serial.print(minutes);
    Serial.println(F(" must be at least flush and sample durations combined."));
    return false;
  }

  configuration.SAPer = minutes;
  configuration.Is_Daily = false; // Switch to periodic mode
  setAlarmPeriod(); // Reset alarm with new period length
  return true;
}

/**
 * Set the duration of a sample in milliseconds.
 *
 * Returns false and fails if the value is not positive.
 */
bool setSampleDuration(unsigned int milliseconds)
{
  if (milliseconds == 0)
  {
    Serial.println(F("ERROR: Sample duration must be a positive value."));
    return false;
  }

  configuration.SDMs = milliseconds;
  return true;
}

/**
 * Set the real time clock and reset the sample alarm.
 *
 * Sets the real time clock to the specified year, month, day, hour, and minute.
 * Also resets daily or periodic alarms to begin from the updated time.
 */
void setClock(unsigned int year, unsigned int month, unsigned int day, unsigned int hour, unsigned int minute)
{
  RTC.adjust(DateTime(year, month, day, hour, minute, 0));

  if (configuration.Is_Daily)
    RTC.setAlarm(ALM1_MATCH_HOURS, configuration.SAMin, configuration.SAHr, 0);
  else
    setAlarmPeriod();
}
