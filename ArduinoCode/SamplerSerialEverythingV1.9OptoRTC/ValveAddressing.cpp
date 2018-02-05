/*
 Water Sampler Addressing
  Author: Chet Udell
  Jan 26, 2017
  Issue, Pump uses TPIC power shift register to control valves.
  8 Ports on each TPIC
  Port 1 dedicated to Flush Valve, + 24 additional valves require 4 TPICS total
  (MSB) V24...V17 | V16...V9 | V8...V1 | xxxx xxx F (LSB)

  Need 4 byte TPIC control array for each sampler
  uint8_t TPICBuffer[4]; // keep in mind, arrays LSB and MSB are ordered opposite from how we'd spell them out
  = {0x01, 0x00, 0x00, 0X01} // Flush.
  = {0x00, 0x01, 0x01, 0X00} // Valve 1 on (flush off)
  = {0x00, 0x00, 0x80, 0X00} // Valve 8 on
  = {0x00, 0x01, 0x00, 0X00} // Valve 9 on
  etc
  = {0x80, 0x00, 0x00, 0X00} // Valve 24 on

  Trick here is doing some shifting to ensure Valve 25 on a modular connected sampler becomes
    uint8_t TPICBuffer[8];
  0x01 0x00, | 0x00, 0x00, 0X00, 0x00
  instead of continuing the bit shift sequence to be
        0x01 | 0x00, 0x00, 0X00, 0x00
  This is because the first byte in each sampler's TPIC buffer should be flush bit only.

  C = 0; // valve counter
  C++; // Increment
  Possible Solution, (int) C / 24 = module# where 0 is base, 1 is expansion 1 etc
            C % 24 = valve#;
            valve# = valve# << 8; // because LSB is flush valve
            valve# = valve# << (module# * 24); // shift up to expansion number if needed
  IF TPIC valve control array is in-fact uint8_t data type,
  then you should be able to send one byte of data at a time
  in sequence out of the array like:
  SPI.write(TPICBuffer[3]); // because MSB in Array is actually LSB in hardware
  SPI.write(TPICBuffer[2]);
  etc...
  Use a for loop of course
*/
#include <Arduino.h>
#include "Configuration.h"
#include "Defaults.h"
#include "ValveAddressing.h"

extern Configuration config;
extern unsigned char TPICBuffer[4];

extern void strobeTPICs();

// ================================================================
// Valve Addressing Functions for serial TPICS
// ================================================================

// Turns off pump and valves
void everythingOff()
{
  // Reset pump and all valves to off
    setPump(pumpState::OFF);
    clearValveBits();  // Clear Valve Control bits, don't override Flush bit
    flushOFF(); // Turns flush off and also strobes TPICs over SPI
}

// Turn flush valve of TPIC3 on, preserve state of other bits
void flushON()
{
  // Add some code here to accomidate ModuleNum bit shifting

  TPICBuffer[3] |= 0x01;   // TPICBuffer[3] is LSB
                               // Logical OR so as not to blow out other bits
  Serial.println(F("Flushing System . . ."));

  // Strobe TPIC buffer data to TPICS via SPI
  strobeTPICs();
}

// Turn flush valve of TPIC3 off, preserve state of other bits
void flushOFF()
{
  // Add some code here to accomidate ModuleNum bit shifting

  TPICBuffer[3] &= 0xFE;   // TPICBuffer[3] is LSB
                               // Logical AND so as not to blow out other bits

  Serial.println(F("Flush Off . . ."));

  // Strobe TPIC buffer data to TPICS via SPI
  strobeTPICs();
}

