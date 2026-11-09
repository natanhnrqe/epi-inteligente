#ifndef WEB_SERVER_MANAGER_H
#define WEB_SERVER_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>

#include "../models/SystemStatus.h"
#include "../storage/EmployeeProfileStorage.h"
#include "../safety/AlertManager.h"
#include "../safety/FallDetector.h"

class WebServerManager {
private:
    WebServer server;

    SystemStatus status;

    EmployeeProfileStorage profileStorage;

    AlertManager& alertManager;
    FallDetector& fallDetector;
    EmergencyProfile& emergencyProfile;

    bool profileDisplayRefreshRequested = false;

    bool wasWifiConnected = false;
    bool connectionTimeoutLogged = false;
    bool timeSynchronized = false;

    unsigned long wifiStartTime = 0;
    unsigned long lastReconnectAttempt = 0;

    static constexpr unsigned long RECONNECT_INTERVAL = 10000;
    static constexpr unsigned long CONNECTION_TIMEOUT = 15000;

    void configureRoutes();
    void updateWifiConnection();
    void printNetworkInfo() const;

public:
    WebServerManager(
        AlertManager& alertManager,
        FallDetector& fallDetector,
        EmergencyProfile& emergencyProfile
    );

    bool begin(const char* ssid, const char* password);

    void handleClient();

    void updateStatus(const SystemStatus& newStatus);

    bool consumeProfileDisplayRefreshRequest();

    String getIpAddress() const;
};

#endif