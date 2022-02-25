#pragma once

#include <Arduino.h>
#include "Core.hpp"

// Disable stupid timer accuracy warning. Why on earth this library used a warning instead of a message is beyond me.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcpp"
#include <ESP8266TimerInterrupt.h>
#pragma GCC diagnostic pop

class CurrentReader {

    private:
        ESP8266Timer timer;
        static inline const uint16_t lastADCValuesLength = 100;
        static volatile uint16_t lastADCValues[]; // Stores the last X ADC values. 
        static volatile uint16_t lastADCValuesIndex ;
        static inline const uint16 thresholdForMachineOn = 200; // If the average of the last X ADC values is above this the machine is considered on.
        static void IRAM_ATTR readADC(); // Having IRAM_ATTR in the header maybe breaks things, idk

    public:
        CurrentReader();
        bool isMachineOn(); // Determines if a machine is on or off by reading the current meter attached to the ADC

};