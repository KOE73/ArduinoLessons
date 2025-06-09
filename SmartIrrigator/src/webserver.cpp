#include <WiFi.h>
#include <WebServer.h>
#include "webserver.h"
#include "scheduler.h"
#include "config.h"

WebServer server(SERVER_PORT);

const char* ssid = "MyWiFi";          // 🔧 сюда можно вставить сохранённые настройки
const char* password = "password123"; // или подгружать из EEPROM

void setupWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");

    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_CONNECT_TIMEOUT) {
        Serial.print(".");
        delay(500);
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected! IP: " + WiFi.localIP().toString());
    } else {
        Serial.println("\nFailed to connect. Starting AP mode...");

        WiFi.softAP(AP_SSID, AP_PASSWORD);
        Serial.println("AP IP address: " + WiFi.softAPIP().toString());
    }
}

void handleRoot() {
    server.send(200, "text/html", "<h1>Irrigation System</h1><p><a href='/manual?on=1'>Pump ON</a> | <a href='/manual?on=0'>Pump OFF</a></p>");
}

void handleManualControl() {
    if (server.hasArg("on")) {
        setManualIrrigation(server.arg("on") == "1");
    }
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "");
}

void setupWebServer() {
    server.on("/", handleRoot);
    server.on("/manual", handleManualControl);
    server.begin();
}

void handleWebRequests() {
    server.handleClient();
}
