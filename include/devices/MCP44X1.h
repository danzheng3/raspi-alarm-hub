#include "hardware_layer/I2CBus.h"
#include <cstdint>
#include <memory>

#define MCP44X1_I2C_ADDRESS 0x2C


class MCP44X1 {
public:
    //enum for the four wipers on the chip
    enum class Wiper: uint8_t {
        W0=0x00,
        W1=0x01,
        W2=0x06,
        W3=0x07 // THESE ARE REGISTER INDEXES
    };

    MCP44X1(std::shared_ptr<I2CBus> bus, uint8_t address);

    bool setWiper(Wiper wiper, uint8_t value);
    bool readWiper(Wiper wiper, uint8_t& value);

private:
    std::shared_ptr<I2CBus> i2cBus;
    uint8_t slaveAddress;

    static const uint8_t CMD_WRITE = 0x00; // Bits 00
    static const uint8_t CMD_READ  = 0x03; // Bits 11 (0x0C)

};