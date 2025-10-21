#include "hardware_layer/I2CBus.h"
#include <cstdint>
#include <memory>


// ADC 
#define MCP3021_ADDRESS 0x4D

class MCP3021 {
    public:
        MCP3021(std::shared_ptr<I2CBus> bus, uint8_t address = MCP3021_ADDRESS);
        bool readValue(uint16_t& value); //raw 10 bit value
        bool readVoltage(float& voltage, float vref=3.3f);
        
        int readValue();

    private:
        std::shared_ptr<I2CBus> i2cBus;
        uint8_t slaveAddress;

};