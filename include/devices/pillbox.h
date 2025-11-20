#pragma once
#include <iostream>
#include <thread>
#include <chrono>
#include <pigpiod_if.h>
#include <pigpio.h>

#define PILLBOX_PWM_PIN 18
#define SERVO_FREQ_HZ 50 //20ms period
#define SERVO_NEUTRAL_US 1500 // stop
#define SERVO_CW_SPEED_US 1400 //1.4ms
#define SERVO_CCW_SPEED_US 1600 //1.6 ms
#define OPEN_DURATION_MS 1000 // NEED TO MANUALLY ADJUST FIX LATER

class pillbox {
    public:
        pillbox();
        ~pillbox();

        bool openPillbox();
        bool closePillbox();

    private:
        bool pillboxState = false;
};


