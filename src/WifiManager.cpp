#include "WifiManager.h"
#include <ESP8266WiFi.h>
#include "Params.h"

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

bool setupWiFi() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(SECRET_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(SECRET_SSID, SECRET_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  printWifiStatus();
  return true;
}

bool checkWiFiConnection() {
    if (WiFi.status() != WL_CONNECTED)
        return setupWiFi();
    return true;
}

long getWiFiStrength() {
  return WiFi.RSSI();
}

void printWifiStatus() {
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());
}
