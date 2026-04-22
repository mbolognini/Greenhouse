#include "src/WebServer.h"
#include "src/header.h"

ESP8266WebServer server(80);

void setupWebServer() {
    server.on("/", handle_root);
    server.on("/data", handle_data);
    server.on("/ACK", handle_ack);
    server.onNotFound(handle_NotFound);
    server.begin();
    Serial.println("HTTP Server started")
}

void handleClient() {
    server.handleClient();
}

void handle_root() {
  server.send(200, F("text/html"), SendStatusHTML());
}

void handle_data() {
  String json = "{";
  json += "\"temperature\":" + String(m.temp, 1) + ",";
  json += "\"humidity\":" + String(m.hum, 1) + ",";
  json += "\"light\":" + String(m.light) + ",";
  json += "\"flame\":" + String(m.isFire ? "true" : "false") + ",";
  json += "\"wifi\":" + String((int)m.wifiStrength) + ",";
  json += "\"is_emergency\":" + String(s.isEmergency ? "true" : "false") + ",";
  json += "\"is_acknowledged\":" + String(s.isAcknowledged ? "true" : "false");
  json += "}";
  server.send(200, F("application/json"), json);
}

void handle_ack() {
  if (s.isEmergency)
    s.isAcknowledged = true;
  }
  server.send(200, F("text/plain"), F("OK"));
}

void handle_NotFound()
{
  server.send(404, F("text/plain"), F("Not found"));
}

String SendStatusHTML()
{
  String ptr = F(
      "<!DOCTYPE html> <html>\n"
      "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n"
      "<title>System Status</title>\n"
      "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n"
      "body{margin-top: 50px;} h1 {color: #444444;margin: 30px auto 30px;} h3 {color: #444444;margin-bottom: 30px;}\n"
      ".button {display: inline-block; background-color: #f39c12; border: none; color: white; padding: 13px 30px; text-decoration: none; font-size: 20px; margin: 20px auto; cursor: pointer; border-radius: 4px;}\n"
      ".button:active {background-color: #e67e22;}\n"
      ".val {font-size: 18px; color: #333; margin-bottom: 10px;}\n"
      ".warning {color: red; font-size: 60px; margin-bottom: 20px;}\n"
      "</style>\n"
      "<script>\n"
      "function updateData() {\n"
      "  fetch('/data')\n"
      "    .then(response => {\n"
      "      if (!response.ok) throw new Error('Network response was not ok');\n"
      "      return response.json();\n"
      "    })\n"
      "    .then(data => {\n"
      "      document.getElementById('temp').innerHTML = 'Temperature: ' + data.temperature + ' &deg;C';\n"
      "      document.getElementById('hum').innerHTML = 'Humidity: ' + data.humidity + ' %';\n"
      "      document.getElementById('light').innerHTML = 'Light: ' + data.light;\n"
      "      document.getElementById('flame').innerHTML = 'Flame: ' + (data.flame ? 'YES' : 'NO');\n"
      "      document.getElementById('wifi').innerHTML = 'WiFi Power: ' + data.wifi + ' dBm';\n"
      "      let eZone = document.getElementById('emergency-zone');\n"
      "      if(data.is_emergency) {\n"
      "        let html = '<div class=\"warning\">&#9888;</div>\\n';\n"
      "        if(!data.is_acknowledged) {\n"
      "          html += '<a class=\"button\" href=\"#\" onclick=\"fetch(\\'/ACK\\'); return false;\">Acknowledge</a>\\n';\n"
      "        }\n"
      "        eZone.innerHTML = html;\n"
      "      } else {\n"
      "        eZone.innerHTML = '';\n"
      "      }\n"
      "      setTimeout(updateData, 5000);\n"
      "    })\n"
      "    .catch(error => {\n"
      "      console.error('Errore:', error);\n"
      "      setTimeout(updateData, 5000);\n"
      "    });\n"
      "}\n"
      "window.onload = updateData;\n"
      "</script>\n"
      "</head>\n"
      "<body>\n"
      "<h1>System Status</h1>\n"
      "<div id=\"emergency-zone\"></div>\n"
      "<h3>Sensor Values</h3>\n"
      "<div class=\"val\" id=\"temp\">Temperature: -- &deg;C</div>\n"
      "<div class=\"val\" id=\"hum\">Humidity: -- %</div>\n"
      "<div class=\"val\" id=\"light\">Light: --</div>\n"
      "<div class=\"val\" id=\"flame\">Flame: --</div>\n"
      "<div class=\"val\" id=\"wifi\">WiFi Power: -- dBm</div>\n"
      "</body>\n"
      "</html>\n");
  return ptr;
}
