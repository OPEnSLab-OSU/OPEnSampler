void sampleSeq()
{
  programCounter = 0;
  //0_Off, 1_OpenFlush, 2_NextValve, 3_OpenValve, 4_PumpOut, 5_PumpIn,
  // flush main
  myProgram[0] = 1; myTimes[0] = MIN_FUNC_TIME; //open flush valve
  myProgram[1] = 5; myTimes[1] = configuration.FDMs; //pump water in
  myProgram[2] = 0; myTimes[2] = MIN_FUNC_TIME; //turn everything off
  //flush vertical
  myProgram[3] = 4; myTimes[3] = MIN_FUNC_TIME; //reverse pump
  myProgram[4] = 3; myTimes[4] = 1; //open valve
  myProgram[5] = 5; myTimes[5] = configuration.BFDMs; //forward pump to flush vertical
  myProgram[6] = 4; myTimes[6] = configuration.BDDMs; //reverse pump to draw from vertical
  myProgram[7] = 0; myTimes[7] = MIN_FUNC_TIME; //turn everything off
  //flush main
  myProgram[8] = 1; myTimes[8] = MIN_FUNC_TIME; //open flush valve
  myProgram[9] = 5; myTimes[9] = configuration.FDMs; //pump water in - set time by amount of tubing for pump
  myProgram[10] = 0; myTimes[10] = MIN_FUNC_TIME; //turn everything off
  //flush vertical
  myProgram[11] = 4; myTimes[11] = MIN_FUNC_TIME; //reverse pump
  myProgram[12] = 3; myTimes[12] = 1; //open valve
  myProgram[13] = 5; myTimes[13] = configuration.BFDMs; //forward pump to flush vertical
  myProgram[14] = 4; myTimes[14] = configuration.BDDMs; //reverse pump to draw from vertical
  myProgram[15] = 0; myTimes[15] = MIN_FUNC_TIME; //turn everything off
  //flush main
  myProgram[16] = 1; myTimes[16] = MIN_FUNC_TIME; //open flush valve
  myProgram[17] = 5; myTimes[17] = configuration.FDMs; //pump water in - set time by amount of tubing for pump
  myProgram[18] = 0; myTimes[18] = MIN_FUNC_TIME; //turn everything off
  // clear vertical and empty bag of air
  myProgram[19] = 4; myTimes[19] = MIN_FUNC_TIME; //reverse pump
  myProgram[20] = 3; myTimes[20] = configuration.BDDMs; //open valve
  //take sample
  myProgram[21] = 5; myTimes[21] = configuration.SDMs; //pump water into bottle
  myProgram[22] = 0; myTimes[22] = MIN_FUNC_TIME; //turn everything off
}
