#ifndef WEB_SERVER_MANAGER_H
#define WEB_SERVER_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include "../models/SystemStatus.h"
#include "../storage/EmployeeProfileStorage.h"
#include "../safety/AlertManager.h"

class WebServerManager {
private:
    WebServer server;

    SystemStatus status;

    void configureRoutes();

    EmployeeProfileStorage profileStorage;

    AlertManager& alertManager;



public:
    explicit WebServerManager(AlertManager& alertManager);

    bool begin(const char* ssid, const char* password);
    void handleClient();
    void updateStatus(const SystemStatus& newStatus);

    String getIpAddress() const;
};

#endif