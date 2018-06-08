// ================================================================
// EEPROM Write
// ================================================================
// Restore OPEnS Lab Factory Defaults
void writeEEPROMdefaults()
{
  // Disable external pin interrupt on wake up pin.
  detachInterrupt(digitalPinToInterrupt(WAKEUP_PIN));

  // EEPROM never written or factory default reset serial received "RST"
  Serial.print(F("Writing EEPROM defaults. . ."));

  configuration.SAMin = SAMPLE_ALARM_MIN_DEF;
  configuration.SAHr = SAMPLE_ALARM_HR_DEF;
  configuration.SAPer = SAMPLE_ALARM_PER_DEF;
  configuration.FDMs = FLUSH_DUR_MS_DEF;
  configuration.SDMs = SAMPLE_DUR_MS_DEF;
  configuration.SVml = SAMPLE_VOL_ML_DEF;
  configuration.VNum = SAMPLE_VALVE_NUM_DEF;
  configuration.written = 0;
  configuration.Is_Daily = 1; // set daily flag in configuration
  configuration.BDDMs = BAG_DRAW_DUR_MS_DEF;
  configuration.BFDMs = BAG_FLUSH_DUR_MS_DEF;

  // Save Defaults into EEPROM
  EEPROM_writeAnything(0, configuration);

  // set RTC timer here:
  RTC.setAlarm(ALM1_MATCH_HOURS, configuration.SAMin, configuration.SAHr, 0);
  Serial.println(F("Done"));
  // Allow wake up pin to trigger interrupt on low.
  attachInterrupt(digitalPinToInterrupt(WAKEUP_PIN), wakeUp, FALLING);
}
