#include "hardware_layer/WifiAdapter.h"
#include <iostream>

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
    std::cout << "wifi connect command after: " << result << std::endl;
    if (result != 0) {
        connected = false;
        ipAddress = "";
        return connected;
    }
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
    auto now = std::chrono::steady_clock::now();
    if (now - lastCheckTime < std::chrono::seconds(5)) {
        return cachedConnectionState;
    }
    lastCheckTime = now;

    // cache and check every 5 seconds
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
    //std::cout << "wifi isConnected: wifi enabled" << std::endl;
    pclose(pipe);
    
    if (!connected) {
        cachedConnectionState = false;
        return false;
    }

    std::string command2 = "nmcli -t -f GENERAL.STATE device show wlan0";
    FILE* pipe2 = popen(command2.c_str(), "r");
    if (!pipe2) return false;

    char buffer2[256];
    bool is_actually_connected = false;
    while (fgets(buffer2, sizeof(buffer2), pipe2)) {
        std::string output(buffer2);
        // nmcli returns state codes. 100 means fully connected.
        //std::cout << "wifi isConnected: state output: " << output << std::endl;
        if (output.find("100 (connected)") != std::string::npos) {
            is_actually_connected = true;
        }
    }

    pclose(pipe2);

    cachedConnectionState = is_actually_connected;
    
    return cachedConnectionState;
}

std::string WifiAdapter::getIPAddress() const {
    return ipAddress;
}




