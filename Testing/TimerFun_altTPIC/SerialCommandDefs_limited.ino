//----------------------------
// Serial Commands
//----------------------------
// M #, turn motor on/off and direction. #=0 for off, #=1 for draw water into sampler, #=-1 for draw water out of sampler
//    example: M 1 will begin drawing water into sampler
// O, turns everything off
// Tx, starts sequence for valve X
// VN (int)#, Sets the valve number
// Vx (int)#, Turn valve on/off where x is valve number (starting at 1, V0 is flush valve). # is 1 for on (open) and 0 for off (close)
//    example: V1 1 opens valve 1. V1 0 closes it

void listenForSerial()
{
  // listen for serial:
  if (Serial.available() > 0) {
    delay(10); // wait for full serial message to receive in RXbuffer
    char anal = Serial.read(); //read in first character
    switch (anal)
    {
      case'M': // M detection
        {
          anal = Serial.parseInt();     // then a number for motor direction, 0=off, 1=on, -1=reverse
          setPump(anal);
          break;
        }

      case 'O': //off
        {
          sequenceFlag = false;  // disable timer
          timerEN = false; // disable master sample
          Serial.println("Turning everything off.");
          setPump(0);
          closeValves();
          break;
        }

      case 'T':
        {
          int theNum = Serial.parseInt(); //read in integer
          delay(10);
          if ((theNum < NUM_VALVES) && (theNum >= 0))
          {
            // if valve number is valid, set valve number
            valveNum = theNum;
            Serial.print("Operations on valve number: "); Serial.println(valveNum);
            sequenceFlag = true;  // enable timer
            timerEN = true; // enable master sample
            programCounter = 0;
            //0_Off, 1_OpenFlush, 2_NextValve, 3_OpenValve, 4_PumpOut, 5_PumpIn,
            // flush main
            myProgram[0] = 1; myTimes[0] = 100; //open flush valve
            myProgram[1] = 5; myTimes[1] = 20000; //pump water in
            myProgram[2] = 0; myTimes[2] = 100; //turn everything off
            //flush vertical
            myProgram[3] = 4; myTimes[3] = 100; //reverse pump
            myProgram[4] = 3; myTimes[4] = 1; //open valve
            myProgram[5] = 5; myTimes[5] = 100; //forward pump to flush vertical
            myProgram[6] = 4; myTimes[6] = 5000; //reverse pump to draw from vertical
            myProgram[7] = 0; myTimes[7] = 100; //turn everything off
            //flush main
            myProgram[8] = 1; myTimes[8] = 100; //open flush valve
            myProgram[9] = 5; myTimes[9] = 20000; //pump water in - set time by amount of tubing for pump
            myProgram[10] = 0; myTimes[10] = 100; //turn everything off
            //flush vertical
            myProgram[11] = 4; myTimes[11] = 100; //reverse pump
            myProgram[12] = 3; myTimes[12] = 1; //open valve
            myProgram[13] = 5; myTimes[13] = 100; //forward pump to flush vertical
            myProgram[14] = 4; myTimes[14] = 5000; //reverse pump to draw from vertical
            myProgram[15] = 0; myTimes[15] = 100; //turn everything off
            //flush main
            myProgram[16] = 1; myTimes[16] = 100; //open flush valve
            myProgram[17] = 5; myTimes[17] = 20000; //pump water in - set time by amount of tubing for pump
            myProgram[18] = 0; myTimes[18] = 100; //turn everything off
            // clear vertical and empty bag of air
            myProgram[19] = 4; myTimes[19] = 100; //reverse pump
            myProgram[20] = 3; myTimes[20] = 5000; //open valve
            //take sample
            myProgram[21] = 5; myTimes[21] = 10000; //pump water into bottle
            myProgram[22] = 0; myTimes[22] = 100; //turn everything off
          }
          else
          {
            Serial.println(F("Invalid valve number"));
          }
          break;
        }

      case'V': // Set Valve Num
        {
          anal = Serial.read();
          if (anal == 'N')  // then next should be N
          {
            valveNum = Serial.parseInt(); // Read in and set Valve number in configuration
            Serial.print(F("Valve number set to: ")); Serial.println(valveNum);
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
                valveNum = value; // set blinkrate to the accumulated value Serial.println(blinkRate);
                value = 0; // reset val to 0 ready for the next sequence of digits
                valveState = Serial.parseInt(); // Read mode number, 0=off, 1=on
              }
            }
            if (valveNum > NUM_VALVES)
            {
              Serial.print(F("This valve input too high: ")); Serial.println(valveNum);
              Serial.print(F("Highest valve available: ")); Serial.println(NUM_VALVES);
              valveNum = NUM_VALVES;
            }
            else
            {
              Serial.print(F("Valve number set to: ")); Serial.println(valveNum);
              Serial.print(F("Valve is turned "));
              //do valve operation
              if (valveState)
              {
                Serial.println(F("on."));
                valvePrint = true;
                currValve = valveNum;
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
    } //end switch
  }
} //end function
