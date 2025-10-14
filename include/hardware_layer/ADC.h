#include "I2CBus.h"

class ADC {
    public:
        ADC();
        ~ADC();
        int readValue();

    private:
        uint8_t address;

};