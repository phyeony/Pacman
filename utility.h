// A utility module
#ifndef _UTILITY_H_
#define _UTILITY_H_

void Utility_sleepForMs(long long delayInMs);
long long Utility_getCurrentTimeInMs(void);
void Utility_exportGpioIfNeeded(int gpioNum);
int Utility_readGPIOValueFile(int gpioNum);
void Utility_configPin(int pinHeaderNum, int pinNum, char *type);

#endif