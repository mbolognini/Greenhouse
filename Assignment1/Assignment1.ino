
#include "header.h"
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
    if (millis() - lastStateUpdate >= STATE_UPDATE_INTERVAL)
    {
        lastStateUpdate = millis();

        measure();
        updateStatus();
    }

    check_button();

    updateDisplay();
    updateEmergency();
    updateServo();
    updateLamp();

    updateDatabase();
    handleClient();
}

Status measure() {
    m.temp = getTemperature();
    m.hum = getHumidity();
    m.light = getLight();
    m.isFire = isFlameDetected();
    m.isAckButtonPressed = isAcknowledgmentButtonPressed();
    m.wifiStrength = getWiFiStrength();
    }
}

void updateStatus() {
    is_emergency = false;
    is_bad_air = false;
    is_fire = false;
    is_dark = false;

    checkTemperature();
    checkHumidity();
    checkLight();
    checkFlame();
    checkWifi();
}

void check_temperature() {
    if (isnan(m.temp)) return;

    if (m.temp <= COLD_THRESHOLD || m.temp >= HOT_THRESHOLD) {
        s.isEmergency = true;
        s.isBadAir = true;
        addEmergencyMessage("Extreme temperature!");
    }
}

void check_humidity() {
    if (isnan(m.hum)) return;

    if (m.hum <= DRY_THRESHOLD || m.hum >= WET_THRESHOLD) {
        s.isEmergency = true;
        s.isBadAir = true;
        addEmergencyMessage(F("Extreme humidity!"));
    }
}

void check_light() {
    if (m.light >= LOW_LIGHT_THRESHOLD)
        s.isDark = true;
}

void check_flame() {
  if (m.isFire) {
    s.isFire = true;
    addEmergencyMessage(F("Flame detected!"));
  }
}

void check_wifi() {
  if (m.wifiStrength <= WIFI_POWER_THRESHOLD) {
    s.isEmergency = true;
    addEmergencyMessage(F("Weak WiFi signal!"));
  }
}

void check_button() {
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
    int targetPos = WindowPos::CLOSE;
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
        updateEmergencyBuzzerLed(EmergencyBuzzerLedState::OFF\)
}
