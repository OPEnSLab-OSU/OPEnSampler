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
      configuration.VNum++;
      Serial.print(F("Valve number advanced to: ")); Serial.println(configuration.VNum);
    }
    void doAction3_OpenValve ()
    {
      //Serial.print ("Opening valve number "); Serial.println(configuration.VNum);
      //delay(100);
      valvePrint = true;
      currValve = configuration.VNum; 
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
