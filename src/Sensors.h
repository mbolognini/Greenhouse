#ifndef SENSORS_H
#define SENSORS_H

void setupSensors();
int getLight();
float getHumidity();
float getTemperature();
bool isFlameDetected();
bool isAcknowledgmentButtonPressed();

#endif
