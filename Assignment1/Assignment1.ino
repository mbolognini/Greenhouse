
#include "header.h"

// DHT
DHT dht = DHT(DHTPIN, DHTTYPE);
// Servo
Servo servo;
// WiFi
char ssid[] = SECRET_SSID; // your network SSID (name)
char pass[] = SECRET_PASS; // your network password
WiFiClient client;
// Display
LiquidCrystal_I2C lcd(DISPLAY_ADDR, DISPLAY_CHARS, DISPLAY_LINES);
// InfluxDB cfg
InfluxDBClient client_idb(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);
Point pointDevice("device_status");

//
// STATUS AND READ VALUES
//
bool is_warning;
bool is_emergency;
bool is_bad_air;
bool is_dark;
bool is_fire;

bool flame_value;
unsigned int light_value;
float humidity_value;
float temperature_value;
unsigned int wifi_power_value;
bool button_value;
bool is_acknowledged = false;

// Variabili per la gestione del display
unsigned long lastDisplayUpdate = 0;
int displayPage = 0;

// Variabili per l'aggiornamento del DB
unsigned long lastDbUpdate = 0;

// Variabili per l'emergenza (Ticker)
Ticker emergencyTicker;
bool is_ticker_active = false;

// Variabili per il servo
unsigned long last_servo_update = 0;
int current_servo_pos = 0; // Inizia a 0, come impostato nel setup()

void setup()
{
  pinMode(LAMP_LED_PIN, OUTPUT);
  digitalWrite(LAMP_LED_PIN, HIGH);
  pinMode(BUZZER_LED_PIN, OUTPUT);
  digitalWrite(BUZZER_LED_PIN, HIGH);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(FLAME_PIN, INPUT);
  pinMode(PHOTORESISTOR, INPUT);
  pinMode(SERVO_PIN, OUTPUT);

  Serial.begin(115200);
  Serial.println();
  Serial.println(F("--- System Starting ---"));

  WiFi.mode(WIFI_STA);

  dht.begin();

  Wire.begin();
  Wire.beginTransmission(DISPLAY_ADDR);
  byte error = Wire.endTransmission();

  servo.attach(SERVO_PIN, SERVO_PWM_MIN, SERVO_PWM_MAX);
  servo.write(current_servo_pos);

  connectToWiFi();
  check_influxdb();

  Serial.println(F("--- System Started ---"));
}

void loop()
{

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
  check_button();

  update_emergency();
  update_display();
  update_servo();
  update_lamp();

  update_db();
}

long connectToWiFi()
{
  long rssi_strength;
  // connect to WiFi (if not already connected)
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(F("Connecting to SSID: "));
    Serial.println(ssid);

    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED)
    {
      Serial.print(F("."));
      delay(250);
    }
    Serial.println(F("\nConnected!"));
    rssi_strength = WiFi.RSSI(); // get wifi signal strength
    printWifiStatus();
  }
  else
  {
    rssi_strength = WiFi.RSSI(); // get wifi signal strength
  }

  return rssi_strength;
}

void printWifiStatus()
{
  Serial.println(F("\n=== WiFi connection status ==="));

  // SSID
  Serial.print(F("SSID: "));
  Serial.println(WiFi.SSID());

  // signal strength
  Serial.print(F("Signal strength (RSSI): "));
  Serial.print(WiFi.RSSI());
  Serial.println(F(" dBm"));

  // current IP
  Serial.print(F("IP Address: "));
  Serial.println(WiFi.localIP().toString());
  // WiFi.localIP().printTo(Serial);

  // subnet mask
  Serial.print(F("Subnet mask: "));
  Serial.println(WiFi.subnetMask().toString());

  // gateway
  Serial.print(F("Gateway IP: "));
  Serial.println(WiFi.gatewayIP().toString());

  // DNS
  Serial.print(F("DNS IP: "));
  Serial.println(WiFi.dnsIP().toString());

  Serial.println(F("==============================\n"));
}

void read_status()
{

  wifi_power_value = connectToWiFi();
  light_value = analogRead(PHOTORESISTOR);
  humidity_value = dht.readHumidity();
  temperature_value = dht.readTemperature();
  flame_value = digitalRead(FLAME_PIN);
  button_value = digitalRead(BUTTON_PIN);
}

void check_temperature()
{
  if (isnan(temperature_value))
    return;

  if (temperature_value <= STRONG_COLD_THRESHOLD || temperature_value >= STRONG_HOT_THRESHOLD)
  {
    is_emergency = true;
    is_bad_air = true;
  }
  else if (temperature_value <= WEAK_COLD_THRESHOLD || temperature_value >= WEAK_HOT_THRESHOLD)
  {
    is_warning = true;
  }
}

