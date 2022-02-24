#pragma once

#include <Arduino.h>
#include <ESP8266TimerInterrupt.h>
#include "Core.hpp"

class CurrentReader {

    private:
        ESP8266Timer timer;
        static const uint16_t lastADCValuesLength = 100;
        static volatile uint16_t lastADCValues[lastADCValuesLength]; // Stores the last X ADC values. 
        static volatile uint16_t lastADCValuesIndex ;
        const uint16 thresholdForMachineOn = 200; // If the average of the last X ADC values is above this the machine is considered on.
        static void IRAM_ATTR readADC(); // Having IRAM_ATTR in the header maybe breaks things, idk

    public:
        CurrentReader();
        bool isMachineOn(); // Determines if a machine is on or off by reading the current meter attached to the ADC

};