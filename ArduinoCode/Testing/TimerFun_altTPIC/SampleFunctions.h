// ================================================================
// Valve and pump functions for serial TPICS
// ================================================================

void closeValves()
{
  // Take rClock low to enable transfer
  digitalWrite(RCLOCK, LOW);
  delay(1);
  //msg = 00000000;
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

  if ((currValve <= NUM_VALVES) && (currValve > 0))
  {
    //use modulo to find valve place on TPIC - will get values of 0(for multiples of 8) to 7
    valvePlace = currValve % 8;
    // determine which TPIC the currValve is on
    // assign currTPIC number to reflect physical setup
    currTPIC;
    // round up
    if (valvePlace > 0) {
      currTPIC = currValve / 8 + 2; //plus two since flush is on first TPIC, and need to round up
    }
    // for cases of 8, 16, 24 - keep on lower TPIC, but set valve place to 8
    else {
      currTPIC = currValve / 8 + 1; //plus 1 since flush in on first TPIC
      valvePlace = 8; //adjust valve place from 0
    }
    if (valvePrint)
    {
      Serial.print(F("Current TPIC in physical order ")); Serial.println(currTPIC);
      delay(200);
      Serial.print(F("Valve ")); Serial.print(currValve); Serial.println(F(" turning on."));
    }
    /*generate and send output bytes - 1 per TPIC, go backwards since using cascading serial
      - TPICs are connected in series - first 8 bits go to first TPIC
        and it pushes rest of bits to next in line, etc. */
  }
  else if (currValve == 0) //flush valve
  {
    currTPIC = 1;
    valvePlace = 1;
  }
  //bitshiftleft: 00000001<< 2 = 00000100 is valve 3
  msg = 00000001 << valvePlace - 1; //subtract 1 since 1 % 8 = 1 etc. and first valve on TPIC needs zero shift
  // loop through all TPICs
  for (int i = TPIC_COUNT; i > 0; i--)
  {
    // if on the TPIC that has the valve, send valve place as a byte
    if (i == currTPIC)
    {
      //transfer byte to TPIC i by using bitshiftleft
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
      //msg = 00000000;
      //transfer byte to TPIC i
      SPI.transfer(0);
      if (valvePrint)
      {
        Serial.println(F("Message sent to TPIC: 0"));
        delay(20);
        printedOnce = true;
      }
    }
  }

  //enable transfer
  digitalWrite(RCLOCK, HIGH);
  delay(1);
  digitalWrite(RCLOCK, LOW);
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
        digitalWrite(PUMP_PIN1, HIGH); // Set Pump pin high
        digitalWrite(PUMP_PIN2, LOW); // Set Pump pin low
      }
      else
        Serial.println(F("Pump draw can't enable; turn a valve on first"));
      break;
    case -1: // Draw outwards
      {
        Serial.println(F("Motor Turned on, Reverse"));
        digitalWrite(PUMP_PIN1, LOW); // Set Pump pin low
        digitalWrite(PUMP_PIN2, HIGH); // Set Pump pin high
        break;
      }
    case 0:
      {
        Serial.println(F("Motor Turned off"));
        digitalWrite(PUMP_PIN1, LOW); // Set Pump pin low
        digitalWrite(PUMP_PIN2, LOW); // Set Pump pin low
        break;
      }
    default:
      Serial.println(F("Invalid pump command received!"));
  }
}

// Turns off pump and valves
void everythingOff()
{
  // Reset pump and all valves to off
  closeValves(); // turn off all valves
  setPump(false); // turn off pump
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
      valvePrint = true;
      currValve = FLUSH_VALVE_NUM;
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
      delay(100);
      valvePrint = true;
      currValve = valveNum;
      openValve();
    }
    void doAction4_PumpOut ()
    {
      Serial.println ("Drawing out of sampler with pump (reverse)");
      setPump(-1);
    }
    void doAction5_PumpIn ()
    {
      Serial.println ("Drawing into sampler with pump (forward)");
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






