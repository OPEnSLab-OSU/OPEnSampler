//----------------------------
// Sampler functions
//----------------------------
// Place to store sampler stage functions

class SamplerFunctions {

  public:

   void doAction0_Off ()
    {
      Serial.println("Turning everything off");
      setPump(0);
      clearValveBits();  // Clear Valve Control bits, don't override Flush bit
      flushOFF(); // Turns flush off and also strobes TPICs over SPI
    }

    void doAction1_OpenFlush ()
    {
      // open flush valve
      clearValveBits();  // Clear Valve Control bits, don't override Flush bit
      Serial.println(F("Opening flush valve"));
      flushON(); // Turn Flush Valve on
    }

    void doAction2_NextValve ()
    {
      valveNum++;
      Serial.print(F("Valve number advanced to: ")); Serial.println(valveNum);
    }

    void doAction3_OpenValve ()
    {
      Serial.print ("Opening valve number "); Serial.println(valveNum);
      setValveBits();
      flushOFF();
    }

    void doAction4_PumpOut ()
    {
      Serial.println ("Drawing out of sampler with pump");
      setPump(-1);
    }

    void doAction5_PumpIn ()
    {
      Serial.println ("Drawing into sampler with pump");
      setPump(1);
    }

// typedef for class function
   typedef void (SamplerFunctions::*GeneralFunction) ();

// Change the size of this array to match the number of actions defined above!
   static const GeneralFunction doActionsArray [6];

};  // end of class SamplerFunctions

 // array of function pointers
const SamplerFunctions::GeneralFunction SamplerFunctions::doActionsArray [6] =
  {
    &SamplerFunctions::doAction0_Off, 
    &SamplerFunctions::doAction1_OpenFlush, 
    &SamplerFunctions::doAction2_NextValve, 
    &SamplerFunctions::doAction3_OpenValve, 
    &SamplerFunctions::doAction4_PumpOut, 
    &SamplerFunctions::doAction5_PumpIn,
  };

// ================================================================
// Valve Addressing Functions for serial TPICS
// ================================================================

// Turns off pump and valves
void everythingOff()
{
  // Reset pump and all valves to off
  setPump(false); // turn off pump
  Serial.println(F("Turning off valves"));
  clearValveBits();  // Clear Valve Control bits, don't override Flush bit
  flushOFF(); // Turns flush off and also strobes TPICs over SPI
}

// Turn flush valve of TPIC3 on, preserve state of other bits
void flushON()
{

  TPICBuffer[3] |= 0x01;   // TPICBuffer[3] is LSB
  // Logical OR so as not to blow out other bits
  Serial.println(F("Flushing System . . ."));

  // Strobe TPIC buffer data to TPICS via SPI
  strobeTPICs();
}

// Turn flush valve of TPIC3 off, preserve state of other bits
void flushOFF()
{

  TPICBuffer[3] &= 0xFE;   // TPICBuffer[3] is LSB
  // Logical AND so as not to blow out other bits

  Serial.println(F("Flush Off . . ."));

  // Strobe TPIC buffer data to TPICS via SPI
  strobeTPICs();
}

// Set Motor State, On(1), Off(0), Reverse(-1)
void setPump(signed int ms)
{

  // SAFTEY FEATURE HERE, DONT TURN MOTOR ON IF ALL VALVES ARE CLOSED
  // If all valves are closed, auto turn pump off to prevent destruction
  switch (ms)
  {
    case 1:  // Draw inwards
      if ((TPICBuffer[0] != 0) || (TPICBuffer[1] != 0) || (TPICBuffer[2] != 0) || (TPICBuffer[3] != 0))
      {
        Serial.println(F("Motor Turned on, Draw"));
        digitalWrite(pumpPin1, HIGH); // Set Pump pin high
        digitalWrite(pumpPin2, LOW); // Set Pump pin high
      }
      else
        Serial.println(F("Pump Draw can't enable; turn a valve on first"));
      break;
    case -1: // Draw outwards
      if ((TPICBuffer[0] != 0) || (TPICBuffer[1] != 0) || (TPICBuffer[2] != 0) || (TPICBuffer[3] != 0))
      {
        Serial.println(F("Motor Turned on, Reverse"));
        digitalWrite(pumpPin1, LOW); // Set Pump pin high
        digitalWrite(pumpPin2, HIGH); // Set Pump pin high
      }
      else
        Serial.println(F("Pump Reverse can't enable; turn a valve on first"));
      break;
    case 0:
      Serial.println(F("Motor turned off"));
      digitalWrite(pumpPin1, LOW); // Set Pump pin low
      digitalWrite(pumpPin2, LOW); // Set Pump pin low
      break;
    default:
      Serial.println(F("Invalid pump command received!"));
  }

}

// clear all bits but Flush bit, TPICBuffer[3] is LSB
void clearValveBits()
{
  TPICBuffer[3] &= 0x01;
  for (int i = 2; i >= 0; i--) // because 4 TPICs in module
  {
    TPICBuffer[i] = 0x00; //Clear upper registers
  }
  // Strobe TPIC buffer data to TPICS via SPI
  strobeTPICs();
}

// Takes current valve number and configures TPIC buffers, ready for transfer for SPI (different function)
void setValveBits()
{ // First, push (save) flush bit
  bool flushBitSave;
  flushBitSave = TPICBuffer[3];
  TPICBuffer[3] &= 0x01;
  // Clear valve buffer
  for (int i = 2; i >= 0; i--) // because 4 TPICs in module
  {
    TPICBuffer[i] = 0x00; //Clear upper registers
  }

  // Set up TPIC buffers for next valve configuration
  TPICBuffer[2] = 0x01; // Set least significant bit for shifting
  // Shift up correct number of valves to configure TPIC buffers
  for (int i = 1; i < numValves; i++)
  {
    shiftl(TPICBuffer, sizeof TPICBuffer);
  }

  // Now restore Flushbit
  TPICBuffer[3] |= flushBitSave;

  // Strobe TPIC buffer data to TPICS via SPI
  strobeTPICs();
}

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







