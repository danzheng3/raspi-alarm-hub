#pragma once
#include <string>
#include <vector>
#include <chrono>

class WifiAdapter {
    public:
        WifiAdapter();
        ~WifiAdapter();

        bool connect(const std::string& ssid, const std::string& password);
        bool disconnect();
        bool isConnected();
        std::string getIPAddress() const;
        bool scan(std::vector<std::string>& networks);

    private:
        bool connected;
        std::string ipAddress;

        std::chrono::steady_clock::time_point lastCheckTime;
        bool cachedConnectionState = false;
};