#include <linux/spi/spidev.h>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <cstdint>

class SPIBus {
    public:
        SPIBus(const std::string& devicePath = "/dev/spidev0.0"); // may need to change LATER with devicePath
        ~SPIBus();

        bool transfer(const uint8_t* txBuffer, uint8_t* rxBuffer, size_t length);




    private:
        int fileDescriptor;
        int mode;
        int speed;
        std::string devicePath;

};