#include "hardware_layer/GPIO.h"
#pragma once

#define STROBE_PIN 25

class strobe {
    public:
        strobe();
        ~strobe();

        bool strobeActivate();
        bool strobeToNormal();
        bool strobeRead(); 

    private:
        GPIOPin strobePin;
        bool strobeState = false;

};