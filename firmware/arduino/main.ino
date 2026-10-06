#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "SEU_WIFI";
const char* password = "SUA_SENHA";

WebServer server(80);

void handleStatus() {

    int battery = 85;
    bool helmet = true;

    float gyroX = 0.2;
    float gyroY = -0.1;
    float gyroZ = 9.8;

    String json = "{";
    json += "\"battery\":" + String(battery) + ",";
    json += "\"helmet\":" + String(helmet ? "true" : "false") + ",";
    json += "\"gyro\":{";
    json += "\"x\":" + String(gyroX) + ",";
    json += "\"y\":" + String(gyroY) + ",";
    json += "\"z\":" + String(gyroZ);
    json += "}";
    json += "}";

    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", json);
}

void setup() {

    Serial.begin(115200);

    WiFi.begin(ssid, password);

    Serial.print("Conectando ao Wi-Fi");

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("Wi-Fi conectado!");

    Serial.print("IP do ESP32: ");
    Serial.println(WiFi.localIP());

    server.on("/api/status", handleStatus);

    server.begin();

    Serial.println("Servidor HTTP iniciado!");
}

void loop() {
    server.handleClient();
}