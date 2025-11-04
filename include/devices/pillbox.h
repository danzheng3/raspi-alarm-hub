#include "hardware_layer/GPIO.h"
#pragma once

class pillbox {
    public:
        pillbox();
        ~pillbox();

        bool openPillbox();

    private:
        bool pillboxState = false;
};


