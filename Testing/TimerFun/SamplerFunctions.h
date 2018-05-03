// ================================================================
// Valve and pump functions for serial TPICS
// ================================================================

// Turns off pump and valves
void everythingOff()
{
  // Reset pump and all valves to off
  setPump(false); // turn off pump
  closeValves(); // turn off all valves
}

// ================================================================
// Valve Operations Functions for serial TPICS
// ================================================================

void closeValves()
{
  // Take rClock low to enable transfer
  digitalWrite(rclock, LOW);
  delay(1);
  for (int i = 1; i < TPICcount; i--)
  {
    //transfer byte to TPIC i
    SPI.transfer(0);
  }
  Serial.print(F("All valves closed"));

  //indicate no valves are open
  valveOpen = 0;

  delay(1);
  digitalWrite(rclock, HIGH);
}

void openValve()
{
  //use modulo to find valve place on TPIC
  int valvePlace = currValve % 8;

  // determine which TPIC the currValve is on
  // assign currTPIC number to reflect physical setup
  int currTPIC;
  // round up
  if (valvePlace > 0) {
    currTPIC = currValve / 8 + 1;
  }
  // No rounding needed
  else {
    currTPIC = currValve / 8;
  }
  //int currTPIC = ceil(testValve/8);
  Serial.print(F("Current TPIC in physical order")); Serial.println(currTPIC);

  /*generate and send output bytes - 1 per TPIC, go backwards since using cascading serial
    - TPICs are connected in series - first 8 bits go to first TPIC
      and it pushes rest of bits to next in line, etc. */

  // Take rClock low to enable transfer
  digitalWrite(rclock, LOW);
  delay(1);
  // loop through all TPICs
  for (int i = 1; i < TPICcount; i--)
  {
    // if on the TPIC that has the valve, send valve place as power of 2
    if (i == currTPIC)
    {
      //transfer byte to TPIC i
      SPI.transfer(pow(2, (valvePlace - 1)));
      // Print to serial
      Serial.print(F("Valve ")); Serial.print(currValve); Serial.println(F(" on."));
    }
    // if not on TPIC that has valve, send 0
    else
    {
      //transfer byte to TPIC i
      SPI.transfer(0);
    }
  }
  delay(1);
  digitalWrite(rclock, HIGH);

  //indicate a valve is open
  valveOpen = 1;
}

// ================================================================
// Pump Operation Function
// ================================================================
// Set Motor State, On(1), Off(0), Reverse(-1)
void setPump(signed int ms)
{
  // SAFTEY FEATURE HERE, DO NOT TURN MOTOR ON IF ALL VALVES ARE CLOSED
  // If all valves are closed, auto turn pump off to prevent destruction
  switch (ms)
  {
    case 1:  // Draw inwards
      if (valveOpen)
      {
        Serial.println(F("Motor Turned on, Draw"));
        digitalWrite(pumpPin1, HIGH); // Set Pump pin high
        digitalWrite(pumpPin2, LOW); // Set Pump pin low
      }
      else
        Serial.println(F("Pump draw can't enable; turn a valve on first"));
      break;
    case -1: // Draw outwards
      if (valveOpen)
      {
        Serial.println(F("Motor Turned on, Reverse"));
        digitalWrite(pumpPin1, LOW); // Set Pump pin low
        digitalWrite(pumpPin2, HIGH); // Set Pump pin high
      }
      else
        Serial.println(F("Pump reverse can't enable; turn a valve on first"));
      break;
    case 0:
      Serial.println(F("Motor Turned off"));
      digitalWrite(pumpPin1, LOW); // Set Pump pin low
      digitalWrite(pumpPin2, LOW); // Set Pump pin low
      break;
    default:
      Serial.println(F("Invalid pump command received!"));
  }
}

//----------------------------
// Sampler functions
//----------------------------
// Place to store sampler stage functions

class SamplerFunctions {
  public:
    void doAction0_Off ()
    {
      Serial.println("Turning everything off");
      everythingOff();
    }
    void doAction1_OpenFlush ()
    {
      Serial.println(F("Opening flush valve"));
      currValve = flushValveNum; 
      openValve();
    }
    void doAction2_NextValve ()
    {
      valveNum++;
      Serial.print(F("Valve number advanced to: ")); Serial.println(valveNum);
    }
    void doAction3_OpenValve ()
    {
      Serial.print ("Opening valve number "); Serial.println(valveNum);
      currValve = valveNum;
      openValve();
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






