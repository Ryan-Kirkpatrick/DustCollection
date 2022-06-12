#include "Logger.h"
#include "Config.h"


Logger::Logger(EspMQTTClient& mqttClient) : client{mqttClient} {}

void Logger::log(std::string msg, LoggingLevel level) {
    if (client.isConnected()) {
        client.publish(Config::logTopic, msg.c_str());
    }
    Serial.println(msg.c_str());
}