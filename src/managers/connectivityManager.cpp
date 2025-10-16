#include "managers/connectivityManager.h"
#include <iostream>

connectivityManager::connectivityManager(WifiAdapter& wifiAdapter, BluetoothAdapter& btAdapter, storageManager& storage)
    : wifiAdapter(wifiAdapter), btAdapter(btAdapter), storage(storage) {
}

void connectivityManager::loadCredentials() {
    currentSSID = storage.getWifiSSID();
    wifiPassword = storage.getWifiPassword();
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
        } else {
            std::cout << "Failed to connect to WiFi: " << currentSSID << std::endl;
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
        return true;
    }
    return false;
}

void connectivityManager::disconnectWifi() {
    wifiAdapter.disconnect();
    currentSSID.clear();
    wifiPassword.clear();
}

bool connectivityManager::isWifiConnected() {
    return wifiAdapter.isConnected();
}

bool connectivityManager::connectBluetooth(const std::string& deviceAddress) {
    if (btAdapter.connectToDevice(deviceAddress)) {
        currentSpeakerID = deviceAddress;
        saveBluetoothSpeakerID(deviceAddress);
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