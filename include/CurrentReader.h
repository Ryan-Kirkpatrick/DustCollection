#pragma once

#include <Arduino.h>

class CurrentReader {

    private:
        static const int lastADCValuesLength = 10;
        int lastADCValues[lastADCValuesLength]; // Stores the last X ADC values. 
        int lastADCValuesIndex ;
        const uint16 thresholdForMachineOn = 15; // If the average of the last X ADC values is above this the machine is considered on.
        
    public:
        CurrentReader();
        bool isMachineOn(); // Determines if a machine is on or off by reading the current meter attached to the ADC
        void loop();

};