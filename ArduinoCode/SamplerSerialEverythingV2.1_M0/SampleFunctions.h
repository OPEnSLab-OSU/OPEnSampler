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
      configData.valveNumber++;
      Serial.print(F("Valve number advanced to: ")); Serial.println(configData.valveNumber);
    }
    void doAction3_OpenValve ()
    {
      Serial.print ("Opening valve number "); Serial.println(configData.valveNumber);
      delay(100);
      valvePrint = true;
      currValve = configData.valveNumber;
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
	void doAction6_TakeSample ()
    {
      	  Serial.print(F("Taking a water sample in valve ")); Serial.println(configData.valveNumber); 
      setPump(1);
	  #if FONA_ENABLED
		sendSMSAll((const char *) F("OPEnSampler says: Taking a sample!"));
	  #endif
    }

    // typedef for class function
    typedef void (SamplerFunctions::*GeneralFunction) ();

    // Change the size of this array to match the number of actions defined above!
    static const GeneralFunction doActionsArray [7];

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
  &SamplerFunctions::doAction6_TakeSample,
};
