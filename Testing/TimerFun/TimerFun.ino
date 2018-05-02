// Include Timer Library
// From here:
// https://github.com/stevemarple/AsyncDelay/blob/master/examples/AsyncDelay_example/AsyncDelay_example.ino
// SamplerFunctions come from here:
// https://arduino.stackexchange.com/questions/21095/how-to-write-array-of-functions-in-arduino-library

#include "Definitions.h"
#include "SamplerFunctions.h"
#include <SPI.h>        // Using SPI hardware to communicate with TPICs
#include <AsyncDelay.h>

// Make Instance of Timer
AsyncDelay delayTimer;

// create instance of SamplerFunctions
SamplerFunctions funcLibrary;

void setup() {
  // put your setup code here, to run once:
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(interruptPin, INPUT_PULLUP);
  pinMode(wakeUpPin, INPUT_PULLUP);
  pinMode(pumpPin1, OUTPUT); // Set motor pin1 to output
  digitalWrite(pumpPin1, LOW); // Turn off Motor
  pinMode(pumpPin2, OUTPUT); // Set motor pin2 to output
  digitalWrite(pumpPin2, LOW); // Turn off Motor

  // Enable and Config SPI hardware:
  pinMode(rclock, OUTPUT);
  SPI.begin();
  SPISettings(16000000, MSBFIRST, SPI_MODE1);

  Serial.begin(115200);
  delay(5000);
  Serial.println("Function Test: Waiting for serial command.");

  // An pin interrupt should do this, but for now,
  // set flags true here so below sequence can start
  //timerEN = true;
  //sequenceFlag = true;

  /*// If program array and timing array not same size, report to user, hang up program
  if ((sizeof(myProgram) >> 1) != (sizeof(myTimes) >> 1))
  {
    Serial.print("Program array and Timing array not same size, please check your code");
    while (true);
  }*/
}

void loop() {

  // Step through program using timings
  if (sequenceFlag)
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

    Serial.print("Executing action number: ");
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
    if (programCounter < ((sizeof(myProgram) >> 1) - 1))
    {
      programCounter++;    // INC Program Counter
      delayTimer.repeat(); // Restart Timer
    }
    else
    { // Else turn program sequence off
      sequenceFlag = false;  // No more timer
      timerEN = false; // No more master sample enable
      
      Serial.println("Done With Program Sequence of Actions!");

    }
  }
  // listen for serial:
  listenForSerial();
} // end loop


