#ifndef GREENHOUSE_H
#define GREENHOUSE_H

struct Measurements {
    float temp;
    float hum;
    int light;
    bool isFire;
    bool isAckButtonPressed;
    long wifiStrength;
};
extern struct Measurements m;

struct Status {
    bool isEmergency;
    bool isBadAir;
    bool isDark;
    bool isFire;
    bool isAcknowledged;
    bool isTempEmergency;
    bool isHumEmergency;
    bool isWifiEmergency;
};
extern struct Status s;

void measure();
void updateStatus();
void checkTemperature();
void checkHumidity();
void checkLight();
void checkFlame();
void checkWifi();
void checkButton();
void updateServo();
void updateLamp();
void updateEmergency();

#endif
