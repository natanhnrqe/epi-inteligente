#include "WebServerManager.h"

#include <ArduinoJson.h>
#include <time.h>
#include "esp_partition.h"

#include "../logging/SystemLogger.h"

WebServerManager::WebServerManager(
    AlertManager& alertManager,
    FallDetector& fallDetector,
    EmergencyProfile& emergencyProfile
)
    : server(80),
      alertManager(alertManager),
      fallDetector(fallDetector),
      emergencyProfile(emergencyProfile) {
}

bool WebServerManager::begin(
    const char* ssid,
    const char* password
) {


    // Inicializa o sistema de arquivos.
    if (!LittleFS.begin(
         false,
         "/littlefs",
         10,
         "spiffs"
     )){
        Serial.println("ERRO: Falha ao inicializar LittleFS!");
        return false;
    }

    SystemLogger::begin();
    SystemLogger::info("Sistema de arquivos inicializado");

    // Inicializa Wi-Fi sem bloquear o restante do sistema.
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.begin(ssid, password);

    wifiStartTime = millis();
    lastReconnectAttempt = millis();

    SystemLogger::info("Conexao Wi-Fi iniciada");

    // Registra todas as rotas.
    configureRoutes();

    // Inicia servidor mesmo sem conexão Wi-Fi.
    server.begin();

    SystemLogger::info("Servidor HTTP iniciado na porta 80");

    return true;
}

