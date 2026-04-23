#include "Display.h"
#include <Wire.h>
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "Greenhouse.h"
#include "Params.h"
#include "Sensors.h"
#include "WifiManager.h"


const int maxMessages = 4;
String emergencyMessages[maxMessages];
int currMessageCount = 0;
unsigned long lastDisplayUpdate = 0;
int displayPage = 0;

LiquidCrystal_I2C lcd(DISPLAY_ADDR, DISPLAY_CHARS, DISPLAY_LINES);

void setupDisplay() {
    Wire.begin(); // Su ESP8266 puoi anche specificare i pin: Wire.begin(SDA_PIN, SCL_PIN);
    Wire.beginTransmission(DISPLAY_ADDR);
    byte error = Wire.endTransmission();

    if (error == 0) {
        Serial.println(F("LCD trovato e inizializzato."));
        lcd.begin(DISPLAY_CHARS, 2);
        lcd.setBacklight(255);
        lcd.clear();
        lcd.print(F("Sistema Avviato"));
    } else {
        Serial.print(F("Errore LCD: Display non trovato all'indirizzo 0x"));
        Serial.println(DISPLAY_ADDR, HEX);
    }
}

void clearEmergencyMessages() {
    for (int i = 0; i < maxMessages; i++) {
        emergencyMessages[i] = "";
    }
    currMessageCount = 0;
}

void addEmergencyMessage(String message) {
    if (currMessageCount < maxMessages) {
        emergencyMessages[currMessageCount] = message;
        currMessageCount++;
    }
}

void updateDisplay() {
    if (millis() - lastDisplayUpdate < DISPLAY_UPDATE_INTERVAL) {
        return;
    }
    lastDisplayUpdate = millis();

    lcd.clear();

    if (s.isEmergency) {
        lcd.setCursor(0, 0);
        lcd.print("!!EMERGENZA!!");

        lcd.setCursor(0, 1);
        if (currMessageCount > 0) {
            lcd.print(emergencyMessages[displayPage % currMessageCount]);
        } else {
            lcd.print("Causa non nota");
        }
        displayPage++;
        return;
    }

    int currentPage = displayPage % 2;
    if (currentPage == 0) {
        lcd.setCursor(0, 0);
        lcd.print("T: " + String(m.temp, 1) + "C");
        lcd.setCursor(0, 1);
        lcd.print("H: " + String(m.hum, 1) + "%");
    } else {
        lcd.setCursor(0, 0);
        lcd.print("Luce: " + String(m.light));
        lcd.setCursor(0, 1);
        lcd.print("WiFi: " + String((int)m.wifiStrength) + "dB");
    }

    displayPage++;
}
