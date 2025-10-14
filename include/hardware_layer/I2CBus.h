#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <cstdint>
#include <cstddef>
#include <string>

class I2CBus {
    public:
        I2CBus(const std::string& devicePath = "/dev/i2c-1"); // may need to change LATER
        ~I2CBus();

        bool setSlaveAddress(uint8_t address);
        bool writeData(const uint8_t* data, size_t length);
        bool readData(uint8_t* buffer, size_t length);
    private:
        int fileDescriptor;
        std::string devicePath;
};