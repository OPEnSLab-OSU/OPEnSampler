#include "Adafruit_BLE_UART.h"

extern Adafruit_BLE_UART BLESerial;

/**
   Log ACI event changes for Bluetooth
*/
void ACICallback(aci_evt_opcode_t event)
{
#ifdef DEBUG
  switch (event)
  {
    case ACI_EVT_DEVICE_STARTED:
      Serial.println(F("DEBUG: BLE: Advertising started."));
      break;

    case ACI_EVT_CONNECTED:
      Serial.println(F("DEBUG: BLE: Connected."));
      break;

    case ACI_EVT_DISCONNECTED:
      Serial.println(F("DEBUG: BLE: Disconnected or advertising timed out."));
      break;

    default:
      break;
  }
#endif
}

/**
   Process data arriving on RX from Bluetooth
*/
void RXCallback(uint8_t *buffer, uint8_t length)
{
#ifdef DEBUG
  Serial.print(F("DEBUG: BLE: Received "));
  Serial.print(length);
  Serial.print(F(" bytes: "));

  for (int i = 0; i < length; i++)
    Serial.print((char) buffer[i]);

  Serial.print(F(" ["));

  for (int i = 0; i < length; i++)
  {
    Serial.print(F(" 0x"));
    Serial.print((char) buffer[i], HEX);
  }

  Serial.println(F(" ]"));
#endif

  BLEParser.process((char *) buffer, length);
}

/**
   Send configuration data struct over Bluetooth Low Energy.
*/
void sendConfigOverBluetooth(configuration config)
{
  const size_t bytesToSend = sizeof(config_flash_t);
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
