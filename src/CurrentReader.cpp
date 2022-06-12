#include "CurrentReader.h"
#include "Config.h"

// Reads the current value of the ADC and stores it to be averaged later.
void CurrentReader::loop() {
    lastADCValues[lastADCValuesIndex] = analogRead(Config::adcPin);
    lastADCValuesIndex++;
    if (lastADCValuesIndex >= lastADCValuesLength) {
        lastADCValuesIndex = 0;
    }
}

bool CurrentReader::isMachineOn() {
    // Take average ADC value and compare against threshold to determine of machine is on.
    unsigned int sum = 0;
    for (unsigned int i = 0; i < lastADCValuesLength; i++) {
        sum += lastADCValues[i];
    }
    uint32 average = sum / lastADCValuesLength;
    return average > thresholdForMachineOn;

}

CurrentReader::CurrentReader() : lastADCValues{} {}

