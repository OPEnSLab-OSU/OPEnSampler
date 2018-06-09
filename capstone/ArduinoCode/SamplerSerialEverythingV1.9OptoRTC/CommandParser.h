#pragma once

#include "Configuration.h"

/**
 * Parses and executes valid commands
 *
 * Valid commands are in the following format:
 * - 1 identifying char corresponding to a command in execute()
 * - any amount of arguments separated by delimiter
 * - 1 terminator
 *
 * The argument delimiters and terminating characters are specifiable upon
 * construction. Should eventually be extended to replace the serial parsing.
 */
class CommandParser
{
public:
  // Construct CommandParser, specifying argument delimiter and command terminator.
  CommandParser(char delimiter, char terminator);

  // Process a string of concatenated commands for execution.
  bool process(char buffer[], uint8_t length) const;

private:
  // cstrings consisting of parser's delimiter/terminator char and \0 (null terminator)
  char delimiter[2];
  char terminator[2];

  // Parse a single command's arguments and execute it.
  bool parseArguments(const char buffer[], uint8_t length) const;

  // Execute a command with arguments.
  bool execute(char identifier,  char *args[], size_t args_size) const;

  // Helper to convert an array of strings to an array of integers of the same length.
  bool stringsToIntegers(unsigned int int_args[], char *args[], size_t args_size) const;
};
