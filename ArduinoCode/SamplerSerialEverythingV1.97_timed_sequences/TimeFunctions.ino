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

// ================================================================
// Timer ISR, Execute every sample period
// ================================================================
void wakeUp()
{
  // Disable external pin interrupt on wake up pin.
  timerEN = true; // Enable timed functions
  sleepEN = false; // dissable sleep flag in loop
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

// ================================================================
// Alarm Functions
// ================================================================
void setAlarmPeriod()
{
  DateTime now = RTC.now(); // Check the current time

  // Calculate new time
  configuration.SAMin = (now.minute() + configuration.SAPer) % 60; // wrap-around using modulo every 60 sec
  configuration.SAHr  = (now.hour() + ((now.minute() + configuration.SAPer) / 60)) % 24; // quotient of now.min+periodMin added to now.hr, wraparound every 24hrs

  Serial.print(F("Resetting Alarm 1 for: ")); Serial.print(configuration.SAHr); Serial.print(F(":")); Serial.println(configuration.SAMin);

  //Set alarm1
  RTC.setAlarm(ALM1_MATCH_HOURS, configuration.SAMin, configuration.SAHr, 0);   //set your wake-up time here
  // *** Commented out *** ///
}

void clearAlarms()
{
  //clear any pending alarms
  RTC.armAlarm(1, false);
  RTC.clearAlarm(1);
}
