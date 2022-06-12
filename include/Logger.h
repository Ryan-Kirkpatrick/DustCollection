#pragma once

#include <EspMQTTClient.h>
#include <string>

enum LoggingLevel {
    DEBUG,
    NORMAL,
    ERROR,
    FATAL
};

class Logger {
    private:
        EspMQTTClient& client;
    public:
        Logger(EspMQTTClient& mqttClient);
        void log(std::string msg, LoggingLevel level = NORMAL);
};