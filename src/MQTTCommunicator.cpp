#include "MQTTCommunicator.hpp"

// Static defintions
Core::State MQTTCommunicator::updatedState = Core::State::CLOSED;
bool MQTTCommunicator::updateFlag = false;
PubSubClient MQTTCommunicator::mqttClient;

/// Map enum SubscriptionTopic to actual MQTT topic strings.
const std::map<MQTTCommunicator::SubscriptionTopic, String> MQTTCommunicator::subscriptionTopicStringMap {
    {MQTTCommunicator::SubscriptionTopic::COMMAND, String("DustCollection/") + Core::MQTT_DEVICE_UID + String("/In/Command")}
};

/// Map enum PublishTopic to actuall MQTT topic strings.
const std::map<MQTTCommunicator::PublishTopic, String> MQTTCommunicator::publishTopicStringMap {
    {MQTTCommunicator::PublishTopic::DEBUG_LOG, String("DustCollection/") + Core::MQTT_DEVICE_UID + String("/Out/ConsoleLog")},
    {MQTTCommunicator::PublishTopic::MACHINE_STATE, String("DustCollection/") + Core::MQTT_DEVICE_UID + String("/Out/MachineState")},
    {MQTTCommunicator::PublishTopic::GATE_STATE, String("DustCollection/") + Core::MQTT_DEVICE_UID + String("/Out/GateState")}
};

/// Constructor
MQTTCommunicator::MQTTCommunicator(WiFiClient wifiClient) {
    mqttClient = PubSubClient(wifiClient);
}


/// Sets up the MQTT client.
void MQTTCommunicator::init() {
    mqttClient.setServer(Core::MQTT_BROKER, Core::MQTT_PORT);
    mqttClient.setCallback(callback);
    connect();
}


/*
 * Publishes to a valid PublishTopic.
 * Does nothing if the mqttClient is not connected.
*/ 
void MQTTCommunicator::publish(String msg, MQTTCommunicator::PublishTopic topic) {
    if (!mqttClient.connected()) {
        return;
    }

    mqttClient.publish(publishTopicStringMap.at(PublishTopic::DEBUG_LOG).c_str(), msg.c_str());
}


// Connect to broker
void MQTTCommunicator::connect() {
    Serial.print("Attempting MQTT connection...");
    
    // Attempt to connect
    if (mqttClient.connect(Core::MQTT_DEVICE_UID)) {
        Serial.println("[MQTT Connection] Connected.");
        
        // Subscribe to topics
        for (auto &pair : subscriptionTopicStringMap) {
            mqttClient.subscribe(pair.second.c_str()); // TODO Check if this works lmao
        }

    } else {
        Serial.print("[MQTT Connection] Failed, rc=");
        Serial.print(mqttClient.state());
        Serial.println();
    }
}


/** 
 * Action to take when an MQTT message is recievied.
 * 
 * For the commands topic:
 *   1. The updateFlag and updateState will be set, if the command was valid.
 * 
*/
void MQTTCommunicator::callback(char* topic, byte* payload, unsigned int length) {
    // Identify the topic
    SubscriptionTopic subbedTopic;
    bool topicFound = false;
    for (auto &pair : subscriptionTopicStringMap) {
        if (pair.second.c_str() == topic) {
            subbedTopic = pair.first;
            topicFound = true;
            break;
        }
    }

    if (!topicFound) {
        return;
    }

    // Convert the payload to a String
    const std::string payloadStr = reinterpret_cast<char*>(payload);
    
    // Take appririate action depending on content and topic.
    switch (subbedTopic) {
        
        // Process commands
        case SubscriptionTopic::COMMAND:
            if (payloadStr == std::string("OPEN")) {
                updateFlag = true;
                updatedState = Core::State::OPEN;
                Core::log("[MQTT] Command recieved to CLOSE");
                return;
            } else if (payloadStr == std::string("CLOSE")) {
                updateFlag = true;
                updatedState = Core::State::OPEN;
                Core::log("[MQTT] Command recieved to CLOSE");
                return;
            } else {
                Core::log("[MQTT] Invalid command recieved.");
                return;
            }
        
        default:
            Core::log("[MQTT] Error. A topic was subscribed to but no logic has been coded for it. Check subscriptionTopicStringMap in MQTTCommunicator.cpp");
            return;
    }
}

// Return the updated state, if available.
Core::State MQTTCommunicator::process() {
    if (!updateFlag) {
        return Core::State::NULL_STATE;
    }

    updateFlag = false;
    return updatedState;
}