#include "MQTTCommunicator.hpp"

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
MQTTCommunicator::MQTTCommunicator(WiFiClient &wifiClient) : mqttClient(wifiClient), updatedState(Core::State::NULL_STATE), updateFlag(false) { 
    mqttClient.setId(Core::MQTT_DEVICE_UID);
}

/*
 * Publishes to a valid PublishTopic.
 * Does nothing if the mqttClient is not connected.
*/ 
void MQTTCommunicator::publish(String msg, MQTTCommunicator::PublishTopic topic) {
    if (!mqttClient.connected()) {
        return;
    }

    mqttClient.beginMessage(publishTopicStringMap.at(topic));
    mqttClient.print(msg);
    mqttClient.endMessage();
    // Do not log this. It will cause an endless loop.
}


// Connect to broker
void MQTTCommunicator::connect() {
    Serial.print("[MQTT] Attempting MQTT connection...");
    
    // Attempt to connect
    if (mqttClient.connect(Core::MQTT_BROKER, Core::MQTT_PORT)) {
        Serial.println("[MQTT Connection] Connected.");
        
        // Subscribe to topics
        for (auto &pair : subscriptionTopicStringMap) {
            mqttClient.subscribe(pair.second); // TODO Check if this works lmao
        }

    } else {
        Serial.println();
        Serial.print("[MQTT Connection] Failed, rc=");
        Serial.print(mqttClient.connectError());
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
// Return the updated state, if available.
Core::State MQTTCommunicator::process() {
    // Check connection
    if (!mqttClient.connected()) {
        connect();
    }

    // Process messages
    String message = "";
    if (mqttClient.parseMessage()) {
        while (mqttClient.available()) {
            message.concat((char) mqttClient.read());
        }
        Core::log("[MQTT] Recvieved MQTT message in topic " + mqttClient.messageTopic() + ", message contents:");
        Core::log(message);
    }

    // Do the real logic.


    if (!updateFlag) {
        return Core::State::NULL_STATE;
    }

    updateFlag = false;
    return updatedState;
}