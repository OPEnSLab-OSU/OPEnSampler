//PumpValveFunctions.cpp

#include <Arduino.h>
#include "PumpValveFunctions.h"



#include <SPI.h>        // Using SPI hardware to communicate with TPICs

// ================================================================
// Valve Operations Functions for serial TPICS
// ================================================================
void closeValves()
{
  // Take rClock low to enable transfer
  digitalWrite(RCLOCK, LOW);
  delay(1);
  // loop through TPICs backward
  for (int i = TPIC_COUNT; i > 0; i--)
  {
    //transfer byte to TPIC i
    SPI.transfer(0);
  }
  Serial.println(F("All valves closed"));

  //indicate no valves are open
  valveOpen = false;

  digitalWrite(RCLOCK, HIGH);
  delay(1);
  digitalWrite(RCLOCK, LOW);
}

void openValve()
{
  digitalWrite(RCLOCK, LOW);

  /*generate and send output bytes - 1 per TPIC, go backwards since using cascading serial
    - TPICs are connected in series - first 8 bits go to first TPIC
    and it pushes rest of bits to next in line, etc. */
  if ((currValve <= numValves) && (currValve > 0))
  {
    //use modulo to find valve place on TPIC - will get values of 0(for multiples of 8) to 7
    valvePlace = currValve % 8;
    // determine which TPIC the currValve is on
    // assign currTPIC number to reflect physical setup
    if (valvePlace > 0)
    {
      currTPIC = currValve / 8 + 2; //plus two since flush is on first TPIC, and need to round up
    }
    // for cases of 8, 16, 24 - keep on lower TPIC, but set valve place to 8
    else
    {
      currTPIC = currValve / 8 + 1; //plus 1 since flush in on first TPIC
      valvePlace = 8; //adjust valve place from 0
    }
    if (valvePrint)
    {
      Serial.print(F("Current TPIC in physical order ")); Serial.println(currTPIC);
      delay(200);
      Serial.print(F("Valve ")); Serial.print(currValve); Serial.println(F(" turning on."));
    }
  }
  else if (currValve == 0) //flush valve
  {
    currTPIC = 1;
    valvePlace = 1;
  }

  // Send messages to TPICs
  //bitshiftleft: 00000001<< 2 = 00000100 is valve 3
  byte msg = 00000001 << valvePlace - 1; //subtract 1 since 1 % 8 = 1 etc. and first valve on TPIC needs zero shift
  // loop through all TPICs backwards
  for (int i = TPIC_COUNT; i > 0; i--)
  {
    // if on the TPIC that has the valve, send valve place as a byte
    if (i == currTPIC)
    {
      //transfer byte to TPIC i
      SPI.transfer(msg);
      // Print to serial
      if (valvePrint)
      {
        Serial.print(F("Message sent to TPIC: ")); Serial.print(msg); Serial.print(F(". Valve place: ")); Serial.println(valvePlace);
        delay(20);
        //indicate a valve is open
        valveOpen = true;
        printedOnce = true;
      }
    }
    // if not on TPIC that has valve, send 0
    else
    {
      //transfer byte to TPIC i
      SPI.transfer(0);
      if (valvePrint)
      {
        Serial.println(F("Message sent to TPIC: 0"));
        delay(20);
        printedOnce = true;
      }
    }
  } //end for loop for sending messages to TPICs

  //enable transfer
  digitalWrite(RCLOCK, HIGH);
  delay(1);
  digitalWrite(RCLOCK, LOW);
} //end openValve

// ================================================================
// Pump Operation Function
// ================================================================
// Set Motor State, On(1), Off(0), Reverse(-1)
void setPump(pumpState state)
{
  // SAFTEY FEATURE HERE, DO NOT TURN MOTOR ON FORWARD IF ALL VALVES ARE CLOSED
  switch (state)
  {
    case pumpState::ON:  // Draw inwards
      {
        if (valveOpen)
        {
          Serial.println(F("Motor Turned on, Draw"));
          digitalWrite(PUMP_PIN1, HIGH); // Set Pump pin high
          digitalWrite(PUMP_PIN2, LOW); // Set Pump pin low
        }
        else
          Serial.println(F("Pump draw can't enable; turn a valve on first"));
        break;
      }
    case pumpState::REVERSE: // Draw outwards
      {
        Serial.println(F("Motor Turned on, Reverse"));
        digitalWrite(PUMP_PIN1, LOW); // Set Pump pin low
        digitalWrite(PUMP_PIN2, HIGH); // Set Pump pin high
        break;
      }
    case pumpState::OFF:
      {
        Serial.println(F("Motor Turned off"));
        digitalWrite(PUMP_PIN1, LOW); // Set Pump pin low
        digitalWrite(PUMP_PIN2, LOW); // Set Pump pin low
        break;
      }
    default:
      Serial.println(F("Invalid pump command received!"));
  } //end switch
} //end setPump

/**
   Helper for mapping integers to the pumpState enum
*/
pumpState getPumpState(int n)
{
  switch (n) {
    case 0:
      return pumpState::OFF;
    case 1:
      return pumpState::ON;
    case -1:
      return pumpState::REVERSE;
  }
}

// Turns off pump and valves
void everythingOff()
{
  // Reset pump and all valves to off
  setPump(false); // turn off pump
  closeValves(); // turn off all valves
}
