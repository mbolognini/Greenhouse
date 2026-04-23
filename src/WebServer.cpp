#include "WebServer.h"
#include <ESP8266WebServer.h>
#include "Greenhouse.h"
#include "Params.h"
#include <math.h>

ESP8266WebServer server(80);

void handleRoot();
void handleData();
void handleAck();
void handleNotFound();
String SendStatusHTML();

const char* htmlIndex = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Greenhouse Status</title>
  <style>
    body {
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      text-align: center;
      background-color: #f4f7f6;
      margin: 0;
      padding: 20px;
      color: #333;
    }
    h1 { color: #2c3e50; margin-bottom: 30px; }
    .container {
      display: flex;
      flex-wrap: wrap;
      justify-content: center;
      gap: 20px;
      max-width: 800px;
      margin: 0 auto;
    }
    .card {
      background: white;
      border-radius: 10px;
      box-shadow: 0 4px 8px rgba(0,0,0,0.1);
      padding: 20px;
      width: 200px;
      text-align: center;
      transition: transform 0.2s;
    }
    .card:hover { transform: translateY(-5px); }
    .card-title {
      font-size: 1.2rem;
      color: #7f8c8d;
      margin-bottom: 10px;
    }
    .card-value {
      font-size: 2rem;
      font-weight: bold;
      color: #2c3e50;
      transition: color 0.3s;
    }
    .warning-icon {
      color: #e74c3c;
      font-size: 3rem;
      display: none;
      margin-bottom: 20px;
      animation: blink 1s infinite alternate;
    }
    @keyframes blink {
      0% { opacity: 1; }
      100% { opacity: 0.5; }
    }
    .card.emergency {
      background-color: #e74c3c;
    }
    .card.emergency .card-title,
    .card.emergency .card-value,
    .card.emergency {
      color: white;
    }
    #ackBtn {
      display: none;
      padding: 15px 30px;
      font-size: 1.2rem;
      background-color: #e74c3c;
      color: white;
      border: none;
      border-radius: 5px;
      cursor: pointer;
      margin-top: 30px;
      box-shadow: 0 4px 6px rgba(0,0,0,0.1);
      transition: background-color 0.3s;
    }
    #ackBtn:hover { background-color: #c0392b; }
  </style>
</head>
<body>
  <h1>Greenhouse Monitor</h1>
  <div id="warning-icon" class="warning-icon">&#9888; EMERGENCY! &#9888;</div>

  <div class="container">
    <div class="card" id="temp-card">
      <div class="card-title">Temperature</div>
      <div class="card-value"><span id="temp">--</span> &deg;C</div>
    </div>
    <div class="card" id="hum-card">
      <div class="card-title">Humidity</div>
      <div class="card-value"><span id="hum">--</span> %</div>
    </div>
    <div class="card">
      <div class="card-title">Darkness</div>
      <div class="card-value"><span id="darkness">--</span></div>
    </div>
    <div class="card" id="flame-card">
      <div class="card-title">Flame detected</div>
      <div class="card-value"><span id="flame">--</span></div>
    </div>
    <div class="card" id="wifi-card">
      <div class="card-title">WiFi power</div>
      <div class="card-value"><span id="wifi">--</span> dBm</div>
    </div>
  </div>

  <button id="ackBtn" onclick="sendAck()">Acknowledge</button>

  <script>
    function updateData() {
      fetch('/data')
        .then(response => response.json())
        .then(data => {
          const tempEl = document.getElementById('temp');
          tempEl.innerText = data.temp !== null ? data.temp.toFixed(1) : "--";
          document.getElementById('temp-card').classList.toggle('emergency', data.temp_status === 'emergency');

          const humEl = document.getElementById('hum');
          humEl.innerText = data.hum !== null ? data.hum.toFixed(1) : "--";
          document.getElementById('hum-card').classList.toggle('emergency', data.hum_status === 'emergency');

          document.getElementById('darkness').innerText = data.darkness + "%";
          document.getElementById('flame').innerText = data.flame ? "YES" : "NO";
          document.getElementById('flame-card').classList.toggle('emergency', data.flame);
          document.getElementById('wifi').innerText = data.wifi;
          document.getElementById('wifi-card').classList.toggle('emergency', data.wifi_status === 'low');

          if (data.is_emergency && !data.is_acknowledged) {
            document.getElementById('warning-icon').style.display = 'block';
            document.getElementById('ackBtn').style.display = 'inline-block';
          } else {
            document.getElementById('warning-icon').style.display = 'none';
            document.getElementById('ackBtn').style.display = 'none';
          }
        })
        .catch(err => {
          console.error('Error fetching data:', err);
        })
        .finally(() => {
          setTimeout(updateData, 2000);
        });
    }

    function sendAck() {
      fetch('/ack', { method: 'POST' })
        .catch(err => console.error('Error acknowledging:', err));
    }

    updateData();
  </script>
</body>
</html>
)rawliteral";

void setupWebServer() {
    server.on("/", HTTP_GET, handleRoot);
    server.on("/data", HTTP_GET, handleData);
    server.on("/ack", HTTP_POST, handleAck);
    server.onNotFound(handleNotFound);
    server.begin();
    Serial.println("HTTP Server started");
}

void handleRoot() {
  server.send(200, F("text/html"), htmlIndex);
}

void handleData() {
    String normal_status = F("normal");
    String emergency_status = F("emergency");
    String low_status = F("low");

    String temp_status = (s.isTempEmergency) ? emergency_status : normal_status;
    String hum_status = (s.isHumEmergency) ? emergency_status : normal_status;
    String darkness_status = (s.isDark) ? low_status : normal_status;
    String wifi_status = (s.isWifiEmergency) ? low_status : normal_status;

    String json = "{";
    json += "\"temp\":" + String(m.temp, 1) + ",";
    json += "\"temp_status\":\"" + temp_status + "\",";
    json += "\"hum\":" + String(m.hum, 1) + ",";
    json += "\"hum_status\":\"" + hum_status + "\",";
    json += "\"darkness\":" + String(ceil((float)m.light / 1024.0f * 100)) + ",";
    json += "\"darkness_status\":\"" + darkness_status + "\",";
    json += "\"flame\":" + String(s.isFire ? "true" : "false") + ",";
    json += "\"wifi\":" + String((int)m.wifiStrength) + ",";
    json += "\"wifi_status\":\"" + wifi_status + "\",";
    json += "\"is_emergency\":" + String(s.isEmergency ? "true" : "false") + ",";
    json += "\"is_acknowledged\":" + String(s.isAcknowledged ? "true" : "false");
    json += "}";
    server.send(200, F("application/json"), json);
}

void handleAck() {
  if (s.isEmergency)
    s.isAcknowledged = true;
  server.send(200, F("text/plain"), F("OK"));
}

void handleNotFound()
{
  server.send(404, F("text/plain"), F("Not found"));
}

void handleClient() {
    server.handleClient();
}
