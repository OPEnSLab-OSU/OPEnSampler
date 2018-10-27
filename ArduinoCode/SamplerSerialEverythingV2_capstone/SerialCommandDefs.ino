/*
  Water Sampler Serial Command Set
  Authors: Chet Udell

  This file contains the input parsing for the OPEnSampler's serial command set.
  These commands are used to update the OPEnSampler's configuration

  Commands (make sure to include a spaces between arguments):

  CLK <day> <month> <year> <hour> <minute>
    Set current RTC time and reset sample alarms.
  FD
    Prints the flush duration (in milliseconds) to Serial.
  FD <milliseconds>
    Set flush duration period in ms. Should be about 20sec, but will change depending on tubing length.
  PR <index>
    Print the phone number located at index. Index must be < numSMSRecipients.
  PW <index> <phone number>
    Write a phone number to location index. Index must be < numSMSRecipients, and phone number may be up to phoneNumberLength long.
  RST
    Reset all configuration values to defaults, overwriting previous settings.
  SAD <hour> <minute>
    Set daily sample alarm to take samples daily at the specified time, and switch to daily sampling mode.
    Uses 24-hour format.
    Example: "SAD 9 30" sets sample alarm to 9:30AM daily.
    Example: "SAD 16 22" sets sample alarm to 4:22PM daily.
  SAP
    Prints the periodic sample alarm length (in minutes) to serial.
  SAP <minutes>
    Set periodic sample alarm to sample at fixed intervals (in minutes), starting from the current time.
    Example: SAP 30 sets sample alarm to go off every 30 minutes.
  SD
    Prints the sample duration (in milliseconds) to Serial.
  SD <milliseconds>
    Set sample duration time, or how long pumps run water into each bag (in milliseconds)
  VN <valve number>
    Set the next valve/bag to sample into. This value is stored persistently, so operation can resume after power failure.
    This can be used to reset the valve count before each new deployment, or manually skip to a bag.

  Puppet-String Commands:

  VS <valve number> <valve state>
    Turn valve on/off (starting at 1, V0 is flush valve). # is 1 for on (open) and 0 for off (close)
    example: V1 1 opens valve 1. V1 0 closes it
  M <direction value>
    Turn motor on (in forward or reverse) and off. <direction value> is 0 for off, 1 to draw water into sampler, -1 to draw water out of sampler
    example: M 1 will begin drawing water into sampler
  O
    turns everything off

  You may use Arduino IDE, Python, or other software to send timed sequences to sampler.
    Example: V0 1 followed by M 1  will begin flush. The sequence (press return where you see / ) V0 0 / V1 1 / M 1  will disable flush and draw water into bag 1.
    You could also write a script in something like Python for drawing water out of the sampler sequentially into your analysis machine without removing bags!
 */

