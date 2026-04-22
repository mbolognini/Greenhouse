#include "src/Actuators.h"
#include "src/header.h"

Servo servo;

// Variabili per il servo
unsigned long lastServoUpdate = 0;
int currServoPos = 0; // Inizia a 0, come impostato nel setup()

// Variabili per l'emergenza (Ticker)
Ticker emergencyTicker;
bool is_ticker_active = false;


void setupActuators() {
    pinMode(LAMP_LED_PIN, OUTPUT);
    digitalWrite(LAMP_LED_PIN, HIGH);
    pinMode(BUZZER_LED_PIN, OUTPUT);
    digitalWrite(BUZZER_LED_PIN, HIGH);

    servo.attach(SERVO_PIN, SERVO_PWM_MIN, SERVO_PWM_MAX);
    servo.write(currServoPos);
}

void moveWindow(WindowPos targetPosition) {
    if (currServoPos != targetPosition) {
        if (millis() - lastServoUpdate >= DELAY) {
            lastServoUpdate = millis();
            if (currServoPos < targetPosition)
                currServoPos++;
            else
                currServoPos--;
            servo.write(currServoPos);
        }
    }
}

void turnLamp(LampState targetState) {
    if (targetState == 1)
        digitalWrite(LAMP_LED_PIN, LOW);
    else
        digitalWrite(LAMP_LED_PIN, HIGH);
}

void toggle_emergency_alert() {
    digitalWrite(BUZZER_LED_PIN, !digitalRead(BUZZER_LED_PIN));
}

void updateEmergencyBuzzerLed(EmergencyBuzzerLedState targetState) {
    if (targetState == EmergencyBuzzerLedState::OFF) {
        if (is_ticker_active) {
            emergencyTicker.detach();
            is_ticker_active = false;
            digitalWrite(BUZZER_LED_PIN, HIGH);
        }
    }
    else {
        if (!is_ticker_active) {
            emergencyTicker.attach(0.5, toggle_emergency_alert);
            is_ticker_active = true;
        }
    }
}
