//----------------------------
// Serial Commands
//----------------------------
// VN (int)X, Sets the next valve/bag to place sample. Sampler saves curent valve number during operation in EEPROM in case of power failure, it picks up where it left off
//             This ensures you can reset the valve count before each new deployment, or manually skip to next available bag should the sampler malfunction
// P, starts priming sequence
// W (int)X, starts rinsing sequence for valve X
// O, turns everything off

void listenForSerial()
{
  // listen for serial:
  if (Serial.available() > 0) {
    delay(10); // wait for full serial message to receive in RXbuffer
    char anal = Serial.read(); //read in first character
    int theNum;
    switch (anal)
    {
      case'V': // Set Valve Num
        {
          anal = Serial.read(); //get next character
          if (anal == 'N')
          {
            anal = Serial.parseInt(); // Read in integer
            delay(10);
            if ((anal < numValves) && (anal >= 0))
            {
              // if valve number is valid, set valve number
              valveNum = anal;
              Serial.print(F("Valve number set to: ")); Serial.println(valveNum);
              delay(10);
            }
            else
            {
              Serial.println(F("Invalid valve number"));
            }
          } // end 'N' detect
          else
          {
            Serial.println(F("Invalid entry for second character"));
          }
        }
        break;

      case 'P': //prime
        {
          theNum = Serial.parseInt(); //read in integer
          valveNum = theNum;
          sequenceFlag = true;  // enable timer
          timerEN = true; // enable master sample
          programCounter = 0;
          myProgram[0] = 3; myTimes[0] = 1; //open bag valve
          myProgram[1] = 5; myTimes[1] = 200; //pump water out
          myProgram[2] = 0; myTimes[2] = 1; //turn everything off
        }
        break;

      case 'W': //rinse
        {
          Serial.println("Starting wash cycle");
          theNum = Serial.parseInt(); //read in integer
          delay(10);
          if ((theNum < numValves) && (theNum >= 0))
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

            Serial.print("My program sequence is this many steps: ");
            Serial.println(sizeof(myTimes) >> 1);
          }
          else
          {
            Serial.println(F("Invalid valve number"));
          }
        }
        break;

      case 'O': //off
        {
          sequenceFlag = false;  // disable timer
          timerEN = false; // disable master sample
          everythingOff;
        }
        break;

      default:
        Serial.println(F("Invalid Command Message Received"));
        break; // ignore invalid Command messages
    } //end switch
  }
} //end function
