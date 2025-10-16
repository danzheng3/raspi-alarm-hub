#include "managers/timeManager.h"
#include <iostream>

timeManager::timeManager(storageManager& storage) : storage(storage) {
    currentTime = storage.getRTCTime();
    if (currentTime.empty()) {
        currentTime = "12:00"; // none from rtc
    std::cout << "initialized timeManager with time " << currentTime << std::endl;
    }
}

timeManager::~timeManager() {}


std::string timeManager::getCurrentTime() const {
    return currentTime;
}

void timeManager::setTime(const std::string& time) {
    currentTime = time;

    std::cout << "Time set to " << currentTime << std::endl;
    std::cout << "saving time to rtc" << std::endl;
    //updateRTC();
}

void timeManager::syncFromRTC() {
    // READ FROM RTC
    // currentTIme = rtc.getTime();
    std::cout << "Time synchronized from RTC: " << currentTime << std::endl;
}