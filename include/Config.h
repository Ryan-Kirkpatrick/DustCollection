#pragma once

#define strconcat(a,b) a b

namespace Config {
    // Wifi
    inline const char* wifiSSID = WIFI_SSID;
    inline const char* wifiPassword = WIFI_PASSWORD;

    // MQTT
    inline const char* deviceName = DEVICE_NAME;
    inline const char* brokerAddress = BROKER_ADDRESS;
    inline const int brokerPort = BROKER_PORT;
    inline const char* mqttPassword = MQTT_PASSWORD;
    inline const char* mqttUsername = MQTT_USERNMAE;
    inline const char* inTopic = strconcat(IN_TOPIC,DEVICE_NAME);
    inline const char* outTopic = strconcat(OUT_TOPIC,DEVICE_NAME);
    inline const char* logTopic = strconcat(LOG_TOPIC,DEVICE_NAME);

    // Behaviour
    inline const uint16_t commandValiditySeconds = 60;
    inline const uint16_t gateKeepOpenSeconds = 10; // How long to keep a gate open after the machine is turned off

    // OTA
    inline const char* otaPassword = OTA_PASSWORD;

    // Hardware
    inline const uint8_t adcPin = A0;
    inline const uint8_t solenoidPin = D8;

}