// Set pump motor state to on, off, or reverse
void setPump(pumpState state)
{
// SAFTEY FEATURE HERE, DONT TURN MOTOR ON IF ALL VALVES ARE CLOSED
// If all valves are closed, auto turn pump off to prevent destruction
  switch(state)
  {
    case pumpState::ON:  // Draw inwards
      if((TPICBuffer[0] != 0) || (TPICBuffer[1] != 0) || (TPICBuffer[2] != 0) || (TPICBuffer[3] != 0))
      {
        Serial.println(F("Motor Turned on, Draw"));
        digitalWrite(pumpPin1,HIGH);  // Set Pump pin high
        digitalWrite(pumpPin2,LOW);  // Set Pump pin high
      }
      else
        Serial.println(F("Pump Draw can't enable; turn a valve on first"));
      break;

    case pumpState::REVERSE: // Draw outwards
      if((TPICBuffer[0] != 0) || (TPICBuffer[1] != 0) || (TPICBuffer[2] != 0) || (TPICBuffer[3] != 0))
      {
        Serial.println(F("Motor Turned on, Reverse"));
        digitalWrite(pumpPin1,LOW);  // Set Pump pin high
        digitalWrite(pumpPin2,HIGH);  // Set Pump pin high
      }
      else
        Serial.println(F("Pump Reverse can't enable; turn a valve on first"));
      break;

    case pumpState::OFF:
      Serial.println(F("Motor Turned off"));
      digitalWrite(pumpPin1,LOW);  // Set Pump pin low
      digitalWrite(pumpPin2,LOW);  // Set Pump pin low
      break;

    default:
      Serial.println(F("WARNING: Invalid pump command received!"));
  }
}

// clear all bits but Flush bit, TPICBuffer[3] is LSB
void clearValveBits()
{
  TPICBuffer[3] &= 0x01;
  for(int i=2; i>=0; i--) // because 4 TPICs in module
  {
    TPICBuffer[i] = 0x00; //Clear upper registers
  }
  // Strobe TPIC buffer data to TPICS via SPI
  strobeTPICs();
}


// Takes current valve number and configures TPIC buffers, ready for transfer for SPI (different function)
void setValveBits()
{   // First, push (save) flush bit
     bool flushBitSave;
     flushBitSave = TPICBuffer[3];
     TPICBuffer[3] &= 0x01;
    // Clear valve buffer
    for(int i=2; i>=0; i--) // because 4 TPICs in module
    {
      TPICBuffer[i] = 0x00; //Clear upper registers
    }

  // Set up TPIC buffers for next valve configuration
      TPICBuffer[2] = 0x01; // Set least significant bit for shifting
      // Shift up correct number of valves to configure TPIC buffers
      for(int i = 1; i < config.getValveNumber(); i++)
      {
        shiftl(TPICBuffer, sizeof TPICBuffer);
      }

      // Now restore Flushbit
      TPICBuffer[3] |= flushBitSave;

      // Strobe TPIC buffer data to TPICS via SPI
      strobeTPICs();
}
/*
// OLD: Uses original addressing scheme

// Takes current valve number and configures TPIC buffers, ready for transfer for SPI (different function)
void setValveBits()
{   // First, push (save) flush bit
     bool flushBitSave;
     TPICBuffer[3] &= 0x01;
     flushBitSave = TPICBuffer[3];

  // Set up TPIC buffers for next valve configuration
      TPICBuffer[3] = 0x01; // Set least significant bit for shifting
      // Shift up correct number of valves to configure TPIC buffers
      for(int i = 0; i < config.getValveNumber(); i++)
      {
        shiftl(TPICBuffer, sizeof TPICBuffer);
      }

      // Now restore Flushbit
      TPICBuffer[3] |= flushBitSave;

      // Strobe TPIC buffer data to TPICS via SPI
      strobeTPICs();
}
*/

// Bit shifting function to set TPIC buffer bits that indicate correct valve on TPICs
void shiftl(void *object, size_t size)
{
   unsigned char *byte; // pointer to Array address
   for ( byte = object; size--; ++byte )
   {
      unsigned char bitz = 0;
      if ( size )
      {
         bitz = byte[1] & (1 << (8 - 1)) ? 1 : 0;
      }
      *byte <<= 1;
      *byte  |= bitz;
   }
}

/**
 * Helper for mapping integers to the pumpState enum
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

bool puppetValveState(unsigned int valveNumber, bool valveState)
{
  if (valveNumber < 0 || valveNumber > numValves)
  {
    Serial.print(F("ERROR: Specified valve does not exist: "));
    Serial.println(valveNumber);
    return false;
  }

  config.setValveNumber(valveNumber);

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