void check_humidity()
{
  if (isnan(humidity_value))
    return;

  if (humidity_value <= STRONG_DRY_THRESHOLD || humidity_value >= STRONG_WET_THRESHOLD)
  {
    is_emergency = true;
    is_bad_air = true;
  }
  else if (humidity_value <= WEAK_DRY_THRESHOLD || humidity_value >= WEAK_WET_THRESHOLD)
  {
    is_warning = true;
  }
}

void check_light()
{
  if (light_value <= LOW_LIGHT_THRESHOLD)
  {
    is_dark = true;
  }
}

void check_flame()
{
  if (flame_value == HIGH)
  {
    is_emergency = true;
    is_fire = true;
  }
}

void check_button()
{
  static bool last_button_reading = HIGH;
  static unsigned long last_debounce_time = 0;
  const unsigned long debounce_delay = 50; // 50 millisecondi di debounce

  if (!is_emergency)
  {
    is_acknowledged = false; // Resetta lo stato se l'emergenza è rientrata
  }
  else
  {
    // Se la lettura è cambiata rispetto all'ultima volta, resetta il timer
    if (button_value != last_button_reading)
    {
      last_debounce_time = millis();
    }

    // Se il valore è rimasto stabile per il tempo di debounce
    if ((millis() - last_debounce_time) > debounce_delay)
    {
      if (button_value == LOW)
      {
        if (!is_acknowledged)
        {
          Serial.println(F("Button pressed: Emergency acknowledged."));
        }
        is_acknowledged = true;
      }
    }
    last_button_reading = button_value;
  }
}

void check_wifi()
{
  if (wifi_power_value <= WIFI_POWER_THRESHOLD)
  {
    is_emergency = true;
  }
}

void check_influxdb()
{
  // check InfluxDB server connection
  if (client_idb.validateConnection())
  {
    Serial.print(F("Connected to InfluxDB: "));
    Serial.println(client_idb.getServerUrl());
  }
  else
  {
    Serial.print(F("InfluxDB connection failed: "));
    Serial.println(client_idb.getLastErrorMessage());
  }
}

void update_servo()
{
  int target_pos = 90; // Posizione normale (chiuso)

  if (is_fire)
  {
    target_pos = 90; // Fuoco: chiudi l'aria
  }
  else if (is_bad_air)
  {
    target_pos = 0; // Aria cattiva: apri l'aria
  }

  // Muovi il servo di un grado alla volta verso target_pos in modo non bloccante
  if (current_servo_pos != target_pos)
  {
    if (millis() - last_servo_update >= DELAY)
    {
      last_servo_update = millis();
      if (current_servo_pos < target_pos)
      {
        current_servo_pos++;
      }
      else
      {
        current_servo_pos--;
      }
      servo.write(current_servo_pos);
    }
  }
}

void update_lamp()
{
  if (is_dark)
  {
    digitalWrite(LAMP_LED_PIN, LOW);
  }
  else
  {
    digitalWrite(LAMP_LED_PIN, HIGH);
  }
}

void toggle_emergency_alert()
{
  digitalWrite(BUZZER_LED_PIN, !digitalRead(BUZZER_LED_PIN));
}

void update_emergency()
{
  // L'allarme suona solo se c'è emergenza E non è stata presa visione
  if (is_emergency && !is_acknowledged)
  {
    // Se non è già attivo, avviamo il ticker (es. ogni 0.5 secondi)
    if (!is_ticker_active)
    {
      emergencyTicker.attach(0.5, toggle_emergency_alert);
      is_ticker_active = true;
    }
  }
  else
  {
    // Se l'emergenza è rientrata o l'utente ha premuto il bottone, spegniamo il pin
    if (is_ticker_active)
    {
      emergencyTicker.detach();
      is_ticker_active = false;
      digitalWrite(BUZZER_LED_PIN, HIGH); // Logica invertita: HIGH = SPENTO
    }
  }
}