void listenForSerial()
{
  if (Serial.available() <= 0)
    return;

  delay(10); // wait for full serial message to receive in RXbuffer
  // Disable external pin interrupt on wake up pin.
  //***detachInterrupt(digitalPinToInterrupt(wakeUpPin));
  // Detach alarm interrupt
  RTC.alarmInterrupt(1, false);
  //MsTimer2::stop(); // disable timer interrupt!

  char chr = Serial.read();
  Serial.println(chr);
  switch(chr)
  {
    case 'C': // Set and echo back RTC time, update alarm time
      chr = Serial.read();
      if (chr == 'L')
      {
        Serial.println(F("at CL"));
        chr = Serial.read();
        if (chr == 'K')
        {
          int d = Serial.parseInt();
          int m = Serial.parseInt();
          int y = Serial.parseInt();
          int hrz = Serial.parseInt();
          int minz = Serial.parseInt();

          setClock(y, m, d, hrz, minz);
        }
      }
      break;

    /**
     * FD <milliseconds>
     *
     * Sets flush duration in milliseconds.
     *
     * FD
     *
     * Prints the flush duration (in milliseconds) to Serial.
     */
    case 'F':
      chr = Serial.read();
      if (chr == 'D')
      {
        int milliseconds = Serial.parseInt();

        if (milliseconds > 0) {
          config.setFlushDuration(milliseconds);
        }
        else {
          Serial.print(F("Flush duration (in milliseconds) is: "));
          Serial.println(config.getFlushDuration());
        }
        break;
      }

    case 'M':
      //     timerEN = false; // First, disable timed functions, puppet mode
      // Disable external pin interrupt from RTC on wake up pin.
      //***detachInterrupt(digitalPinToInterrupt(wakeUpPin));

      setPump(getPumpState(Serial.parseInt()));
      break;

    case 'P':
      int index;
      chr = Serial.read();

      /**
       * PR <index>
       *
       * Read a status update recipient's phone number located at index.
       * Example: "PR 23" prints the 23rd phone number.
       */
      if (chr == 'R') {
        index = Serial.parseInt();
        const char * str = config.getSMSNumber(index);

        if (str)
          Serial.println(str);
        else
          Serial.println(F("There is no status update recipient defined at that index."));
      }

      /**
       * PW <index> <phone number>
       *
       * Write a status update recipient's phone number located at index.
       * Example: "PW 2 123456890" overwrites the 2nd phone number.
       */
      else if (chr == 'W')
      {
        char buffer[phoneNumberLength + 2]; // + 2 for 1 space & 1 null terminator
        int read;

        index = Serial.parseInt();
        read = Serial.readBytes(buffer, phoneNumberLength + 1);
        buffer[read] = '\0';

        config.setSMSNumber(index, buffer + 1); // + 1 to ignore the space

        Serial.print(F("Status update recipient #"));
        Serial.print(index);
        Serial.print(F(" set to phone number "));
        Serial.print(buffer + 1);
      }
      break;

    /**
     * RST
     *
     * Reset the stored configuration to default values.
     */
    case 'R':
      chr = Serial.read();
      if (chr == 'S')
      {
        chr = Serial.read();
        if (chr == 'T')
        {
          writeEEPROMDefaults();
        }
        break;
      }

    case 'S':
      chr = Serial.read();
      if (chr == 'A')
      {
        chr = Serial.read();
        /**
         * SAD <hour> <minute>
         *
         * Set daily sample alarm to sample once per day at the specified time,
         * and switch to daily sampling mode. Uses 24-hour format.
         * Example: "SAD 9 30" sets sample alarm to 9:30AM daily.
         * Example: "SAD 16 22" sets sample alarm to 4:22PM daily.
         */
        if (chr == 'D')
        {
          unsigned int hour   = Serial.parseInt();
          unsigned int minute = Serial.parseInt();

          config.setDailyAlarm(hour, minute);
          break;
        }
        /**
         * SAP <minutes>
         *
         * Set periodic alarm to sample at fixed intervals (in minutes),
         * starting from the current time.
         * Example: SAP 30 sets sample alarm to go off every 30 minutes.

         * SAP
         *
         * Prints the periodic sample alarm length (in minutes) to serial.
         */
        if (chr == 'P')
        {
          int minutes = Serial.parseInt();

          if (minutes > 0) {
            config.setPeriodicAlarm(minutes);
          }
          else {
            Serial.print(F("Periodic sample alarm length (in minutes) is: "));
            Serial.println(config.getPeriodicAlarmLength());
          }

          break;
        }
      }
      /**
       * SD <milliseconds>
       *
       * Set the sample duration, or the time in milliseconds that the pump runs
       * when taking a single sample
       *
       * Prints the sample duration (in milliseconds) to Serial.
       */
      else if (chr == 'D')
      {
        int milliseconds = Serial.parseInt();

        if (milliseconds > 0) {
          config.setSampleDuration(milliseconds);
        }
        else {
          Serial.print(F("Sample duration length (in milliseconds) is: "));
          Serial.println(config.getSampleDuration());
        }


        break;
      }

    case 'V':
      chr = Serial.read();
      /**
       * "VN <valve number>"
       *
       * Set the next valve/bag to sample into.
       * Example: "VN 2" sets valve #2 as the next sample destination.
       */
      if (chr == 'N')
      {
        int valveNumber = Serial.parseInt();

        if (valveNumber <= 0)
        {
          Serial.println(F("ERROR: Next valve number must be a positive value."));
          break;
        }
        else
        {
          Serial.print(F("Valve number set to: ")); Serial.println(valveNumber);
        }

        // The valve number is decremented before storing because it will be
        // incremented automatically in the main loop.
        config.setValveNumber(valveNumber - 1);
        break;
      }
      /**
       * "VS <valve number> <valve state>"
       *
       * Set the specified valve to be open or closed.
       * A valve state of 1 is open and a state of 0 is closed.
       *
       * Example: "VS 2 1" will open valve #2.
       */
      if (chr == 'S')
      {
        //      timerEN = false; // First, disable timed functions, puppet mode
        // Disable external pin interrupt from RTC on wake up pin.
        //***detachInterrupt(digitalPinToInterrupt(wakeUpPin));

        int valveNumber = Serial.parseInt();
        int valveState = Serial.parseInt();

        puppetValveState(valveNumber, valveState);

        break;
      }

    default:
      Serial.println(F("ERROR: Invalid command received."));
      break;

  }

  // Save Sample configuration into EEPROM for next power-up, *IF* settings not changed in puppet mode
  config.writeToEEPROM();
  // If nothing has disabled timer, or if timer has been resumed, resume sampler period
  // Allow wake up pin to trigger interrupt on low.
  //*** attachInterrupt(digitalPinToInterrupt(wakeUpPin), wakeUp, FALLING);
}
