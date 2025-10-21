#include "devices/MCP44X1.h"

MCP44X1::MCP44X1(std::shared_ptr<I2CBus> bus, uint8_t address)
    : i2cBus(bus), slaveAddress(address) {}

bool MCP44X1::setWiper(Wiper wiper, uint8_t value) {
    if (!i2cBus->setSlaveAddress(slaveAddress)) {
        return false;
    }

    //2 byte command
    uint8_t command[2];
    command[0] = static_cast<uint8_t>(wiper);
    command[1]=value;
    return i2cBus->writeData(command, 2);
}

bool MCP44X1::readWiper(Wiper wiper, uint8_t& value) {
    if (!i2cBus->setSlaveAddress(slaveAddress)) {
        return false;
    }
    
    uint8_t command = static_cast<uint8_t>(wiper);
    if (!i2cBus->writeData(&command,1)) {
        return false;
    }
    return i2cBus->readData(&value,1);
}

