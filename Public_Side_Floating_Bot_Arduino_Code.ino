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
    String html = "<h2>Smart Tap Monitoring</h2>";
    html += "<p><strong>Total Water Used: </strong>" + String(totalWaterUsed, 2) + " L</p>";
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
