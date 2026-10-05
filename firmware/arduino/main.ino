#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "SEU_WIFI";
const char* password = "SUA_SENHA";

WebServer server(80);

void handleRoot() {
    server.send(200, "text/html",
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
        "<meta charset='UTF-8'>"
        "<title>EPI Inteligente</title>"
        "</head>"
        "<body>"
        "<h1>EPI Inteligente</h1>"
        "<p>Servidor do ESP32 funcionando!</p>"
        "</body>"
        "</html>"
    );
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
    Serial.println("Wifi conectado");

    Serial.print("IP do ESP32: ");
    Serial.println(WiFi.localIP());

    server.on("/", handleRoot);

    server.begin();

    Serial.println("Servidor rodando...");
}

void loop() {
    server.handleClient();
}