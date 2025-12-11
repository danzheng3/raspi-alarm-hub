#include "devices/pillbox.h"


#ifndef TEST_MODE 

pillbox::pillbox() {
    if (gpioInitialise() < 0) {
        std::cerr << "failed to initialize pigpio. check daemon" << std::endl;\

    }
    gpioSetPWMfrequency(PILLBOX_PWM_PIN, SERVO_FREQ_HZ);
    gpioServo(PILLBOX_PWM_PIN, SERVO_NEUTRAL_US);
    std::cout << "Pillbox Driver initialized on GPIO " << PILLBOX_PWM_PIN << std::endl;
}

pillbox::~pillbox() {
    gpioServo(PILLBOX_PWM_PIN, 0);
}

bool pillbox::openPillbox() {
    if (pillboxState) {
        std::cout << "pillbox already open." << std::endl;
        return true;
    }

    gpioServo(PILLBOX_PWM_PIN, SERVO_OPEN_POS_US);
    std::this_thread::sleep_for(std::chrono::milliseconds(OPEN_DURATION_MS));
    // TURNED OFF TO DETACH
    //gpioServo(PILLBOX_PWM_PIN, 0);
    pillboxState=true;
    return true;
}

bool pillbox::closePillbox() {
    if (!pillboxState) {
        std::cout << "Pillbox is already closed." << std::endl;
        return true;
    }

    std::cout << "Closing Pillbox..." << std::endl;
    // close CW

    gpioServo(PILLBOX_PWM_PIN, SERVO_CLOSED_POS_US); // MOVE TO CLOSED POS ANGLE

    std::this_thread::sleep_for(std::chrono::milliseconds(OPEN_DURATION_MS));
    gpioServo(PILLBOX_PWM_PIN, 0); // TURN OFF TO DETACH

    pillboxState = false;
    return true;
}

#else
// Mock Pillbox
pillbox::pillbox() { std::cout << "[Mock] Pillbox Initialized" << std::endl; }
pillbox::~pillbox() {}
bool pillbox::openPillbox() { 
    std::cout << "[Mock] Pillbox Opening..." << std::endl; 
    pillboxState = true; 
    return true; 
}
bool pillbox::closePillbox() { 
    std::cout << "[Mock] Pillbox Closing..." << std::endl; 
    pillboxState = false; 
    return true; 
}

#endif