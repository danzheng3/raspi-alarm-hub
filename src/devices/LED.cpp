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
    // dayOfWeek is 0-6 for Sunday-Saturday

    bool a0State = dayOfWeek & 0x01;
    bool a1State = (dayOfWeek >> 1) & 0x01;
    bool a2State = (dayOfWeek >> 2) & 0x01;

    if (a0State) {
        A0Pin.pinHigh();
    } else {
        A0Pin.pinLow();
    }

    if (a1State) {
        A1Pin.pinHigh();
    } else {
        A1Pin.pinLow();
    }

    if (a2State) {
        A2Pin.pinHigh();
    } else {
        A2Pin.pinLow();
    }

    return true;
}

bool LED::turnOff() {
    A0Pin.pinLow();
    A1Pin.pinLow();
    A2Pin.pinLow();
    return true;
}

