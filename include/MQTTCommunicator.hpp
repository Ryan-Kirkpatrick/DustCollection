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

        /*
         * SubscriptionTopic and PublishTopic are the MQTT topics that are used by the device.
         *
         * Internally in this class they will get mapped to Strings.
         * When refrencing a MQTT topic around the program use the values of these enums. Do not use Strings.
        */
        enum class SubscriptionTopic {
            COMMAND // used to recieve OPEN and CLOSE commands
        };
        enum class PublishTopic {
            DEBUG_LOG, // For debugging
            MACHINE_STATE, // Notifies the broker when a machine (e.g. tablesaw) is turned ON/OFF
            GATE_STATE // Notifies the borker if the blast gate is opened or closed
        };

        /// Publishes a message to an MQTT topic
        static void publish(String msg, MQTTCommunicator::PublishTopic topic);
        
        /// Gets the object ready for use.
        void init();

        /**
         * Will return the state that the last MQTT command directed.
         * 
         * Note that when this is called it 'consumes' the MQTT command that has been issued. I.e.
         *   1. An MQTT command is received to OPEN
         *   2. The next time process() is called it will return Core::State::OPEN
         *   3. process() is called again. This time it returns Core::State::NULL_STATE, assuming there is no new MQTT command.
         * 
        */
        Core::State process();
        

    private:
        // Attrributes
        static PubSubClient mqttClient;
        static bool updateFlag; // A flag that is set if a command has been recvied from the broker.
        static Core::State updatedState; // The state that the command specified.
    
        // Converts the values in the enum classes SubscribtionTopic and PublishTopic to MQTT topic strings.
        const static std::map<MQTTCommunicator::SubscriptionTopic, String> subscriptionTopicStringMap;
        const static std::map<MQTTCommunicator::PublishTopic, String> publishTopicStringMap;

        /// Connect to MQTT broker
        static void connect();

        /// The callback function when a message is posted to a subscriced topic.
        static void callback(char* topic, byte* payload, unsigned int length); 

};