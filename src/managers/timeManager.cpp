#include "managers/timeManager.h"
#include <iostream>

timeManager::timeManager(storageManager& storage, std::shared_ptr<MCP7940N> rtc_module) 
    : storage(storage), shared_rtc(rtc_module) {
    //currentTime = storage.getRTCTime();
    syncFromRTC();
    
    if (currentTime.empty()) {
        currentTime = "12:00"; // none from rtc
        std::cout << "initialized timeManager with time " << currentTime << ". not from rtc" << std::endl;
    }
}

timeManager::~timeManager() {}


std::string timeManager::getCurrentTime() const {
    return currentTime;
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
    //updateRTC();
}

void timeManager::syncFromRTC() {
    // READ FROM RTC
    RTC_Time currTime;
    if (!(shared_rtc->getTime(currTime))) {
        std::cout << "error reading rtc time in sync" << std::endl; 
        return;
    }
    char buffer[9];
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", currTime.hours, currTime.minutes, currTime.seconds);
    std::string adj_time = std::string(buffer);
    adj_time = adj_time.substr(0,5); // HH:MM
    currentTime = std::string(buffer); // NEED TO FIX LATER FOR RTC LOGIC

    std::cout << "Time synchronized from RTC: " << currentTime << std::endl;

}