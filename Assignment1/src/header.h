
#ifndef HEADER_H
#define HEADER_H
/*-------------------------------
-----------LIBRARIES-------------
-------------------------------*/

#include <Servo.h>
#include <ESP8266WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <DHT.h>
#include <Ticker.h>
#include <InfluxDbClient.h>
#include <ESP8266WebServer.h>

/*-------------------------------
------SENSORS AND ACTUATORS------
-------------------------------*/

#define STATE_UPDATE_INTERVAL 1000

// DHT
#define DHTPIN D7
#define DHTTYPE DHT11

// Flame
#define FLAME_PIN D5

// LEDs
#define LAMP_LED_PIN D0

// Buzzer e LedRosso
#define BUZZER_LED_PIN D4

// Button
#define BUTTON_PIN D3

// Photoresistor
#define PHOTORESISTOR A0

// Servo
#define SERVO_PIN D6
#define SERVO_PWM_MIN 550
#define SERVO_PWM_MAX 2500
#define DELAY 20

// Display
#define DISPLAY_CHARS 16                    // number of characters on a line
#define DISPLAY_LINES 2                     // number of display lines
#define DISPLAY_ADDR 0x27                   // display address on I2C bus
#define DISPLAY_UPDATE_INTERVAL 2000        // 2 secondi

/*-------------------------------
-----------WIFI e DB-------------
-------------------------------*/

// WiFi configuration
#define SECRET_SSID "IoTLabThingsU14"              // SSID
#define SECRET_PASS "L@b%I0T*Ui4!P@sS**0%Lessons!" // WiFi password

// InfluxDB cfg
#define INFLUXDB_URL "http://149.132.176.75:8086"                                                                   // IP and port of the InfluxDB server
#define INFLUXDB_TOKEN "FlcfbYuuR_hoX26ghlqvtD0wDJ4p-9yaySFndn2u2hi591m42ZRm_BZSkGqf575RUSKzoHAWMZ87ry5uk7mi9Q=="
#define INFLUXDB_ORG "labiot-org"                                                                                   // organization id (Use: InfluxDB UI -> Profile -> About -> <name under organization profile> )
#define INFLUXDB_BUCKET "GiacomoMaggioni-bucket"                                                                    // bucket name (Use: InfluxDB UI -> Load Data -> Buckets)
#define INFLUXDB_UPDATE_INTERVAL 15000                                                                              // Invia dati ogni 15 secondi

/*----------------------
-------THRESHOLDS-------
----------------------*/

#define WIFI_POWER_THRESHOLD -75 // Sotto i -75 dBm la connessione è considerata debole

#define LOW_LIGHT_THRESHOLD 500 // analogico (0-1023).  Soglia di buio: se valore sopra la soglia, c'è poca luce   (500 va bene per l'aula studio)

#define COLD_THRESHOLD 5 // Vicino al limite minimo del sensore
#define HOT_THRESHOLD 40 // Molto caldo (il sensore arriva a 50)

#define DRY_THRESHOLD 25 // Il sensore legge male sotto il 20%
#define WET_THRESHOLD 75 // Vicino al limite massimo del sensore (80%)

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
};
extern struct Status s;

#endif
