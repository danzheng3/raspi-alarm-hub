#include "devices/MCP7940N.h"

MCP7940N::MCP7940N(std::shared_ptr<I2CBus> bus, uint8_t address)
    : i2cBus(bus), slaveAddress(address) {}



bool MCP7940N::getTime(RTC_Time& time) {
    if (!i2cBus->setSlaveAddress(slaveAddress)) return false;

    uint8_t startReg = REG_SECONDS;
    if (!i2cBus->writeData(&startReg, 1)) return false;

    uint8_t buffer[7];
    if (!i2cBus->readData(buffer, 7)) return false;
    time.seconds = BCD_to_BIN(buffer[0] & 0x7F);
    time.minutes = BCD_to_BIN(buffer[1]);
    time.hours   = BCD_to_BIN(buffer[2] & 0x3F);
    time.dayOfWeek = BCD_to_BIN(buffer[3] & 0x07);
    time.day     = BCD_to_BIN(buffer[4]);
    time.month   = BCD_to_BIN(buffer[5] & 0x1F);
    time.year    = BCD_to_BIN(buffer[6]);

    return true;
}

bool MCP7940N::setTime(const RTC_Time& time) {
    if (!i2cBus->setSlaveAddress(slaveAddress)) return false;

    uint8_t buffer[8];
    buffer[0] = REG_SECONDS; // Start at the seconds register
    

    buffer[1] = BIN_to_BCD(time.seconds) | (1 << 7); // ST bit =1
    buffer[2] = BIN_to_BCD(time.minutes);
    buffer[3] = BIN_to_BCD(time.hours); // Assumes 24hr
    
    buffer[4] = BIN_to_BCD(time.dayOfWeek) | (1 << 3); // VBATEN = 1
    buffer[5] = BIN_to_BCD(time.day);
    buffer[6] = BIN_to_BCD(time.month);
    buffer[7] = BIN_to_BCD(time.year);

    return i2cBus->writeData(buffer, 8);
}

bool MCP7940N::isClockRunning() {
    if (!i2cBus->setSlaveAddress(slaveAddress)) return false;

    uint8_t startReg = REG_SECONDS;
    if (!i2cBus->writeData(&startReg, 1)) return false;

    uint8_t secondsReg;
    if (!i2cBus->readData(&secondsReg, 1)) return false;

    // Return true if the Start (ST) bit (bit 7) is set
    return (secondsReg & (1 << 7));
}

bool MCP7940N::startClock() {
    if (!i2cBus->setSlaveAddress(slaveAddress)) return false;

    // 1. Read the current seconds register
    uint8_t startReg = REG_SECONDS;
    if (!i2cBus->writeData(&startReg, 1)) return false;
    uint8_t secondsReg;
    if (!i2cBus->readData(&secondsReg, 1)) return false;

    // 2. Prepare data to write back, setting the ST bit
    uint8_t buffer[2];
    buffer[0] = REG_SECONDS; // Register address
    buffer[1] = secondsReg | (1 << 7); // Set ST bit

    // 3. Write it back
    return i2cBus->writeData(buffer, 2);
}

uint8_t MCP7940N::BCD_to_BIN(uint8_t bcd) {
    return (bcd >> 4) * 10 + (bcd & 0x0F);
}

uint8_t MCP7940N::BIN_to_BCD(uint8_t bin) {
    return ((bin / 10) << 4) | (bin % 10);
}