#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>

const char* ssid = "Oi_7C96";
const char* password = "A4vP6PEQ";

WebServer server(80);


void handleStatus() {

    int battery = 85;
    bool helmet = true;

    float gyroX = 0.2;
    float gyroY = -0.1;
    float gyroZ = 9.8;

    String json = "{";

    json += "\"battery\":";
    json += battery;

    json += ",";

    json += "\"helmet\":";
    json += (helmet ? "true" : "false");

    json += ",";

    json += "\"gyro\":{";

    json += "\"x\":";
    json += gyroX;

    json += ",";

    json += "\"y\":";
    json += gyroY;

    json += ",";

    json += "\"z\":";
    json += gyroZ;

    json += "}";

    json += "}";

    server.send(200, "application/json", json);
}



void handleRoot() {

    File file = LittleFS.open("/index.html", "r");

    if (!file) {
        server.send(500, "text/plain", "Erro ao abrir index.html");
        return;
    }

    server.streamFile(file, "text/html");

    file.close();
}


void handleCSS() {

    File file = LittleFS.open("/style.css", "r");

    if (!file) {
        server.send(404, "text/plain", "style.css nao encontrado");
        return;
    }

    server.streamFile(file, "text/css");

    file.close();
}


void handleJS() {

    File file = LittleFS.open("/script.js", "r");

    if (!file) {
        server.send(404, "text/plain", "script.js nao encontrado");
        return;
    }

    server.streamFile(file, "application/javascript");

    file.close();
}


void setup() {

    Serial.begin(115200);

    delay(1000);

    Serial.println();
    Serial.println("=================================");
    Serial.println("       EPI INTELIGENTE");
    Serial.println("=================================");


    Serial.println("Iniciando LittleFS...");

    if (!LittleFS.begin(true)) {

        Serial.println("Erro ao iniciar LittleFS!");

        return;
    }

    Serial.println("LittleFS iniciado!");


    Serial.print("Conectando ao Wi-Fi");

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {

        delay(500);

        Serial.print(".");
    }

    Serial.println();

    Serial.println("Wi-Fi conectado!");

    Serial.print("IP do ESP32: ");
    Serial.println(WiFi.localIP());


    server.on("/", HTTP_GET, handleRoot);

    server.on("/style.css", HTTP_GET, handleCSS);

    server.on("/script.js", HTTP_GET, handleJS);

    server.on("/api/status", HTTP_GET, handleStatus);


    server.begin();

    Serial.println("Servidor HTTP iniciado!");

    Serial.println();
    Serial.print("Dashboard: http://");
    Serial.println(WiFi.localIP());

    Serial.print("API: http://");
    Serial.print(WiFi.localIP());
    Serial.println("/api/status");

    Serial.println("=================================");
}


void loop() {

    server.handleClient();
}