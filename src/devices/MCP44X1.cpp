#include "devices/MCP44X1.h"

MCP44X1::MCP44X1(std::shared_ptr<I2CBus> bus, uint8_t address)
    : i2cBus(bus), slaveAddress(address) {}

bool MCP44X1::setWiper(Wiper wiper, uint8_t value) {
    if (!i2cBus->setSlaveAddress(slaveAddress)) {
        return false;
    }
    uint8_t regIndex = static_cast<uint8_t>(wiper);
    uint8_t buf[2];

    buf[0] = (regIndex << 4) | ((value >> 8) & 0x03);

    // --- DATA BYTE (buf[1]) ---
    buf[1] = value & 0xFF;
    return i2cBus->writeData(buf, 2);
}

bool MCP44X1::readWiper(Wiper wiper, uint8_t& value) {
    if (!i2cBus->setSlaveAddress(slaveAddress)) {
        return false;
    }

    uint8_t reg = static_cast<uint8_t>(wiper);


    uint8_t commandByte = (reg << 4) | 0x0C;

    if (!i2cBus->writeData(&commandByte, 1))
        return false;


    uint8_t rxBuf[2];
    if (!i2cBus->readData(rxBuf, 2))
        return false;

    value = ((static_cast<uint16_t>(rxBuf[0] & 0x01) << 8) | rxBuf[1]);

    return true;
}

