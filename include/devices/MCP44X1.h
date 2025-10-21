#include "hardware_layer/I2CBus.h"
#include <cstdint>
#include <memory>

#define MCP44X1_I2C_ADDRESS 0x2C


class MCP44X1 {
public:
    //enum for the four wipers on the chip
    enum class Wiper: uint8_t {
        W0=0x00,
        W1=0x10,
        W2=0x20,
        W3=0x30
    };

    MCP44X1(std::shared_ptr<I2CBus> bus, uint8_t address);

    bool setWiper(Wiper wiper, uint8_t value);
    bool readWiper(Wiper wiper, uint8_t& value);

private:
    std::shared_ptr<I2CBus> i2cBus;
    uint8_t slaveAddress;


};