#include "managers/timeManager.h"
#include <iostream>

timeManager::timeManager(storageManager& storage, std::shared_ptr<MCP7940N> rtc_module, EventBus* eventBus) 
    : storage(storage), shared_rtc(rtc_module), m_eventBus(eventBus) {
    //currentTime = storage.getRTCTime();
    if (!syncFromRTC()) {
        std::cerr << "error syncing time from rtc during timeManager init" << std::endl;
    }


    lastPublishedTime = getFormattedTime();
    std::cout << "timeManager initialized, current time: " << lastPublishedTime << std::endl;
}

timeManager::~timeManager() {
    updateRTC();
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
        std::cerr << "error setting system time" << std::endl;
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
        std::cerr << "error setting system time from rtc" << std::endl;
        return false;
    }

}

void timeManager::updateRTC() {
    struct tm time = getSystemTime();
    RTC_Time rtcTime = tmToRtc(time);

    if (shared_rtc->setTime(rtcTime)) {
        std::cout << "rtc time updated from system time" << std::endl;
    } else {
        std::cerr << "error updating rtc time from system time" << std::endl;
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
