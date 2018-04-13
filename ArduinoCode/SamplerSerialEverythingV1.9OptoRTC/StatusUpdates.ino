extern Configuration config;

/**
 * Sends an SMS message to all stored phone numbers.
 */
void sendSMSAll(char * message)
{
  // Do nothing if SMS functionality isn't enabled
  if (! enableSMS) return;
  
  uint8_t status = fona.getNetworkStatus();

  // TODO: Does this help?
  // Delay up to 10 seconds while waiting for good network status
  for (int count = 0; status != 1 && status != 5; count++) {
    if (count == 10) {
      Serial.println(F("Delayed 10 seconds, continuing regardless of network status."));
      break; 
    }

    Serial.println(F("Trying to send SMS. Delaying 1s until network is ready."));
    delay(1000);
    status = fona.getNetworkStatus();
  }

  for (int i = 0; i < numSMSRecipients; i++) {
    char * phoneNumber = config.getSMSNumber(i);

    if (phoneNumber) {
      Serial.print(F("DEBUG: Attempting to send SMS message to "));
      Serial.print(phoneNumber);

      if (!fona.sendSMS(phoneNumber, message)) {
        Serial.print(F("ERROR: Failed to send SMS to "));
        Serial.println(phoneNumber);
      }
    }
  }
}
