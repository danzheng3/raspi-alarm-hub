#include "GPIO.h"

class strobe {
    public:
        strobe();
        ~strobe();

        bool strobeActivate();
        bool strobeToNormal();

};