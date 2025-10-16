#include <gpiod.h>
#include <string>

class GPIOPin {
    public:
        GPIOPin(int pin);
        ~GPIOPin();

        bool pinHigh(); // set pin high/low
        bool pinLow();
        bool pinModeOut(); // set pin as I/O
        bool pinModeIn();
        bool pinRead(); //pin state
    private:
        int pinNumber;
        gpiod_chip* chip;
        gpiod_line* line;
        std::string chipName;

};