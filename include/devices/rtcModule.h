#include "hardware_layer/I2CBus.h"

class rtcModule {
    public:
        rtcModule();
        ~rtcModule();
        bool setTime();
        int getTime();

    private:
        uint8_t module_address;
};