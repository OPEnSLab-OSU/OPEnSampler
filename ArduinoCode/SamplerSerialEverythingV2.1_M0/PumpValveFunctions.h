//PumpValveFunctions.h
#pragma once

enum class pumpState { ON, OFF, REVERSE };

extern const byte pumpPin1;
extern const byte pumpPin2;

void closeValves();
void openValve();
void everythingOff();
void setPump(pumpState state);
pumpState getPumpState(int n);
