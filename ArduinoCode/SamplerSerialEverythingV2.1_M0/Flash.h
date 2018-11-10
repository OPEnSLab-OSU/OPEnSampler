//Flash.h

// ================================================================
// ===                        LIBRARIES                         ===
// ================================================================
#include <FlashStorage.h>
#include <RTClibExtended.h>	// from sparkfun low power library found here https://github.com/FabioCuomo/FabioCuomo-DS3231/
//#include "Definitions.h"
//#include "Globals.h"

// ================================================================
// ===                   FUNCTION PROTOTYPES                    ===
// ================================================================
void write_non_volatile();
void read_non_volatile();
void setup_flash_config();
//void writeNonVolatileDefaults();

// ================================================================
// ===                          SETUP                           ===
// ================================================================

// Select how to read from and write to non volatile memory
// based on which device is being used

FlashStorage(flash_config, config_flash_t);   // Setup the flash storage for the structure

void read_non_volatile() {
  configData = flash_config.read();
}
void write_non_volatile() {
  flash_config.write(configData);
}

// --- SETUP FLASH CONFIG ---
//
// Handles configuration of reading device configuration settings from flash (if config is saved)
// else uses Configuration.cpp specified settings, and then writes these to flash
//
void setup_flash_config()
{
  read_non_volatile(); //reads configuration from non_volatile memory
  Serial.println("Reading from non-volatile memory...");
  Serial.print("Checksum: ");
  Serial.println(configData.checksum);

  if (configData.checksum != memValidationValue) {     // Write default values to flash
    // The or '!enable_flash' here is to make sure the configuration struct is still populated (there wont be a valid checksum)
    //configData.instance_number = INIT_INST;
    //sprintf(configData.packet_header_string, "%s%d\0", PacketHeaderString, configData.instance_number);

    writeNonVolatileDefaults()

    // Flash memory has limited writes and we don't want to waste it on unnecessary tests
  } // of if (configData.checksum != memValidationValue)
}

/*// Restore OPEnS Lab Factory Defaults
void writeNonVolatileDefaults()
{
  // Disable external pin interrupt on wake up pin.
  detachInterrupt(digitalPinToInterrupt(WAKEUP_PIN));

  // Flash never written or factory default reset serial received "RST"
  Serial.print(F("Writing Flash defaults. . ."));

  configData.sampleAlarmMinute = SampleAlarmMinDef;
  configData.sampleAlarmHour = SampleAlarmHrDef;
  configData.periodicAlarmMinutes = SampleAlarmPerDef;
  configData.flushDurationMs = FlushDurMsDef;
  configData.bagFlushDurationMs = BagFlushDurMsDef;
  configData.bagDrawDurationMs = BagDrawDurMsDef;
  configData.sampleDurationMs = SampleDurMsDef;
  configData.sampleVolumeMl = SampleVolMlDef;
  configData.valveNumber = SampleValveNumDef;
  configData.flushValveNumber = FlushValveNumDef;
  configData.written = 0;
  configData.Is_Daily = 1; // set daily flag in configuration  

  configData.checksum = memValidationValue;      // Configuration has been written successfully, so we write the checksum

  // Save Defaults into Flash
  write_non_volatile();

  // set RTC timer here:
  RTC.setAlarm(ALM1_MATCH_HOURS, configuration.SAMin, configuration.SAHr, 0);
  Serial.println(F("Done"));
  // Allow wake up pin to trigger interrupt on low.
  attachInterrupt(digitalPinToInterrupt(WAKEUP_PIN), wakeUp, FALLING);
}*/
