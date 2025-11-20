#include "managers/timeManager.h"
#include <iostream>

timeManager::timeManager(storageManager& storage, std::shared_ptr<MCP7940N> rtc_module, EventBus* eventBus) 
    : storage(storage), shared_rtc(rtc_module), m_eventBus(eventBus) {
    //currentTime = storage.getRTCTime();
    syncFromRTC();

    
    if (currentTime.empty()) {
        currentTime = "12:00"; // none from rtc
        std::cout << "initialized timeManager with time " << currentTime << ". not from rtc" << std::endl;
    }

    lastPublishedTime = getCurrentTime();
}

timeManager::~timeManager() {
    updateRTC();
}


std::string timeManager::getCurrentTime() const {
    struct tm local_time = getSystemTime();
    char buffer[6];
    strftime(buffer, sizeof(buffer), "%H:%M", &local_time);
    return std::string(buffer);
}

void timeManager::setTime(const std::string& time) {
    currentTime = time;
    std::string command = "sudo date -s '" + time + ":00'";
    int result = system(command.c_str());
    if (result==0) {
        std::cout << "Time set to " << currentTime << std::endl;
    } else {
        std::cout << "time set fail";
    }

    std::cout << "saving time to rtc" << std::endl;
    updateRTC();
}

void timeManager::syncFromRTC() {
    // READ FROM RTC
    RTC_Time rtcTime;
    if (!(shared_rtc->getTime(rtcTime))) {
        std::cout << "error reading rtc time in sync" << std::endl; 
        return;
    }
    /* 
    char buffer[9];
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", currTime.hours, currTime.minutes, currTime.seconds); // NEED TO FIX TIME SYNC
    std::string adj_time = std::string(buffer);
    adj_time = adj_time.substr(0,5); // HH:MM
    currentTime = std::string(buffer); // NEED TO FIX LATER FOR RTC LOGIC
    */

    struct tm timeinfo = {}; // convert RTC time to system time
    timeinfo.tm_hour = rtcTime.hours;
    timeinfo.tm_min = rtcTime.minutes;
    timeinfo.tm_sec = rtcTime.seconds;
    timeinfo.tm_mday = rtcTime.day;
    timeinfo.tm_mon = rtcTime.month-1;
    timeinfo.tm_year = rtcTime.year+100;
    timeinfo.tm_wday = rtcTime.dayOfWeek;
    timeinfo.tm_isdst =-1;
    
    time_t new_time = mktime(&timeinfo);

    struct timeval tv;
    tv.tv_sec = new_time;
    tv.tv_usec = 0;

    if(settimeofday(&tv, nullptr) == 0) {
        std::cout << "system time synced from rtc: " << getCurrentTime() << std::endl;

        if (m_eventBus) {
            TimeUpdatedEvent event;
            event.currentTime = getCurrentTime();
            m_eventBus->publish(event);
        }
    } else {
        std::cerr << "error: failed to set sys time in timeManager" << std::endl;
    }

    std::cout << "Time synchronized from RTC: " << currentTime << std::endl;

}

void timeManager::updateRTC() {
    struct tm local_time = getSystemTime();
    
    RTC_Time rtcTime;
    rtcTime.hours = local_time.tm_hour;
    rtcTime.minutes = local_time.tm_min;
    rtcTime.seconds = local_time.tm_sec;
    rtcTime.day = local_time.tm_mday;
    rtcTime.month = local_time.tm_mon + 1; // RTC expects 1-12
    rtcTime.year = local_time.tm_year - 100; // RTC stores years since 2000
    rtcTime.dayOfWeek = local_time.tm_wday;
    
    if (shared_rtc->setTime(rtcTime)) {
        std::cout << "RTC updated from system time: " << getCurrentTime() << std::endl;
    } else {
        std::cerr << "Error: Failed to update RTC" << std::endl;
    }
}

struct tm timeManager::getSystemTime() const {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    struct tm local_time;
    localtime_r(&time_t_now, &local_time);
    return local_time;
}