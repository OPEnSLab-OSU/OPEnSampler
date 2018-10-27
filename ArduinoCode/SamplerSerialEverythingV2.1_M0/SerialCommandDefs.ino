/*
  Water Sampler Serial Command Set
  Author: Chet Udell
  Mar 23, 2017

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
    Turns everything off

  You may use Arduino IDE, Python, or other software to send timed sequences to sampler.
    Example: V0 1 followed by M 1  will begin flush. The sequence (press return where you see / ) V0 0 / V1 1 / M 1  will disable flush and draw water into bag 1.
    You could also write a script in something like Python for drawing water out of the sampler sequentially into your analysis machine without removing bags!
*/

void listenForSerial()
{
  // listen for serial:
  if (Serial.available() <= 0)
    return;

  delay(10); // wait for full serial message to receive in RXbuffer
  // Detach alarm interrupt
  RTC.alarmInterrupt(1, false);

  char anal = Serial.read();
  Serial.println(anal);
  switch (anal)
  {
    // Set RTC time
    case'C':
      {
        anal = Serial.read();
        if (anal == 'L')  // then next should be L
        { // Change Sample Duration
          Serial.println(F("at CL"));
          anal = Serial.read();
          if (anal == 'K')  // then next should be K
          { // Update clock using computer time:
            int d = Serial.parseInt(); // Set alarm day
            int m = Serial.parseInt(); // Set alarm month
            int y = Serial.parseInt(); // Set alarm year
            int hrz = Serial.parseInt(); // Set alarm hr
            int minz = Serial.parseInt(); // Set alarm min

            setClock(y, m, d, hrz, minz);
          } // end 'K' detect
        } // end 'CL' detect
      }
      break;

    /**
       FD <milliseconds>
       Sets flush duration in milliseconds.

       FD
       Prints the flush duration (in milliseconds) to Serial.
    */
    case'F':
      {
        anal = Serial.read();
        if (anal == 'D')  // then next should be D
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
      }


    case'M': // M detection
      {
        setPump(getPumpState(Serial.parseInt()));
        break;
      } //end case M

    case 'O': //off
      {
        sequenceFlag = false;  // disable timer
        timerEN = false; // disable master sample
        Serial.println("Turning everything off.");
        setPump(0);
        closeValves();
        break;
      }

    case 'P':
      int index;
      chr = Serial.read();

      /**
         PR <index>

         Read a status update recipient's phone number located at index.
         Example: "PR 23" prints the 23rd phone number.
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
         PW <index> <phone number>

         Write a status update recipient's phone number located at index.
         Example: "PW 2 123456890" overwrites the 2nd phone number.
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
       RST

       Reset the stored configuration to default values.
    */
    case'R': // RST detection
      {
        anal = Serial.read();
        if (anal == 'S')  // then next should be S
        { // Change Sample Duration
          anal = Serial.read();
          if (anal == 'T')  // then next should be T
          { // Change Sample Duration
            writeNonVolatileDefaults(); // Restore OPEnS Lab Factory Defaults
          } // end 'T' detect
          break;
        } // end 'RS' detect
      } //end case R

    case'S':  // Set Sample Alarm "SA 9 0" will set daily samples at 9:00AM
      {
        anal = Serial.read();
        if (anal == 'A')  // then next should be A
        {
          anal = Serial.read();
          /**
            SAD <hour> <minute>

            Set daily sample alarm to sample once per day at the specified time,
            and switch to daily sampling mode. Uses 24-hour format.
            Example: "SAD 9 30" sets sample alarm to 9:30AM daily.
            Example: "SAD 16 22" sets sample alarm to 4:22PM daily.
          */
          if (anal == 'D')  // then next should be D
          {
            unsigned int hour   = Serial.parseInt();
            unsigned int minute = Serial.parseInt();

            config.setDailyAlarm(hour, minute);
            break;
          }  //end SAD layer
          /**
            SAP <minutes>

            Set periodic alarm to sample at fixed intervals (in minutes),
            starting from the current time.
            Example: SAP 30 sets sample alarm to go off every 30 minutes.

            SAP

            Prints the periodic sample alarm length (in minutes) to serial.
          */
          if (anal == 'P')  // then next should be P
          {
            Serial.println(F("Setting Sample Alarm to Periodic Mode, take sample every X min: "));
            int minutes = Serial.parseInt();

            if (minutes > 0) {
              config.setPeriodicAlarm(minutes);
            }
            else {
              Serial.print(F("Periodic sample alarm length (in minutes) is: "));
              Serial.println(config.getPeriodicAlarmLength());
            }

            break;
          }  //end SAP layer

        }
        /**
          SD <milliseconds>

          Set the sample duration, or the time in milliseconds that the pump runs
          when taking a single sample

          Prints the sample duration (in milliseconds) to Serial.
        */
        else if (anal == 'D')  // then next should be D
        { // Change Sample Duration in milliseconds
          int milliseconds = Serial.parseInt();

          if (milliseconds > 0) {
            config.setSampleDuration(milliseconds);
          }
          else {
            Serial.print(F("Sample duration length (in milliseconds) is: "));
            Serial.println(config.getSampleDuration());
          }
          break;
        } // End SD anal
      } //end case S

    case'V': // Set Valve Num
      {
        anal = Serial.read();
        /**
          "VN <valve number>"

          Set the next valve/bag to sample into.
          Example: "VN 2" sets valve #2 as the next sample destination.
        */
        if (anal == 'N')  // then next should be N
        {
          int valveNumber = Serial.parseInt(); // Read in and set Valve number in configuration
          if (valveNumber <= 0)
          {
            Serial.println(F("ERROR: Next valve number must be a positive value."));
            break;
          }
          else
          {
            config.setValveNumber(valveNumber);
            Serial.print(F("Valve number set to: ")); Serial.println(configClass.valveNumber);
          }
          break;
        } // end 'N' detect
        if (isDigit(anal))  // else is this a digit? e.g. V#
        {
          Serial.println(F("Entering Puppet Mode"));
          int valveState = 0; // Read mode number, 0=off, 1=on
          value = (anal - 48); // yes, accumulate the value
          while ( Serial.available())
          {
            anal = Serial.read();
            if ( isDigit(anal) ) // is this an ascii digit between 0 and 9?
            {
              value = (value * 10) + (anal - 48); // yes, accumulate the value
            }
            else if (anal == 32) // is the character a [space]?
            {
              configClass.valveNumber = value; // set blinkrate to the accumulated value Serial.println(blinkRate);
              value = 0; // reset val to 0 ready for the next sequence of digits
              valveState = Serial.parseInt(); // Read mode number, 0=off, 1=on
            }
          }
          if (configClass.valveNumber > numValves)
          {
            Serial.print(F("This valve input too high: ")); Serial.println(configClass.valveNumber);
            Serial.print(F("Highest valve available: ")); Serial.println(numValves);
            configClass.valveNumber = numValves;
          }
          else
          {
            Serial.print(F("Valve number set to: ")); Serial.println(configClass.valveNumber);
            Serial.print(F("Valve is turned "));
            //do valve operation
            if (valveState)
            {
              Serial.println(F("on."));
              valvePrint = true;
              currValve = configClass.valveNumber;
              openValve();
            }
            else
            {
              Serial.println(F("off"));
              closeValves();
            }
          } // end else for valid valve num entry
          break;
        } // end 'N' detect
      } //end case V

    default:
      Serial.println(F("Invalid Command Message Received"));
      break; // ignore invalid Command messages

  } // End Switch Case Function

  // Save Sample configuration into flash for next power-up, *IF* settings not changed in puppet mode
  config.write_non_volatile();
} // End serial available state-machine
}
