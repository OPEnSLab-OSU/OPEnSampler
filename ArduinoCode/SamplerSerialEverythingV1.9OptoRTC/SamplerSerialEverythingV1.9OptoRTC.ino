/* OPEnS Water Sampler
  Chet Udell, Mitch Nelke

SamplerSerialEverythingVx.x is the main Arduino Sketch file.

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
Serial Command Set, You can communicate with the sampler over USB serial at baud rate specified in Defaults.h

Arduino Pinouts:
GPIO2  - Sample enable/disable low-true
GPIO3  - RTC Interrupt, low-true
GPIO8  - pump pin 1 (motor driver 1)
GPIO9  - pump pin 2 (motor driver 2)
~ SPI ~
GPIO10 - rClockPin - TPIC register clock
GPIO11 - SPI MOSI - TPIC Data
GPIO12 - SPI MISO
GPIO13 - SPI SCLK - TPIC serial clock
~ i2c ~
SCL/SDA = to RTC SCL/SDA

Sampler Use:r
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

#include <Wire.h>              // i2c connection for RTC DS3231
#include <RTClibExtended.h>    // Library to configure DS3231
#include "LowPower.h"          // Use Low-Power library from Sparkfun and RTC pin interrupt (P3) to manage sleep and scheduling at same time
#include <EEPROM.h>            // Will be writing params to non-volatile memory to save between uses
#include <SPI.h>               // Using SPI hardware to communicate with TPICs

#include "CommandParser.h"
#include "Configuration.h"
#include "Defaults.h"
#include "Globals.h"
#include "ValveAddressing.h"

Configuration config;

//----------------------------
// Pins
//----------------------------
// Switch Interrupt pin is LOW-True Logic, GND == enabled
const byte interruptPin = 2;  // Digital pin switch is attached to that enables or disables the sampler timer

// TODO: Not currently a proper interrupt pin, as PinChangeInterrupt conflicts with SoftwareSerial...
const byte wakeUpPin = A0; // Pin to recieve RTC's wake up interrupt.

const byte pumpPin1 = 8; // Motor MOSFET 1=forward, 0=reverse
const byte pumpPin2 = 9; // Motor MOSFET 0=forward, 1=reverse
const byte rClockPin = 10; // TPIC register clock pin, acts as SPI CS (chip select)

//----------------------------
// Bluetooth Pins & Serial
//----------------------------
const byte bleReqPin = A1; // SPI CS (chip select) pin
const byte bleRdyPin = 3;  // Interrupt pin for when data is ready // TODO: See wakeUpPin, swap as needed
const byte bleRstPin = A2; // Used to reset board on startup

//----------------------------
// GSM/SMS Breakout Pins
//----------------------------
const byte fonaRXPin  = 5;
const byte fonaTXPin  = 6;
const byte fonaRstPin = 4;
//----------------------------

volatile bool timerEN = false; // Flag to enable or disable sampler timed functions
volatile bool sleepEN = false; // Flag to check in loop to see if we should sleep or not
// These maintain the current state
volatile bool SampleState = LOW; // SampleState used to set valve and motor states for taking new sample
volatile bool FlushState = LOW;  // used to set state-machine flush flag

volatile unsigned long previousMillis = 0; // will store last time Sample was taken

//---------------------------
// Valve Addressing Variables
//---------------------------
unsigned char TPICBuffer[4] = {0x00}; // Store status bits of TPICs, see ValveAddressing for details

//----------------------------------
// Bluetooth Serial & Command Parser
//----------------------------------
CommandParser BLEParser(',', '|');
Adafruit_BLE_UART BLESerial = Adafruit_BLE_UART(bleReqPin, bleRdyPin, bleRstPin);

//--------------------------------
// FONA 808 for SMS Status Updates
//--------------------------------
#if FONA_ENABLED
SoftwareSerial fonaSS = SoftwareSerial(fonaTXPin, fonaRXPin);
SoftwareSerial *fonaSerial = &fonaSS;
Adafruit_FONA fona = Adafruit_FONA(fonaRstPin);
#endif
//--------------------------------

RTC_DS3231 RTC;

void setup() {
  Serial.begin(baud);
  while (!Serial)
    delay(1000);
  Serial.println(F("Beginning setup."));

  pinMode(interruptPin, INPUT_PULLUP);
  pinMode(wakeUpPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), sampleEN, CHANGE);

  // Read EEPROM into Configuration
  config.readFromEEPROM();

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(pumpPin1, OUTPUT); // Set motor pin1 to output
  digitalWrite(pumpPin1, LOW); // Turn off Motor
  pinMode(pumpPin2, OUTPUT); // Set motor pin2 to output
  digitalWrite(pumpPin2, LOW); // Turn off Motor

  if (!config.getWritten())
    writeEEPROMDefaults();

  BLESerial.setRXcallback(RXCallback);
  BLESerial.setACIcallback(ACICallback);
  BLESerial.setDeviceName((const char*) F("Sampler")); // Can be no longer than 7 characters
  BLESerial.begin();
  Serial.println(F("Bluetooth initialized."));

  // FONA 808 Setup (for SMS status updates)
#if FONA_ENABLED
    fonaSerial->begin(baud);
    if (! fona.begin(*fonaSerial)) {
      Serial.println(F("ERROR: Couldn't find FONA..."));
    }
#endif

  // Enable and Config SPI hardware:
  pinMode(rClockPin, OUTPUT);
  SPI.begin();
  SPISettings(16000000, MSBFIRST, SPI_MODE1);

  // RTC Timer settings here
  if (! RTC.begin()) {
    Serial.println(F("Couldn't find RTC. Hanging..."));
    while (true);
  }

  Serial.print(F("Flush duration (ms) is: "));
  Serial.println(config.getFlushDuration());
  Serial.print(F("Sample duration (ms) is: "));
  Serial.println(config.getSampleDuration());

  Serial.print(F("Next bag to sample is: "));
  Serial.println((config.getValveNumber() + 1)); // add 1 for the next to be sampled

  // This may end up causing a problem in practice - what if RTC loses power in field?
  // Shouldn't happen with coin cell batt backup
  if (RTC.lostPower()) {
    Serial.println(F("RTC lost power, setting time."));
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  RTCReportTime(); // print current RTC time

  clearAlarms();

  //Set SQW pin to OFF (in my case it was set by default to 1Hz)
  //The output of the DS3231 INT pin is connected to this pin
  //It must be connected to wakeUpPin
  RTC.writeSqwPinMode(DS3231_OFF);
  // Now set alarm based on mode flag (daily or periodic)
  if(config.getMode() == Mode::DAILY)
  {
    //Set alarm1 every day at Hr:Mn
    // You may use different Alarm types for different periods (e.g. ALM1_MATCH_MINUTES to go off every hr)
    // RTC.setAlarm(ALM1_MATCH_MINUTES, config.getSampleMinute(), 0, 0); // This might work to go off every hour at specified minute every hour
    RTC.setAlarm(ALM1_MATCH_HOURS, config.getSampleMinute(), config.getSampleHour(), 0);
    Serial.print(F("the current sample Alarm set for Daily at"));
    Serial.print(config.getSampleHour()); Serial.print(F(":")); Serial.println(config.getSampleMinute());
  }
  else if (config.getMode() == Mode::PERIODIC)
  {
    config.refreshPeriodicAlarm();
  }
  // Reset pump and all valves to off
  Serial.print(F("resetting electronics ..."));
  everythingOff();
  delay(1000); // 1s

  // Attach alarm interrupt
  RTC.alarmInterrupt(1, true);

  // Read switch pin to enable or disable sampler on startup
  sampleEN();
}

void loop()
{
  if(timerEN)
  {
    Serial.println(F("Exiting Sleep Mode ..."));
    Serial.print(F("Woke up."));
    RTCReportTime();

    // Disable RTC's wakeup interrupt pin
    detachInterrupt(digitalPinToInterrupt(wakeUpPin));

    // If target valve number is greater than total number of valves
    if (config.getValveNumber() >= numValves)
    {
      Serial.println(F("Total number of samples reached! Sleeping forever..."));
      // Because wakeup pin is disabled above, sleep forever, no wakeup from here

#if FONA_ENABLED
      sendSMSAll((const char *) F("OPEnSampler says: HELLO WORLD"));
#endif

      sleepEN = true; // Set sleep flag to sleep at end of loop
    }
    else
    {
      // Disable external pin interrupt on wake up pin.
      detachInterrupt(digitalPinToInterrupt(interruptPin));

      clearAlarms();
      //set alarm for next cycle
      if(config.getMode() == Mode::DAILY)
      {
        RTC.setAlarm(ALM1_MATCH_HOURS, config.getSampleMinute(), config.getSampleHour(), 0);
        Serial.print(F("Resetting Alarm for next day at "));
        Serial.print(config.getSampleHour()); Serial.print(F(":")); Serial.println(config.getSampleMinute());
      }
      else if (config.getMode() == Mode::PERIODIC)
      {
        config.refreshPeriodicAlarm();
      }

      Serial.println(F("Taking a water sample."));
#if FONA_ENABLED
      sendSMSAll((const char *) F("OPEnSampler says: Taking a sample!"));
#endif

      previousMillis = millis();  // Remember the time at this point
      SampleState = HIGH; // Trigger new sample cycle, raise sample flag for Loop
      FlushState = HIGH; // Begin sample cycle by flushing system for period of time

      // Now that we have the module number and valve number. Set TPICBuffer
      clearValveBits();  // Clear Valve Control bits, don't override Flush bit
      flushON(); // Turn Flush Valve on, Turn on Motors

      //Turn Motors on to flush system
      setPump(pumpState::ON);
      //digitalWrite(LED_BUILTIN, SampleState);
    }
    timerEN = false; // disable timed functions
  }
  else if (SampleState) // Will be TRUE if new sample alarm ISR has been triggered
  { // Enter Flush mode. . .
    // check to see if it's time to start or stop this newly triggered sample
    unsigned long elapsedMilliseconds = millis() - previousMillis;
    // Begin sample cycle by flushing for period of time, Flush already on by this point ( see sampleTrig() )

    if ((FlushState == HIGH) && (elapsedMilliseconds >= config.getFlushDuration()))
    { // End Flush, enter Sample Mode
      Serial.print(F("Flushed System for ")); Serial.print(config.getFlushDuration()); Serial.println(F("ms"));
      FlushState = LOW; // Lower Flush flag, done flushing, Now Entering Sample Mode
      setPump(pumpState::OFF);

      config.setValveNumber(config.getValveNumber() + 1);
      config.writeToEEPROM(); // Save new valve number to EEPROM

      Serial.print(F("Moving onto sampling valve number "));
      Serial.println(config.getValveNumber());

      // Configure TPIC buffers according to current valve, strobe out to SPI
      setValveBits();    // need to do here, else Pump won't turn on (valve must be open for pump on)
      flushOFF();        // Turn off flush valve here
      // Send MSB of Array first, which is really LSB of the actual Valve State
      Serial.print(F("TPICBuffer3: ")); Serial.println(TPICBuffer[3]);
      Serial.print(F("TPICBuffer2: ")); Serial.println(TPICBuffer[2]);
      Serial.print(F("TPICBuffer1: ")); Serial.println(TPICBuffer[1]);
      Serial.print(F("TPICBuffer0: ")); Serial.println(TPICBuffer[0]);

      setPump(pumpState::ON); // Turn on pump, draw sample in
    }
    // If a sample is being actively drawn
    else if ((SampleState == HIGH) && (FlushState == LOW) && (elapsedMilliseconds
              < (config.getFlushDuration() + config.getSampleDuration())))
    {
      delay(1000);
      flushOFF();
      setValveBits();  // Resend Valve signal at periodic rate because noise from brushed motor may turn this off
      Serial.println(F("Re-sent Courtesy Valve Signal"));
    }
    else if ((SampleState == HIGH) && (elapsedMilliseconds
              >= (config.getFlushDuration() + config.getSampleDuration())))
    { // Exit Sample Mode and enter Wait Mode
      SampleState = LOW;  // Turn Sample off, lower flag for loop

      // Also Turn off sample valve here
      Serial.println(F("Finished sampling, turning off motors and sample valve"));
      everythingOff();
      digitalWrite(LED_BUILTIN, SampleState); // indicator LED

      // Allow wake up pin to trigger interrupt on low.
      attachInterrupt(digitalPinToInterrupt(wakeUpPin), wakeUp, FALLING);
      // Allow switch pin to change sampler mode
      attachInterrupt(digitalPinToInterrupt(interruptPin), sampleEN, CHANGE);

      // Clear Timer EN
      timerEN = false; // disable timed functions
      // Enter power down state with ADC and BOD module disabled.
      // Wake up when wake up pin via RTC is exerted low.
      sleepEN = true; // Set sleep flag to sleep at end of loop

    } // End Sample cycle

  }  // End Sample State-machine

  listenForSerial();
  BLESerial.pollACI();

 // Should we go to sleep?
 sleepcheck();
} // End Loop

// ================================================================
// Timer ISR, Execute every sample period
// ================================================================

void wakeUp()
{
  // Disable external pin interrupt on wake up pin.
  //detachInterrupt(digitalPinInterrupt(wakeUpPin));
  timerEN = true; // Enable timed functions
  sleepEN = false; // disable sleep flag in loop
}

// ================================================================
// Switch Pin Interrupt Service Routine
// Switch pin is LOW-True Logic, GND == enabled
// If switch off, disable timed events
// In switch on,  enable  timed events
// ================================================================
void sampleEN()
{
    bool intPinState = digitalRead(interruptPin);
    if (!intPinState) // Low-true
    {
      Serial.println(F("Timed functions enabled."));
      // RTC.setAlarm(ALM1_MATCH_HOURS, config.getSampleMinute(), config.getSampleHour(), 0);
      // Serial.print(F("Resetting Alarm 1 for: ")); Serial.print(config.getSampleHour()); Serial.print(F(":"));Serial.println(config.getSampleMinute());
      // clearAlarms();

      // Allow wake up pin from RTC to trigger interrupt on low.
      attachInterrupt(digitalPinToInterrupt(wakeUpPin), wakeUp, FALLING);
      sleepEN = true; // set sleep flag to enable sleep at end of loop
    }
    else
    {
      // Disable external pin interrupt from RTC on wake up pin.
      detachInterrupt(digitalPinToInterrupt(wakeUpPin));
      Serial.println(F("Timed functions disabled."));
      Serial.println(F("Processor AWAKE and standing by for serial commands"));
      // everythingOff(); // Turn everything off
      sleepEN = false;
      SampleState = false;
      timerEN = false; // disable timed functions

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
    everythingOff();
    delay(200); // wait for serial to complete printing
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
  }
}

/**
 * Set the real time clock and updates the sample alarm.
 *
 * Sets the real time clock to the specified year, month, day, hour, and minute.
 * Then resets the daily or periodic alarm to use the updated time.
 */
