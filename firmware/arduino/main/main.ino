#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>

#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

const char* ssid = "Oi_7C96";
const char* password = "Av4P6PEQ";



WebServer server(80);

// MPU-6050

// O GY-521 utiliza comunicação I2C.
//
// ESP32:
// SDA -> GPIO 21
// SCL -> GPIO 22
// VCC -> 3.3V
// GND -> GND

Adafruit_MPU6050 mpu;


// BUZZER
const int BUZZER_PIN = 15;
const int BUZZER_PIN = 25;
const int BUZZER_FREQ = 2000;
const int BUZZER_CHANNEL = 0;
const int BUZZER_RESOLUTION = 8;
bool buzzerAtivo = false;





// ============================================================
// DADOS DO MPU-6050
// ============================================================

// Variáveis que armazenam os valores atuais do sensor.

float aceleracaoX = 0.0;
float aceleracaoY = 0.0;
float aceleracaoZ = 0.0;

float giroX = 0.0;
float giroY = 0.0;
float giroZ = 0.0;


// ***BATERIA***


// Bateria utilizada:
//
// 18650
// 1S1P
// 2200 mAh
// 3.7 V nominal
// 4.2 V carregada
// BMS
//
// ATENÇÃO:
// O ESP32 NÃO deve receber diretamente a tensão da bateria
// em um GPIO.
//
// Para medir a bateria será necessário um circuito divisor
// de tensão adequado.
//
// Por enquanto deixamos um valor simulado.
//
// Futuramente:
//
// Bateria -> divisor de tensão -> ADC do ESP32
//
// e então calculamos a tensão e a porcentagem.

int bateria = 85;



// ESTADO DO CAPACETE

// Por enquanto está foi utilizado um valor fixo.
//
// Futuramente a gente substituir por um sensor/chave que
// detecte se o capacete está sendo utilizado.

bool capacete = true;


void atualizarMPU() {

    sensors_event_t aceleracao;
    sensors_event_t giro;
    sensors_event_t temperatura;

    mpu.getEvent(
        &aceleracao,
        &giro,
        &temperatura
    );

    aceleracaoX = aceleracao.acceleration.x;
    aceleracaoY = aceleracao.acceleration.y;
    aceleracaoZ = aceleracao.acceleration.z;

    giroX = giro.gyro.x;
    giroY = giro.gyro.y;
    giroZ = giro.gyro.z;
}

// API: /api/status
void handleStatus() {

    // Atualiza os valores do MPU antes de enviar a resposta.

    atualizarMPU();

    String json = "{";

    // BATERIA
    json += "\"battery\":";
    json += bateria;

    json += ",";

    // CAPACETE
    json += "\"helmet\":";
    json += (capacete ? "true" : "false");

    json += ",";

    // BUZZER
    json += "\"buzzer\":";
    json += (buzzerAtivo ? "true" : "false");

    json += ",";

    // ACELERÔMETRO
    json += "\"accelerometer\":{";

    json += "\"x\":";
    json += aceleracaoX;

    json += ",";

    json += "\"y\":";
    json += aceleracaoY;

    json += ",";

    json += "\"z\":";
    json += aceleracaoZ;

    json += "},";

    // GIROSCÓPIO
    json += "\"gyroscope\":{";

    json += "\"x\":";
    json += giroX;

    json += ",";

    json += "\"y\":";
    json += giroY;

    json += ",";

    json += "\"z\":";
    json += giroZ;

    json += "}";


    json += "}";


    server.send(
        200,
        "application/json",
        json
    );
}


void handleBuzzerOn() {

    buzzerAtivo = true;

    ledcWrite(BUZZER_CHANNEL, 128);

    server.send(200, "application/json",
                "{\"buzzer\":true}");
}

void handleBuzzerOff() {

    buzzerAtivo = false;

    ledcWrite(BUZZER_CHANNEL, 0);

    server.send(200, "application/json",
                "{\"buzzer\":false}");
}


