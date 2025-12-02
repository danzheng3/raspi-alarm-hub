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

        struct tm getCurrentTime() const;
        std::string getFormattedTime(const char* format = "%H:%M") const;
        std::string getFormattedDate(const char* format = "%Y-%m-%d") const;

        void setTime(const struct tm& time);
        void setTimeFromString(const std::string& time);

        bool syncFromRTC();
        void updateRTC();

        void checkAndPublishTimeUpdate();
    
    private:
        std::string lastPublishedTime;
        storageManager& storage;
        std::shared_ptr<MCP7940N> shared_rtc;
        EventBus* m_eventBus;

        struct tm getSystemTime() const; // struct to help get system time
        RTC_Time tmToRtc(const struct tm& time) const;
        struct tm rtcToTm(const RTC_Time& rtcTime) const;

        bool setSystemTime(const struct tm& time);


};