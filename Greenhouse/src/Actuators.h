#ifndef ACTUATORS_H
#define ACTUATORS_H

enum class WindowPos { CLOSE = 0, OPEN = 90 };
enum class LampState { OFF = 0, ON = 1 };
enum class EmergencyBuzzerLedState { OFF = 0, ON = 1 };

void setupActuators();

void moveWindow(WindowPos targetPosition);
void turnLamp(LampState targetState);
void updateEmergencyBuzzerLed(EmergencyBuzzerLedState targetState);

#endif
