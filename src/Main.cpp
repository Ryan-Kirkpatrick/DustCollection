#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>

#include "Core.hpp"
#include "Secrets.hpp"
#include "CurrentReader.hpp"
#include "MQTTCommunicator.hpp"

WiFiClient wifiClient;
MQTTCommunicator mqttCommunicator(wifiClient);
CurrentReader currentReader;

namespace Core {
    void log(String msg) {
        Serial.println(msg);
        mqttCommunicator.publish(msg, MQTTCommunicator::PublishTopic::DEBUG_LOG);
    }
}

// Setup and connect to WiFi
inline void setupWifI() {
    Serial.println("Starting WiFi");
    WiFi.mode(WIFI_STA);
    WiFi.begin(Core::WIFI_SSID, Secrets::WIFI_PASSWORD);
    WiFi.hostname(Core::DEVICE_NAME);

    // Block while WiFi connects
    while (WiFi.status() != WL_CONNECTED) {
        delay(250);
        Serial.println("...");
    }
    Core::log("WiFi Connected");
    Core::log(WiFi.localIP().toString());
}


// Does what it says on the tin.
inline void setupOTA() {
    ArduinoOTA.setPort(8266);
    ArduinoOTA.setHostname(Core::DEVICE_NAME);
    ArduinoOTA.setPassword(Secrets::OTA_PASSWORD);
    ArduinoOTA.onStart([]() {
        Core::log("[OTA] Start");
    });
    ArduinoOTA.onEnd([]() {
        Core::log("\n[OTA] End");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("[OTA] Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("[OTA] Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR)
            Core::log("[OTA] Auth Failed. Incorrect OTA password");
        else if (error == OTA_BEGIN_ERROR)
            Core::log("[OTA] Begin Failed. Check firmware size.");
        else if (error == OTA_CONNECT_ERROR)
            Core::log("[OTA] Connect Failed");
        else if (error == OTA_RECEIVE_ERROR)
            Core::log("[OTA] Receive Failed");
        else if (error == OTA_END_ERROR)
            Core::log("[OTA] End Failed");
    });
    ArduinoOTA.begin();
}


// Set relvant pins as inputs and outputs
inline void setupPins() {
    // Analog pin does not need to be set in/out
    pinMode(Core::SOLENOID_PIN, OUTPUT);

    // TODO

}

void setup() {
    // State
    Core::currentState = Core::State::BOOTING;

    //Serial
    Serial.begin(9600);

    // Wifi
    setupWifI();

    // MQTT
    mqttCommunicator.init();

    // OTA
    setupOTA();

    // State
    Core::currentState = Core::State::CLOSED;
}

/**
 * Drives the solenoid to open and close the blast blast gate.
 * 
 * With no param:
 *   Activates the air solenoid to match Core::currentState
 * 
 * With optional param:
 *   If Core::State::OPEN is passed in the blast gate will open regardless of the internal state
 *   If Core::State::CLOSED is passed in the blast gate will close regardless of the internal state.
 * 
 *   Useful if you want to cycle the gate in attempt to clear a jam.
*/
void updateSolenoid(const Core::State overrideState = Core::State::BOOTING) {
    // Handle optional param
    Core::State solenoidState = Core::currentState;
    if (overrideState == Core::State::OPEN || overrideState == Core::State::CLOSED) {
        solenoidState == overrideState;
    }

    // Open/ close gate
    if (solenoidState == Core::State::OPEN) {
        digitalWrite(Core::SOLENOID_PIN, HIGH);
    } else {
        digitalWrite(Core::SOLENOID_PIN, LOW);
    }

}


/**
 * There are two ways to enter the OPEN state:
 *     1. The toggle button was pressed and the last state was CLOSED
 *     2. An MQTT command to OPEN is received.
 * 
 * Upon entering the OPEN state the following steps will be taken:
 *     1. The relay for the solenoid will be driven, supplying compressed air to the piston.
*/
inline void open() {
    Core::log("Opening gate.");
    updateSolenoid();
}   


/**
 * There are two ways to enter the CLOSED state:
 *     1. The toggle button was pressed and the last state was OPEN
 *     2. An MQTT command to CLOSED is received.
 * 
 * Upon entering the OPEN state the following steps will be taken:
 *     1. The relay for the solenoid will be driven, supplying compressed air to the piston.
*/
inline void close() {
    Core::log("Closing gate");
    updateSolenoid();
}   


/**
 *  Updates Core::currentState and handles MQTT processing.
 * 
 * Hierarchy:
 *   1. CurrentReader indicates machine is on -> open gate.
 *   2. MQTT commands open gate -> open gate. The command has to be sent more than once every 120 seconds to keep the gate open.
 *   3. MQTT commands closed gate -> close gate
 *   4. If no MQTT command has been received for 120s and the current reader idicates machine is off -> close gate.
 *       
*/
inline void decideState() {
    static unsigned long lastMqttUpdate = 0;

    // Handle MQTT
    Core::State mqttSuggestedState = mqttCommunicator.process();

    // If the machine is on the blast gate should always be open.
    if (currentReader.isMachineOn()) {
        Core::currentState = Core::State::OPEN;
    }

    // Check MQTT for state updates.
    if (mqttSuggestedState != Core::State::NULL_STATE) {
        lastMqttUpdate = millis();
        Core::currentState = mqttSuggestedState;
    }

    // Close the gate if no MQTT commands have come in for 120s.
    if (millis() - 120000 > lastMqttUpdate) {
        Core::currentState = Core::State::CLOSED;
    }
    

}


void loop() {
    // Static intiialization
    static bool lastMachineState = false;
    static Core::State lastState = Core::State::NULL_STATE;

    //Check for state updates.
    decideState(); // Updates Core::currentState

    if (lastState != Core::currentState) {
        // Take state actions and report to MQTT broker
        switch (Core::currentState) {
            case Core::State::OPEN:
                open();
                mqttCommunicator.publish("OPEN", MQTTCommunicator::PublishTopic::GATE_STATE);
                break;
            
            case Core::State::CLOSED:
                close();
                mqttCommunicator.publish("OPEN",MQTTCommunicator::PublishTopic::GATE_STATE);
                break;
            
            default:
                Core::log("A bad thing happened.");
                break;
        }

    }

    // Report to MQTT broker if the machine has ben turned on or off.
    bool currentMachineState = currentReader.isMachineOn();

    if (currentMachineState != lastMachineState) {
        lastMachineState = currentMachineState;
        mqttCommunicator.publish((currentMachineState) ? "ON" : "OFF", MQTTCommunicator::PublishTopic::MACHINE_STATE);
    }
    
}