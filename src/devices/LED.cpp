#include "devices/LED.h"

LED::LED() : A0Pin(A0), A1Pin(A1), A2Pin(A2) {
    A0Pin.pinModeOut();
    A1Pin.pinModeOut();
    A2Pin.pinModeOut();
}

LED::~LED() {
    turnOff();
}

bool LED::setLED(int dayOfWeek) {
    // The input dayOfWeek (1=Sat to 7=Sun) is directly used as the MUX address.
    int muxAddress = dayOfWeek; 

    if (muxAddress < 0) muxAddress = 0; 
    if (muxAddress > 7) muxAddress = 7; 

    // Decode the 3-bit address (muxAddress is now between 0 and 7)
    bool a0State = muxAddress & 0x01;        // A0 (GPIO 22) = LSB
    bool a1State = (muxAddress >> 1) & 0x01; // A1 (GPIO 23)
    bool a2State = (muxAddress >> 2) & 0x01; // A2 (GPIO 24) = MSB

    if (a0State) A0Pin.pinHigh(); else A0Pin.pinLow();
    if (a1State) A1Pin.pinHigh(); else A1Pin.pinLow();
    if (a2State) A2Pin.pinHigh(); else A2Pin.pinLow();

    return true;
}

bool LED::turnOff() {
    A0Pin.pinLow();
    A1Pin.pinLow();
    A2Pin.pinLow();
    return true;
}

std::array<int, 3> LED::getLEDStates() {
    // Returns {val22, val23, val24}
    return { A0Pin.pinRead(), A1Pin.pinRead(), A2Pin.pinRead() };
}
