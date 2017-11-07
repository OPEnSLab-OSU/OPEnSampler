/*
  OPEnS Water sampler Firmware Version 1.3
  Chet Udell, Mitch Nelke, 2017

  New in this version: Add RTC pin interrupt on pin 3 and Sparkfun Low-Power library to put to Arduino to sleep and wake up
  This may eliminate the need for MSTimer2 and ms_DEBUG
  MSTimer2 ISR will be ported over to the Pin3 ISR
  Remove SP serail command, replace with SA (sample alarm) to set (hr min ) each day to take sample. for testing, use this command to set to the next known min and wait. Or use switch on P2)

  TODO: Add code to handle RTC timer period set. Double check all enable/dissable timer things and replace with RTC interrupt functions instead
      Also, Add code for RTC timed functions for debug mode = 1 specify time in ms VS debug=0 for time in min
    May also need to remove or modify RES serial commmand function, no longer internal timer specific.
    Make SA serail function
    Make switch pin 2 ISR read time from RTC, reset alarm time for that time (send serial notification), proceed with taking sample.

You may need to download RTCLibExtended:
https://github.com/FabioCuomo/FabioCuomo-DS3231

And Sparkfun Low Power Library:
https://github.com/rocketscream/Low-Power/archive/master.zip

Instructions on how to install library:
https://learn.sparkfun.com/tutorials/installing-an-arduino-library

RTC and RTCLib-extended tutorial/reference:
http://www.instructables.com/id/Arduino-Sleep-and-Wakeup-Test-With-DS3231-RTC/

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
24 sampler bags, 1 flush valve, 2 peristaltic pumps.
It takes about 2 min to draw 250ml, or 480ms per ml
1 Power Switch, 1 Sampler enable switch (because we want independent control of the timed sampler functions without cutting power, features that use this will be explained).
Serial Command Set, You can communicate with the sampler over USB serial at 9600 Baud. 

Arduino Pinouts:
GPIO2   - Sample enable/dissable low-true
GPIO3   - RTC Interrupt, low-true
GPIO9   - pump pin 1 (motor driver 1)
GPIO10  - pump pin 2 (motor driver 2)
~ SPI ~
GPIO8 - rClock - TPIC register clock
GPIO11 - SPI MOSI - TPIC Data
GPIO13 - sClk - TPIC serial clock
~ i2c ~
SCL/SDA = to RTC SCL/SDA

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
// If this is 0, use min units for sample period, normal field use. 
// If set to 1, use millisecond units for sample period instead of Min for lab testing of functionality
#define ms_DEBUG 1

//#include <MsTimer2.h>   // MsTimer2 for pump timing
#include <Wire.h> // i2c connection for RTC DS3231
#include <RTClibExtended.h> // Library to configure DS3231
#include "LowPower.h" // Use Low-Power library from Sparkfun and RTC pin interrupt (P3) to manage sleep and scheduling at same time
#include <EEPROM.h>     // Will be writing params to non-volatile memory to save between uses
#include "EEPROMAnything.h" // EEPROM read/write functions
#include <SPI.h>        // Using SPI hardware to communicate with TPICs

const byte eepromValidationValue = 99; // Value to test to see if EEPROM has been written before
// Determine Sampler "Factory Default" behaviour. These can be changed and saved using the SerialCommandDefs (see tab with same label)
// If never set before, default sample time is once per day, at 8AM (08:00)
const uint8_t SampleAlarmMinDef = 00; // Default sample time: Min
const uint8_t SampleAlarmHrDef = 8; // Default sample time: Hr
#if (ms_DEBUG == 0)
  const unsigned long SampleDurMsDef = 120000;   // 2min Factory Default, How long pumps should run to take one sample, in ms: 250ml takes about 2min
  const unsigned long FlushDurMsDef = 60000; // 1min Factory default, low long in ms to flush system

#elif (ms_DEBUG == 1)
  const uint16_t SampleDurMsDef = 5000;   // Factory Default, How long pumps should run to take one sample, in ms
  const uint16_t FlushDurMsDef = 3000; // Factory default, low long in ms to flush system
#endif

const uint8_t SampleVolMlDef = 250;  // Factory Default, sample vol in ml, 250ml takes about 2min, or about 480ms per ml
const bool SampleValveNumDef = 0; // Factory Default, current valve number

// Switch Interrupt pin is LOW-True Logic, GND == enabled
const byte interruptPin = 2;  // Digital pin switch is attached to that enables or dissables the sampler timer
const byte wakeUpPin = 3; // Use pin 3 as wake up pin
const byte pumpPin1 = 9; // Motor MOSFET attached to digital pin 9,  1=forward, 0=reverse
const byte pumpPin2 = 10; // Motor MOSFET attached to digital pin 10 0=forward, 1=reverse
signed int directionMotor = 0; // 0=off, 1=draw water in, -1=pull water out

volatile bool ledState = 0;
volatile bool timerEN = false; // Flag to enable or dissable sampler timed functions
volatile bool sleepEN = false; // Flag to check in loop to see if we should sleep or not
// These maintain the current state
volatile bool SampleState = LOW; // SampleState used to set valve and motor states for taking new sample
volatile bool FlushState = LOW;  // used to set state-machine flush flag
//volatile bool SampleNow = true; // Flag to enable take sample on startup or anytime switch is turned off then back on.

volatile unsigned long previousMillis = 0;   // will store last time Sample was taken

int value = 0; // Place to accumulate valve number input (see SerialCommandDefs tab)

// Struct for saving Sampler params in EEPROM, see http://playground.arduino.cc/Code/EEPROMWriteAnything
struct config_t
{
  uint8_t SAMin; // Sample alarm time Min
  uint8_t SAHr; // Sample alarm time Hr
  unsigned long FDMs; // Flush duration in ms
  unsigned long SDMs;  // Sample Duration in ms
  uint16_t SVml; // Sample volume in ml
  uint8_t VNum;  // Current Valve number sampled, 0=reset/default
  byte written;  // Has the EEPROM Memory been written before?
  bool Is_Daily;
//  bool Is_Hourly;

} configuration;

//----------------------------
// Valve Addressing Variables
//----------------------------
const int numValves = 24; // how many valves in each module
int numModules = 1; // how many modules? 1 master module for now. Will be used to calculate number of shifts to TPICs for expansion modules
unsigned char TPICBuffer[4] = {0x00}; // Store Status bits of TPICs, see ValveAddressing tab for details
uint16_t valveCount = 0;
uint8_t moduleNum = 0;  // number of module depending on how high valve count is (groups of 25)
uint8_t valveNum = 0;  // number of valve relative to current module

//----------------------------
// SPI Variables
//----------------------------
const byte sclock = 13; // SPI SCK pin 13
const byte rclock = 8; // Register clock pin, acts as CS, could be moved to SPI SS pin 10?
const byte datapin = 11; // SPI MOSI pin 11

// Init DS3231 RTC
RTC_DS3231 RTC;

void setup() {
  pinMode(interruptPin, INPUT_PULLUP);
  pinMode(wakeUpPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), sampleEN, CHANGE);
  // Read EEPROM memory struct configuration
  EEPROM_readAnything(0, configuration);
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(pumpPin1, OUTPUT); // Set motor pin1 to output
  digitalWrite(pumpPin1, LOW); // Turn off Motor
  pinMode(pumpPin2, OUTPUT); // Set motor pin2 to output
  digitalWrite(pumpPin2, LOW); // Turn off Motor

  Serial.begin(115200); // send and receive at 9600 baud
  while (!Serial)
    delay(1000);
  if (configuration.written != eepromValidationValue)
  {
    writeEEPROMdefaults();
  }

  Serial.print("the current Flush duration in ms is: ");
  Serial.println(configuration.FDMs);
  Serial.print("the current Sample duration in ms is: ");
  Serial.println(configuration.SDMs);
  
// Enable and Config SPI hardware:
  pinMode(rclock, OUTPUT);
  SPI.begin();
  SPISettings(16000000, MSBFIRST, SPI_MODE1);
// RTC Timer settings here
  if (! RTC.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
// This may end up causing a problem in practice - what if RTC looses power in field? Shouldn't happen with coin cell batt backup
  if (RTC.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
  // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  //clear any pending alarms
  RTC.armAlarm(1, false);
  RTC.clearAlarm(1);
  RTC.alarmInterrupt(1, false);
  RTC.armAlarm(2, false);
  RTC.clearAlarm(2);
  RTC.alarmInterrupt(2, false);

  //Set SQW pin to OFF (in my case it was set by default to 1Hz)
  //The output of the DS3231 INT pin is connected to this pin
  //It must be connected to arduino D2 pin for wake-up
  RTC.writeSqwPinMode(DS3231_OFF);
// Now set Alarm based on whether Daily or Hourly flag is set
if(configuration.Is_Daily) // is daily flag set?
{
  //Set alarm1 every day at Hr:Mn
  // You may use different Alarm types for different periods (e.g. ALM1_MATCH_MINUTES to go off every hr)
  // RTC.setAlarm(ALM1_MATCH_MINUTES, configuration.SAMin, 0, 0); // This might work to go off every hour at specified minute every hour
  RTC.setAlarm(ALM1_MATCH_HOURS, configuration.SAMin, configuration.SAHr, 0);   //set your wake-up time here
  Serial.print("the current sample Alarm set for Daily at");
  Serial.print(configuration.SAHr); Serial.print(":"); Serial.println(configuration.SAMin);
}
else  // is hourly?
{
  //Set alarm1 every hr at Mn
  RTC.setAlarm(ALM1_MATCH_MINUTES, configuration.SAMin, 0, 0); // go off every hour at specified minute every hour
  Serial.print("the current sample Alarm set for Hourly at");
  Serial.println(configuration.SAMin);
}
  // Reset pump and all valves to off
  Serial.print("resetting electronics . . .");
  everythingOff(); // Turn everything off
  delay(1000); // wait a second

  // Attach alarm interrupt
  RTC.alarmInterrupt(1, true);

  // Read switch pin to enable or dissable sampler on startup
  sampleEN(); 
}

// the loop function runs over and over again forever
void loop() {
if(timerEN)
{
  // Exiting Sleep mode...
  Serial.println("Exit Sleep Mode . . .");
  Serial.print("Woke up at ");
  DateTime now = RTC.now();
  Serial.print(now.hour(), DEC); Serial.print(':'); Serial.print(now.minute(), DEC); Serial.print(':'); Serial.print(now.second(), DEC); Serial.println();
 // Disable external pin interrupt on wake up pin.
  detachInterrupt(digitalPinToInterrupt(wakeUpPin));
  if (configuration.VNum >= numValves) // If target valve number is greater than total number of desired valves
  {
    Serial.println("Total number of samples reached!");
    Serial.println("Sleeping forever, bye!");
    // warning, because wakeup pin is dissabled above, sleep forever, no wakeup from here
    sleepEN = true; // Set sleep flag to sleep at end of loop 
  }
  else
  {
    Serial.println("time to take a sample");
    previousMillis = millis();  // Remember the time at this point
    SampleState = HIGH; // Trigger new sample cycle, raise sample flag for Loop
    FlushState = HIGH; // Begin sample cycle by flushing system for period of time

    // Now that we have the module number and valve number. Set TPICBuffer
    clearValveBits();  // Clear Valve Control bits, don't override Flush bit
    flushON(); // Turn Flush Valve on, Turn on Motors

    //Turn Motors on to draw new sample
    setPump(SampleState); // Turn on Motor
    digitalWrite(LED_BUILTIN, SampleState);
  }
  timerEN = false; // dissable timed functions
}
else if (SampleState) // Will be TRUE if new sample timer ISR has gone off, or switch flipped from off to on
  { // Enter Flush mode. . .
    // check to see if it's time to start or stop this newly triggered sample
    unsigned long currentMillis = millis();
    // Begin sample cycle by flushing for period of time, Flush already on by this point ( see sampleTrig() )

    if ((FlushState == HIGH) && (currentMillis - previousMillis >= configuration.FDMs))
    { // End Flush, enter Sample Mode
      Serial.print("Flushed System for "); Serial.print(configuration.FDMs); Serial.println("ms");
      FlushState = LOW;  // Lower Flush flag, done flushing, Now Entering Sample Mode
      setPump(0);       // Turn off pump
      flushOFF();        // Turn off flush valve here

      configuration.VNum++; // Increment Valve number
      configuration.written = eepromValidationValue; // ensure we remember we've written new value to EEPROM
      EEPROM_writeAnything(0, configuration); // SAVE new Valve Number in EEPROM

      Serial.print("moving onto sampling Valve number "); Serial.println(configuration.VNum);

      // Configure TPIC buffers according to current valve, strobe out to SPI
      setValveBits();
      // Send MSB of Array first, which is really LSB of the actual Valve State
      Serial.print("TPICBuffer3: "); Serial.println(TPICBuffer[3]);
      Serial.print("TPICBuffer2: "); Serial.println(TPICBuffer[2]);
      Serial.print("TPICBuffer1: "); Serial.println(TPICBuffer[1]);
      Serial.print("TPICBuffer0: "); Serial.println(TPICBuffer[0]);

      setPump(1);   // Turn on pump, draw sample in
    }
    else if ((SampleState == HIGH) && ((currentMillis - previousMillis) >= (configuration.FDMs + configuration.SDMs)))
    { // Exit Sample Mode and enter Wait Mode
      SampleState = LOW;  // Turn Sample off, lower flag for loop

      // Also Turn off sample valve here
      Serial.println("Finished sampling, turning off motors and sample valve");
      everythingOff(); // Turn everything off
      digitalWrite(LED_BUILTIN, SampleState); // indicator LED

      //clear the alarm
      RTC.armAlarm(1, false);
      RTC.clearAlarm(1);
      RTC.alarmInterrupt(1, false);

      if(configuration.Is_Daily)
      {
        RTC.setAlarm(ALM1_MATCH_HOURS, configuration.SAMin, configuration.SAHr, 0);   //set your wake-up time here
        Serial.print("Resetting Alarm for next day at ");
        Serial.print(configuration.SAHr); Serial.print(":"); 
      }
      else
      {
        RTC.setAlarm(ALM1_MATCH_MINUTES, configuration.SAMin, configuration.SAHr, 0);   //set your wake-up time here
        Serial.print("Resetting Alarm for next hour at ");
      }
      Serial.println(configuration.SAMin);
      
      // Allow wake up pin to trigger interrupt on low.
      attachInterrupt(digitalPinToInterrupt(wakeUpPin), wakeUp, FALLING);
      // Clear Timer EN
      timerEN = false; // dissable timed functions
      // Enter power down state with ADC and BOD module disabled.
      // Wake up when wake up pin via RTC is exerted low.
      sleepEN = true; // Set sleep flag to sleep at end of loop 

    } // End Sample cycle
    
  }  // End Sample State-machine

 // listen for serial:
  listenForSerial();
 // Should we go to sleep?
 sleepcheck();
} // End Loop

// ================================================================
// Timer ISR, Execute every sample period
// ================================================================

void wakeUp()
{
  // Disable external pin interrupt on wake up pin.
  //detachInterrupt(digitalPinToInterrupt(wakeUpPin));
  timerEN = true; // Enable timed functions
  sleepEN = false; // dissable sleep flag in loop
}// end ISR

// ================================================================
// Switch Pin Interrupt Service Routine
// Switch pin is LOW-True Logic, GND == enabled
// If switch off, dissable timed events
// In on, enable timed events
// ================================================================
void sampleEN()
{
    Serial.println("Sampler EN function");
  
    bool intPinState = digitalRead(interruptPin);
    if (!intPinState) // Low-true
    {
      Serial.println("Sampler timed functions enabled");
      // Allow wake up pin from RTC to trigger pin 3 interrupt on low.
      attachInterrupt(digitalPinToInterrupt(wakeUpPin), wakeUp, FALLING);
      sleepEN = true; // set sleep flag to enable sleep at end of loop
    }
    else
    {
      // Disable external pin interrupt from RTC on wake up pin.
      detachInterrupt(digitalPinToInterrupt(wakeUpPin));
      Serial.println("Sampler timed functions disabled");
      Serial.println("Processor AWAKE and standing by for serial commands");
//      everythingOff(); // Turn everything off
      sleepEN = false; //dissable sleep flag
      // We'll just sit awake in loop listening to the serial port
    }
}

// ================================================================
// Other Functions
// ================================================================
void sleepcheck()
{
  if(sleepEN)
  {
    Serial.println("Going to sleep");
    delay(200); // wait for serial to complete printing
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); 
  }
}

// Restore OPEnS Lab Factory Defaults
void writeEEPROMdefaults()
{
  // Disable external pin interrupt on wake up pin.
  detachInterrupt(digitalPinToInterrupt(wakeUpPin));
  
  // EEPROM never written or factory default reset serial received "RST"
  Serial.print("Writing EEPROM defaults. . .");

  configuration.SAMin = SampleAlarmMinDef;
  configuration.SAHr = SampleAlarmHrDef;
  configuration.FDMs = FlushDurMsDef;
  configuration.SDMs = SampleDurMsDef;
  configuration.SVml = SampleVolMlDef;
  configuration.VNum = SampleValveNumDef;
  configuration.written = 0;
  configuration.Is_Daily = 1; // set daily flag in configuration
//  configuration.Is_Hourly = 0; // clear hourly flag in configuration

  // Save Defaults into EEPROM
  EEPROM_writeAnything(0, configuration);
  
// set RTC timer here:
   RTC.setAlarm(ALM1_MATCH_HOURS, configuration.SAMin, configuration.SAHr, 0);
  Serial.println("Done");
  // Allow wake up pin to trigger interrupt on low.
  attachInterrupt(digitalPinToInterrupt(wakeUpPin), wakeUp, FALLING);
}



