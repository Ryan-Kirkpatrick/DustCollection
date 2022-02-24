#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>

#include "Core.hpp"
#include "Secrets.hpp"
#include "CurrentReader.hpp"
#include "MQTTCommunicator.hpp"

WiFiClient wifiClient;
MQTTCommunicator mqttCommunicator(wifiClient);

namespace Core {
    void log(String msg) {
        Serial.println(msg);
        mqttCommunicator.publishLog(msg);
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

/*
Drives the solenoid to open and close the blast blast gate.

With no param:
  Activates the air solenoid to match Core::currentState

With optional param
  If Core::State::OPEN is passed in the blast gate will open regardless of the internal state
  If Core::State::CLOSED is passed in the blast gate will close regardless of the internal state.

  Useful if you want to cycle the gate in attempt to clear a jam.
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


/*
There are two ways to enter the OPEN state:
    1. The toggle button was pressed and the last state was CLOSED
    2. An MQTT command to OPEN is received.

Upon entering the OPEN state the following steps will be taken:
    1. The relay for the solenoid will be driven, supplying compressed air to the piston.
*/
inline void open() {
    Core::log("Opening gate.");
    updateSolenoid();
}   


/*
There are two ways to enter the CLOSED state:
    1. The toggle button was pressed and the last state was OPEN
    2. An MQTT command to CLOSED is received.

Upon entering the OPEN state the following steps will be taken:
    1. The relay for the solenoid will be driven, supplying compressed air to the piston.
*/
inline void close() {
    Core::log("Closing gate");
    updateSolenoid();
}   



void loop() {
    // put your main code here, to run repeatedly:

    switch (Core::currentState) {
        case Core::State::OPEN:
            open();
            break;
        
        case Core::State::CLOSED:
            close();
            break;
        
        default:
            Core::log("A bad thing happened.");
            break;
        }
}