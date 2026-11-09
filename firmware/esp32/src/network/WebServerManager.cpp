#include "WebServerManager.h"
#include <ArduinoJson.h>

WebServerManager::WebServerManager(
    AlertManager& alertManager,
    FallDetector& fallDetector
)
    : server(80),
      alertManager(alertManager),
      fallDetector(fallDetector) {
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

    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "application/json", json);
    });

    // POST /api/emergency/reset
    server.on("/api/emergency/reset", HTTP_POST, [this]() {
        fallDetector.reset();

        server.send(200, "application/json",
                    "{\"message\":\"Emergencia reiniciada\"}");
    });

    // GET /api/funcionario
    server.on("/api/funcionario", HTTP_GET, [this]() {
        if (!profileStorage.exists()) {
            server.send(404, "application/json",
                        "{\"message\":\"Perfil nao encontrado\"}");
            return;
        }

        EmergencyProfile profile;

        if (!profileStorage.load(profile)) {
            server.send(500, "application/json",
                        "{\"message\":\"Erro ao carregar perfil\"}");
            return;
        }

        JsonDocument doc;

        doc["nome"] = profile.name;
        doc["contato_nome"] = profile.contactName;
        doc["contato_telefone"] = profile.contactPhone;
        doc["tipo_sanguineo"] = profile.blood;
        doc["alergias"] = profile.allergies;
        doc["observacoes"] = profile.observations;

        String response;
        serializeJson(doc, response);

        server.sendHeader("Cache-Control", "no-store");
        server.send(200, "application/json", response);
    });

    // POST /api/funcionario
    server.on("/api/funcionario", HTTP_POST, [this]() {
        if (profileStorage.exists()) {
            server.send(409, "application/json",
                        "{\"message\":\"Este capacete ja possui um perfil\"}");
            return;
        }

        JsonDocument doc;

        DeserializationError error =
            deserializeJson(doc, server.arg("plain"));

        if (error) {
            server.send(400, "application/json",
                        "{\"message\":\"JSON invalido\"}");
            return;
        }

        EmergencyProfile profile;

        profile.name = doc["nome"] | "";
        profile.contactName = doc["contato_nome"] | "";
        profile.contactPhone = doc["contato_telefone"] | "";
        profile.blood = doc["tipo_sanguineo"] | "";
        profile.allergies = doc["alergias"] | "";
        profile.observations = doc["observacoes"] | "";

        // Validação no próprio ESP32.
        if (profile.name.length() < 3 ||
            profile.contactName.length() < 2 ||
            profile.contactPhone.length() < 10) {
            server.send(400, "application/json",
                        "{\"message\":\"Dados obrigatorios invalidos\"}");
            return;
        }

        if (!profileStorage.save(profile)) {
            server.send(500, "application/json",
                        "{\"message\":\"Erro ao salvar perfil\"}");
            return;
        }

        server.send(201, "application/json",
                    "{\"message\":\"Perfil cadastrado com sucesso\"}");
    });

    // DELETE /api/funcionario
    server.on("/api/funcionario", HTTP_DELETE, [this]() {
        if (!profileStorage.exists()) {
            server.send(404, "application/json",
                        "{\"message\":\"Perfil nao encontrado\"}");
            return;
        }

        if (!profileStorage.remove()) {
            server.send(500, "application/json",
                        "{\"message\":\"Erro ao excluir perfil\"}");
            return;
        }

        server.send(200, "application/json",
                    "{\"message\":\"Perfil excluido com sucesso\"}");
    });

        // POST /api/buzzer/on
server.on("/api/buzzer/on", HTTP_POST, [this]() {
    alertManager.attentionAlert();

    server.send(
        200,
        "application/json",
        "{\"message\":\"Buzzer acionado\"}"
    );
});

// POST /api/buzzer/off
server.on("/api/buzzer/off", HTTP_POST, [this]() {
    alertManager.stop();

    server.send(
        200,
        "application/json",
        "{\"message\":\"Buzzer desligado\"}"
    );
});



 server.on("/", HTTP_GET, [this]() {
     File file = LittleFS.open("/index.html", FILE_READ);

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