void setClock(unsigned int year, unsigned int month, unsigned int day, unsigned int hour, unsigned int minute)
{
  RTC.adjust(DateTime(year, month, day, hour, minute, 0));

  if (config.getMode() == Mode::PERIODIC)
    config.refreshPeriodicAlarm();
  else if (config.getMode() == Mode::DAILY)
    RTC.setAlarm(ALM1_MATCH_HOURS, config.getSampleMinute(), config.getSampleHour(), 0);

  RTCReportTime(); // print current RTC time
  Serial.print(F("Next Sample Alarm set for: "));
  Serial.print(config.getSampleHour());
  Serial.print(F(":"));
  Serial.println(config.getSampleMinute());
}

/**
 * Print out the RTC's current time to Serial.
 */
void RTCReportTime()
{
  // Report time and next alarm back
  Serial.println(F("RTC time is"));
  DateTime now = RTC.now();
  Serial.print(now.hour(), DEC); Serial.print(':'); Serial.print(now.minute(), DEC); Serial.print(':'); Serial.print(now.second(), DEC); Serial.println();
}

/**
 * Set configuration to default values and write them to EEPROM.
 *
 * Overwrites the configuration with default values, writes them to
 * persistent storage in EEPROM, and resets the sample alarm to use the default
 * values. The RTC's wakeup interrupt is detached and reattached before and after
 * these operations.
 */
void writeEEPROMDefaults()
{
  detachInterrupt(digitalPinToInterrupt(wakeUpPin));
  Serial.println(F("Writing EEPROM defaults..."));

  config.setDefaults();
  config.writeToEEPROM();
  Serial.println(F("Finished writing to EEPROM."));

  uint8_t minute = config.getSampleMinute();
  uint8_t hour   = config.getSampleHour();

  attachInterrupt(digitalPinToInterrupt(wakeUpPin), wakeUp, FALLING);
}

/**
 * Send configuration data struct over Bluetooth Low Energy.
 */
void sendConfigOverBluetooth(Configuration config)
{
  const size_t bytesToSend = sizeof(config_t);
  size_t totalSent = 0;
  uint8_t configData[bytesToSend];

  config.getConfigData(configData);

  while (totalSent < bytesToSend) {
    size_t sent = 0;

    sent = BLESerial.write(configData, bytesToSend);
    totalSent += sent;

    if (sent == 0 && totalSent < bytesToSend) {
      Serial.println(F("ERROR: BLE library failed to send full configuration."));
      break;
    }
  }
}
