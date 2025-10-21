//mcp3021 adc 

#include "devices/MCP3021.h"

MCP3021::MCP3021(std::shared_ptr<I2CBus> bus, uint8_t address) 
    : i2cBus(bus), slaveAddress(address) {}

bool MCP3021::readValue(uint16_t& value) {
    if (!i2cBus->setSlaveAddress(slaveAddress)) return false;

    // 2 bytes into 10-bit value
    uint8_t buffer[2];
    if (!i2cBus->readData(buffer, 2)) return false;

    // This is the 10-bit conversion logic from your main.c
    // Byte 1: [0, 0, 0, 0, D9, D8, D7, D6]
    // Byte 2: [D5, D4, D3, D2, D1, D0, 0, 0]
    value = ((buffer[0] & 0x0F) << 6) | (buffer[1] >> 2);
    
    return true;
}

bool MCP3021::readVoltage(float& voltage, float vref) {
    uint16_t rawValue;
    if (!readValue(rawValue)) {
        return false;
    }

    // Convert the 10-bit value (0-1023) to a voltage
    voltage = (static_cast<float>(rawValue) / 1023.0f) * vref;
    return true;
}