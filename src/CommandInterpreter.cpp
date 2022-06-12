#include "CommandInterpreter.h"
#include <Arduino.h>
#include "Config.h"

void CommandInterpreter::subscribe() {
    client.subscribe(Config::inTopic, [this](const String& msg) {
        this->interpretCommand(msg.c_str());
    });
}

// Parses the MQTT messages for commands to open or close the blast gate
void CommandInterpreter::interpretCommand(std::string command) {
    if (command == "OPEN") {
        logger.log("Received MQTT OPEN command");
        lastMQTTCommand = GATE_OPEN;
    } else if (command == "CLOSE") {
        logger.log("Received MQTT CLOSE command");
        lastMQTTCommand = GATE_CLOSED;
    } else {
        logger.log("Received an invalid MQTT command", ERROR);
        return;
    }
    lastMQTTCommandTime = millis();
}

State CommandInterpreter::yield() {
    // Check time since the last command, they are only valid for so long.
    if (millis() - lastMQTTCommandTime > Config::commandValiditySeconds * 1000) {
        // Command is expired.
        return GATE_CLOSED;
    }
    // Command is still valid
    return lastMQTTCommand;
}

CommandInterpreter::CommandInterpreter(EspMQTTClient& mqttClient, Logger& logger) : 
    client{mqttClient}, 
    logger{logger}, 
    lastMQTTCommand{GATE_CLOSED}, 
    lastMQTTCommandTime{millis()} 
    {}
