#include "devices/strobe.h"

strobe::strobe() : strobePin(STROBE_PIN) {
    strobePin.pinModeOut();
}

strobe::~strobe() {
    strobePin.pinLow(); // Ensure strobe is off
}

bool strobe::strobeActivate() {
    strobeState = true; // need to change based on strobing
    return strobePin.pinHigh();
}

int strobe::strobeRead() {
    return strobePin.pinRead();
}

bool strobe::strobeToNormal() {
    strobeState = false;
    return strobePin.pinLow();
}