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

// Switch Interrupt pin is LOW-True Logic, GND == enabled
const byte INTERRUPT_PIN = 2;  // Digital pin switch is attached to that enables or disables the sampler timer
const byte WAKEUP_PIN = 3; // Use pin 3 as wake up pin
const byte PUMP_PIN1 = 8; // Motor MOSFET attached to digital pin 9,  1=forward, 0=reverse
const byte PUMP_PIN2 = 9; // Motor MOSFET attached to digital pin 10 0=forward, 1=reverse

// Flags
volatile bool ledState = 0;
volatile bool timerEN = false; // Flag to enable or disable sampler timed functions
volatile long stateTimerMS = 0;  // Initialize state timer
volatile bool sequenceFlag = false; // Is set True when timer has run its course
volatile int programCounter = 0; // counter to step through program array
volatile bool valvePrint = true; //flag to control when valve number is printing in valveOpen
volatile bool printedOnce = false; //flag to control when valve number is printing in valveOpen

// This array determines the sequence of actions to take - declaring size
short myProgram[] = {
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0,
  0, 0, 0
};

// This array determines the timing of the above actions sequence numbers - declaring size
uint16_t myTimes[] = {
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0,
  0, 0, 0
};

//----------------------------
// Valve Operation Variables
//----------------------------
const int NUM_VALVES = 24; // how many valves in each module
const int TPIC_COUNT = ceil(NUM_VALVES / 8) + 1;
// flush valve is first valve on last TPIC
const int FLUSH_VALVE_NUM = 0;
// variable to keep track of if any valves are open
volatile bool valveOpen = false;
uint8_t valveNum = 0;  // valve number to keep track of
uint8_t currValve = 0; // valve number that is currently being operated - needed to deal with flush vs. sample valves
int value = 0; // Place to accumulate valve number input (see SerialCommandDefs tab)
byte msg;
int valvePlace;
int currTPIC;

//----------------------------
// SPI Variables
//----------------------------
const byte SCLOCK = 13; // SPI SCK pin 13
const byte RCLOCK = 10; // Register clock pin, acts as CS, could be moved to SPI SS pin 10?
const byte DATA_PIN = 11; // SPI MOSI pin 11

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

