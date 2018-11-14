//TimeFunctions

// ======= BOARD SPECIFIC SETTINGS ======

//NOTE: Must include the following line in the RTClibExtended.h file to use with M0:
//#define _BV(bit) (1 << (bit))
#include <RTClibExtended.h>
#include <LowPower.h>

#define EI_NOTEXTERNAL
#include <EnableInterrupt.h>

// ================================================================
// RTC
// ================================================================
void RTCReportTime()
{
  // Report time and next alarm back
  Serial.println(F("RTC time is"));
  DateTime now = RTC.now();
  Serial.print(now.hour(), DEC); Serial.print(':'); Serial.print(now.minute(), DEC); Serial.print(':'); Serial.print(now.second(), DEC); Serial.println();
}

/**
   Set the real time clock and updates the sample alarm.

   Sets the real time clock to the specified year, month, day, hour, and minute.
   Then resets the daily or periodic alarm to use the updated time.
*/
void setClock(unsigned int year, unsigned int month, unsigned int day, unsigned int hour, unsigned int minute)
{
  RTC.adjust(DateTime(year, month, day, hour, minute, 0));

  if (configInst.getMode() == Mode::PERIODIC)
    configInst.refreshPeriodicAlarm();
  else if (configInst.getMode() == Mode::DAILY)
    RTC.setAlarm(ALM1_MATCH_HOURS, configInst.getSampleMinute(), configInst.getSampleHour(), 0);

  RTCReportTime(); // print current RTC time
  Serial.print(F("Next Sample Alarm set for: "));
  Serial.print(configInst.getSampleHour());
  Serial.print(F(":"));
  Serial.println(configInst.getSampleMinute());
}

// ================================================================
// Timer ISR, Execute every sample period
// ================================================================
void wakeUp()
{
  // Disable external pin interrupt on wake up pin.
  detachInterrupt(digitalPinToInterrupt(WAKEUP_PIN));
  timerEN = true; // Enable timed functions
  sleepEN = false; // disable sleep flag in loop
  //RTCFlag   = true;
}// end ISR

// ================================================================
// Sleep check, Executed every loop
// ================================================================
void sleepcheck()
{
  if (sleepEN)
  {
    Serial.println(F("Going to sleep"));
    everythingOff(); // Turn everything off
    delay(200); // wait for serial to complete printing
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
  }
}

void prep_before_sleep()
{
  Serial.println("Entering STANDBY");
  Serial.end();
  USBDevice.detach();
  interrupt_reset(); //clear interrupt registers, attach interrupts
  delay(10);
}

void prep_after_sleep()
{
  clearAlarm(); //prevent double trigger of alarm interrupt
  USBDevice.attach();
  Serial.begin(baud);
  Serial.println("WAKE");
  RTCReportTime();
  interrupt_reset();
  timer = millis();
  timerEN = false; // disable timed functions
}

void InitializeRTC()
{
  // RTC Timer settings here
  if (! RTC_DS.begin()) {
    Serial.println(F("Couldn't find RTC");
    while (1);
  }
  // This may end up causing a problem in practice - what if RTC looses power in field? Shouldn't happen with coin cell batt backup
  if (RTC_DS.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    // Set the RTC to the date & time this sketch was compiled
    RTC_DS.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  
  //RTCReportTime(); // print current RTC time
  
  // Clear any pending alarms
  clearAlarm();

  // Query Time and print
  DateTime now = RTC_DS.now();
  RTC_DS.writeSqwPinMode(DS3231_OFF);

}

// ================================================================
// Switch Pin Interrupt Service Routine
// Switch pin is LOW-True Logic, GND == enabled
// If switch off, disable timed events
// In on, enable timed events
// ================================================================
void sampleEN()
{
  Serial.println(F("Sampler EN function"));

  bool intPinState = digitalRead(INTERRUPT_PIN);
  if (!intPinState) // Low-true
  {
    Serial.println(F("Sampler timed functions enabled"));
    // Allow wake up pin from RTC to trigger pin 3 interrupt on low.
    attachInterrupt(digitalPinToInterrupt(WAKEUP_PIN), wakeUp, FALLING);
    sleepEN = true; // set sleep flag to enable sleep at end of loop
  }
  else
  {
    // Disable external pin interrupt from RTC on wake up pin.
    detachInterrupt(digitalPinToInterrupt(WAKEUP_PIN));
    Serial.println(F("Sampler timed functions disabled"));
    Serial.println(F("Processor AWAKE and standing by for serial commands"));
    sleepEN = false; //disable sleep flag
    timerEN = false; // disable timed functions flag

    // We'll just sit awake in loop listening to the serial port
  }
}

void interrupt_reset()
{
  // Clears any interrupts than may be pending on M0
  detachInterrupt(digitalPinToInterrupt(WAKEUP_PIN));
  delay(20);
  attachInterrupt(digitalPinToInterrupt(WAKEUP_PIN), wakeUp, LOW);
}

// ================================================================
// Alarm Functions
// ================================================================
void setAlarmPeriod()
{
  DateTime now = RTC.now(); // Check the current time

  // Calculate new time
  configData.sampleAlarmMinute = (now.minute() + configInst.periodicAlarmMinutes) % 60; // wrap-around using modulo every 60 sec
  configData.sampleAlarmHr  = (now.hour() + ((now.minute() + configData.periodicAlarmMinutes) / 60)) % 24; // quotient of now.min+periodMin added to now.hr, wraparound every 24hrs

  Serial.print(F("Resetting Alarm 1 for: ")); Serial.print(configData.sampleAlarmHr); Serial.print(F(":")); Serial.println(configData.sampleAlarmMinute);

  //Set alarm1
  RTC.setAlarm(ALM1_MATCH_HOURS, configData.sampleAlarmMinute, configData.sampleAlarmHr, 0);   //set your wake-up time here
  // *** Commented out *** ///
}

void clearAlarms()
{
  //clear any pending alarms
  RTC.armAlarm(1, false);
  RTC.clearAlarm(1);
  /*RTC_DS.alarmInterrupt(1, false);
  RTC_DS.armAlarm(2, false);
  RTC_DS.clearAlarm(2);
  RTC_DS.alarmInterrupt(2, false);*/
}
