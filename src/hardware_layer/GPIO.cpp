#include <hardware_layer/GPIO.h>
#include <cstring>
#include <iostream>

#ifndef TEST_MODE


GPIOPin::GPIOPin(int pin) : pinNumber(pin) {
    chip = gpiod_chip_open_by_name("gpiochip0"); // MOST LIKELY NEED TO MODIFY BASED ON CHIPNUM
    line = gpiod_chip_get_line(chip, pinNumber);
    if (!chip || !line) {
        std::cerr << "Failed to open GPIO chip or get line" << std::endl;
        return;
    }

    std::cout << "GPIO Initialized" << std::endl;
}

GPIOPin::~GPIOPin() {
    if (line) {
        gpiod_line_release(line);
    }
    if (chip) {
        gpiod_chip_close(chip);
    }
}

bool GPIOPin::pinHigh() {
    int ret;
    std::cout << "Setting GPIO " << pinNumber << " high" << std::endl;
    ret = gpiod_line_set_value(line, 1);

    return ret == 0;
}



bool GPIOPin::pinLow() {
    return gpiod_line_set_value(line, 0) == 0;
}

bool GPIOPin::pinModeOut() {
    return gpiod_line_request_output(line, "my_gpio", 0) == 0;
}

bool GPIOPin::pinModeIn(GPIOBias bias) {
    if (line) {
        gpiod_line_release(line);
    }
    
    struct gpiod_line_request_config config;
    config.consumer = "RaspiAlarmHub";
    config.request_type = GPIOD_LINE_REQUEST_DIRECTION_INPUT;
    
    // Set bias flags based on enum
    config.flags = 0;
    if (bias == GPIOBias::PULL_UP) {
        config.flags |= GPIOD_LINE_REQUEST_FLAG_BIAS_PULL_UP;
    } else if (bias == GPIOBias::PULL_DOWN) {
        config.flags |= GPIOD_LINE_REQUEST_FLAG_BIAS_PULL_DOWN;
    }

    int ret = gpiod_line_request(line, &config, 0);
    return ret == 0;
}

int GPIOPin::pinRead() {
    return gpiod_line_get_value(line);
}

#else

// --- MOCK IMPLEMENTATION ---
GPIOPin::GPIOPin(int pin) : pinNumber(pin) {
    std::cout << "[Mock] GPIO " << pin << " Initialized" << std::endl;
}
GPIOPin::~GPIOPin() {}

bool GPIOPin::pinHigh() {
    std::cout << "[Mock] GPIO " << pinNumber << " HIGH" << std::endl;
    return true;
}
bool GPIOPin::pinLow() {
    std::cout << "[Mock] GPIO " << pinNumber << " LOW" << std::endl;
    return true;
}
bool GPIOPin::pinModeOut() { return true; }
bool GPIOPin::pinModeIn(GPIOBias bias) { return true; }
bool GPIOPin::pinRead() { return false; } // Default to 0 (Low)
#endif
