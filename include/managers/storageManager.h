#include <string>
#include <nlohmann/json.hpp>
#pragma once

class storageManager {
    public:
        storageManager(const std::string& filepath_ = "config.json");
        ~storageManager();

        bool load();
        bool save();

        std::string get(const std::string& key);
        void set(const std::string& key, const std::string& value);

        std::string getWifiSSID();
        std::string getWifiPassword();
        void setWifiCredentials(const std::string& ssid, const std::string& password);
        std::string getAlarmTime();
        void setAlarmTime(const std::string& time);
        std::string getRTCTime();
        void setRTCTime(const std::string& time);


    private:
        std::string filepath_;
        nlohmann::json data_;
};