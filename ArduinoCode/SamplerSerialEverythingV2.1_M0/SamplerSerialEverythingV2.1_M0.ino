/*
  OPEnS Water sampler Firmware Version 1.97
  Chet Udell, Mitch Nelke, Cara Walter, 2018

  New in this version:
  Reconfigured files according to capstone team's organization
  Added GSM from Capstone
  Configured for M0

  SamplerSerialEverythingVx.x is the main Arduino Sketch file. Other dependent tabs include:
  Bluetooth
  CommandParser.cpp
  CommandParser.h
  Configuration.cpp
  Configuration.h
  Definitions.h
  Flash.h
  Globals.h
  PumpValveFunctions.cpp
  PumpValveFunctions.h
  SampleFunctions.h
  SerialCommandDefs.ino
  StatusUpdates.ino
  TimeFunctions.ino

  Dependencies:
  You need to download RTCLibExtended:
  https://github.com/FabioCuomo/FabioCuomo-DS3231

  And Sparkfun Low Power Library:
  https://github.com/rocketscream/Low-Power/archive/master.zip

  Include Timer Library from here:
  https://github.com/stevemarple/AsyncDelay/

  Include flash library from here:
  https://github.com/cmaglie/FlashStorage

  RTC and RTCLib-extended tutorial/reference:
  http://www.instructables.com/id/Arduino-Sleep-and-Wakeup-Test-With-DS3231-RTC/

  Interrupt library
  https://github.com/GreyGnome/EnableInterrupt

  Instructions on how to install library:
  https://learn.sparkfun.com/tutorials/installing-an-arduino-library

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.

  The features of OPEnS Sampler 1.0 include:
  24 sampler bags, 1 flush valve, 1 peristaltic pumps.
  It takes about 2 min to draw 250ml, or 480ms per ml
  1 Power Switch, 1 Sampler enable switch (because we want independent control of the timed sampler functions without cutting power, features that use this will be explained).
  Serial Command Set, You can communicate with the sampler over USB serial at 9600 Baud.

   Sampler Use:
  Power cycle on best practice: Be sure to supply the 12VDC source before plugging Arduino into USB
  Power cycle off: unplug Arduino USB before unplugging 12VDC supply

  1) Set global default variables and constants below to determine sampler behaviour.
  2) Ensure ms_DEBUG is properly set ot 0 or 1 depending on what you want (lab ms units or field Min units)
  3) Program Arduino using this sketch connected by computer using USB cable. Ensure Board and Port settings under Tools tab are properly set
  4) You may continue configuring and testing sampler functions over USB serial connection using the Serial Command Set (look at SerialCommandDefs tab)
  5) At time of Deployment, send any serial commands you need to ensure you begin at your desired valve number or other settings

  Firmware SAFETY Features:
  Pumps will not turn on if no valves are open at that time.
  Likewise, turning off all valves (including flush valve) will automatically shut off pumps
*/

#include <Wire.h> // i2c connection for RTC DS3231
#include <SPI.h>        // Using SPI hardware to communicate with TPICs
#include <AsyncDelay.h>
#include <LowPower.h> // Use Low-Power library from Sparkfun and RTC pin interrupt (P3) to manage sleep and scheduling at same time
#include <Arduino.h>  // for type definitions
#include <FlashStorage.h>

#include "Definitions.h" //where everything is defined
#include "SampleFunctions.h" //functions for sampler operations
#include "CommandParser.h"
#include "Configuration.h"
#include "Globals.h"
#include "Flash.h"

configuration config;

// create instance of SamplerFunctions
SamplerFunctions funcLibrary;

// Make Instance of Timer
AsyncDelay delayTimer;

void setup()
{
  pinMode(INTERRUPT_PIN, INPUT_PULLUP);
  pinMode(WAKEUP_PIN, INPUT_PULLUP);

  Serial.begin(baud); // send and receive at 115200 baud (defined in definitions
  while (!Serial)
    delay(10000);
  Serial.println(F("Beginning setup."));

  InitializeRTC()
  timer = millis();

  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), sampleEN, CHANGE);

  // Read flash memory struct configuration
  read_non_volatile();

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PUMP_PIN1, OUTPUT); // Set motor pin1 to output
  digitalWrite(PUMP_PIN1, LOW); // Turn off Motor
  pinMode(PUMP_PIN2, OUTPUT); // Set motor pin2 to output
  digitalWrite(PUMP_PIN2, LOW); // Turn off Motor

  if (configuration.getWritten())
  {
    writeNonVolatileDefaults();
  }

#if BLE_ENABLED
  BLESerial.setRXcallback(RXCallback);
  BLESerial.setACIcallback(ACICallback);
  BLESerial.setDeviceName((const char*) F("Sampler")); // Can be no longer than 7 characters
  BLESerial.begin();
  Serial.println(F("Bluetooth initialized."));
#endif

  // FONA 808 Setup (for SMS status updates)
#if FONA_ENABLED
  fonaSerial->begin(baud);
  if (! fona.begin(*fonaSerial)) {
    Serial.println(F("ERROR: Couldn't find FONA..."));
  }
