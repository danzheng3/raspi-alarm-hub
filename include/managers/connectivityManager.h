#include <string>
#include <vector>
#pragma once
#include "hardware_layer/BluetoothAdapter.h"
#include "hardware_layer/WifiAdapter.h"
#include "storageManager.h"

class connectivityManager {
public:
    connectivityManager(WifiAdapter& wifiAdapter, BluetoothAdapter& btAdapter, storageManager& storage);
    ~connectivityManager();

    void init();

    bool connectToWifi(const std::string& ssid, const std::string& password);
    void disconnectWifi();
    bool connectBluetooth(const std::string& deviceAddress);
    void disconnectBluetooth();

    bool isWifiConnected();
    bool isBluetoothConnected();

private:
    WifiAdapter& wifiAdapter;
    BluetoothAdapter& btAdapter;
    storageManager& storage;

    std::string currentSSID;
    std::string currentSpeakerID;
    std::string wifiPassword;

    void loadCredentials();
    void saveWifiCredentials(const std::string& ssid, const std::string& password);
    void saveBluetoothSpeakerID(const std::string& speakerID);

};