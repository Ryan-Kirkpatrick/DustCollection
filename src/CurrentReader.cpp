#include "CurrentReader.hpp"

// Static members
volatile int CurrentReader::lastADCValues[CurrentReader::lastADCValuesLength]  =  { };
volatile int CurrentReader::lastADCValuesIndex = 0;



// Constructor and start the hardware timer.
CurrentReader::CurrentReader() : timer() {
    // Setup timer. Frequency ~= 160Hz. Callback to readADC().
    timer.attachInterruptInterval(1000000/105, readADC);
}

// ISR for the timer interupt.
// Reads the current value of the ADC and stores it to be averaged later.
void IRAM_ATTR CurrentReader::readADC() {
    lastADCValues[lastADCValuesIndex] = analogRead(Core::ADC_PIN);
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

