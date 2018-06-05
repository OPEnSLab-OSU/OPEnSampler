//----------------------------
// Serial Commands
//----------------------------
// VN (int)#, Sets the valve number
// P (int)X, starts priming sequence for valve X
// O, turns everything off
// Vx (int)#, Turn valve on/off where x is valve number (starting at 1, V0 is flush valve). # is 1 for on (open) and 0 for off (close)
//    example: V1 1 opens valve 1. V1 0 closes it
// M #, turn motor on/off and direction. #=0 for off, #=1 for draw water into sampler, #=-1 for draw water out of sampler
//    example: M 1 will begin drawing water into sampler

void listenForSerial()
{
  // listen for serial:
  if (Serial.available() > 0) {
    delay(10); // wait for full serial message to receive in RXbuffer
    char anal = Serial.read(); //read in first character
    int theNum;
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

      case 'P': //prime
        {
          theNum = Serial.parseInt(); //read in integer
          valveNum = theNum;
          Serial.print("Operations on valve number: "); Serial.println(valveNum);
          sequenceFlag = true;  // enable timer
          timerEN = true; // enable master sample
          programCounter = 0;
          myProgram[0] = 1; myTimes[0] = 100; //open flush valve
          myProgram[1] = 5; myTimes[1] = 10000; //pump water in
          myProgram[2] = 0; myTimes[2] = 100; //turn everything off
          myProgram[3] = 4; myTimes[3] = 5000; //reverse pump
          myProgram[4] = 3; myTimes[4] = 5000; //open valve
          myProgram[5] = 0; myTimes[5] = 100; //turn everything off
          myProgram[6] = 1; myTimes[6] = 100; //open flush valve
          myProgram[7] = 5; myTimes[7] = 10000; //pump water in - set time by amount of tubing for pump
          myProgram[8] = 0; myTimes[8] = 100; //turn everything off
          myProgram[9] = 4; myTimes[9] = 5000; //reverse pump
          myProgram[10] = 3; myTimes[10] = 100; //open valve
          myProgram[11] = 5; myTimes[11] = 10000; //pump water into bottle
          myProgram[12] = 0; myTimes[12] = 100; //turn everything off
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
                openValve(valveNum);
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

      case 'W': //rinse
        {
          Serial.println("Starting wash cycle");
          theNum = Serial.parseInt(); //read in integer
          delay(10);
          if ((theNum < NUM_VALVES) && (theNum >= 0))
          {
            // if valve number is valid, set valve number
            valveNum = theNum;
            Serial.print("Valve number set to: "); Serial.println(valveNum);
            programCounter = 0;
            sequenceFlag = true;  // enable timer
            timerEN = true; // enable master sample
            // main flush
            myProgram[0] = 1; myTimes[0] = 1; //open flush valve
            myProgram[1] = 5; myTimes[1] = 30000; //pump water in
            myProgram[2] = 0; myTimes[2] = 1; //turn everything off
            // draw air/water out of bag
            myProgram[3] = 3; myTimes[3] = 1; //open valve
            myProgram[4] = 4; myTimes[4] = 5000; //draw water/air out of bag/bottle for this valve
            myProgram[5] = 0; myTimes[5] = 1; //turn everything off
            // purge main line with  100 ml
            myProgram[6] = 1; myTimes[6] = 1; //open flush valve
            myProgram[7] = 5; myTimes[7] = 10000; //pump water in
            myProgram[8] = 0; myTimes[8] = 1; //turn everything off
            // put 3 ml water in valve/bag
            myProgram[9] = 3; myTimes[9] = 1; //open valve
            myProgram[10] = 5; myTimes[10] = 10; //pump water into valve/bag
            // Draw 3 ml water out of valve/bag
            myProgram[11] = 4; myTimes[11] = 5000; //pump water out of valve/bag
            myProgram[12] = 0; myTimes[12] = 1; //everything off

            Serial.print("My program sequence is this many steps: ");
            Serial.println(sizeof(myProgram) >> 1);

            Serial.print("My timer sequence is this many steps: ");
            Serial.println(sizeof(myTimes) >> 1);
          }
          else
          {
            Serial.println(F("Invalid valve number"));
          }
          break;
        }

      default:
        Serial.println(F("Invalid Command Message Received"));
        break; // ignore invalid Command messages
    } //end switch
  }
} //end function
