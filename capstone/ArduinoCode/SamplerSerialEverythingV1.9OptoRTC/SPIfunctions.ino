// SPI write function to strobe TPIC Buffer Vals to TPICs
void strobeTPICs(){
  digitalWrite(rClockPin,LOW); // Take rClockPin low to enable transfer
  delay(1);
  for(int i=0; i<4; i++) // because 4 TPICs in module
  {
    SPI.transfer(TPICBuffer[i]); //Transfer contents of TPIC buffers
    Serial.print(F("Transfer to TPIC:")); Serial.print(i); Serial.print(F(": "));Serial.println(TPICBuffer[i]); // Print to serial
  }
  delay(1);
  digitalWrite(rClockPin,HIGH);

  // SAFETY FEATURE
  // If all valves are closed, auto turn pump off to prevent destruction
  if((TPICBuffer[0] == 0) && (TPICBuffer[1] == 0) && (TPICBuffer[2] == 0) && (TPICBuffer[3] == 0))
    setPump(pumpState::OFF);
}
