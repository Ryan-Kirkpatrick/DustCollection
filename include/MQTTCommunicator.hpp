#pragma once

#include "Core.hpp"
#include "Secrets.hpp"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <map>
#include <string>

class MQTTCommunicator {

    public:
        MQTTCommunicator(WiFiClient wifiClient);

        // These get mapped to strings
        // It's a little sparse now but will be useful later
        enum SubscriptionTopic {
            COMMAND
        };
        enum PublishTopic {
            LOG
        };

        void publishLog(String msg);
        
        // Connects to the MQTT broker
        void init();

        /*
        Returns the state that it was commanded to be in by the MQTT broker.
        If no command has been issued it simply returns the current state.

        Will also attempt to reconnect to the MQTT broker if required. 
        */
        Core::State process();
        

    private:
        PubSubClient mqttClient;
        bool updateFlag = false; // A flag that is set if a command has been recvied from the broker.
        Core::State updatedState = Core::State::CLOSED;
        void connect(); // Connect to MQTT broker
        static void callback(char* topic, byte* payload, unsigned int length); // The callback function when a message is posted to a subscriced topic.


        // Converts subscription/publish topics to actual strings.
        static std::map<MQTTCommunicator::SubscriptionTopic, String> subscriptionTopicStringMap {
            {MQTTCommunicator::SubscriptionTopic::COMMAND, String("DustCollection/") + Core::MQTT_DEVICE_UID + String("/In/Command")}
        };
        static std::map<MQTTCommunicator::PublishTopic, String> publishTopicStringMap {
            {MQTTCommunicator::PublishTopic::LOG, String("DustCollection/") + Core::MQTT_DEVICE_UID + String("/Out/Log")}
        };

};