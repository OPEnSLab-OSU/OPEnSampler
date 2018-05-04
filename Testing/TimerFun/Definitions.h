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

// SamplerFunctions come from here:
// https://arduino.stackexchange.com/questions/21095/how-to-write-array-of-functions-in-arduino-library

#include <SPI.h>        // Using SPI hardware to communicate with TPICs

// Switch Interrupt pin is LOW-True Logic, GND == enabled
//make all constants all caps
const byte interruptPin = 2;  // Digital pin switch is attached to that enables or disables the sampler timer
const byte wakeUpPin = 3; // Use pin 3 as wake up pin
const byte pumpPin1 = 8; // Motor MOSFET attached to digital pin 9,  1=forward, 0=reverse
const byte pumpPin2 = 9; // Motor MOSFET attached to digital pin 10 0=forward, 1=reverse

// Flags
volatile bool ledState = 0;
volatile bool timerEN = false; // Flag to enable or disable sampler timed functions
volatile long stateTimerMS = 0;  // Initialize state timer
volatile bool sequenceFlag = false; // Is set True when timer has run its course
volatile int programCounter = 0; // counter to step through program array
volatile bool valvePrint = 0; //flag to control when valve number is printing in valveOpen
volatile bool printedOnce = 1; //flag to control when valve number is printing in valveOpen

// This array determines the sequence of actions to take
// during a sample condition, see action library in SamplerFunctions.h
short myProgram[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// This array determines the timing of the above actions sequence numbers
// during a sample condition, see action library in SamplerFunctions.h
uint16_t myTimes[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

//----------------------------
// Valve Operation Variables
//----------------------------
const int numValves = 24; // how many valves in each module
const int TPICcount = ceil(numValves / 8) + 1;
// flush valve is first valve on last TPIC
const int flushValveNum = (TPICcount - 1) * 8 + 1;
// variable to keep track of if any valves are open
int valveOpen;
uint8_t valveNum = 0;  // number of valve relative to current module
uint8_t currValve = 0; //valve to perform operations on - would like to just use valveNum, but kept getting errors with openValve function
int value = 0; // Place to accumulate valve number input (see SerialCommandDefs tab)

//----------------------------
// SPI Variables
//----------------------------
const byte sclock = 13; // SPI SCK pin 13
const byte rclock = 10; // Register clock pin, acts as CS, could be moved to SPI SS pin 10?
const byte datapin = 11; // SPI MOSI pin 11

//----------------------------
// Declare Valve Functions
//----------------------------
void everythingOff();
void openValve();
void closeValves();
void setPump(signed int ms);

//----------------------------
// Other functions
//----------------------------
void listenForSerial();

