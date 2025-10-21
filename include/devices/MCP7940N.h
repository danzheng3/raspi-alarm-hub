#include "hardware_layer/I2CBus.h"
#include <cstdint>
#include <memory>

//MCP7940N I2C RTC

#define MCP7940N_I2C_ADDRESS 0x6F
#define RTC_SECONDS_ADDR 0x00
#define RTC_WEEKDAY_ADDR 0x03

struct RTC_Time {
        uint8_t seconds;
        uint8_t minutes;
        uint8_t hours;
        uint8_t dayOfWeek;
        uint8_t day;
        uint8_t month;
        uint8_t year;
};

class MCP7940N {
    public:

    MCP7940N(std::shared_ptr<I2CBus> bus, uint8_t address = MCP7940N_I2C_ADDRESS);

    bool getTime(RTC_Time& time);
    bool setTime(const RTC_Time& time);

    bool isClockRunning(); //check if oscillator running
    bool startClock();

    private:
        std::shared_ptr<I2CBus> i2cBus;
        uint8_t slaveAddress;

    // Register map
        static const uint8_t REG_SECONDS = RTC_SECONDS_ADDR;
        static const uint8_t REG_WEEKDAY = RTC_WEEKDAY_ADDR;

        uint8_t BCD_to_BIN(uint8_t bcd);
        uint8_t BIN_to_BCD(uint8_t bin);
};  