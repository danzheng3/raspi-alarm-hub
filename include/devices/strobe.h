#include "hardware_layer/GPIO.h"

class strobe {
    public:
        strobe();
        ~strobe();

        bool strobeActivate();
        bool strobeToNormal();

};