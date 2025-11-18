#include "managers/connectivityManager.h"
#include <iostream>

connectivityManager::connectivityManager(WifiAdapter& wifiAdapter, BluetoothAdapter& btAdapter, storageManager& storage, EventBus* eventBus)
    : wifiAdapter(wifiAdapter), btAdapter(btAdapter), storage(storage), m_eventBus(eventBus) {
        init();
}

void connectivityManager::loadCredentials() {
    currentSSID = storage.getWifiSSID();
    wifiPassword = storage.getWifiPassword();
    std::cout << "loaded wifi ssid, pswd" << std::endl;
    //currentSpeakerID = storage.get("bluetooth_speaker_id");
}

void connectivityManager::saveWifiCredentials(const std::string& ssid, const std::string& password) {
    storage.setWifiCredentials(ssid, password);
    storage.save();
}

void connectivityManager::saveBluetoothSpeakerID(const std::string& speakerID) {
    //storage.set("bluetooth_speaker_id", speakerID);
    //storage.save();
}

void connectivityManager::init() {
    loadCredentials();

    if (!currentSSID.empty() && !wifiPassword.empty()) {
        if (wifiAdapter.connect(currentSSID, wifiPassword)) {
            std::cout << "Connected to WiFi: " << currentSSID << std::endl;

            if (m_eventBus) {
                WifiStatusChangedEvent event;
                event.isConnected = true;
                m_eventBus->publish(event);
            }
        } else {
            std::cout << "Failed to connect to WiFi: " << currentSSID << std::endl;

            if (m_eventBus) {
                WifiStatusChangedEvent event;
                event.isConnected = false;
                m_eventBus->publish(event);
            }
        }
    }

    // add bluetooth fx here

}

connectivityManager::~connectivityManager() {
    disconnectWifi();
    disconnectBluetooth();
}

bool connectivityManager::connectToWifi(const std::string& ssid, const std::string& password) {
    if (wifiAdapter.connect(ssid, password)) {
        currentSSID = ssid;
        wifiPassword = password;
        saveWifiCredentials(ssid, password);

        if (m_eventBus) {
            WifiStatusChangedEvent event;
            event.isConnected = true;
            m_eventBus->publish(event);
        }
        return true;
    }

    if (m_eventBus) {
        WifiStatusChangedEvent event;
        event.isConnected = false;
        m_eventBus->publish(event);
    }
    return false;
}

void connectivityManager::disconnectWifi() {
    wifiAdapter.disconnect();
    currentSSID.clear();
    wifiPassword.clear();

    if (m_eventBus) {
        WifiStatusChangedEvent event;
        event.isConnected = false;
        m_eventBus->publish(event);
    }
}

bool connectivityManager::isWifiConnected() {
    return wifiAdapter.isConnected();
}

bool connectivityManager::connectBluetooth(const std::string& deviceAddress) {
    if (btAdapter.connectToDevice(deviceAddress)) {
        currentSpeakerID = deviceAddress;
        saveBluetoothSpeakerID(deviceAddress);

        if (m_eventBus) {
            BluetoothSpeakerConnectedEvent event;
            event.deviceName = deviceAddress;
            m_eventBus->publish(event);
        }
        return true;
    }
    return false;
}

void connectivityManager::disconnectBluetooth() {
    btAdapter.disconnect();
    currentSpeakerID.clear();
}

bool connectivityManager::isBluetoothConnected() {
    return btAdapter.isConnected();
}