void update_display()
{
  // Esegui l'aggiornamento solo ogni 'DISPLAY_UPDATE_INTERVAL' millisecondi
  if (millis() - lastDisplayUpdate < DISPLAY_UPDATE_INTERVAL)
  {
    return;
  }
  lastDisplayUpdate = millis();

  Serial.println(F("--- Sensor Values ---"));

  Serial.print(F("Temp: "));
  Serial.print(temperature_value, 1);
  Serial.print(F(" C "));
  if (temperature_value <= WEAK_COLD_THRESHOLD)
    Serial.println(F("*<>"));
  else if (temperature_value >= WEAK_HOT_THRESHOLD)
    Serial.println(F("<>*"));
  else
    Serial.println(F("<*>"));

  Serial.print(F("Hum: "));
  Serial.print(humidity_value, 1);
  Serial.print(F(" % "));
  if (humidity_value <= WEAK_DRY_THRESHOLD)
    Serial.println(F("*<>"));
  else if (humidity_value >= WEAK_WET_THRESHOLD)
    Serial.println(F("<>*"));
  else
    Serial.println(F("<*>"));

  Serial.print(F("Light: "));
  Serial.print(light_value);
  Serial.print(F(" "));
  if (light_value <= LOW_LIGHT_THRESHOLD)
    Serial.println(F("*<>"));
  else
    Serial.println(F("<*>"));

  Serial.print(F("Flame: "));
  Serial.print(flame_value ? F("YES") : F("NO"));
  Serial.print(F(" "));
  if (flame_value == HIGH)
    Serial.println(F("<>*"));
  else
    Serial.println(F("<*>"));

  Serial.print(F("WiFi: "));
  Serial.print((int)wifi_power_value);
  Serial.print(F(" dBm "));
  if (wifi_power_value <= WIFI_POWER_THRESHOLD)
    Serial.println(F("*<>"));
  else
    Serial.println(F("<*>"));

  Serial.println(F("---------------------"));

  lcd.clear();

  if (is_emergency)
  {
    lcd.setCursor(0, 0);
    lcd.print("!!EMERGENZA!!");

    // Creiamo una lista di messaggi di emergenza da ciclare
    String emergencyMessages[4];
    int messageCount = 0;

    // Controlla le cause specifiche dell'emergenza
    if (is_fire)
    {
      emergencyMessages[messageCount++] = "FUOCO RILEVATO!";
    }
    if (temperature_value <= STRONG_COLD_THRESHOLD || temperature_value >= STRONG_HOT_THRESHOLD)
    {
      emergencyMessages[messageCount++] = "Temp: " + String(temperature_value, 1) + "C";
    }
    if (humidity_value <= STRONG_DRY_THRESHOLD || humidity_value >= STRONG_WET_THRESHOLD)
    {
      emergencyMessages[messageCount++] = "Hum: " + String(humidity_value, 1) + "%";
    }
    if (wifi_power_value <= WIFI_POWER_THRESHOLD)
    {
      emergencyMessages[messageCount++] = "WiFi: " + String(wifi_power_value) + "dBm";
    }

    // Se ci sono messaggi, ciclali sulla seconda riga
    if (messageCount > 0)
    {
      lcd.setCursor(0, 1);
      // Usa modulo per ciclare tra i messaggi disponibili
      lcd.print(emergencyMessages[displayPage % messageCount]);
    }
    else
    {
      // Fallback nel caso in cui is_emergency sia true ma nessuna causa trovata
      lcd.setCursor(0, 1);
      lcd.print("Causa non nota");
    }
  }
  else
  {
    // Funzionamento normale: cicla 2 pagine di informazioni
    int currentPage = displayPage % 2;
    if (currentPage == 0)
    {
      // Pagina 1: Temperatura e Umidità
      lcd.setCursor(0, 0);
      lcd.print("Temp: " + String(temperature_value, 1) + " C");
      lcd.setCursor(0, 1);
      lcd.print("Hum:  " + String(humidity_value, 1) + " %");
    }
    else
    { // currentPage == 1
      // Pagina 2: Luce e WiFi
      lcd.setCursor(0, 0);
      lcd.print("Luce: " + String(light_value));
      lcd.setCursor(0, 1);
      lcd.print("WiFi: " + String(wifi_power_value) + " dBm");
    }
  }

  // Incrementa l'indice della pagina per il prossimo ciclo
  displayPage++;
}

void update_db()
{
  // Esegui l'aggiornamento solo ogni 'INFLUXDB_UPDATE_INTERVAL' millisecondi
  if (millis() - lastDbUpdate < INFLUXDB_UPDATE_INTERVAL)
  {
    return;
  }
  lastDbUpdate = millis();

  // Pulisce i campi del punto per il nuovo invio
  pointDevice.clearFields();

  // Aggiunge i valori dei sensori.
  // È buona norma controllare i valori non validi (NaN) prima di inviarli.
  if (!isnan(temperature_value))
  {
    pointDevice.addField("temperature", temperature_value);
  }
  if (!isnan(humidity_value))
  {
    pointDevice.addField("humidity", humidity_value);
  }
  pointDevice.addField("light", light_value);
  pointDevice.addField("flame", flame_value);

  // Aggiunge gli stati del sistema
  pointDevice.addField("lamp_on", is_dark);
  pointDevice.addField("air_vent_pos", current_servo_pos);
  pointDevice.addField("wifi_power", (int)wifi_power_value);
  pointDevice.addField("emergency", is_emergency);

  // Scrive il punto nel bucket di InfluxDB solo se il WiFi è connesso
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.print(F("Writing data to InfluxDB... "));
    if (!client_idb.writePoint(pointDevice))
    {
      Serial.print(F("Failed: "));
      Serial.println(client_idb.getLastErrorMessage());
    }
    else
    {
      Serial.println(F("Success!"));
    }
  }
  else
  {
    Serial.println(F("WiFi disconnected. Skipping InfluxDB update."));
  }
}
