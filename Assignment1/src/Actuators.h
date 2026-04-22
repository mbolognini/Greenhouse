#ifndef ACTUATORS_H
#define ACTUATORS_H

enum WindowPos { CLOSE = 0, OPEN = 90 };
enum LampState { OFF = 0, ON = 1 };
enum EmergencyBuzzerLedState { OFF = 0, ON = 1 };

void setupActuators();

void moveWindow(WindowPos targetPosition);
void turnLamp(LampState targetState);
void updateEmergencyBuzzerLed();

#endif
