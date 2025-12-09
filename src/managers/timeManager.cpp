#include "managers/timeManager.h"
#include <iostream>

timeManager::timeManager(storageManager& storage, std::shared_ptr<MCP7940N> rtc_module, EventBus* eventBus) 
    : storage(storage), shared_rtc(rtc_module), m_eventBus(eventBus) {
    
    std::cout << "Initializing Time: Attempting to sync from RTC..." << std::endl;
    if (syncFromRTC()) {
        std::cout << "Time initialized from RTC: " << getFormattedTime() << std::endl;
    } else {
        std::cerr << "[Warning] Failed to sync from RTC (Battery dead or first boot)." << std::endl;
    }

    lastPublishedTime = getFormattedTime();
    std::cout << "timeManager initialized, current time: " << lastPublishedTime << std::endl;

    if (m_eventBus) {
        m_eventBus->subscribe<SystemWakeEvent>(this, &timeManager::onSystemWake);
    }
}

timeManager::~timeManager() {
    updateRTC();
}

bool timeManager::trySyncFromNTP() {
    // Force OS to check NTP
    std::cout << "Starting NTP Sync..." << std::endl;
    
    system("sudo systemctl restart systemd-timesyncd");
    
    for(int i = 0; i < 20; i++) {
        time_t now = time(nullptr);
        struct tm* t = localtime(&now);
        
        // Check if year is valid (e.g., > 2023)
        // If system time was 1970/2000, and is now 2025, NTP worked.
        if (t->tm_year + 1900 > 2023) {
            std::string newTime = getFormattedTime();
            std::string newDate = getFormattedDate();

            std::cout << "NTP Sync Successful! System Time: " << newDate << " " << newTime << std::endl;
            
            updateRTC(); 
            std::cout << "RTC updated with accurate NTP time." << std::endl;
            
            // notify UI
            if (m_eventBus) {
                TimeUpdatedEvent event;
                event.currentTime = newTime;
                m_eventBus->publish(event);
            }
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    
    std::cerr << "NTP Sync timed out (Internet issues?). Keeping RTC time." << std::endl;
    return false;
}

bool timeManager::setSystemTime(const struct tm& time) {
    time_t new_time = mktime(const_cast<struct tm*>(&time));
    
    struct timeval tv;
    tv.tv_sec = new_time;
    tv.tv_usec = 0;
    
    return settimeofday(&tv, nullptr) == 0;
}



void timeManager::setTime(const struct tm& time) {
    if (setSystemTime(time)) {
        std::cout << "system time set to: " << getFormattedTime() << std::endl;
        updateRTC();

        if (m_eventBus) {
            TimeUpdatedEvent event;
            event.currentTime = getFormattedTime();
            m_eventBus->publish(event);
            lastPublishedTime = event.currentTime;
        }
    } else {
        std::cerr << "[Error] setting system time" << std::endl;
    }
}

bool timeManager::syncFromRTC() {
    // READ FROM RTC
    if (!shared_rtc) {
        std::cerr << "error: no rtc module in timeManager sync" << std::endl;
        return false;
    }
    RTC_Time rtcTime;
    if (!(shared_rtc->getTime(rtcTime))) {
        std::cout << "error reading rtc time in sync" << std::endl; 
        return false;
    }

    if (rtcTime.year == 0 && rtcTime.month == 0 && rtcTime.day == 0) {
        std::cerr << "rtc time not set, skipping sync" << std::endl;
        return false;
    }

    struct tm time = rtcToTm(rtcTime);

    if (setSystemTime(time)) {
        std::cout << "system time synced from rtc: " << getFormattedTime() << std::endl;

        if (m_eventBus) {
            TimeUpdatedEvent event;
            event.currentTime = getFormattedTime();
            m_eventBus->publish(event);
        }

        return true;
    } else {
        std::cerr << "[Error] setting system time from rtc" << std::endl;
        return false;
    }

}

void timeManager::updateRTC() {
    struct tm time = getSystemTime();
    RTC_Time rtcTime = tmToRtc(time);

    if (shared_rtc->setTime(rtcTime)) {
        std::cout << "RTC time updated from system time" << std::endl;
    } else {
        std::cerr << "[Error] updating rtc time from system time" << std::endl;
    }
}

struct tm timeManager::getSystemTime() const {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    struct tm local_time;
    localtime_r(&time_t_now, &local_time);
    return local_time;
}

struct tm timeManager::getCurrentTime() const {
    return getSystemTime();
}

void timeManager::checkAndPublishTimeUpdate() {
    std::string currentTime = getFormattedTime();
    if (currentTime != lastPublishedTime) {
        std::cout << "[Time Manager]: Time changed from " << lastPublishedTime 
                  << " to " << currentTime << " - Publishing Event" << std::endl;
        lastPublishedTime = currentTime;
        
        if (m_eventBus) {
            TimeUpdatedEvent event;
            event.currentTime = currentTime;
            m_eventBus->publish(event);
        }
    }
}


// struct to convert between tm and RTC_Time

RTC_Time timeManager::tmToRtc(const struct tm& time) const {
    RTC_Time rtcTime;
    rtcTime.seconds = time.tm_sec;
    rtcTime.minutes = time.tm_min;
    rtcTime.hours = time.tm_hour;
    rtcTime.day = time.tm_mday;
    rtcTime.month = time.tm_mon + 1;        // RTC expects 1-12
    rtcTime.year = time.tm_year - 100;      // RTC stores years since 2000
    rtcTime.dayOfWeek = time.tm_wday;
    return rtcTime;
}

struct tm timeManager::rtcToTm(const RTC_Time& rtcTime) const {
    struct tm time = {};
    time.tm_sec = rtcTime.seconds;
    time.tm_min = rtcTime.minutes;
    time.tm_hour = rtcTime.hours;
    time.tm_mday = rtcTime.day;
    time.tm_mon = rtcTime.month - 1;        // tm_mon is 0-11
    time.tm_year = rtcTime.year + 100;      // tm_year is years since 1900
    time.tm_wday = rtcTime.dayOfWeek;
    time.tm_isdst = -1;
    return time;
}

std::string timeManager::getFormattedTime(const char* format) const {
    struct tm time = getSystemTime();
    char buffer[64];
    strftime(buffer, sizeof(buffer), format, &time);
    return std::string(buffer);
}

std::string timeManager::getFormattedDate(const char* format) const {
    struct tm time = getSystemTime();
    char buffer[64];
    strftime(buffer, sizeof(buffer), format, &time);
    return std::string(buffer);
}

// EVENT HANDLER

void timeManager::onSystemWake(const SystemWakeEvent& event) {
    std::cout << "[TimeMgr] System woke up. Checking if NTP sync is needed..." << std::endl;

    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::minutes>(now - lastNtpSyncTime).count();

    // Only sync if it has been > 5 minutes (or never synced)
    if (elapsed >= 5 || lastNtpSyncTime.time_since_epoch().count() == 0) {
        //run in thread to avoid blocking
        std::thread([this]() {
            if (trySyncFromNTP()) {
               lastNtpSyncTime = std::chrono::steady_clock::now();
            }
        }).detach();

    } else {
        std::cout << "[TimeMgr] Skipping NTP sync (synced " << elapsed << " mins ago)" << std::endl;
    }
}