#include "Sensors.h"
#include <DHT.h>
#include "Params.h"

DHT dht = DHT(DHTPIN, DHTTYPE);

void setupSensors() {
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(FLAME_PIN, INPUT);
    pinMode(PHOTORESISTOR, INPUT);
    pinMode(SERVO_PIN, OUTPUT);

    dht.begin();
}

int getLight() {
    return analogRead(PHOTORESISTOR);
}

float getHumidity() {
    return dht.readHumidity();
}

float getTemperature() {
    return dht.readTemperature();
}

bool isFlameDetected() {
    if (digitalRead(FLAME_PIN) == HIGH)
        return true;
    else
        return false;
}

bool isAcknowledgmentButtonPressed() {
    return digitalRead(BUTTON_PIN);
}
