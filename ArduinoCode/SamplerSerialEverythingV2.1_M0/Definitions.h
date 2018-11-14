//Definitions.h

/*Arduino Pinouts:
  GPIO2   - Sample enable/disable low-true
  GPIO3   - RTC Interrupt, low-true
  GPIO8   - pump pin 1 (motor driver 1) *was GPIO09
  GPIO9  - pump pin 2 (motor driver 2)  *was GPIO10
  ~ SPI ~
  GPIO10 - rClock - TPIC register clock  *was GPIO08
  GPIO11 - SPI MOSI - TPIC Data
  GPIO13 - sClk - TPIC serial clock
  ~ i2c ~
  SCL/SDA = to RTC SCL/SDA
*/

#include <Arduino.h>  // for type definitions
#include <stdint.h>

//----------------------------
// Pins
//----------------------------
// Switch Interrupt pin is LOW-True Logic, GND == enabled
const byte INTERRUPT_PIN = 2;  // Digital pin switch is attached to that enables or disables the sampler timer
const byte WAKEUP_PIN = 12;

const byte PUMP_PIN1 = 8; // Motor MOSFET attached to digital pin 9,  1=forward, 0=reverse
const byte PUMP_PIN2 = 9; // Motor MOSFET attached to digital pin 10 0=forward, 1=reverse

//----------------------------
// SPI Variables
//----------------------------
const byte SCLOCK = 13; // SPI SCK pin 13 - Not used in code
const byte RCLOCK = 10; // Register clock pin, acts as CS, could be moved to SPI SS pin 10?
const byte DATA_PIN = 11; // SPI MOSI pin 11 - Not used in code

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

//Flash
#define memValidationValue 99
//#define INIT_INST 1

//--------------------------------
// Real Time Clock
//--------------------------------

volatile int  count = 10; 		//number of seconds to wait before running loop()
unsigned long timer;
#define EI_NOTEXTERNAL

//----------------------------
// Defaults
//----------------------------
// Determine Sampler "Factory Default" behaviour. These can be changed and saved using the SerialCommandDefs (see tab with same label)
// If never set before, default sample time is once per day, at 8AM (08:00)
const uint8_t SampleAlarmMinDef = 00; // Default sample time: Min
const uint8_t SampleAlarmHrDef = 8; // Default sample time: Hr
const uint16_t SampleAlarmPerDef = 3; // Default sample period time, 3 min for testing, make longer for factory release

//in future, base instead on pump timing and do calculations (e.g. 500 mL/min)
const uint16_t SampleDurMsDef = 30000; // Factory Default 30 sec, takes 60 seconds for 250ml so - don't overfill, How long pumps should run to take one sample, in ms (pump version 2)
const uint16_t FlushDurMsDef = 10000; // Factory default, low long in ms to flush system
const uint16_t BagFlushDurMsDef = 5000; //Factory default
const uint16_t BagDrawDurMsDef = 1000; //Factory default
const uint8_t SampleVolMlDef = 250;  // Factory Default, sample vol in ml, 250ml takes about 2min, or about 480ms per ml (pump version 1)
const bool SampleValveNumDef = 1; // Factory Default, current valve number
const int FlushValveNumDef = 0;

const uint8_t MIN_FUNC_TIME = 100;

const uint32_t baud = 115200; // BAUD rate for USB serial (when debugging ensure serial monitor matches)

enum class Mode {DAILY, PERIODIC};

/* --------------------
   Phone Number Storage
   --------------------
   WARNING: Raising this too high might may cause instability/crashing if too
   little dynamic memory is available, or if the configuration struct becomes too
   large for EEPROM.

   bytes used = numSMSRecipients * (phoneNumberLength + 1)
*/

// Quantity of phone numbers to store in flash that will receive SMS status updates.
const uint8_t numSMSRecipients = 5;

// Maximum number of digits in a phone number
const uint8_t phoneNumberLength = 11;

#define FONA_ENABLED false
#define BLE_ENABLEd false

//Configuration

