#include "managers/alarmManager.h"
#include <iostream>

alarmManager::alarmManager(storageManager& storage, timeManager& timeMgr) : storage(storage), timeMgr(timeMgr) {
    loadFromStorage();
}

alarmManager::~alarmManager() {}

void alarmManager::loadFromStorage() {
    alarmTime = storage.getAlarmTime();
    alarmEnabled = true;
    std::cout << "loaded alarm " << alarmTime << std::endl;
}

void alarmManager::saveToStorage() {
    storage.setAlarmTime(alarmTime);
    storage.save();
    std::cout << "saved alarm " << alarmTime << std::endl;
}

void alarmManager::setAlarm(const std::string& time) {
    alarmTime = time;
    alarmEnabled = true;
    alarmTriggered = false;
    std::cout << "set alarm " << alarmTime << std::endl;
}

bool alarmManager::isAlarmEnabled() const {
    return alarmEnabled;
}

void alarmManager::resetTrigger() {
    alarmTriggered = false;
}

bool alarmManager::shouldTrigger() {
    if (!alarmEnabled || alarmTriggered) {
        return false;
    }

    std::string currentTime = timeMgr.getCurrentTime();
    if (currentTime == alarmTime) {
        alarmTriggered = true;
        std::cout << "Alarm triggered at " << currentTime << std::endl;
        return true;
    }
    return false;
}



