extern Configuration config;

// https://github.com/adafruit/Adafruit_FONA/blob/master/examples/FONAtest/FONAtest.ino#L269
enum NetworkStatus { NOT_REGISTERED           = 0,
                     REGISTERED_HOME          = 1,
                     NOT_REGISTERED_SEARCHING = 2,
                     DENIED                   = 3,
                     NOT_KNOWN                = 4,
                     REGISTERED_ROAMING       = 5 };

/**
 * Send an SMS message to all stored phone numbers.
 */
void sendSMSAll(char * message)
{
  // Do nothing if SMS functionality isn't enabled
  if (! enableSMS) return;
  
  NetworkStatus status = (NetworkStatus) fona.getNetworkStatus();

  // Delay briefly while waiting for good network status TODO
  const unsigned int seconds = 5;
  for (unsigned int count = 0; count <= seconds; count++) {
    if (status == REGISTERED_HOME || status == REGISTERED_ROAMING) break;

    // TODO: Way of ensuring no flash storage for strings w/o this guard everywhere
    // Wrap it in a function/macro?
    #ifdef DEBUG
      Serial.println(F("DEBUG: Network not ready to send SMS, delaying 1s"));
    #endif

    delay(1000);
    status = (NetworkStatus) fona.getNetworkStatus();
  }

  // Send SMS to recipients
  for (unsigned int i = 0; i < numSMSRecipients; i++) {
    const char * phoneNumber = config.getSMSNumber(i);

    if (phoneNumber) {
      #ifdef DEBUG
        Serial.print(F("DEBUG: Attempting to send SMS to "));
        Serial.print(phoneNumber);
      #endif

      if (!fona.sendSMS(phoneNumber, message)) {
        #ifdef DEBUG
          Serial.print(F("ERROR: Failed to send SMS to "));
          Serial.println(phoneNumber);
        #endif
      }
    }
  }
}