// Values are stored in a struct within the Configuration class to make them easy to copy to/from flash
struct config_flash_t {
  uint8_t sampleAlarmMinute; // Minute to take a sample
  uint8_t sampleAlarmHour;   // Hour   to take a sample
  uint16_t periodicAlarmMinutes; // Time (in minutes) between samples in Periodic mode
  unsigned long flushDurationMs; // How long to perform a flush of main line (in milliseconds)
  unsigned long bagFlushDurationMs; // How long to perform a flush of individual bag (in milliseconds)
  unsigned long bagDrawDurationMs; // How long to perform a draw of individual bag (in milliseconds)
  unsigned long sampleDurationMs; // How long to draw a sample (in milliseconds)
  unsigned long sampleVolumeMl; // How big is the bag (in mL)
  uint8_t valveNumber; // Current number of valves sampled, 0=reset/default
  uint8_t flushValveNumber; // Number for flush valve
  bool written; // Has flash been written to?
  Mode mode; // Daily or Periodic sample timer
  char SMSNumbers[numSMSRecipients][phoneNumberLength + 1]; // Phone numbers for SMS status update recipients
  byte checksum;                 // Value is changed when flash memory is written to.
//  uint8_t     instance_number;          // Default 0, should be set on startup from a patch
//  char        packet_header_string[80]; // String of expected packet header (dynamically formed based on config.h)
};

//----------------------------
// Function action and time arrays
//----------------------------

// SamplerFunctions come from here:
// https://arduino.stackexchange.com/questions/21095/how-to-write-array-of-functions-in-arduino-library

// This array determines the sequence of actions to take during a sample condition
//0_Off, 1_OpenFlush, 2_NextValve, 3_OpenValve, 4_PumpOut, 5_PumpIn, 6_TakeSample
short myProgram[] =
{
  1, 5, 0,
  4, 3, 5, 4, 0,
  1, 5, 0,
  4, 3, 5, 4, 0,
  1, 5, 0,
  4, 3,
  6, 0
};
//flush main line, clear pre-valve, flush main line, clear pre valve, take sample

// This array determines the timing of the above actions sequence numbers
// during a sample condition, see action library in SampleFunctions.h
uint16_t myTimes[] =
{
  MIN_FUNC_TIME, FlushDurMsDef, MIN_FUNC_TIME,
  MIN_FUNC_TIME, 1, BagFlushDurMsDef, BagDrawDurMsDef, MIN_FUNC_TIME,
  MIN_FUNC_TIME, FlushDurMsDef, MIN_FUNC_TIME,
  MIN_FUNC_TIME, 1, BagFlushDurMsDef, BagDrawDurMsDef, MIN_FUNC_TIME,
  MIN_FUNC_TIME, FlushDurMsDef, MIN_FUNC_TIME,
  MIN_FUNC_TIME, BagDrawDurMsDef,
  SampleDurMsDef, MIN_FUNC_TIME
};

//----------------------------
// Flags
//----------------------------
volatile bool ledState = 0;
volatile bool timerEN = false; // Flag to enable or disable sampler timed functions
volatile bool sleepEN = false; // Flag to check in loop to see if we should sleep or not
volatile bool SampleState = LOW; // SampleState used to set valve and motor states for taking new sample
volatile long stateTimerMS = 0;  // Initialize state timer
volatile bool sequenceFlag = false; // Is set True when timer has run its course
volatile int programCounter = 0; // counter to step through program array
volatile bool valvePrint = true; //flag to control when valve number is printing in valveOpen
volatile bool printedOnce = false; //flag to control when valve number is printing in valveOpen

//----------------------------
// Valve Operation Variables
//----------------------------
const int numValves = 24; // how many valves in each module
const int TPIC_COUNT = ceil(numValves / 8) + 1;
volatile bool valveOpen = false; // variable to keep track of if any valves are open
uint8_t currValve = 0; // valve number that is currently being operated - needed to deal with flush vs. sample valves on courtesy signal
int value = 0; // Place to accumulate valve number input (see SerialCommandDefs tab)
signed int directionMotor = 0; // 0=off, 1=draw water in, -1=pull water out
int valvePlace;
int currTPIC;

//----------------------------
// Declare Other Functions
//----------------------------
void listenForSerial();
//void writeNonVolatiledefaults();

// ======= TIME FUNCTION PROTOTYPES ======
void print_countdown();
void InitializeRTC();
void interrupt_reset();
//void print_DateTime(DateTime time);
void wakeUp();
void sampleEN();
void setAlarmPeriod();
void clearAlarms();
void sleepcheck();
void RTCReportTime();
void setClock();
