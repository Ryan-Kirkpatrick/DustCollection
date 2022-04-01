#pragma once

#include <Arduino.h>
#include "Core.hpp"

class CurrentReader {

    private:
        static inline const int lastADCValuesLength = 10;
        static int lastADCValues[]; // Stores the last X ADC values. 
        static int lastADCValuesIndex ;
        static inline const uint16 thresholdForMachineOn = 15; // If the average of the last X ADC values is above this the machine is considered on.

    public:
        static void readADC();
        CurrentReader();
        bool isMachineOn(); // Determines if a machine is on or off by reading the current meter attached to the ADC

};