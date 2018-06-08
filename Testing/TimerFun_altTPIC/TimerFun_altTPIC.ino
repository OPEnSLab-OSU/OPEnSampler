// Include Timer Library
// From here:
// https://github.com/stevemarple/AsyncDelay/blob/master/examples/AsyncDelay_example/AsyncDelay_example.ino
// SamplerFunctions come from here:
// https://arduino.stackexchange.com/questions/21095/how-to-write-array-of-functions-in-arduino-library

#include <SPI.h>        // Using SPI hardware to communicate with TPICs
#include <AsyncDelay.h>
#include "Definitions.h"
#include "SampleFunctions.h"

// Make Instance of Timer
AsyncDelay delayTimer;

// create instance of SamplerFunctions
SamplerFunctions funcLibrary;

void setup() {
  // put your setup code here, to run once:
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(INTERRUPT_PIN, INPUT_PULLUP);
  pinMode(WAKEUP_PIN, INPUT_PULLUP);
  pinMode(PUMP_PIN1, OUTPUT); // Set motor pin1 to output
  digitalWrite(PUMP_PIN1, LOW); // Turn off Motor
  pinMode(PUMP_PIN2, OUTPUT); // Set motor pin2 to output
  digitalWrite(PUMP_PIN2, LOW); // Turn off Motor

  // Enable and Config SPI hardware:
  pinMode(RCLOCK, OUTPUT);
  SPI.begin();
  SPISettings(16000000, MSBFIRST, SPI_MODE1);

  Serial.begin(115200);
  delay(1000);
  //Serial.print("TPIC count: "); Serial.println(TPIC_COUNT);
  //Serial.print("Flush valve number: "); Serial.println(FLUSH_VALVE_NUM);
  Serial.println("Function Test: Waiting for serial command.");

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

    Serial.print("Executed action number: ");
    Serial.print(myProgram[programCounter]);
    Serial.print(" for ");
    Serial.print(stateTimerMS);
    Serial.println("ms");
    delay(20);

    // Start a new timer based on
    delayTimer.start(stateTimerMS, AsyncDelay::MILLIS);
  }

  // Check if timer has finished, AND if timerEN are enabled
  // if so, set sequenceFlag to step through next function
  if (delayTimer.isExpired() && timerEN)
  {
    // INC Program Counter if we're not done with program
    if (programCounter < ((sizeof(myProgram) >> 1) - 1))
    {
      // Set Flag to start next program
      sequenceFlag = true;
      programCounter++;    // INC Program Counter
      delayTimer.repeat(); // Restart Timer
    }
    else
    { // Else turn program sequence off
      sequenceFlag = false;  // No more timer
      timerEN = false; // No more master sample enable

      Serial.println("Done With Programmed Sequence of Actions!");
    }
  }
  // listen for serial:
  listenForSerial();

  if (valveOpen)
  {
    //setup courtesy message to keep valve open
    if (printedOnce)
    {
      valvePrint = false; //flag to stop printing valve number after first time
    }
    openValve();
  }
} // end loop