void handleRoot() {

    File file = LittleFS.open(
        "/index.html",
        "r"
    );


    if (!file) {

        server.send(
            500,
            "text/plain",
            "Erro ao abrir index.html"
        );

        return;
    }


    server.streamFile(
        file,
        "text/html"
    );


    file.close();
}


void handleCSS() {

    File file = LittleFS.open(
        "/style.css",
        "r"
    );


    if (!file) {

        server.send(
            404,
            "text/plain",
            "style.css nao encontrado"
        );

        return;
    }


    server.streamFile(
        file,
        "text/css"
    );


    file.close();
}

void handleJS() {

    File file = LittleFS.open(
        "/script.js",
        "r"
    );


    if (!file) {

        server.send(
            404,
            "text/plain",
            "script.js nao encontrado"
        );

        return;
    }


    server.streamFile(
        file,
        "application/javascript"
    );


    file.close();
}

void setup() {

    Serial.begin(115200);

    delay(1000);


    Serial.println();
    Serial.println("=================================");
    Serial.println("       EPI INTELIGENTE");
    Serial.println("=================================");


    pinMode(BUZZER_PIN, OUTPUT);

    ledcSetup(BUZZER_CHANNEL, BUZZER_FREQ, BUZZER_RESOLUTION);
    ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);

    ledcWrite(BUZZER_CHANNEL, 0);

    Serial.println("Buzzer configurado.");


    Serial.println("Iniciando I2C...");

    Wire.begin(
        21,
        22
    );

    Serial.println("Procurando MPU-6050...");

    if (!mpu.begin()) {

        Serial.println(
            "AVISO: MPU-6050 nao encontrado!"
        );

        Serial.println(
            "O sistema continuara funcionando."
        );

        Serial.println(
            "Conecte o GY-521 e reinicie o ESP32."
        );

    } else {

        Serial.println(
            "MPU-6050 encontrado!"
        );


        mpu.setAccelerometerRange(
            MPU6050_RANGE_8_G
        );

        mpu.setGyroRange(
            MPU6050_RANGE_500_DEG
        );


        mpu.setFilterBandwidth(
            MPU6050_BAND_21_HZ
        );


        Serial.println(
            "MPU-6050 configurado."
        );
    }

    Serial.println(
        "Iniciando LittleFS..."
    );

    if (!LittleFS.begin(true)) {

        Serial.println(
            "Erro ao iniciar LittleFS!"
        );

        return;
    }

    Serial.println(
        "LittleFS iniciado!"
    );

    Serial.print(
        "Conectando ao Wi-Fi"
    );


    WiFi.begin(
        ssid,
        password
    );


    while (
        WiFi.status() != WL_CONNECTED
    ) {

        delay(500);

        Serial.print(".");
    }


    Serial.println();

    Serial.println(
        "Wi-Fi conectado!"
    );


    Serial.print(
        "IP do ESP32: "
    );

    Serial.println(
        WiFi.localIP()
    );


    // ROTAS
    server.on(
        "/",
        HTTP_GET,
        handleRoot
    );

    server.on(
        "/style.css",
        HTTP_GET,
        handleCSS
    );

    server.on(
        "/script.js",
        HTTP_GET,
        handleJS
    );


    // API de status geral
    server.on(
        "/api/status",
        HTTP_GET,
        handleStatus
    );


    // Buzzer Ligado
    server.on(
        "/api/buzzer/on",
        HTTP_POST,
        handleBuzzerOn
    );

    // Buzzer Desligado
    server.on(
        "/api/buzzer/off",
        HTTP_POST,
        handleBuzzerOff
    );

    server.begin();


    Serial.println(
        "Servidor HTTP iniciado!"
    );


    Serial.println();

    Serial.print(
        "Dashboard: http://"
    );

    Serial.println(
        WiFi.localIP()
    );


    Serial.print(
        "API: http://"
    );

    Serial.print(
        WiFi.localIP()
    );

    Serial.println(
        "/api/status"
    );


    Serial.println(
        "================================="
    );
}



void loop() {

    server.handleClient();

    // Não precisamos chamar atualizarMPU() continuamente.
    //
    // O sensor é atualizado quando o navegador chama:
    //
    // GET /api/status
    //
}