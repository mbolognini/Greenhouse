#ifndef SENSORS_H
#define SENSORS_H

void setupSensors();

// Funzioni pubbliche che potrai chiamare nel setup() e nel loop() del file .ino
int getLight();
void getHumidity();
void getTemperature();
void isFlameDetected();
void isAcknowledgmentButtonPressed();

#endif
