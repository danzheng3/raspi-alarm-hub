#include "hardware_layer/SPIBus.h"
#include <cstring>

SPIBus::SPIBus(const std::string& devicePath) {
    fileDescriptor = open(devicePath.c_str(), O_RDWR);
    if (fileDescriptor < 0) {
        exit(1);
    }
    mode = SPI_MODE_0; // change SPEED AND MODE AS NEEDED
    if (ioctl(fileDescriptor, SPI_IOC_WR_MODE, &mode) < 0) {
        close(fileDescriptor);
        exit(1);
    }

    speed = 500000; // change SPEED AND MODE AS NEEDED
    if (ioctl(fileDescriptor, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
        close(fileDescriptor);
        exit(1);
    }
}

SPIBus::~SPIBus() {
    if (fileDescriptor > 0) close(fileDescriptor);
}

bool SPIBus::transfer(const uint8_t* txBuffer, uint8_t* rxBuffer, size_t length) {
    struct spi_ioc_transfer tr;
    memset(&tr, 0, sizeof(tr));
    tr.tx_buf = reinterpret_cast<__u64>(txBuffer);
    tr.rx_buf = reinterpret_cast<__u64>(rxBuffer);
    tr.len = length;
    tr.speed_hz = speed;
    tr.delay_usecs = 0;
    tr.bits_per_word = 8;

    if (ioctl(fileDescriptor, SPI_IOC_MESSAGE(1), &tr) < 0) {
        return false;
    }
    return true;
}