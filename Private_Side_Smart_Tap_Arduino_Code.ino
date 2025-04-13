#include <WiFi.h>
#include <WebServer.h>

// WiFi Credentials
const char* ssid = "POCO";
const char* password = "123456789";

// Pin Setup
const int needlePins[] = {13, 12, 14, 27, 26};
const float flowRates[] = {1.0, 2.0, 3.0, 4.0, 5.0}; // L/min
const int numNeedles = sizeof(needlePins) / sizeof(needlePins[0]);

int activeIndex = -1;
unsigned long openTime = 0;
float totalWaterUsed = 0;

WebServer server(80); // HTTP server on port 80

void setup() {
  Serial.begin(115200);
  
  // Set up pins
  for (int i = 0; i < numNeedles; i++) {
    pinMode(needlePins[i], INPUT);
  }

  // Connect WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi..");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Connected!");
  Serial.println(WiFi.localIP());

  // Webpage route
server.on("/", []() {
  String html = R"rawliteral(
    <!DOCTYPE html>
    <html lang="en">
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>Smart Tap Monitoring</title>
      <style>
        body {
          margin: 0;
          font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
          background-color: #0f172a;
          color: #f1f5f9;
          display: flex;
          justify-content: center;
          align-items: center;
          height: 100vh;
        }
        .container {
          background-color: #1e293b;
          padding: 2rem;
          border-radius: 1rem;
          box-shadow: 0 0 20px rgba(0, 0, 0, 0.5);
          text-align: center;
          width: 90%;
          max-width: 400px;
        }
        h2 {
          color: #38bdf8;
          margin-bottom: 1rem;
        }
        p {
          font-size: 1.2rem;
        }
        strong {
          color: #cbd5e1;
        }
      </style>
    </head>
    <body>
      <div class="container">
        <h2>Smart Tap Monitoring</h2>
        <p><strong>Total Water Used: </strong>)rawliteral";
  html += String(totalWaterUsed, 2);
  html += R"rawliteral( L</p>
      </div>
    </body>
    </html>
  )rawliteral";
  
  server.send(200, "text/html", html);
});


  server.begin();
}

void loop() {
  server.handleClient();

  int detected = -1;
  for (int i = 0; i < numNeedles; i++) {
    if (digitalRead(needlePins[i]) == HIGH) {
      detected = i;
      break;
    }
  }

  if (detected != -1 && activeIndex == -1) {
    activeIndex = detected;
    openTime = millis();
    Serial.print("Tap opened at needle: ");
    Serial.println(detected);
  }

  else if (detected == -1 && activeIndex != -1) {
    unsigned long closeTime = millis();
    float durationMin = (closeTime - openTime) / 60000.0;
    float usedWater = durationMin * flowRates[activeIndex];   
    totalWaterUsed += usedWater;

    Serial.print("Tap closed. Water used: ");
    Serial.print(usedWater);
    Serial.println(" L");

    activeIndex = -1;
  }

  delay(100);
}
