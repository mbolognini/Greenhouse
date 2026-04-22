#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <ESP8266WiFi.h>

// Funzioni pubbliche che potrai chiamare nel setup() e nel loop() del file .ino
void setupWiFi();
void checkWiFiConnection();
long getWiFiStrength();
void printWifiStatus();

#endif
