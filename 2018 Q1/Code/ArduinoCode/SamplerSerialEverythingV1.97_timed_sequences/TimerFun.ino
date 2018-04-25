// Include Timer Library
// From here:
// https://github.com/stevemarple/AsyncDelay/blob/master/examples/AsyncDelay_example/AsyncDelay_example.ino
// SamplerFunctions come from here:
// https://arduino.stackexchange.com/questions/21095/how-to-write-array-of-functions-in-arduino-library


#include "SamplerFunctions.h"

volatile bool ledState = false;

volatile bool timerEN = false; // Should correspond to flag raised on Interrupt, e.g. TimerEn
volatile long stateTimerMS = 0;  // Initialize state timer 
volatile bool sequenceFlag = false; // Is set True when timer has run its course
volatile int programCounter = 0; // counter to step through program array

// This array determines the sequence of actions to take
// during a sample condition, see action library below
const short myProgram[] = {5, 1, 2, 3, 4, 5, 4, 3, 2, 1, 0};

// This array determines the timing of the above actions sequence numbers
// during a sample condition, see action library below
const uint16_t myTimes[] = {5000, 250, 600, 600, 250, 3000, 250, 600, 600, 250, 4000};

// create instance of SamplerFunctions
  SamplerFunctions funcLibrary;
  
void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  delay(5000);
  Serial.println("FunTest");
//  LEDTimer.start(250, AsyncDelay::MILLIS);

  // An pin interrupt should do this, but for now,
  // set flags true here so below sequence can start
  timerEN = true;
  sequenceFlag = true;

  Serial.print("My program sequence is this many steps: ");
  Serial.println(sizeof(myProgram)>>1);

  Serial.print("My program sequence is this many steps: ");
  Serial.println(sizeof(myTimes)>>1);

// If program array and timing array not same size, report to user, hang up program
  if((sizeof(myProgram)>>1) != (sizeof(myTimes)>>1))
  {
    Serial.print("Program array and Timing array not same size, please check your code");
    while(true);
  }
}

void loop() {

  // Step through program using timings
  if(sequenceFlag)
  {
    // unset timer flag because this should only be executed when timer has run out
    sequenceFlag = false;
    
    // get member function pointer from array,
    // determined by programCounter sequencing through
    // program counter Array
    SamplerFunctions::GeneralFunction f = SamplerFunctions::doActionsArray [myProgram[programCounter]];
    // call the function
    (funcLibrary.*f) ();

    //Update the time to execute this next function:
    stateTimerMS = myTimes[programCounter];

    Serial.print("Executing Action number: ");
    Serial.print(myProgram[programCounter]);
    Serial.print(" for ");
    Serial.print(stateTimerMS);
    Serial.println("ms");
    
    
    // Start a new timer based on 
    delayTimer.start(stateTimerMS, AsyncDelay::MILLIS);
  }

  // Check if timer has finished, AND if timerEN are enabled
  // if so, set sequenceFlag to step through next function
  if (delayTimer.isExpired() && timerEN)
  {
    // Set Timer Flag
    sequenceFlag = true;

    // INC Program Counter if we're not done with program
    if(programCounter < ((sizeof(myProgram)>>1)-1))
    {
      programCounter++;    // INC Program Counter
      delayTimer.repeat(); // Restart Timer
    }
    else
    { // Else turn program sequence off
      sequenceFlag = false;  // No more timer
      timerEN = false; // No more master sample enable
      Serial.print("Done With Program Sequence of Actions!");
    
    // This is where you *might* put the processor to sleep
    
    }
  }

}


