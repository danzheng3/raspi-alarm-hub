#include "hardware_layer/I2CBus.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

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

