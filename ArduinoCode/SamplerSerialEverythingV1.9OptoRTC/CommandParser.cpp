#include <errno.h>
#include <string.h>

#include "Adafruit_BLE_UART.h"

#include "CommandParser.h"
#include "ValveAddressing.h"

// Globals defined in SamplerSerialEverything
extern Configuration config;
extern Adafruit_BLE_UART BLESerial;
extern void sendConfigOverBluetooth();
extern void setClock(uint16_t year, uint16_t month, uint16_t day, uint16_t hour, uint16_t minute);
extern void writeEEPROMDefaults();

/**
 * Construct CommandParser, specifying argument delimiter and command terminator.
 *
 * @param delimiter The char that separates (and does not occur in) arguments.
 * @param terminator The char that all commands end with.
 */
CommandParser::CommandParser(char delimiter=',', char terminator='|')
{
  this->delimiter[0] = delimiter;
  this->delimiter[1] = '\0';

  this->terminator[0] = terminator;
  this->terminator[1] = '\0';
}

/**
 * Process a string of concatenated commands for execution.
 *
 * Valid commands begin with a char identifying a command, are followed by any
 * amount of arguments separated by the parser's delimiter, and end with the
 * parser's terminator.
 *
 * @param buffer A string of concatenated commands to execute.
 * @param length Amount of characters in buffer.
 * @return true if all commands execute successfully, false otherwise.
 */
bool CommandParser::process(char *buffer, uint8_t length) const
{
  const char *command;
  char *location;

  command = strtok_r(buffer, this->terminator, &location);

  while (command != NULL) {
    size_t length = strlen(command);

    // Parse arguments and execute
    if (length > 1) {
      CommandParser::parseArguments(command, length); // TODO: remove space here?
    }
    // No arguments, so go straight to execute
    else {
      if(!execute(command[0], NULL, 0)) {
        Serial.print(F("ERROR: Failed to execute command, not enough arguments: "));
        Serial.println(command);
        return false;
      }
    }

    command = strtok_r(NULL, this->terminator, &location);
  }

  return true;
}

/**
 * Parse a single command's arguments and execute it.
 *
 * Command must begin with an identifying character, followed by one or more
 * arguments separated by the parser's delimiter. Command should not end in
 * the parser's terminator.
 *
 * @param buffer The command string to execute.
 * @param length The amount of characters in buffer.
 * @return true if the command is executed successfully, false otherwise.
 */
bool CommandParser::parseArguments(const char *buffer, uint8_t length) const
{
  int argc;

  // Count number of arguments (by counting delimiting characters)
  char *chr = (char *) buffer;
  for (argc = 0; chr[argc]; chr[argc] == delimiter[0] ? argc++ : *chr++);
  argc++; // There's one more argument than delimiters

  // Parse arguments
  char *token = strtok((char *) buffer + 1, delimiter); // TODO: Remove space earlier

  char *argv[argc];
  for (int count = 0; token != NULL; count++) {
    argv[count] = token;
    token = strtok(NULL, delimiter);
  }

  // Execute command
  if(!CommandParser::execute(buffer[0], argv, argc)) { // TODO: What is buffer[0]
    Serial.println(F("ERROR: Invalid command received."));
    return false;
  }
  return true;
}

/**
 * Execute a command with arguments.
 *
 * Will run a command if passed the appropriate identifier with enough valid arguments.
 * Returns true if a command was executed, or false if not enough arguments were provided.
 * @param identifier Char specifying which command to execute.
 * @param args An array of cstring arguments for the command to execute with.
 * @param args_size Amount of arguments in args.
 */
