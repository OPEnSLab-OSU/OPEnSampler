/*
  OPEnS Water sampler Firmware Version 1.9
  Chet Udell, Mitch Nelke, 2017

  New in this version: 
  Added re-send courtesy valve signal at periodic rate during sampling because noise from brushed motor may interrupt this signal if sent only once at begining
  Optomize RTC, "CLK" serial command now expects [ CLK day month year hr min ] for example: CLK 13 6 1983 13 20  will set clock to 13 June 1983 1:20PM
  Also set next sample alarm at the beginning of each sample cycle instead of at end
  - so duration of sample does not affect the sample schedule
  (e.g. a sample period of 10 min will always take place every 10min instead of 10min + [sample time])

    To Do:
    Add OLED Menu with Buttons to program sampler in-field and see status
    Add GSM for live updates

SamplerSerialEverythingVx.x is the main Arduino Sketch file. Other dependent tabs include:
EEPROMAnything.h
SPIfunctions.ino
SerialCommandDefs.ino
ValveAddressing.ino

Dependencies:
Saving Sampler params in EEPROM, include files from here:
http://playground.arduino.cc/Code/EEPROMWriteAnything

You may need to download RTCLibExtended:
https://github.com/FabioCuomo/FabioCuomo-DS3231

And Sparkfun Low Power Library:
https://github.com/rocketscream/Low-Power/archive/master.zip

RTC and RTCLib-extended tutorial/reference:
http://www.instructables.com/id/Arduino-Sleep-and-Wakeup-Test-With-DS3231-RTC/

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
24 sampler bags, 1 flush valve, 2 peristaltic pumps.
It takes about 2 min to draw 250ml, or 480ms per ml
1 Power Switch, 1 Sampler enable switch (because we want independent control of the timed sampler functions without cutting power, features that use this will be explained).
Serial Command Set, You can communicate with the sampler over USB serial at 9600 Baud. 

Arduino Pinouts:
GPIO2   - Sample enable/dissable low-true
GPIO3   - RTC Interrupt, low-true
GPIO8   - pump pin 1 (motor driver 1) *was GPIO09
GPIO9  - pump pin 2 (motor driver 2)  *was GPIO10
~ SPI ~
GPIO10 - rClock - TPIC register clock  *was GPIO08
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

#include <Wire.h> // i2c connection for RTC DS3231
#include <RTClibExtended.h> // Library to configure DS3231
#include "LowPower.h" // Use Low-Power library from Sparkfun and RTC pin interrupt (P3) to manage sleep and scheduling at same time
#include <EEPROM.h>     // Will be writing params to non-volatile memory to save between uses
#include "EEPROMAnything.h" // EEPROM read/write functions
#include <SPI.h>        // Using SPI hardware to communicate with TPICs

#define NUM_PHONES 30 // Max number of phone numbers to store in configuration

const byte eepromValidationValue = 99; // Value to test to see if EEPROM has been written before
// Determine Sampler "Factory Default" behaviour. These can be changed and saved using the SerialCommandDefs (see tab with same label)
// If never set before, default sample time is once per day, at 8AM (08:00)
const uint8_t SampleAlarmMinDef = 00; // Default sample time: Min
const uint8_t SampleAlarmHrDef = 8; // Default sample time: Hr
const uint16_t SampleAlarmPerDef = 3; // Default sample period time, 3 min for testing, make longer for factory release

const uint16_t SampleDurMsDef = 30000; // Factory Default 30 sec, takes 60 seconds for 250ml so - don't overfill, How long pumps should run to take one sample, in ms
const uint16_t FlushDurMsDef = 30000; // Factory default, low long in ms to flush system

const uint8_t SampleVolMlDef = 250;  // Factory Default, sample vol in ml, 250ml takes about 2min, or about 480ms per ml
const bool SampleValveNumDef = 0; // Factory Default, current valve number

// Switch Interrupt pin is LOW-True Logic, GND == enabled
const byte interruptPin = 2;  // Digital pin switch is attached to that enables or dissables the sampler timer
const byte wakeUpPin = 3; // Use pin 3 as wake up pin
const byte pumpPin1 = 8; // Motor MOSFET attached to digital pin 9,  1=forward, 0=reverse
const byte pumpPin2 = 9; // Motor MOSFET attached to digital pin 10 0=forward, 1=reverse
signed int directionMotor = 0; // 0=off, 1=draw water in, -1=pull water out

volatile bool ledState = 0;
volatile bool timerEN = false; // Flag to enable or dissable sampler timed functions
volatile bool sleepEN = false; // Flag to check in loop to see if we should sleep or not
// These maintain the current state
volatile bool SampleState = LOW; // SampleState used to set valve and motor states for taking new sample
volatile bool FlushState = LOW;  // used to set state-machine flush flag

volatile unsigned long previousMillis = 0;   // will store last time Sample was taken

int value = 0; // Place to accumulate valve number input (see SerialCommandDefs tab)

// Struct for saving Sampler params in EEPROM, see http://playground.arduino.cc/Code/EEPROMWriteAnything
struct config_t
{
  uint8_t SAMin; // Sample alarm time Min
  uint8_t SAHr; // Sample alarm time Hr
  uint16_t SAPer; // Sample period Time
  unsigned long FDMs; // Flush duration in ms
  unsigned long SDMs;  // Sample Duration in ms
  uint16_t SVml; // Sample volume in ml
  uint8_t VNum;  // Current Valve number sampled, 0=reset/default
  byte written;  // Has the EEPROM Memory been written before?
  bool Is_Daily; // alarm mode set 1 for daily sample, 0 for periodic sample?
  char phones[NUM_PHONES][16]; // Phone numbers for SMS status update recipients
} configuration;

//----------------------------
// Valve Addressing Variables
//----------------------------
const int numValves = 24; // how many valves in each module
int numModules = 1; // how many modules? 1 master module for now. Will be used to calculate number of shifts to TPICs for expansion modules
unsigned char TPICBuffer[4] = {0x00}; // Store Status bits of TPICs, see ValveAddressing tab for details
// uint16_t valveCount = 0;
uint8_t moduleNum = 0;  // number of module depending on how high valve count is (groups of 25)
uint8_t valveNum = 0;  // number of valve relative to current module

//----------------------------
// SPI Variables
//----------------------------
const byte sclock = 13; // SPI SCK pin 13
const byte rclock = 10; // Register clock pin, acts as CS, could be moved to SPI SS pin 10?
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

  Serial.print(F("the current Flush duration in ms is: "));
  Serial.println(configuration.FDMs);
  Serial.print(F("the current Sample duration in ms is: "));
  Serial.println(configuration.SDMs);
  Serial.print(F("Next bag to sample is: "));
  Serial.println((configuration.VNum+1)); // add 1 to this number for the actual bag number
  
// Enable and Config SPI hardware:
  pinMode(rclock, OUTPUT);
  SPI.begin();
  SPISettings(16000000, MSBFIRST, SPI_MODE1);
// RTC Timer settings here
  if (! RTC.begin()) {
    Serial.println(F("Couldn't find RTC"));
    while (1);
  }

// This may end up causing a problem in practice - what if RTC looses power in field? Shouldn't happen with coin cell batt backup
  if (RTC.lostPower()) {
    Serial.println(F("RTC lost power, lets set the time!"));
  // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  
  RTCReportTime(); // print current RTC time
  
  //clear any pending alarms
  clearAlarms();

  //Set SQW pin to OFF (in my case it was set by default to 1Hz)
  //The output of the DS3231 INT pin is connected to this pin
  //It must be connected to arduino D2 pin for wake-up
  RTC.writeSqwPinMode(DS3231_OFF);
// Now set Alarm based on whether Mode flag Is_Daily (1) or Periodic (0)
if(configuration.Is_Daily) // is daily flag set?
{
  //Set alarm1 every day at Hr:Mn
  // You may use different Alarm types for different periods (e.g. ALM1_MATCH_MINUTES to go off every hr)
  // RTC.setAlarm(ALM1_MATCH_MINUTES, configuration.SAMin, 0, 0); // This might work to go off every hour at specified minute every hour
  RTC.setAlarm(ALM1_MATCH_HOURS, configuration.SAMin, configuration.SAHr, 0);   //set your wake-up time here
  Serial.print(F("the current sample Alarm set for Daily at"));
  Serial.print(configuration.SAHr); Serial.print(F(":")); Serial.println(configuration.SAMin);
}
else  // is Periodic
{
  //Set alarm1 every hr at Mn
  setAlarmPeriod(); // found at bottom of SerialCommandDefs tab
}
  // Reset pump and all valves to off
  Serial.print(F("resetting electronics . . ."));
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
  Serial.println(F("Exit Sleep Mode . . ."));
  Serial.print(F("Woke up at "));
  DateTime now = RTC.now();
  Serial.print(now.hour(), DEC); Serial.print(':'); Serial.print(now.minute(), DEC); Serial.print(':'); Serial.print(now.second(), DEC); Serial.println();
 // Disable external pin interrupt on wake up pin.
  detachInterrupt(digitalPinToInterrupt(wakeUpPin)); // Disable external pin interrupt for switch until finished sampling
 
  if (configuration.VNum >= numValves) // If target valve number is greater than total number of desired valves
  {
    Serial.println(F("Total number of samples reached!"));
    Serial.println(F("Sleeping forever, bye!"));
    // warning, because wakeup pin is disabled above, sleep forever, no wakeup from here
    sleepEN = true; // Set sleep flag to sleep at end of loop 
  }
  else
  {
    // Disable external pin interrupt on wake up pin.
    detachInterrupt(digitalPinToInterrupt(interruptPin));

    //clear the alarm
    clearAlarms();
    //set alarm for next cycle
    if(configuration.Is_Daily)
    {
      RTC.setAlarm(ALM1_MATCH_HOURS, configuration.SAMin, configuration.SAHr, 0);   //set your wake-up time here
      Serial.print(F("Resetting Alarm for next day at "));
      Serial.print(configuration.SAHr); Serial.print(F(":")); Serial.println(configuration.SAMin);
    }
    else
    {
      setAlarmPeriod();
    }
     
    
    Serial.println(F("time to take a sample"));
    previousMillis = millis();  // Remember the time at this point
    SampleState = HIGH; // Trigger new sample cycle, raise sample flag for Loop
    FlushState = HIGH; // Begin sample cycle by flushing system for period of time

    // Now that we have the module number and valve number. Set TPICBuffer
    clearValveBits();  // Clear Valve Control bits, don't override Flush bit
    flushON(); // Turn Flush Valve on, Turn on Motors

    //Turn Motors on to flush system
    setPump(SampleState); // Turn on Pump
    //digitalWrite(LED_BUILTIN, SampleState);
  }
  timerEN = false; // dissable timed functions
}
else if (SampleState) // Will be TRUE if new sample alarm ISR has been triggered
  { // Enter Flush mode. . .
    // check to see if it's time to start or stop this newly triggered sample
    unsigned long currentMillis = millis();
    // Begin sample cycle by flushing for period of time, Flush already on by this point ( see sampleTrig() )

    if ((FlushState == HIGH) && (currentMillis - previousMillis >= configuration.FDMs))
    { // End Flush, enter Sample Mode
      Serial.print(F("Flushed System for ")); Serial.print(configuration.FDMs); Serial.println(F("ms"));
      FlushState = LOW;  // Lower Flush flag, done flushing, Now Entering Sample Mode
      setPump(0);       // Turn off pump
 //     flushOFF();        // Turn off flush valve here

      configuration.VNum++; // Increment Valve number
      configuration.written = eepromValidationValue; // ensure we remember we've written new value to EEPROM
      EEPROM_writeAnything(0, configuration); // SAVE new Valve Number in EEPROM

      Serial.print(F("moving onto sampling Valve number ")); Serial.println(configuration.VNum);

      // Configure TPIC buffers according to current valve, strobe out to SPI
      setValveBits();    // need to do here, else Pump won't turn on (valve must be open for pump on)
      flushOFF();        // Turn off flush valve here
      // Send MSB of Array first, which is really LSB of the actual Valve State
      Serial.print(F("TPICBuffer3: ")); Serial.println(TPICBuffer[3]);
      Serial.print(F("TPICBuffer2: ")); Serial.println(TPICBuffer[2]);
      Serial.print(F("TPICBuffer1: ")); Serial.println(TPICBuffer[1]);
      Serial.print(F("TPICBuffer0: ")); Serial.println(TPICBuffer[0]);

      setPump(1);   // Turn on pump, draw sample in
    }
    else if ((SampleState == HIGH) && (FlushState == LOW) && ((currentMillis - previousMillis) < (configuration.FDMs + configuration.SDMs))) // If Sample is being actively drawn, this will be true
    { 
      delay(1000);
      flushOFF();
      setValveBits();  // Resend Valve signal at periodic rate because noise from brushed motor may turn this off
      Serial.println(F("Re-sent Courtesy Valve Signal"));
    }
    else if ((SampleState == HIGH) && ((currentMillis - previousMillis) >= (configuration.FDMs + configuration.SDMs)))
    { // Exit Sample Mode and enter Wait Mode
      SampleState = LOW;  // Turn Sample off, lower flag for loop

      // Also Turn off sample valve here
      Serial.println(F("Finished sampling, turning off motors and sample valve"));
      everythingOff(); // Turn everything off
      digitalWrite(LED_BUILTIN, SampleState); // indicator LED

      // Allow wake up pin to trigger interrupt on low.
      attachInterrupt(digitalPinToInterrupt(wakeUpPin), wakeUp, FALLING);
      // Allow switch pin to change sampler mode
      attachInterrupt(digitalPinToInterrupt(interruptPin), sampleEN, CHANGE);

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
    Serial.println(F("Sampler EN function"));
  
    bool intPinState = digitalRead(interruptPin);
    if (!intPinState) // Low-true
    {
      Serial.println(F("Sampler timed functions enabled"));
     // RTC.setAlarm(ALM1_MATCH_HOURS, configuration.SAMin, configuration.SAHr, 0);
     // Serial.print(F("Resetting Alarm 1 for: ")); Serial.print(configuration.SAHr); Serial.print(F(":"));Serial.println(configuration.SAMin);
     // clearAlarms();
      // Allow wake up pin from RTC to trigger pin 3 interrupt on low.
      attachInterrupt(digitalPinToInterrupt(wakeUpPin), wakeUp, FALLING);
      sleepEN = true; // set sleep flag to enable sleep at end of loop
    }
    else
    {
      // Disable external pin interrupt from RTC on wake up pin.
      detachInterrupt(digitalPinToInterrupt(wakeUpPin));
      Serial.println(F("Sampler timed functions disabled"));
      Serial.println(F("Processor AWAKE and standing by for serial commands"));
//      everythingOff(); // Turn everything off
      sleepEN = false; //dissable sleep flag
      SampleState = false; // dissable sample flag
      timerEN = false; // dissable timed functions flag
      
      // We'll just sit awake in loop listening to the serial port
    }
}

// ================================================================
// Other Functions
// ================================================================
void clearAlarms()
{
//clear any pending alarms
  RTC.armAlarm(1, false);
  RTC.clearAlarm(1);
//  RTC.alarmInterrupt(1, false);
//  RTC.armAlarm(2, false);
//  RTC.clearAlarm(2);
//  RTC.alarmInterrupt(2, false);
}

void sleepcheck()
{
  if(sleepEN)
  {
    Serial.println(F("Going to sleep"));
    everythingOff(); // Turn everything off
    delay(200); // wait for serial to complete printing
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); 
  }
}

void RTCReportTime()
{
  // Report time and next alarm back
  Serial.println(F("RTC time is"));
  DateTime now = RTC.now();
  Serial.print(now.hour(), DEC); Serial.print(':'); Serial.print(now.minute(), DEC); Serial.print(':'); Serial.print(now.second(), DEC); Serial.println();
}

// Restore OPEnS Lab Factory Defaults
void writeEEPROMdefaults()
{
  // Disable external pin interrupt on wake up pin.
  detachInterrupt(digitalPinToInterrupt(wakeUpPin));
  
  // EEPROM never written or factory default reset serial received "RST"
  Serial.print(F("Writing EEPROM defaults. . ."));

  configuration.SAMin = SampleAlarmMinDef;
  configuration.SAHr = SampleAlarmHrDef;
  configuration.SAPer = SampleAlarmPerDef;
  configuration.FDMs = FlushDurMsDef;
  configuration.SDMs = SampleDurMsDef;
  configuration.SVml = SampleVolMlDef;
  configuration.VNum = SampleValveNumDef;
  configuration.written = 0;
  configuration.Is_Daily = 1; // set daily flag in configuration

  for (int i = 0; i < NUM_PHONES; i++) {
    configuration.phones[i][0] = '\0';
  }

  // Save Defaults into EEPROM
  EEPROM_writeAnything(0, configuration);
  
// set RTC timer here:
  RTC.setAlarm(ALM1_MATCH_HOURS, configuration.SAMin, configuration.SAHr, 0);
  Serial.println(F("Done"));
  // Allow wake up pin to trigger interrupt on low.
  attachInterrupt(digitalPinToInterrupt(wakeUpPin), wakeUp, FALLING);
}



