/**
 * Set the phone number of a status update recipient.
 *
 * Sets a null-terminated string containing a phone number at the specified
 * index to be stored persistently in configuration. Phone numbers can be up to
 * 15 digits in length. It's assumed that buffer can store at least 16 chars.
 * The number should be stored without punctuation or whitespace
 * (e.g. 123456789012345).
 */
void setPhone(int index, char *buffer)
{
  if (index >= NUM_PHONES)
    return;

  strncpy(configuration.phones[index], buffer, 15);
  configuration.phones[index][15] = '\0';

  return;
}

/**
 * Get the phone number of a status update recipient.
 *
 * Returns a pointer to an ASCII phone number, or null if the phone number at
 * index is undefined. The number is stored without punctuation or whitespace
 * (e.g. 123456789012345).
 */
char * getPhone(int index)
{
  if (index >= NUM_PHONES)
    return NULL;

  if (strlen(configuration.phones[index]) == 0)
    return NULL;

  return configuration.phones[index];
}