#endif

  Serial.print(F("The current main line flush duration in ms is: "));
  Serial.println(config.getFlushDuration());
  Serial.print(F("The current bag flush duration in ms is: "));
  Serial.println(config.getBagFlushDuration());
  Serial.print(F("The current bag draw duration in ms is: "));
  Serial.println(config.getBagDrawDuration());
  Serial.print(F("The current sample duration in ms is: "));
  Serial.println(config.getSampleDuration());
  Serial.print(F("Next bag to sample is: "));
  Serial.println((config.getValveNumber() + 1)); // add 1 to this number for the actual bag number

  // Enable and Config SPI hardware:
  pinMode(RCLOCK, OUTPUT);
  SPI.begin();
  SPISettings(16000000, MSBFIRST, SPI_MODE1);


  // Now set alarm based on mode flag (daily or periodic)
  if (config.getMode() == Mode::DAILY)
  {
    //Set alarm1 every day at Hr:Mn
    // You may use different Alarm types for different periods (e.g. ALM1_MATCH_MINUTES to go off every hr)
    // RTC.setAlarm(ALM1_MATCH_MINUTES, configuration.SAMin, 0, 0); // This might work to go off every hour at specified minute every hour
    RTC.setAlarm(ALM1_MATCH_HOURS, config.getSampleMinute(), config.getSampleHour(), 0);
    Serial.print(F("The current sample alarm set for daily at "));
    Serial.print(config.getSampleHour()); Serial.print(F(":")); Serial.println(config.getSampleMinute());
  }
  else if (config.getMode() == Mode::PERIODIC)
  {
    config.refreshPeriodicAlarm();
  }
  // Reset pump and all valves to off
  Serial.println(F("Resetting electronics . . ."));
  everythingOff(); // Turn everything off
  delay(1000); // wait a second

  // Attach alarm interrupt
  RTC.alarmInterrupt(1, true);

  // Read switch pin to enable or disable sampler on startup
  sampleEN();
}

// the loop function runs over and over again forever
void loop()
{
  if (timerEN) //only way to get here is from wakeUp
  {
    // Do wake-up activities: reset flags, advance to next valve if haven't reached maximum valves
    //**************** VERY IMPORTANT ****************
    //clear interrupt registers, attach interrupts EVERY TIME THE INTERRUPT IS CALLED
    interrupt_reset();
    timer = millis();
    timeEN = false;

    if (config.getValveNumber() >= numValves) // If target valve number is greater than total number of desired valves
    {
      Serial.println(F("Total number of samples reached!"));
      Serial.println(F("Sleeping forever, bye!"));
      // warning, because wakeup pin is disabled above, sleep forever, no wakeup from here
      sleepEN = true; // Set sleep flag to sleep at end of loop
    }
    else //setup for next sample
    {
      Serial.println(F("Setting up for next sample"));
      SampleState = HIGH; // Trigger new sample cycle, raise sample flag for Loop
      sequenceFlag = true;

      // Advance and save valve number -
      configData.valveNumber = configData.valveNumber + 1; // Increment valve number
      configData.written = memValidationValue; // ensure we remember we've written new value to Flash
      configData.checksum = configData.checksum ++;
      write_non_volatile(); // SAVE new Valve Number in flash
      Serial.print(F("Moving onto valve number ")); Serial.println(configData.valveNumber);

      // Disable external pin interrupt on wake up pin.
 //     detachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN));

      //clear the alarm
      clearAlarms();
      //set alarm for next cycle
      if (config.getMode() == Mode::DAILY)
      {
        RTC.setAlarm(ALM1_MATCH_HOURS, config.getSampleMinute(), config.getSampleHour(), 0);
        Serial.print(F("Resetting alarm for next day at "));
        Serial.print(config.getSampleHour()); Serial.print(F(":")); Serial.println(config.getSampleMinute());
      }
      else if (config.getMode() == Mode::PERIODIC)
      {
        config.refreshPeriodicAlarm();
      }
    }
  }
  else if (SampleState) // Will be TRUE if new sample alarm ISR has been triggered
  {
    //Setup next action in sequence

    if (sequenceFlag)  //True if set to run sequence
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
    }

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
    { // Else turn program sequence off and reset for next wake-up
      sequenceFlag = false;  // No more timer
      timerEN = false; // No more master sample enable
      SampleState = LOW;  // Turn Sample off, lower flag for loop
            timeEN = false;
      setRTCAlarm_Relative(0, 0, 7); // 7 seconds
      prep_before_sleep();

      LowPower.standby();     // Go to sleep here

      prep_after_sleep();     
     /* // Allow wake up pin to trigger interrupt on low.
      attachInterrupt(digitalPinToInterrupt(WAKEUP_PIN), wakeUp, FALLING);
      // Allow switch pin to change sampler mode
      attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), sampleEN, CHANGE);*/
      // Enter power down state with ADC and BOD module disabled.
      // Wake up when wake up pin via RTC is exerted low.
      sleepEN = true; // Set sleep flag to sleep at end of loop

      Serial.print("Done With Programmed Sequence of Actions!");
    } //end programmed sequence
  }
}  // End Sample State-machine

// listen for serial:
listenForSerial();
#if BLE_ENABLED
BLESerial.pollACI();
#endif

// Should we go to sleep?
sleepcheck();

if (valveOpen)
{
  //setup courtesy message to keep valve open
  if (printedOnce)
  {
    valvePrint = false; //flag to stop printing valve number after first time
  }
  openValve();
}
} // End Loop
