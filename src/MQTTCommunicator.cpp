#include "MQTTCommunicator.hpp"

// Constructor
MQTTCommunicator::MQTTCommunicator(WiFiClient wifiClient) : mqttClient(wifiClient) {
    
}


// Sets up the MQTT client.
void MQTTCommunicator::init() {
    mqttClient.setServer(Core::MQTT_BROKER, Core::MQTT_PORT);
    mqttClient.setCallback(callback);
    connect();
}


// Publishes to the LOG topic.
void MQTTCommunicator::publishLog(String msg) {
    if (!mqttClient.connected()) {
        return;
    }

    mqttClient.publish(publishTopicStringMap.at(PublishTopic::LOG).c_str(), msg.c_str());
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


// Action to take when an MQTT message is recivied
void MQTTCommunicator::callback(char* topic, byte* payload, unsigned int length) {
    // Identify the topic
    SubscriptionTopic subbedTopic;
    for (auto &pair : subscriptionTopicStringMap) {
        if (pair.second.c_str() == topic) {
            subbedTopic = pair.first;
            break;
        }
    }

    // Convert the payload to a String
    std::string payloadStr((uint8_t) payload);
    
    switch (subbedTopic) {
        case SubscriptionTopic::COMMAND:
            if (payloadStr == std::string("OPEN")) {

            }
            break;
        
        default:
            break;
    }


}