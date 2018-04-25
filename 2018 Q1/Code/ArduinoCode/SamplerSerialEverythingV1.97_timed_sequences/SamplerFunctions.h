// SamplerFunctions come from here:
// https://arduino.stackexchange.com/questions/21095/how-to-write-array-of-functions-in-arduino-library

#include <AsyncDelay.h>

// Make Instance of Timer
AsyncDelay delayTimer;

// Place to store sampler Phase functions
// First thing in each function is to update the stateTimerMS above
// This lets the millis counter know how long to execute the function before 
// seqquencing to the next
class SamplerFunctions {

  public:

   void doAction0 ()
    {
      // open flush valve
      // start pump forward
      // close flush valve
      Serial.println ("Full Purge Main Line");
    }

    void doAction1 ()
    {
      
      Serial.println ("Open new Valve Num");
    }

    void doAction2 ()
    {
      Serial.println ("Pump 3mil out of Bag");
    }

    void doAction3 ()
    {
      Serial.println ("Pump 3mil into Bag");
    }

    void doAction4 ()
    {
      Serial.println ("Close Valve");
    }

    void doAction5 ()
    {
      Serial.println ("Small Purge Main Line 100mil");
    }

// typedef for class function
   typedef void (SamplerFunctions::*GeneralFunction) ();

// Change the size of this array to match the number of actions defined above!
   static const GeneralFunction doActionsArray [6];

};  // end of class SamplerFunctions

 // array of function pointers
const SamplerFunctions::GeneralFunction SamplerFunctions::doActionsArray [6] =
  {
    &SamplerFunctions::doAction0, 
    &SamplerFunctions::doAction1, 
    &SamplerFunctions::doAction2, 
    &SamplerFunctions::doAction3, 
    &SamplerFunctions::doAction4, 
    &SamplerFunctions::doAction5,
  };


