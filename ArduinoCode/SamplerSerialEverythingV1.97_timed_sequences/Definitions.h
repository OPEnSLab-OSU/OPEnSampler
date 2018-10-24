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

//----------------------------
// Pins
//----------------------------
// Switch Interrupt pin is LOW-True Logic, GND == enabled
const byte INTERRUPT_PIN = 2;  // Digital pin switch is attached to that enables or disables the sampler timer
const byte WAKEUP_PIN = 3; // Use pin 3 as wake up pin
const byte PUMP_PIN1 = 8; // Motor MOSFET attached to digital pin 9,  1=forward, 0=reverse
const byte PUMP_PIN2 = 9; // Motor MOSFET attached to digital pin 10 0=forward, 1=reverse

// Initialize Real Time Clock
RTC_DS3231 RTC;

//----------------------------
// Defaults
//----------------------------
// Determine Sampler "Factory Default" behaviour. These can be changed and saved using the SerialCommandDefs (see tab with same label)
// If never set before, default sample time is once per day, at 8AM (08:00)
const uint8_t SAMPLE_ALARM_MIN_DEF = 00; // Default sample time: Min
const uint8_t SAMPLE_ALARM_HR_DEF = 8; // Default sample time: Hr
const uint16_t SAMPLE_ALARM_PER_DEF = 3; // Default sample period time, 3 min for testing, make longer for factory release

//in future, base instead on pump timing and do calculations (e.g. 500 mL/min)
const uint16_t SAMPLE_DUR_MS_DEF = 30000; // Factory Default 30 sec, takes 60 seconds for 250ml so - don't overfill, How long pumps should run to take one sample, in ms (pump version 2)
const uint16_t FLUSH_DUR_MS_DEF = 10000; // Factory default, low long in ms to flush system
const uint16_t BAG_FLUSH_DUR_MS_DEF = 100; //Factory default
const uint16_t BAG_DRAW_DUR_MS_DEF = 5000; //Factory default
const uint8_t SAMPLE_VOL_ML_DEF = 250;  // Factory Default, sample vol in ml, 250ml takes about 2min, or about 480ms per ml (pump version 1)
const bool SAMPLE_VALVE_NUM_DEF = 0; // Factory Default, current valve number
const int FLUSH_VALVE_NUM = 0;

const uint8_t MIN_FUNC_TIME = 100;

//----------------------------
// EEPROM
//----------------------------
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
  unsigned long BDDMs; //bag draw duration in ms
  unsigned long BFDMs; //bag flush duration in ms
} configuration;

const byte EEPROM_VALIDATION_VALUE = 99; // Value to test to see if EEPROM has been written before

template <class T> int EEPROM_writeAnything(int ee, const T& value)
{
  const byte* p = (const byte*)(const void*)&value;
  unsigned int i;
  for (i = 0; i < sizeof(value); i++)
    EEPROM.write(ee++, *p++);
  return i;
}

template <class T> int EEPROM_readAnything(int ee, T& value)
{
  byte* p = (byte*)(void*)&value;
  unsigned int i;
  for (i = 0; i < sizeof(value); i++)
    *p++ = EEPROM.read(ee++);
  return i;
}

//----------------------------
// Function action and time arrays
//----------------------------

// SamplerFunctions come from here:
// https://arduino.stackexchange.com/questions/21095/how-to-write-array-of-functions-in-arduino-library

// This array determines the sequence of actions to take during a sample condition
//0_Off, 1_OpenFlush, 2_NextValve, 3_OpenValve, 4_PumpOut, 5_PumpIn
short myProgram[] =
{
  1, 5, 0,
  4, 3, 5, 4, 0,
  1, 5, 0,
  4, 3, 5, 4, 0,
  1, 5, 0,
  4, 3,
  5, 0
};
//flush main line, clear pre-valve, flush main line, clear pre valve, take sample

// This array determines the timing of the above actions sequence numbers
// during a sample condition, see action library in SampleFunctions.h
uint16_t myTimes[] =
{
  MIN_FUNC_TIME, configuration.FDMs, MIN_FUNC_TIME,
  MIN_FUNC_TIME, 1, configuration.BFDMs, configuration.BDDMs, MIN_FUNC_TIME,
  MIN_FUNC_TIME, configuration.FDMs, MIN_FUNC_TIME,
  MIN_FUNC_TIME, 1, configuration.BFDMs, configuration.BDDMs, MIN_FUNC_TIME,
  MIN_FUNC_TIME, configuration.FDMs, MIN_FUNC_TIME,
  MIN_FUNC_TIME, configuration.BDDMs,
  configuration.SDMs, MIN_FUNC_TIME
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
const int NUM_VALVES = 24; // how many valves in each module
const int TPIC_COUNT = ceil(NUM_VALVES / 8) + 1;
volatile bool valveOpen = false; // variable to keep track of if any valves are open
uint8_t currValve = 0; // valve number that is currently being operated - needed to deal with flush vs. sample valves on courtesy signal
int value = 0; // Place to accumulate valve number input (see SerialCommandDefs tab)
signed int directionMotor = 0; // 0=off, 1=draw water in, -1=pull water out
int valvePlace;
int currTPIC;

//----------------------------
// SPI Variables
//----------------------------
const byte SCLOCK = 13; // SPI SCK pin 13 - Not used in code
const byte RCLOCK = 10; // Register clock pin, acts as CS, could be moved to SPI SS pin 10?
const byte DATA_PIN = 11; // SPI MOSI pin 11 - Not used in code

//----------------------------
// Declare Valve Functions
//----------------------------
void everythingOff();
void openValve();
void closeValves();
void setPump(signed int ms);

//----------------------------
// Declare Other Functions
//----------------------------
void listenForSerial();
void wakeUp();
void sampleEN();
void setAlarmPeriod();
void clearAlarms();
void sleepcheck();
void RTCReportTime();
void writeEEPROMdefaults();

