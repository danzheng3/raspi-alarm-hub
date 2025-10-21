#pragma once

#include <string>
#include "storageManager.h"
#include "devices/MCP7940N.h"

class timeManager {
    public:
        timeManager(storageManager& storage, std::shared_ptr<MCP7940N> rtc_module);
        ~timeManager();

        std::string getCurrentTime() const;
        void setTime(const std::string& time);
        void syncFromRTC();
    
    private:
        std::string currentTime;
        storageManager& storage;
        std::shared_ptr<MCP7940N> shared_rtc;
};