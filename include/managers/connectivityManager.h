#include <string>
#include <vector>
#include <thread>
#include <algorithm>
#pragma once
#include "hardware_layer/BluetoothAdapter.h"
#include "hardware_layer/WifiAdapter.h"
#include "storageManager.h"
#include "events/EventBus.h"
#include "events/Events.h"
#include "hardware_layer/GPIO.h"

class connectivityManager {
public:
    connectivityManager(WifiAdapter& wifiAdapter, BluetoothAdapter& btAdapter, storageManager& storage, EventBus* eventBus);
    ~connectivityManager();

    void init();

    bool connectToWifi(const std::string& ssid, const std::string& password);
    void disconnectWifi();
    bool connectBluetooth(const std::string& deviceAddress);
    void disconnectBluetooth();

    bool isWifiConnected();
    bool isBluetoothConnected();
    void checkDockStatus();
    void checkConnectionStatus();
    void sendBleAlert(const std::string& handle, const std::string& message) {

        btAdapter.sendGattMessage(handle, message);
    }

private:
    WifiAdapter& wifiAdapter;
    BluetoothAdapter& btAdapter;
    storageManager& storage;
    EventBus* m_eventBus;

    std::string currentSSID;
    std::string currentSpeakerID;
    std::string wifiPassword;

    void loadCredentials();
    void saveWifiCredentials(const std::string& ssid, const std::string& password);
    void saveBluetoothSpeakerID(const std::string& speakerID);

    std::unique_ptr<GPIOPin> dockDetectPin;
    bool isDocked=false;

};