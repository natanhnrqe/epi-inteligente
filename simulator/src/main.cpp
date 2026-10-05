#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>



int main() {
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Erro ao inicializar Winsock.\n";
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Erro ao criar socket.\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddress{};

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddress.sin_port = htons(8080);

    if (bind(
        serverSocket,
        reinterpret_cast<sockaddr*>(&serverAddress),
        sizeof(serverAddress)
    ) == SOCKET_ERROR) {

        std::cerr << "Erro no bind.\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Erro no listen.\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "=================================\n";
    std::cout << "   EPI INTELIGENTE - SIMULADOR\n";
    std::cout << "=================================\n";
    std::cout << "Servidor iniciado!\n";
    std::cout << "http://localhost:8080\n";
    std::cout << "API: http://localhost:8080/api/status\n";
    std::cout << "=================================\n";

    while (true) {

        SOCKET clientSocket = accept(
            serverSocket,
            nullptr,
            nullptr
        );

        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Erro ao aceitar conexao.\n";
            continue;
        }

        char buffer[4096];

        int bytesReceived = recv(
            clientSocket,
            buffer,
            sizeof(buffer) - 1,
            0
        );

        if (bytesReceived <= 0) {
            closesocket(clientSocket);
            continue;
        }

        buffer[bytesReceived] = '\0';

        std::string request(buffer);

        std::cout << "\nRequisicao recebida:\n";
        std::cout << request << "\n";

        // =========================
        // DADOS SIMULADOS
        // =========================

        int battery = 85;
        bool helmet = true;

        double gyroX = 0.2;
        double gyroY = -0.1;
        double gyroZ = 1;

        // =========================
        // JSON
        // =========================

        std::string json = "{";
        json += "\"battery\":" + std::to_string(battery);
        json += ",";
        json += "\"helmet\":";
        json += helmet ? "true" : "false";
        json += ",";
        json += "\"gyro\":{";
        json += "\"x\":" + std::to_string(gyroX);
        json += ",";
        json += "\"y\":" + std::to_string(gyroY);
        json += ",";
        json += "\"z\":" + std::to_string(gyroZ);
        json += "}";
        json += "}";

        // =========================
        // RESPOSTA HTTP
        // =========================

        std::string response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Content-Length: " + std::to_string(json.size()) + "\r\n"
            "\r\n" +
            json;

        send(
            clientSocket,
            response.c_str(),
            static_cast<int>(response.size()),
            0
        );

        closesocket(clientSocket);
    }

    closesocket(serverSocket);
    WSACleanup();

    return 0;
}