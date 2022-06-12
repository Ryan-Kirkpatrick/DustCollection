#include <Arduino.h>
#include <EspMQTTClient.h>
#include <ESP8266WiFi.h>

#include "Config.h"
#include "Logger.h"
#include "CurrentReader.h"
#include "CommandInterpreter.h"


// MQTT, Wifi, and OTA setup
EspMQTTClient mqttClient (
    Config::wifiSSID,
    Config::wifiPassword,
    Config::brokerAddress,
    Config::mqttUsername,
    Config::mqttPassword,
    Config::deviceName,
    Config::brokerPort
);

Logger logger(mqttClient);
CurrentReader currentReader;
CommandInterpreter commandInterpreter(mqttClient, logger);

void setup() {
    pinMode(Config::solenoidPin, OUTPUT);
    digitalWrite(Config::solenoidPin, LOW);
    Serial.begin(9600);
    mqttClient.enableOTA(Config::otaPassword, 8266);
    mqttClient.setKeepAlive(60);
    mqttClient.loop();
    while (!Serial) { delay(100); }
}

void loop() {
    mqttClient.loop();
    delay(200); // MUST delay to handle WiFi.
    currentReader.loop();

    // Decide state
    static BlastGateState currentState = GATE_CLOSED;
    static unsigned long lastMachineOnTime = millis();

    if (currentReader.isMachineOn()) {
        currentState = GATE_OPEN;
        lastMachineOnTime = millis();
    } else {
        currentState = commandInterpreter.yield();
    }

    // Keep the gate open if the machine was on recently. Configurable in Config.h
    if (currentState == GATE_CLOSED && (millis() - lastMachineOnTime < Config::gateKeepOpenSeconds * 1000)) {
        currentState = GATE_OPEN;
    }

    // Carry out the action for the current state if a state transition has occured
    static BlastGateState lastState = BOOTING;
    if (currentState == lastState) {
        return;
    }

    switch (currentState) {
        case GATE_OPEN:
            logger.log("Opening the blast gate.");
            mqttClient.publish(Config::outTopic, "OPEN");
            digitalWrite(Config::solenoidPin, HIGH);
            logger.log(std::to_string(ESP.getFreeHeap()));
            break;

        case GATE_CLOSED:
            logger.log("Closing the blast gate.");
            mqttClient.publish(Config::outTopic, "CLOSED");
            digitalWrite(Config::solenoidPin, LOW);
            break;

        default:
            break;
    }

    lastState = currentState;
}

// This function has to be implemented, it's a requirement of the EspMQTTClient library.
void onConnectionEstablished() {
    logger.log("MQTT connection established");
    Serial.println(WiFi.localIP());
    commandInterpreter.subscribe();
}

// #include "ESP8266WiFi.h"
// #include <Arduino.h>

// const char* ssid = "FBI Van"; //Enter SSID
// const char* password = "blackplum574"; //Enter Password

// void setup(void)
// { 
//   Serial.begin(9600);
//   // Connect to WiFi
//   WiFi.begin(ssid, password);
//   while (WiFi.status() != WL_CONNECTED) 
//   {
//      delay(500);
//      Serial.print("*");
//   }
  
//   Serial.println("");
//   Serial.println("WiFi connection Successful");
//   Serial.print("The IP Address of ESP8266 Module is: ");
//   Serial.print(WiFi.localIP());// Print the IP address
// }

// void loop() 
// {
//   // EMPTY
// }