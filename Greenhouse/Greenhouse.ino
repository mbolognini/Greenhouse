#include <Arduino.h>
#include "src/Greenhouse.h"
#include "src/Params.h"
#include "src/Actuators.h"
#include "src/DatabaseManager.h"
#include "src/Display.h"
#include "src/Sensors.h"
#include "src/WebServer.h"
#include "src/WifiManager.h"

struct Measurements m;
struct Status s;

void setup()
{
    Serial.begin(115200);

    setupSensors();
    setupActuators();
    setupDisplay();
    setupWiFi();
    setupWebServer();
    setupDatabase();
}

void loop()
{
    static unsigned long lastWifiCheck = 0;
    if (millis() - lastWifiCheck > 10000) {
        lastWifiCheck = millis();
        checkWiFiConnection();
    }

    static unsigned long lastDbCheck = 0;
    if (millis() - lastDbCheck >= 60000) {
        lastDbCheck = millis();
        checkDatabaseConnection();
    }

    static unsigned long lastStateUpdate = 0;
    if (millis() - lastStateUpdate >= STATE_UPDATE_INTERVAL) {
        lastStateUpdate = millis();

        measure();
        updateStatus();
    }

    m.isAckButtonPressed = isAcknowledgmentButtonPressed();
    checkButton();

    updateDisplay();
    updateEmergency();
    updateServo();
    updateLamp();

    static unsigned long lastDbUpdate = 0;
    if (millis() - lastDbUpdate >= 5000) {
        lastDbUpdate = millis();
        updateDatabase();
    }

    handleClient();
}

void measure() {
    m.temp = getTemperature();
    m.hum = getHumidity();
    m.light = getLight();
    m.isFire = isFlameDetected();
    m.wifiStrength = getWiFiStrength();
}

void updateStatus() {
    s.isEmergency = false;
    s.isBadAir = false;
    s.isFire = false;
    s.isDark = false;

    clearEmergencyMessages();

    checkTemperature();
    checkHumidity();
    checkLight();
    checkFlame();
    checkWifi();
}

void checkTemperature() {
    if (isnan(m.temp)) return;

    if (m.temp <= COLD_THRESHOLD || m.temp >= HOT_THRESHOLD) {
        s.isEmergency = true;
        s.isBadAir = true;
        addEmergencyMessage(F("Extreme temperature!"));
    }
}

void checkHumidity() {
    if (isnan(m.hum)) return;

    if (m.hum <= DRY_THRESHOLD || m.hum >= WET_THRESHOLD) {
        s.isEmergency = true;
        s.isBadAir = true;
        addEmergencyMessage(F("Extreme humidity!"));
    }
}

void checkLight() {
    if (m.light >= LOW_LIGHT_THRESHOLD)
        s.isDark = true;
}

void checkFlame() {
  if (m.isFire) {
    s.isEmergency = true;
    s.isFire = true;
    addEmergencyMessage(F("Flame detected!"));
  }
}

void checkWifi() {
  if (m.wifiStrength <= WIFI_POWER_THRESHOLD) {
    s.isEmergency = true;
    addEmergencyMessage(F("Weak WiFi signal!"));
  }
}

void checkButton() {
    static bool lastButtonReading = HIGH;
    static unsigned long lastDebounceTime = 0;
    const unsigned long debounceDelay = 50; // 50 millisecondi di debounce

    if (!s.isEmergency) {
        s.isAcknowledged = false;
        return;
    }

    if (m.isAckButtonPressed != lastButtonReading)
        lastDebounceTime = millis();
    if ((millis() - lastDebounceTime) > debounceDelay && m.isAckButtonPressed == LOW)
        s.isAcknowledged = true;

    lastButtonReading = m.isAckButtonPressed;
}

void updateServo() {
    enum WindowPos targetPos = WindowPos::CLOSE;
    if (!s.isFire && s.isBadAir)
        targetPos = WindowPos::OPEN;
    moveWindow(targetPos);
}

void updateLamp() {
  if (s.isDark)
    turnLamp(LampState::ON);
  else
    turnLamp(LampState::OFF);
}

void updateEmergency() {
    if (s.isEmergency && !s.isAcknowledged)
        updateEmergencyBuzzerLed(EmergencyBuzzerLedState::ON);
    else
        updateEmergencyBuzzerLed(EmergencyBuzzerLedState::OFF);
}
