
#include "SystemLogger.h"
#include <time.h>

namespace {
    constexpr const char* LOG_PATH = "/system.log";

    constexpr size_t MAX_LOG_SIZE = 32 * 1024;
    constexpr size_t LOG_OVERHEAD = 256;
    constexpr size_t MAX_MESSAGE_SIZE =
        MAX_LOG_SIZE - LOG_OVERHEAD * 2;

    constexpr time_t VALID_TIMESTAMP = 1700000000;
}

String SystemLogger::timestamp() {
    time_t now = time(nullptr);

    if (now < VALID_TIMESTAMP) {
        return "[UPTIME " + String(millis() / 1000) + "s]";
    }

    struct tm timeInfo;
    localtime_r(&now, &timeInfo);

    char buffer[24];

    strftime(
        buffer,
        sizeof(buffer),
        "%Y-%m-%d %H:%M:%S",
        &timeInfo
    );

    return "[" + String(buffer) + "]";
}

void SystemLogger::begin() {
    if (!LittleFS.exists(LOG_PATH)) {
        File file = LittleFS.open(LOG_PATH, FILE_WRITE);
        file.close();
    }

    LOG_ATTACH_FS_AUTO(LittleFS, LOG_PATH, FILE_APPEND);

    LOG_FILE_SET_LEVEL(DebugLogLevel::LVL_INFO);

    info("Logger inicializado");
}

bool SystemLogger::rotateIfNeeded(size_t incomingBytes) {
    if (!LOG_FILE_IS_OPEN()) {
        return false;
    }

    File file = LittleFS.open(LOG_PATH, FILE_READ);

    if (!file) {
        return false;
    }

    size_t currentSize = file.size();
    file.close();

    if (currentSize + incomingBytes <= MAX_LOG_SIZE) {
        return true;
    }

    LOG_FILE_CLOSE();

    if (!LittleFS.remove(LOG_PATH)) {
        Serial.println("[LOGGER] Falha ao remover arquivo antigo.");

        LOG_ATTACH_FS_AUTO(LittleFS, LOG_PATH, FILE_APPEND);

        return false;
    }

    LOG_ATTACH_FS_AUTO(LittleFS, LOG_PATH, FILE_APPEND);

    LOG_FILE_SET_LEVEL(DebugLogLevel::LVL_INFO);

    LOG_INFO(
        "Arquivo de log rotacionado. Limite: ",
        MAX_LOG_SIZE,
        " bytes"
    );

    return true;
}

void SystemLogger::info(const String& message) {
    String safeMessage = message;

    if (safeMessage.length() > MAX_MESSAGE_SIZE) {
        safeMessage = safeMessage.substring(0, MAX_MESSAGE_SIZE);
    }

    String dateTime = timestamp();

    rotateIfNeeded(
        dateTime.length() + safeMessage.length() + LOG_OVERHEAD
    );

    LOG_INFO(dateTime.c_str(), safeMessage.c_str());
}

void SystemLogger::warn(const String& message) {
    String safeMessage = message;

    if (safeMessage.length() > MAX_MESSAGE_SIZE) {
        safeMessage = safeMessage.substring(0, MAX_MESSAGE_SIZE);
    }

    String dateTime = timestamp();

    rotateIfNeeded(
        dateTime.length() + safeMessage.length() + LOG_OVERHEAD
    );

    LOG_WARN(dateTime.c_str(), safeMessage.c_str());
}

void SystemLogger::error(const String& message) {
    String safeMessage = message;

    if (safeMessage.length() > MAX_MESSAGE_SIZE) {
        safeMessage = safeMessage.substring(0, MAX_MESSAGE_SIZE);
    }

    String dateTime = timestamp();

    rotateIfNeeded(
        dateTime.length() + safeMessage.length() + LOG_OVERHEAD
    );

    LOG_ERROR(dateTime.c_str(), safeMessage.c_str());
}
