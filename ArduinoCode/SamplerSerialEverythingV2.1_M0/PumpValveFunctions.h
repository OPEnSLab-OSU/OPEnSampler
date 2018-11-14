//PumpValveFunctions.h
#pragma once

enum class pumpState { ON, OFF, REVERSE };

//----------------------------
// Declare Valve Functions
//----------------------------
void closeValves();
void openValve();
void everythingOff();
void setPump(pumpState state);
pumpState getPumpState(int n);