bool CommandParser::execute(char identifier, char *args[], size_t args_size) const
{
  unsigned int int_args[args_size];
  unsigned long milliseconds;
  pumpState state;
  int n;
  char * str;

  // DEBUG
  Serial.print(F("DEBUG: About to execute with identifier: "));
  Serial.println(identifier);
  Serial.print(F("DEBUG: Hex is: "));
  Serial.println((char) identifier, HEX);
  Serial.print(F("DEBUG: args_size is: "));
  Serial.println(args_size);
  //------------------------------

  switch (identifier)
  {
    case 'B':
      sendConfigOverBluetooth();
      break;

    case 'C':
      if (args_size < 5 || !stringsToIntegers(int_args, args, args_size))
        return false;

      setClock(int_args[0], int_args[1], int_args[2], int_args[3], int_args[4]);
      break;

    case 'D':
      if (args_size < 2 || !stringsToIntegers(int_args, args, args_size))
        return false;

      config.setDailyAlarm(int_args[0], int_args[1]);
      break;

    case 'F':
      if (args_size < 1)
        return false;

      milliseconds = strtoul(args[0], NULL, 10);
      if (milliseconds == 0L) {
        Serial.println(F("ERROR: Flush duration could not be converted to an unsigned long."));
        return false;
      }

      config.setFlushDuration(milliseconds);
      break;

    case 'G':
      if (args_size < 1 || !stringsToIntegers(int_args, args, args_size))
        return false;

      str = config.getSMSNumber(int_args[0]);

      if (str)
        Serial.println(str);
      else
        Serial.println(F("There is no status update recipient defined at that index."));

      break;

    case 'M':
      if (args_size < 1)
        return false;

      n = (int) strtol(args[0], NULL, 10);

      if (n == 0L) {
        Serial.println(F("ERROR: Pump motor state argument could not be converted to an integer."));
        return false;
      }

      setPump(getPumpState(n));
      break;

    case 'P':
      if (args_size < 1 || !stringsToIntegers(int_args, args, args_size))
        return false;

      config.setPeriodicAlarm(int_args[0]);
      break;

    case 'R':
      writeEEPROMDefaults();
      break;

    case 'S':
      if (args_size < 1 || !stringsToIntegers(int_args, args, args_size))
        return false;

      config.setSampleDuration(int_args[0]);
      break;

    case 'U':
      if (args_size < 2 || !stringsToIntegers(int_args, args, args_size))
        return false;

      puppetValveState(int_args[0], int_args[1]);
      break;

    case 'V':
      if (args_size < 1 || !stringsToIntegers(int_args, args, args_size))
        return false;

      if (int_args[0] <= 0) {
        Serial.println(F("ERROR: Next valve number must be a positive value."));
        return false;
      }

      config.setValveNumber(int_args[0] - 1);
      break;

    case 'W':
      if (args_size < 2)
        return false;

      n = (int) strtoul(args[0], NULL, 10);
      if (n == 0L) {
        Serial.println(F("ERROR: Index argument could not be converted to an integer."));
        return false;
      }

      config.setSMSNumber(n, args[1]);
      break;

    default:
      Serial.print(F("ERROR: Command identifier ("));
      Serial.print(identifier);
      Serial.println(F(") unrecognized, could not be matched to a command."));
      return false;
  }

  return true;
}

/**
 * Helper that converts array of cstrings to an array of unsigned ints.
 *
 * Attempts to convert each cstring in cstrings to an unsigned int, and stores
 * them in ints at the same index.
 *
 * @param ints Array to be filled with each member of cstrings converted to an
 *             unsigned integer.
 * @param cstrings Array of cstrings to convert to unsigned integers.
 * @param amount Number of elements to convert from cstrings to ints.
 * @return true if all cstrings can be converted successfully, false otherwise.
 */
bool CommandParser::stringsToIntegers(unsigned int ints[], char *cstrings[], size_t amount) const
{
  for (int i = 0; i < amount; i++) {
    unsigned long integer = strtoul(cstrings[i], NULL, 10);

    if (integer == 0L && errno != 0) {
      Serial.print(F("ERROR: Argument \""));
      Serial.print(cstrings[i]);
      Serial.println(F("\" could not be converted to an unsigned integer."));
      return false;
    }
    else {
      ints[i] = (unsigned int) integer;
    }
  }
  return true;
}
