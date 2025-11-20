#pragma once

#include <string>
#include "storageManager.h"
#include "devices/MCP7940N.h"
#include "events/EventBus.h"
#include "events/Events.h"
#include <unistd.h>
#include <chrono>
#include <sys/time.h>

class timeManager {
    public:
        timeManager(storageManager& storage, std::shared_ptr<MCP7940N> rtc_module, EventBus* eventBus);
        ~timeManager();

        // get current system time

        std::string getCurrentTime() const;
        void setTime(const std::string& time);
        void syncFromRTC();
        void updateRTC();
    
    private:
        std::string currentTime;
        std::string lastPublishedTime;
        storageManager& storage;
        std::shared_ptr<MCP7940N> shared_rtc;
        EventBus* m_eventBus;

        struct tm getSystemTime() const; // struct to help get system time
};