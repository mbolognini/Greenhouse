#include <Servo.h>
#include <ESP8266WiFi.h>

// SENSORS AND ACTUATORS DATA
#define DHTPIN //TODO  
#define DHTTYPE DHT11
DHT dht = DHT(DHTPIN, DHTTYPE);

#define FLAME_PIN //TODO
#define EMERGENCY_LED_PIN //TODO
#define LAMP_LED_PIN //TODO
#define BUZZER_PIN //TODO

#define SERVO_PIN //TODO
#define SERVO_PWM_MIN 550    
#define SERVO_PWM_MAX 2500
#define DELAY 20 
Servo servo;

char ssid[] = SECRET_SSID;   // your network SSID (name)
char pass[] = SECRET_PASS;   // your network password
#ifdef IP
IPAddress ip(IP);
IPAddress subnet(SUBNET);
IPAddress dns(DNS);
IPAddress gateway(GATEWAY);
#endif
WiFiClient client;

//
// THRESHOLD CONSTANTS
//
#define WIFI_POWER_THRESHOLD //TODO
#define LOW_LIGHT_THRESHOLD //TODO
#define WEAK_COLD_THRESHOLD //TODO
#define WEAK_HOT_THRESHOLD //TODO
#define STRONG_COLD_THRESHOLD //TODO
#define STRONG_HOT_THRESHOLD //TODO
#define WEAK_WET_THRESHOLD //TODO
#define WEAK_DRY_THRESHOLD //TODO
#define STRONG_WET_THRESHOLD //TODO
#define STRONG_DRY_THRESHOLD //TODO

//
// GREENHOUSE STATUS AND READ VALUES
//
bool is_warning;
bool is_emergency;
bool is_bad_air;
bool is_dark;
bool is_fire;

bool flame;
unsigned int light;
float humidity;
float temperature;
unsigned int wifi_power;


void setup() {
  servo.attach(SERVO_PIN, SERVO_PWM_MIN, SERVO_PWM_MAX);
  servo.write(0);

  WiFi.mode(WIFI_STA);
  WiFi.setPhyMode(WIFI_PHY_MODE_11G);

}

void loop() {
  connect_wifi();

  read_status();

  is_warning = false;
  is_emergency = false;
  is_bad_air = false;
  is_fire = false;
  is_dark = false;

  check_temperature();
  check_humidity();
  check_light();
  check_flame();
  check_wifi();

  update_emergency_led();
  update_buzzer();
  update_display();
  update_servo();
  update_lamp();
}

void connect_wifi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print(F("Connecting to SSID: "));
    Serial.println(SECRET_SSID);

#ifdef IP
    WiFi.config(ip, dns, gateway, subnet);
#endif

    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) {
      switch (WiFi.status()) {   
        case WL_NO_SSID_AVAIL:
          Serial.println(F("\nSSID not in range"));
          delay(5000);
          break;
        case WL_CONNECT_FAILED:
          Serial.println(F("\nWiFi connection failed!"));
          delay(5000);
          break;
        case WL_WRONG_PASSWORD:
          Serial.println(F("\nWiFi wrong password!"));
          delay(100);
          break;
        case WL_CONNECTION_LOST:
          Serial.println(F("\nWiFi connection lost!"));
          delay(250);
          break;
        default:
          Serial.print(F("."));
          delay(250);
          break;
      }
    }
    Serial.println(F("\nWiFi connected!"));
  }
}

void read_status() {
  flame = readDigital(FLAME_PIN);

  lightSensorValue = analogRead(PHOTORESISTOR);

  humidity = dht.readHumidity();      // humidity percentage, range 20-80% (±5% accuracy)
  temperature = dht.readTemperature();   // temperature Celsius, range 0-50°C (±2°C accuracy)

  wifi_power = WiFi.RSSI()
}

void check_temperature() {
  if (isnan(temperature)) {
    return;
  }

  if (temperature > WEAK_COLD_THRESHOLD && temperature < WEAK_HOT_THRESHOLD) {
    return;
  }

  if (temperature <= WEAK_COLD_THRESHOLD && temperature > STRONG_COLD_THRESHOLD) {
    is_warning = true;
    return;
  }

  if (temperature >= WEAK_HOT_THRESHOLD && temperature < STRONG_HOT_THRESHOLD) {
    is_warning = true;
    return;
  }
  
  if (temperature <= STRONG_COLD_THRESHOLD) {
    is_emergency = true;
    is_bad_air = true;
    return;
  } 

  if (temperature >= STRONG_HOT_THRESHOLD) {
    is_emergency = true;
    is_bad_air = true;
    return;
  } 
}

void check_humidity() {
  if (isnan(humidity)) {
    return;
  }

  if (humidity > WEAK_DRY_THRESHOLD && humidity < WEAK_WET_THRESHOLD) {
    return;
  }

  if (humidity <= WEAK_DRY_THRESHOLD && humidity > STRONG_DRY_THRESHOLD) {
    is_warning = true;
    return;
  }

  if (humidity >= WEAK_WET_THRESHOLD && humidity < STRONG_WET_THRESHOLD) {
    is_warning = true;
    return;
  }
  
  if (humidity <= STRONG_DRY_THRESHOLD) {
    is_emergency = true;
    is_bad_air = true;
    return;
  } 

  if (humidity >= STRONG_WET_THRESHOLD) {
    is_emergency = true;
    is_bad_air = true;
    return;
  } 
}

void check_light() {
  if (light <= LOW_LIGHT_THRESHOLD) {
    is_dark = true;
    return;
  }
}

void check_flame() {
  if (flame == HIGH) {
    is_emergency = true;
    is_fire = true;
    return;
  }
}

void check_wifi() {
  if (wifi_power <= WIFI_POWER_THRESHOLD) {
    is_emergency = true;
    return;
  }
}

void update_emergency_led() {
  if (is_emergency) {
    digitalWrite(EMERGENCY_LED_PIN, HIGH);
  } else {
    digitalWrite(EMERGENCY_LED_PIN, LOW);
  }
}

void update_buzzer() {
  if (is_emergency) {
    digitalWrite(BUZZER_PIN, HIGH);
  } else {
    digitalWrite(BUZZER_PIN, LOW);
  }
}

void update_display() {
  //print info

  // TODO STAMPARE SU DISPLAY ANZICHè SERIAL
  Serial.println(F("\n=== WiFi connection status ==="));

  // signal strength
  Serial.print(F("Signal strength (RSSI): "));
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");

  // current IP
  Serial.print(F("IP Address: "));
  Serial.println(WiFi.localIP().toString());
  // WiFi.localIP().printTo(Serial);

  // subnet mask
  Serial.print(F("Subnet mask: "));
  Serial.println(WiFi.subnetMask().toString());

  Serial.println(F("==============================\n"));
}

void update_servo() {
  if (is_fire) {
    for (int pos = 0; pos <= 90; pos++) { 
      servo.write(pos);                   
      delay(DELAY);                 
    }
    return;
  }

  if (is_bad_air) {
    for (int pos = 90; pos >= 0; pos--) { 
      servo.write(pos);                   
      delay(DELAY);                 
    }
    return;
  }

  for (int pos = 0; pos <= 90; pos++) { 
      servo.write(pos);                   
      delay(DELAY);                 
  }
}

void update_lamp() {
  if (is_dark) {
    digitalWrite(LAMP_LED_PIN, HIGH);
  } else {
    digitalWrite(LAMP_LED_PIN, LOW);
  }
}