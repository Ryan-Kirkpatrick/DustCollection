#include "MQTTCommunicator.hpp"

/// Map enum SubscriptionTopic to actual MQTT topic strings.
const std::map<MQTTCommunicator::SubscriptionTopic, std::string> MQTTCommunicator::subscriptionTopicStringMap {
    {MQTTCommunicator::SubscriptionTopic::COMMAND, std::string("DustCollection/") + Core::MQTT_DEVICE_UID + std::string("/In/Command")}
};

/// Map enum PublishTopic to actuall MQTT topic strings.
const std::map<MQTTCommunicator::PublishTopic, std::string> MQTTCommunicator::publishTopicStringMap {
    {MQTTCommunicator::PublishTopic::DEBUG_LOG, std::string("DustCollection/") + Core::MQTT_DEVICE_UID + std::string("/Out/ConsoleLog")},
    {MQTTCommunicator::PublishTopic::MACHINE_STATE, std::string("DustCollection/") + Core::MQTT_DEVICE_UID + std::string("/Out/MachineState")},
    {MQTTCommunicator::PublishTopic::GATE_STATE, std::string("DustCollection/") + Core::MQTT_DEVICE_UID + std::string("/Out/GateState")}
};

/// Constructor
MQTTCommunicator::MQTTCommunicator(WiFiClient& wifiClient) : mqttClient(wifiClient), updateFlag(false), updatedState(Core::State::NULL_STATE) { 
    mqttClient.setId(Core::MQTT_DEVICE_UID);
}

/*
 * Publishes to a valid PublishTopic.
 * Does nothing if the mqttClient is not connected.
*/ 
void MQTTCommunicator::publish(std::string msg, MQTTCommunicator::PublishTopic topic) {
    if (!mqttClient.connected()) {
        Serial.println("No connection to MQTT broker!");
        return;
    }

    mqttClient.beginMessage(publishTopicStringMap.at(topic).c_str());
    mqttClient.print(msg.c_str());
    mqttClient.endMessage();
    // Do not log this. It will cause an endless loop.
}


// Connect to broker
void MQTTCommunicator::connect() {
    Serial.println("[MQTT] Attempting MQTT connection...");
    
    // Attempt to connect
    if (mqttClient.connect(Core::MQTT_BROKER, Core::MQTT_PORT)) {
        Serial.println("[MQTT Connection] Connected.");
        
        // Subscribe to topics
        for (auto &pair : subscriptionTopicStringMap) {
            mqttClient.subscribe(pair.second.c_str()); // TODO Check if this works lmao
        }

    } else {
        Serial.println();
        Serial.print("[MQTT Connection] Failed, rc=");
        Serial.print(mqttClient.connectError());
        Serial.println();
    }
}


/** 
 * For the commands topic:
 *   1. The updateFlag and updateState will be set, if the command was valid.
 * 
*/
// Return the updated state, if available.
Core::State MQTTCommunicator::process() {
    // Check connection
    if (!mqttClient.connected()) {
        connect();
    }

    mqttClient.poll();

    // Parse messages
    if (mqttClient.parseMessage()) {
        std::string message = "";
        std::string topic = mqttClient.messageTopic().c_str();
        while (mqttClient.available()) {
            message += (char) mqttClient.read();
        }
        Core::log("[MQTT] Recvieved MQTT message in topic " + topic + ", message contents:");
        Core::log(message);
    
        // Process the message
        // COMMANDS
        if (topic == subscriptionTopicStringMap.at(SubscriptionTopic::COMMAND)) {
            // OPEN
            if (message == "OPEN") {
                Core::log("[MQTT] Recevied OPEN command");
                updatedState = Core::State::OPEN;
                updateFlag = true;
            // CLOSE
            } else if (message == "CLOSE") {
                Core::log("[MQTT] Recevied CLOSE command");
                updatedState = Core::State::CLOSED;
                updateFlag = true;
            //ERROR
            } else {
                Core::log("[MQTT] Warning: received and invalid command. Valid commands are \"OPEN\" or \"CLOSE\" (case sensitive).");
                Core::log("[MQTT] Command = " + message);
            }
        // Error
        } else {
            Core::log("[MQTT] Warning: recived message for a topic that I should not have. Check the subcription topic map.");
            Core::log("[MQTT] Topic = " + topic);
            Core::log("[MQTT] Message = " + message);
        }
    }

    // Return
    if (!updateFlag) {
        return Core::State::NULL_STATE;
    }

    updateFlag = false;
    return updatedState;
}