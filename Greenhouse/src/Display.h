#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>

void setupDisplay();
void clearEmergencyMessages();
void addEmergencyMessage(String message);
void updateDisplay();

#endif
