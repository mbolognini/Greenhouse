
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

/*-------------------------------
------SENSORS AND ACTUATORS------
-------------------------------*/

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
#define INFLUXDB_TOKEN "FlcfbYuuR_hoX26ghlqvtD0wDJ4p-9yaySFndn2u2hi591m42ZRm_BZSkGqf575RUSKzoHAWMZ87ry5uk7mi9Q=="   // API authentication token. Use an existing token or create a new one:
                                                                                                                    //  InfluxDB UI -> Load Data -> API Tokens -> Generate API Token -> Custom API Token
                                                                                                                    //  set a name for the token and select read and write permissions for your buket only!
#define INFLUXDB_ORG "labiot-org"                                                                                   // organization id (Use: InfluxDB UI -> Profile -> About -> <name under organization profile> )
#define INFLUXDB_BUCKET "GiacomoMaggioni-bucket"                                                                    // bucket name (Use: InfluxDB UI -> Load Data -> Buckets)
#define INFLUXDB_UPDATE_INTERVAL 15000                                                                              // Invia dati ogni 15 secondi

/*----------------------
-------THRESHOLDS-------
----------------------*/

#define WIFI_POWER_THRESHOLD -75 // Sotto i -75 dBm la connessione è considerata debole

#define LOW_LIGHT_THRESHOLD 100 // analogico (0-1023).  In aula studio c'è 118

#define STRONG_COLD_THRESHOLD 5 // Vicino al limite minimo del sensore
#define WEAK_COLD_THRESHOLD 18  // Fresco, soglia tipica per riscaldamento
#define WEAK_HOT_THRESHOLD 27   // Inizio sensazione di afa
#define STRONG_HOT_THRESHOLD 40 // Molto caldo (il sensore arriva a 50)

#define STRONG_DRY_THRESHOLD 25 // Il sensore legge male sotto il 20%
#define WEAK_DRY_THRESHOLD 35   // Aria secca
#define WEAK_WET_THRESHOLD 65   // Inizio umidità alta
#define STRONG_WET_THRESHOLD 75 // Vicino al limite massimo del sensore (80%)
