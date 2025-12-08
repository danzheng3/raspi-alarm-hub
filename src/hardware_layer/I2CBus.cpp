#include "hardware_layer/I2CBus.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <iostream>

#ifndef TEST_MODE
I2CBus::I2CBus(const std::string& devicePath) {
    //open i2c device
    fileDescriptor = open(devicePath.c_str(), O_RDWR);
    if (fileDescriptor < 0) {
        exit(1);
    }

}

I2CBus::~I2CBus() {
    if (fileDescriptor >= 0) {
        close(fileDescriptor);
    }
}

bool I2CBus::setSlaveAddress(uint8_t address) {
    if (ioctl(fileDescriptor, I2C_SLAVE, address) < 0) {
        return false;
    }
    return true;

}

bool I2CBus::writeData(const uint8_t* data, size_t length) {
    if (write(fileDescriptor, data, length) != static_cast<ssize_t>(length)) {
        return false;
    }
    return true;
}

bool I2CBus::readData(uint8_t* buffer, size_t length) {
    if (read(fileDescriptor, buffer, length) != static_cast<ssize_t>(length)) {
        return false;
    }
    return true;
}

#else

I2CBus::I2CBus(const std::string& devicePath) {
    std::cout << "[Mock] I2C Bus opened: " << devicePath << std::endl;
}
I2CBus::~I2CBus() {}
bool I2CBus::setSlaveAddress(uint8_t address) { return true; }
bool I2CBus::writeData(const uint8_t* data, size_t length) { return true; }
bool I2CBus::readData(uint8_t* buffer, size_t length) { 
    // Return dummy data so devices don't get stuck
    for(size_t i=0; i<length; i++) buffer[i] = 0;
    return true; 
}
#endif