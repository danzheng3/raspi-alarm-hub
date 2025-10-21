#include "hardware_layer/WifiAdapter.h"

WifiAdapter::WifiAdapter() : connected(false), ipAddress("") {}

WifiAdapter::~WifiAdapter() {}

bool WifiAdapter::scan(std::vector<std::string> &networks) {
    std::string command = "nmcli -t -f SSID dev wifi";
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        return false;
    }
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        std::string ssid(buffer);
        ssid.erase(ssid.find_last_not_of(" \n\r\t") + 1); 
        if (!ssid.empty()) {
            networks.push_back(ssid);
        }
    }
    pclose(pipe);
    return !networks.empty();
}

bool WifiAdapter::connect(const std::string& ssid, const std::string& password) {
    std::string command = "nmcli dev wifi connect '" + ssid + "' password '" + password + "'";
    int result = system(command.c_str());
    connected = true; // Simulate successful connection
    ipAddress = "";
    return connected;
}

bool WifiAdapter::disconnect() {
    std::string command = "nmcli con down id '" + ipAddress + "'";
    int result = system(command.c_str());
    connected = false;
    ipAddress = "";
    return !connected;
}

bool WifiAdapter::isConnected() {
    std::string command = "nmcli -t -f WIFI g";
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        return false;
    }
    char buffer[256];
    if (fgets(buffer, sizeof(buffer), pipe)) {
        std::string status(buffer);
        connected = (status.find("enabled") != std::string::npos);
    }
    return connected;
}

std::string WifiAdapter::getIPAddress() const {
    return ipAddress;
}


