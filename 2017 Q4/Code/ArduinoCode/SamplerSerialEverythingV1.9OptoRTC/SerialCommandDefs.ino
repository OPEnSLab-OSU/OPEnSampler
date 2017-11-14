/*
  Water Sampler Serial Command Set
  Author: Chet Udell
  Mar 23, 2017

  Objective, create serial command set to send from a computer program to change sampler behaviour 

  Command Defs (make sure to include a [space] between the characters and int:
  CLK (int)d (int)m (int)y (int)hr (int)min, Sets the current RTC time, D:M:Y:Hr:Min - and resets sample alarm
  SAD (int)X (int)X, sets "Daily Sample Alarm" Hr:Mn to take samples daily at time. Uses 24hr format.
    ex: SAD 9 30 sets sample alarm to 9:30AM daily. 
    ex: SAD 16 22 sets sample alarm to 4:22PM daily.
    Also sets Is_Daily flag for initialization if power-down restart, clears Is_Hourly flag
  SAP (int)X, sets "Periodic Sample Alarm" to take samples at specified period duration in Min.
    ex: SAH 30 sets sample alarm to go off every 30min. 
    ex: SAH 47 sets sample alarm to go off 47min.
  FD (int)X, sets "Flush duration" period in ms, should be about 20sec, but will change with the length of tubing you use to get from sampler to water source
  SD (int)X, sets "Sample Duration" time that pumps run water into each bag in milliseconds
  ** removed for this version : SV (int)X, sets "Sample Volume" in ml, a transform of Sample Duration, may not be 100% accurate, 2min per 250ml,
  VN (int)X, Sets the next valve/bag to place sample. Sampler saves curent valve number during operation in EEPROM in case of power failure, it picks up where it left off
             This ensures you canreset the valve count before each new deployment, or manually skip to next available bag should the sampler malfunction
  RST, Full system "factory" reset - set default sample period, sample duration, reset vlve counter, writes defaults to EEPROM (overwirghting previous settings)
  Pupet-String Commands:
  WARNING, receiving any of these will disable Arduino P3 Interrupt, RTC wakeup alarm pin. Typing RES or enabling the P2 TimerEn switch will also re-enable the P3 interrupt
  Vx (int)#, Turn valve on/off where x is valve number (starting at 1, V0 is flush valve). # is 1 for on (open) and 0 for off (close)
    example: V1 1 opens valve 1. V1 0 closes it
  M #, turn motor on/off and direction. #=0 for off, #=1 for draw water into sampler, #=-1 for draw water out of sampler
    example: M 1 will begin drawing water into sampler
  From here, you may use Arduino IDE, Python, or other software to send timed sequences to sampler.
    example: V0 1 followed by M 1  will begin flush.  The sequence (press return where you see / ) V0 0 / V1 1 / M 1  will dissable flush and draw water into bag1
    You could also make a "macro" or timed program in something like Python for drawing water out of the sampler sequentially into your analysis machine without removing bags!

 */

 // To Do, dissable SQW RTC pin interrupt!!! then re-enable

 void listenForSerial()
{
  // listen for serial:
  if (Serial.available() > 0) {
    delay(10); // wait for full serial message to receive in RXbuffer
    // Disable external pin interrupt on wake up pin.
    //***detachInterrupt(digitalPinToInterrupt(wakeUpPin));
    // Detach alarm interrupt
    RTC.alarmInterrupt(1, false); 
    //MsTimer2::stop(); // dissable timer interrupt!
    char anal = Serial.read(); 
Serial.println(anal); 
  switch(anal)
  {
    case'C': // Set and echo back RTC time, update alarm time
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
        RTC.adjust(DateTime(y, m, d, hrz, minz, 0));
        
        RTCReportTime(); // print current RTC time
        
        if (configuration.Is_Daily) // check which alarm configuration to set 
          RTC.setAlarm(ALM1_MATCH_HOURS, configuration.SAMin, configuration.SAHr, 0);  // Set alarm1 every day at hr:mn
        else
          setAlarmPeriod(); // else set alarm based on period
          
        Serial.print(F("Next Sample Alarm set for: ")); Serial.print(configuration.SAHr); Serial.print(F(":"));Serial.println(configuration.SAMin);
  
        } // end 'K' detect
    break;
    } // end 'CL' detect
    
    case'S':  // Set Sample Alarm "SA 9 0" will set daily samples at 9:00AM  
    anal = Serial.read();
    if (anal == 'A')  // then next should be A
    {
      anal = Serial.read();
      if (anal == 'D')  // then next should be D
      {
        Serial.println(F("Setting Sample Alarm to Daily Mode: "));
        configuration.SAHr = Serial.parseInt(); // Set alarm Hr
        configuration.SAMin = Serial.parseInt(); // Set alarm Min
        Serial.print(configuration.SAHr); Serial.print(F(":")); Serial.println(configuration.SAMin);
        configuration.Is_Daily = 1; // set daily flag in configuration
//        configuration.Is_Hourly = 0; // clear hourly flag in configuration
        //Set alarm1 every day at hr:mn
        RTC.setAlarm(ALM1_MATCH_HOURS, configuration.SAMin, configuration.SAHr, 0);   //set your wake-up time here
        //*** Commented out***// 
        break;
      }  //end SAD layer
      if (anal == 'P')  // then next should be P
      {
        Serial.println(F("Setting Sample Alarm to Periodic Mode, take sample every X min: "));
        configuration.SAPer = Serial.parseInt(); // Set alarm Min
        Serial.println(configuration.SAPer);
        if((configuration.SAPer*60000) > (configuration.FDMs+configuration.SDMs))
        {
          configuration.Is_Daily = 0; // clear daily flag in configuration to set periodic mode
          setAlarmPeriod();  // Reset Alarm to new hr and min based on current time and received period in Min
        }
        else
        {
          Serial.print(F("Error, sample period of ")); Serial.print(configuration.SAPer); Serial.println(F(" is less than set FLush and Sample time required"));
        }
        break;
      }  //end SAP layer
      
    }
    // Still if "S" is first received char...
    else if (anal == 'D')  // then next should be D
    { // Change Sample Duration in milliseconds
      //Serial.println(F("at SD"));
      configuration.SDMs = Serial.parseInt();     // then an number for sample duration ms
      Serial.print(F("Sample duration in ms set to ")); Serial.println(configuration.SDMs);
      break; 
    } // End SD anal
    
    /*
// Still if "S" is first received char...
    else if (anal == 'V')  // then next should be V
    { // Change Sample Volume, calculate new sample duration @480ms per ml
      //Serial.println(F("at SV"));
      unsigned long SVmlTest = Serial.parseInt();     // then an number for sample volume,
      Serial.print(F("Requested sample volume: ")); Serial.print(SVmlTest); Serial.print(F("ml. "));
      SVmlTest = SVmlTest*480;    // Convert ml to ms, About 480ms to pump 1 ml.
      configuration.SVml = SVmlTest;
      configuration.SDMs = SVmlTest; // set duration based on this value
      Serial.print(F("Sample duration set to ")); Serial.println(configuration.SDMs);
      break; 
    } // End SV anal
    */
    
  case'F':
    anal = Serial.read();
    if (anal == 'D')  // then next should be D
    { // Change Sample Duration
      //Serial.println(F("at FD"));
      configuration.FDMs = Serial.parseInt();     // then an number for sample duration ms
      Serial.print(F("Flush duration set to ")); Serial.println(configuration.FDMs);
      break; 
    } // End FD anal

  case'V': // Set Valve Num
    anal = Serial.read();
    if (anal == 'N')  // then next should be N
    { // Change Sample Duration
      //Serial.println(F("at VN"));
      configuration.VNum = Serial.parseInt() - 1; // Read in and set Valve number in configuration
                                                  // minus 1 because Valve num increments before turning on, so setting to 1 would have been incrimented to 2 otherwise, see sample sequence in Loop above for details
      Serial.print(F("Valve number set to: ")); Serial.println(configuration.VNum+1);
    break;
    } // end 'N' detect
    if (isDigit(anal))  // else is this a digit? e.g. V#
    { // Change Sample Duration
//      timerEN = false; // First, dissable timed functions, puppet mode
      // Disable external pin interrupt from RTC on wake up pin.
      //***detachInterrupt(digitalPinToInterrupt(wakeUpPin));
      Serial.println(F("Entering Puppet Mode"));
//      configuration.VNum = anal-48; // Set target valve number, -48 because ASCII (48=0, 49=1, 50=2, etc)
      int valveState = 0; // Read mode number, 0=off, 1=on

      value = (anal - 48); // yes, accumulate the value
    while( Serial.available())
    {  
      anal = Serial.read();
      if( isDigit(anal) )// is this an ascii digit between 0 and 9? 
      {
         value = (value * 10) + (anal - 48); // yes, accumulate the value
       }
      else if (anal == 32) // is the character a [space]?
      {
        configuration.VNum = value; // set blinkrate to the accumulated value Serial.println(blinkRate);
        value = 0; // reset val to 0 ready for the next sequence of digits
        valveState = Serial.parseInt(); // Read mode number, 0=off, 1=on
      }
    }
    if(configuration.VNum > numValves)
    {
        Serial.print(F("This valve input too high: ")); Serial.println(configuration.VNum);
        Serial.print(F("Highest valve available: ")); Serial.println(numValves);
        configuration.VNum = numValves;
    }
    else
    { 
      Serial.print(F("Valve number set to: ")); Serial.println(configuration.VNum);
      Serial.print(F("Valve is turned ")); 

      // check if Valve 0, Flush on
      if(configuration.VNum == 0)
      {
        if(valveState)  // then turn flush on or off depending on valveState
        {
          flushON();  // Flush bit on, Preserves valve bits
          Serial.println(F("on"));
        }
        else
        {
          flushOFF();
          Serial.println(F("off"));
        }
      }
      
      //else 
      else if(valveState)
      {
        Serial.println(F("on"));
        // Configure TPIC buffers according to current valve, strobe out to SPI
      setValveBits(); // Assumes you only want one valve on at a time, auto closes other valves
      }
      else
      {
        Serial.println(F("off"));
        clearValveBits(); // CLEAR valve bits, preserve flush bit
      } 
    } // end else for valid valve num entry  
    break;
    } // end 'N' detect
  
  case'R': // RST detection
    anal = Serial.read();
    if (anal == 'S')  // then next should be S
    { // Change Sample Duration
//      Serial.println(F("at RS"));
      anal = Serial.read();
      if (anal == 'T')  // then next should be T
      { // Change Sample Duration
//        Serial.println(F("at RST"));
        writeEEPROMdefaults(); // Restore OPEnS Lab Factory Defaults
      } // end 'T' detect
    break;
    } // end 'RS' detect
  
   case'M': // M detection
//     timerEN = false; // First, dissable timed functions, puppet mode
     // Disable external pin interrupt from RTC on wake up pin.
      //***detachInterrupt(digitalPinToInterrupt(wakeUpPin));
     directionMotor = Serial.parseInt();     // then an number for motor duration, 0=off, 1=on, -1=reverse
     switch(directionMotor)
     {
       case 1: // set motor direction to normal, draw water into sampler
         setPump(1);
       break;

       case -1:
         setPump(-1);
       break;

       case 0:
         setPump(0);
       break;

       default:
         Serial.println(F("Invalad Motor Direction Message Received"));
       break;
     }
    break;

   
  default:
  Serial.println(F("Invalad Command Message Received"));
    break; // ignore invalid Command messages
    
   } // End Switch Case Function

  // Save Sample configuration into EEPROM for next power-up, *IF* settings not changed in puppet mode
    configuration.written = eepromValidationValue; //Set EEPROM Confirmation value so we know this has been written before
    EEPROM_writeAnything(0, configuration);
    // If nothing has dissabled timer, or if timer has been resumed, resume sampler period
    // Allow wake up pin to trigger interrupt on low.
    //*** attachInterrupt(digitalPinToInterrupt(wakeUpPin), wakeUp, FALLING);
 } // End serial available state-machine
}

void setAlarmPeriod()
{
  DateTime now = RTC.now(); // Check the current time

  // Calculate new time
  configuration.SAMin = (now.minute()+configuration.SAPer)%60;  // wrap-around using modulo every 60 sec
  configuration.SAHr  = (now.hour()+((now.minute()+configuration.SAPer)/60))%24; // quotient of now.min+periodMin added to now.hr, wraparound every 24hrs

  Serial.print(F("Resetting Alarm 1 for: ")); Serial.print(configuration.SAHr); Serial.print(F(":"));Serial.println(configuration.SAMin);
  

   //Set alarm1
   RTC.setAlarm(ALM1_MATCH_HOURS, configuration.SAMin, configuration.SAHr, 0);   //set your wake-up time here
   // *** Commented out *** /// 
}

