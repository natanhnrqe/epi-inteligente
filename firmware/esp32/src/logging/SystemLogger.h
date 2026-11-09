
#ifndef SYSTEM_LOGGER_H
#define SYSTEM_LOGGER_H

#include <Arduino.h>
#include <LittleFS.h>

#define DEBUGLOG_ENABLE_FILE_LOGGER
#define DEBUGLOG_DEFAULT_FILE_LEVEL_INFO
#define DEBUGLOG_DEFAULT_LOG_LEVEL_INFO
#include <DebugLog.h>

class SystemLogger {
private:
    static String timestamp();
    static bool rotateIfNeeded(size_t incomingBytes);

public:
    static void begin();

    static void info(const String& message);
    static void warn(const String& message);
    static void error(const String& message);
};

#endif