void WebServerManager::configureRoutes() {

    // ==========================================
    // GET /api/status
    // ==========================================

    server.on("/api/status", HTTP_GET, [this]() {

        JsonDocument doc;

        doc["battery"] = status.battery;
        doc["sensorReady"] = status.sensorReady;

        JsonObject accelerometer =
            doc["accelerometer"].to<JsonObject>();

        accelerometer["x"] = status.sensorData.accelerationX;
        accelerometer["y"] = status.sensorData.accelerationY;
        accelerometer["z"] = status.sensorData.accelerationZ;

        JsonObject gyroscope =
            doc["gyroscope"].to<JsonObject>();

        gyroscope["x"] = status.sensorData.gyroscopeX;
        gyroscope["y"] = status.sensorData.gyroscopeY;
        gyroscope["z"] = status.sensorData.gyroscopeZ;

        doc["buzzer"] = status.buzzerActive;
        doc["fallState"] = status.fallState;
        doc["emergency"] = status.emergency;
        doc["temperature"] = status.sensorData.temperature;

        String response;
        serializeJson(doc, response);

        server.sendHeader("Cache-Control", "no-store");
        server.send(200, "application/json", response);
    });


    // ==========================================
    // POST /api/emergency/reset
    // ==========================================

    server.on("/api/emergency/reset", HTTP_POST, [this]() {

        fallDetector.reset();

        SystemLogger::info("Detector de queda reiniciado");

        server.send(
            200,
            "application/json",
            "{\"message\":\"Emergencia reiniciada\"}"
        );
    });


    // ==========================================
    // GET /api/funcionario
    // ==========================================

    server.on("/api/funcionario", HTTP_GET, [this]() {

        if (!profileStorage.exists()) {
            server.send(
                404,
                "application/json",
                "{\"message\":\"Perfil nao encontrado\"}"
            );
            return;
        }

        EmergencyProfile profile;

        if (!profileStorage.load(profile)) {

            SystemLogger::error("Falha ao carregar perfil");

            server.send(
                500,
                "application/json",
                "{\"message\":\"Erro ao carregar perfil\"}"
            );
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

        server.send(
            200,
            "application/json",
            response
        );
    });


    // ==========================================
    // POST /api/funcionario
    // ==========================================

    server.on("/api/funcionario", HTTP_POST, [this]() {

        if (profileStorage.exists()) {
            server.send(
                409,
                "application/json",
                "{\"message\":\"Este capacete ja possui um perfil\"}"
            );
            return;
        }

        JsonDocument doc;

        DeserializationError error =
            deserializeJson(doc, server.arg("plain"));

        if (error) {

            SystemLogger::warn(
                "Recebido JSON invalido no cadastro"
            );

            server.send(
                400,
                "application/json",
                "{\"message\":\"JSON invalido\"}"
            );
            return;
        }

        EmergencyProfile profile;

        profile.name = doc["nome"] | "";
        profile.contactName = doc["contato_nome"] | "";
        profile.contactPhone = doc["contato_telefone"] | "";
        profile.blood = doc["tipo_sanguineo"] | "";
        profile.allergies = doc["alergias"] | "";
        profile.observations = doc["observacoes"] | "";

        // Validação dos dados obrigatórios.
        if (
            profile.name.length() < 3 ||
            profile.contactName.length() < 2 ||
            profile.contactPhone.length() < 10
        ) {

            SystemLogger::warn(
                "Dados obrigatorios do perfil invalidos"
            );

            server.send(
                400,
                "application/json",
                "{\"message\":\"Dados obrigatorios invalidos\"}"
            );
            return;
        }

        if (!profileStorage.save(profile)) {

            SystemLogger::error("Falha ao salvar perfil");

            server.send(
                500,
                "application/json",
                "{\"message\":\"Erro ao salvar perfil\"}"
            );
            return;
        }

        // Atualiza o perfil compartilhado em memória.
        emergencyProfile = profile;

        // Solicita atualização do display no loop principal.
        profileDisplayRefreshRequested = true;

        SystemLogger::info(
            "Perfil do funcionario cadastrado"
        );

        server.send(
            201,
            "application/json",
            "{\"message\":\"Perfil cadastrado com sucesso\"}"
        );
    });


    // ==========================================
    // POST /api/funcionario/refresh-display
    // ==========================================

    server.on(
        "/api/funcionario/refresh-display",
        HTTP_POST,
        [this]() {

            if (!profileStorage.exists()) {

                server.send(
                    404,
                    "application/json",
                    "{\"message\":\"Perfil nao encontrado\"}"
                );
                return;
            }

            EmergencyProfile updatedProfile;

            if (!profileStorage.load(updatedProfile)) {

                SystemLogger::error(
                    "Falha ao atualizar perfil do display"
                );

                server.send(
                    500,
                    "application/json",
                    "{\"message\":\"Erro ao carregar perfil\"}"
                );
                return;
            }

            emergencyProfile = updatedProfile;

            profileDisplayRefreshRequested = true;

            SystemLogger::info(
                "Perfil do display atualizado"
            );

            server.send(
                200,
                "application/json",
                "{\"message\":\"Perfil do display atualizado\"}"
            );
        }
    );


    // ==========================================
    // DELETE /api/funcionario
    // ==========================================

    server.on("/api/funcionario", HTTP_DELETE, [this]() {

        if (!profileStorage.exists()) {

            server.send(
                404,
                "application/json",
                "{\"message\":\"Perfil nao encontrado\"}"
            );
            return;
        }

        if (!profileStorage.remove()) {

            SystemLogger::error(
                "Falha ao excluir perfil"
            );

            server.send(
                500,
                "application/json",
                "{\"message\":\"Erro ao excluir perfil\"}"
            );
            return;
        }

        // Limpa o perfil compartilhado em memória.
        emergencyProfile = EmergencyProfile{};

        // Solicita atualização do display.
        profileDisplayRefreshRequested = true;

        SystemLogger::info(
            "Perfil do funcionario removido"
        );

        server.send(
            200,
            "application/json",
            "{\"message\":\"Perfil excluido com sucesso\"}"
        );
    });


    // ==========================================
    // POST /api/buzzer/on
    // ==========================================

    server.on("/api/buzzer/on", HTTP_POST, [this]() {

        alertManager.attentionAlert();

        SystemLogger::info(
            "Comando recebido: acionar buzzer"
        );

        server.send(
            200,
            "application/json",
            "{\"message\":\"Buzzer acionado\"}"
        );
    });


    // ==========================================
    // POST /api/buzzer/off
    // ==========================================

    server.on("/api/buzzer/off", HTTP_POST, [this]() {

        alertManager.stop();

        SystemLogger::warn(
            "Comando recebido: desligar buzzer"
        );

        server.send(
            200,
            "application/json",
            "{\"message\":\"Buzzer desligado\"}"
        );
    });


    // ==========================================
    // GET /api/logs
    // ==========================================

    server.on("/api/logs", HTTP_GET, [this]() {

        File file = LittleFS.open(
            "/system.log",
            FILE_READ
        );

        if (!file) {

            server.send(
                404,
                "text/plain",
                "Arquivo de log nao encontrado"
            );
            return;
        }

        server.sendHeader("Cache-Control", "no-store");

        server.streamFile(file, "text/plain");

        file.close();
    });


    // ==========================================
    // GET /
    // ==========================================

    server.on("/", HTTP_GET, [this]() {

        File file = LittleFS.open(
            "/index.html",
            FILE_READ
        );

        if (!file) {

            server.send(
                404,
                "text/plain",
                "index.html nao encontrado"
            );
            return;
        }

        server.streamFile(file, "text/html");

        file.close();
    });


    // ==========================================
    // Arquivos estáticos
    // ==========================================

    server.serveStatic("/", LittleFS, "/");


    // ==========================================
    // Rota inexistente
    // ==========================================

    server.onNotFound([this]() {

        server.send(
            404,
            "text/plain",
            "Rota nao encontrada"
        );
    });
}


// ==========================================
// HTTP + Wi-Fi
// ==========================================

void WebServerManager::handleClient() {

    server.handleClient();

    updateWifiConnection();
}


void WebServerManager::updateWifiConnection() {

    unsigned long now = millis();

    bool connected = WiFi.status() == WL_CONNECTED;

    // Detecta conexão nova ou reconexão.
    if (connected && !wasWifiConnected) {

        wasWifiConnected = true;
        connectionTimeoutLogged = false;

        SystemLogger::info("Wi-Fi conectado");

        printNetworkInfo();

        // Horário de Brasília: UTC-3.
        configTime(
            -3 * 3600,
            0,
            "pool.ntp.org",
            "time.nist.gov"
        );
    }

    // Detecta perda de conexão.
    if (!connected && wasWifiConnected) {

        wasWifiConnected = false;

        timeSynchronized = false;

        SystemLogger::warn(
            "Conexao Wi-Fi perdida"
        );
    }

    // Registra falha inicial após 15 segundos.
    if (
        !connected &&
        !connectionTimeoutLogged &&
        now - wifiStartTime >= CONNECTION_TIMEOUT
    ) {

        connectionTimeoutLogged = true;

        SystemLogger::error(
            "Tempo limite excedido ao conectar ao Wi-Fi"
        );
    }

    // Reconexão não bloqueante.
    if (
        !connected &&
        now - lastReconnectAttempt >= RECONNECT_INTERVAL
    ) {

        lastReconnectAttempt = now;

        WiFi.reconnect();
    }

    // Verifica sincronização do relógio.
    if (connected && !timeSynchronized) {

        time_t currentTime = time(nullptr);

        if (currentTime >= 1700000000) {

            timeSynchronized = true;

            SystemLogger::info(
                "Horario sincronizado via NTP"
            );
        }
    }
}


// ==========================================
// Informações de rede
// ==========================================

void WebServerManager::printNetworkInfo() const {

    Serial.println();
    Serial.println("========== REDE ESP32 ==========");

    Serial.print("IP: ");
    Serial.println(WiFi.localIP());

    Serial.print("MAC: ");
    Serial.println(WiFi.macAddress());

    Serial.println("Status: CONECTADO");
    Serial.println("================================");
    Serial.println();
}


String WebServerManager::getIpAddress() const {

    return WiFi.localIP().toString();
}


// ==========================================
// Atualização do status
// ==========================================

void WebServerManager::updateStatus(
    const SystemStatus& newStatus
) {

    status = newStatus;
}


// ==========================================
// Solicitação de atualização do display
// ==========================================

bool WebServerManager::consumeProfileDisplayRefreshRequest() {

    const bool requested = profileDisplayRefreshRequested;

    profileDisplayRefreshRequested = false;

    return requested;
}