#pragma once

enum class pumpState { ON, OFF, REVERSE };

extern const byte pumpPin1;
extern const byte pumpPin2;

void everythingOff();
void flushON();
void flushOFF();
void setPump(pumpState state);
void clearValveBits();
void setValveBits();
void shiftl(void *object, size_t size);
pumpState getPumpState(int n);
bool puppetValveState(unsigned int valveNumber, bool valveState);
