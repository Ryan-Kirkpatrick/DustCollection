#pragma once

#include "State.h"
#include "Logger.h"
#include "EspMQTTClient.h"

class CommandInterpreter {
    public:
        void subscribe();
        // Yields the state suggested by the last MQTT command.
        State yield();
        CommandInterpreter(EspMQTTClient& mqttClient, Logger& logger);

    private:
        EspMQTTClient& client;
        Logger& logger;
        BlastGateState lastMQTTCommand;
        unsigned long lastMQTTCommandTime;
        void interpretCommand(std::string command);

};
