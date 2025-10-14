#include <hardware_layer/GPIO.h>
#include <cstring>
#include <iostream>

GPIOPin::GPIOPin(int pin) : pinNumber(pin) {
    chip = gpiod_chip_open_by_name("gpiochip0");
    line = gpiod_chip_get_line(chip, pinNumber);
    if (!chip || !line) {
        std::cerr << "Failed to open GPIO chip or get line" << std::endl;
        return;
    }
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
    printf("Setting GPIO %d HIGH\n", pinNumber);
    ret = gpiod_line_set_value(line, 1);

    return ret == 0;
}

bool GPIOPin::pinLow() {
    return gpiod_line_set_value(line, 0) == 0;
}

bool GPIOPin::pinModeOut() {
    return gpiod_line_request_output(line, "my_gpio", 0) == 0;
}

bool GPIOPin::pinModeIn() {
    return gpiod_line_request_input(line, "my_gpio") == 0;
}

bool GPIOPin::pinRead() {
    return gpiod_line_get_value(line) == 1;
}
