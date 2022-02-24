#pragma once

#ifndef DUST_COLLECTION_CORE
#define DUST_COLLECTION_CORE

#include <Arduino.h>
#include <vector>
#include <ESP8266WiFi.h>


namespace Core {
    // Hardware
    inline const int ADC_PIN = A0;
    inline const int SOLENOID_PIN = -1;
    inline const int TOGGLE_BUTTON_PIN = -1;

    // WiFi
    inline const char* WIFI_SSID = "HSBNE-Infra";
    inline const char* DEVICE_NAME = "BG-WS-TableSaw";

    //MQTT
    inline const char* MQTT_BROKER = "junk";
    inline const char* MQTT_DEVICE_UID = "junk";
    inline const int MQTT_PORT = 1883;
    // MQTT Topics are configured in MQTTCommunicator.h
    
    // Functions
    void log(String msg);

    // Internal Logic
    enum State {
        OPEN,
        CLOSED,
        BOOTING,
        STUCK_GATE
    };

    extern State currentState;

}

#endif