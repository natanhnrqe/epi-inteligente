#include "WebServerManager.h"

WebServerManager::WebServerManager()
    : server(80) {
}

bool WebServerManager::begin(const char* ssid, const char* password) {

    if (!LittleFS.begin(false)) {
        Serial.println("Erro ao inicializar LittleFS!");
        return false;
    }

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    Serial.print("Conectando ao Wi-Fi");

    unsigned long startAttempt = millis();

    while (WiFi.status() != WL_CONNECTED &&
           millis() - startAttempt < 15000) {
        delay(250);
        Serial.print(".");
           }

    Serial.println();

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Falha na conexão Wi-Fi!");
        return false;
    }

    configureRoutes();

    server.begin();

    Serial.println("Servidor HTTP iniciado!");
    Serial.print("Acesse: http://");
    Serial.println(getIpAddress());

    return true;
}

void WebServerManager::configureRoutes() {

    server.on("/api/status", HTTP_GET, [this]() {

    String json = "{";

    json += "\"battery\":" + String(status.battery) + ",";

    json += "\"accelerometer\":{";
    json += "\"x\":" + String(status.sensorData.accelerationX, 2) + ",";
    json += "\"y\":" + String(status.sensorData.accelerationY, 2) + ",";
    json += "\"z\":" + String(status.sensorData.accelerationZ, 2);
    json += "},";

    json += "\"gyroscope\":{";
    json += "\"x\":" + String(status.sensorData.gyroscopeX, 2) + ",";
    json += "\"y\":" + String(status.sensorData.gyroscopeY, 2) + ",";
    json += "\"z\":" + String(status.sensorData.gyroscopeZ, 2);
    json += "},";

    json += "\"buzzer\":" + String(status.buzzerActive ? "true" : "false") + ",";
    json += "\"fallState\":\"" + status.fallState + "\",";
    json += "\"emergency\":" + String(status.emergency ? "true" : "false") + ",";
    json += "\"temperature\":" + String(status.sensorData.temperature, 2);

    json += "}";

    server.on("/", HTTP_GET, [this]() {
        File file = LittleFS.open("/index.html", "r");

        if (!file) {
            server.send(404, "text/plain", "index.html nao encontrado");
            return;
        }

        server.streamFile(file, "text/html");
        file.close();
    });

    server.serveStatic("/", LittleFS, "/");

    server.onNotFound([this]() {
        server.send(404, "text/plain", "Rota nao encontrada");
    });

    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "application/json", json);
});
}

void WebServerManager::handleClient() {
    server.handleClient();
}

String WebServerManager::getIpAddress() const {
    return WiFi.localIP().toString();
}

void WebServerManager::updateStatus(const SystemStatus& newStatus) {
    status = newStatus;
}