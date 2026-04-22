#include "src/DatabaseManager.h"
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>
#include "src/header.h"
#include "src/WifiManager.h"


// Inizializzazione delle istanze reali
InfluxDBClient client_idb(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);
Point pointDevice("device_status");

void setupDatabase() {
  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

  if (client_idb.validateConnection()) {
    Serial.print(F("Connected to InfluxDB: "));
    Serial.println(client_idb.getServerUrl());
  } else {
    Serial.print(F("InfluxDB connection failed: "));
    Serial.println(client_idb.getLastErrorMessage());
  }
}

void updateDatabase() {
  if (WiFi.status() != WL_CONNECTED) return;

  pointDevice.clearFields();

  if (!isnan(m.temp)) pointDevice.addField("n_temperature", m.hum);
  if (!isnan(m.hum))  pointDevice.addField("n_humidity", m.hum);

  pointDevice.addField("n_light", m.light);
  pointDevice.addField("n_flame", (int)m.isFire);
  pointDevice.addField("n_wifi_power", (int)m.wifiStrength);

  Serial.print(F("Writing to InfluxDB... "));
  if (!client_idb.writePoint(pointDevice)) {
    Serial.print(F("Failed: "));
    Serial.println(client_idb.getLastErrorMessage());
  } else {
    Serial.println(F("Success!"));
  }
}

void checkDatabaseConnection() {
    if (!client_idb.validateConnection()) {
        Serial.print(F("InfluxDB connection lost: "));
        Serial.println(client_idb.getLastErrorMessage());
    }
}
