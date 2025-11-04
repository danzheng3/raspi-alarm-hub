#include "hardware_layer/GPIO.h"
#pragma once

#define A0 22
#define A1 23
#define A2 24

class LED {
    public:
        LED();
        ~LED();

        bool setLED(GPIOPin A0Pin, GPIOPin A1Pin, GPIOPin A2Pin, int dayOfWeek);
        bool turnOff(GPIOPin A0Pin, GPIOPin A1Pin, GPIOPin A2Pin);

    private:
        GPIOPin A0Pin;
        GPIOPin A1Pin;
        GPIOPin A2Pin;
};