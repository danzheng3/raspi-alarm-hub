#include "managers/storageManager.h"
#include <fstream>
#include <iostream>

storageManager::storageManager(const std::string& filepath_) : filepath_(filepath_) {}

bool storageManager::load() {
    std::ifstream file(filepath_);
    if (!file.is_open()) {
        data_ = {
            {"alarm_time", "07:00"},
            {"rtc_time", "12:00"},
            {"wifi_credentials", {{"ssid", ""}, {"password", ""}}}
        };
        return false;
    }
    try {
        file >> data_;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        return false;
    }
}

bool storageManager::save() {
    std::ofstream file(filepath_);
    if (!file.is_open()) {
        std::cerr << "Could not open file for writing: " << filepath_ << std::endl;
        return false;
    }
    try {
        file << data_.dump(4);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error writing JSON: " << e.what() << std::endl;
        return false;
    }
}

std::string storageManager::get(const std::string& key) {
    if (data_.contains(key)) return data_.at(key).dump();
    return "";
}

void storageManager::set(const std::string& key, const std::string& value) {
    data_[key] = value;
}

std::string storageManager::getWifiSSID() {
    return data_["wifi_credentials"]["ssid"];
}

std::string storageManager::getWifiPassword() {
    return data_["wifi_credentials"]["password"];
}

void storageManager::setWifiCredentials(const std::string& ssid, const std::string& password) {
    data_["wifi_credentials"]["ssid"] = ssid;
    data_["wifi_credentials"]["password"] = password;
}

std::string storageManager::getAlarmTime() {
    return data_["alarm_time"];
}

void storageManager::setAlarmTime(const std::string& time) {
    data_["alarm_time"] = time;
}

std::string storageManager::getRTCTime() {
    return data_["rtc_time"];
}

void storageManager::setRTCTime(const std::string& time) {
    data_["rtc_time"] = time;
}

