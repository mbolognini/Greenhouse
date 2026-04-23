
#ifndef PARAMS_H
#define PARAMS_H

#include "Secrets.h"

/*-------------------------------
------SENSORS AND ACTUATORS------
-------------------------------*/

#define STATE_UPDATE_INTERVAL 1000

// DHT
#define DHTPIN D7
#define DHTTYPE DHT11

// Flame
#define FLAME_PIN D5

// LampLed
#define LAMP_LED_PIN D0

// Buzzer and EmergencyLed
#define BUZZER_LED_PIN D4

// AckButton
#define BUTTON_PIN D3

// Photoresistor
#define PHOTORESISTOR A0

// Servo
#define SERVO_PIN D6
#define SERVO_PWM_MIN 550
#define SERVO_PWM_MAX 2500
#define DELAY 20

// Display
#define DISPLAY_CHARS 16                    // Chars per line
#define DISPLAY_LINES 2                     // Displays per line
#define DISPLAY_ADDR 0x27                   // Display address on I2C bus
#define DISPLAY_UPDATE_INTERVAL 2000

/*-------------------------------
------------------DB-------------
-------------------------------*/

#define INFLUXDB_UPDATE_INTERVAL 15000
#define TZ_INFO "CET-1CEST,M3.5.0,M10.5.0/3"

/*----------------------
-------THRESHOLDS-------
----------------------*/

#define WIFI_POWER_THRESHOLD -75

#define LOW_LIGHT_THRESHOLD 500     // Analog 0-1023, higher when darker

#define COLD_THRESHOLD 5
#define HOT_THRESHOLD 40            // Upper limit is 50

#define DRY_THRESHOLD 25
#define WET_THRESHOLD 75

#endif
