#include "managers/connectivityManager.h"
#include <iostream>

connectivityManager::connectivityManager(WifiAdapter& wifiAdapter, BluetoothAdapter& btAdapter, storageManager& storage, EventBus* eventBus)
    : wifiAdapter(wifiAdapter), btAdapter(btAdapter), storage(storage), m_eventBus(eventBus) {
        init();
        dockDetectPin = std::make_unique<GPIOPin>(27);
        dockDetectPin->pinModeIn(GPIOBias::PULL_UP);

        std::cout << "[connMgr] Pin27 dock detection initialized" << std::endl;

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

    if (wifiAdapter.isConnected()) {
        std::cout << "WiFi is already connected. Skipping configuration." << std::endl;
        
        if (m_eventBus) {
            WifiStatusChangedEvent event;
            event.isConnected = true;
            m_eventBus->publish(event);
        }
        return; // EXIT EARLY
    }

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


void connectivityManager::checkDockStatus() {
    if (!dockDetectPin) return;

    // READ: 0 = Connected (Grounded), 1 = Disconnected (Pulled Up)
    bool physicallyConnected = (dockDetectPin->pinRead() == 0);

    if (physicallyConnected != isDocked) {
        isDocked = physicallyConnected;
        
        if (isDocked) {
            std::cout << "[connMgr] Docking: Speaker CONNECTED (Handshake detected)" << std::endl;
            if (m_eventBus) {
                SpeakerDockedEvent event;
                m_eventBus->publish(event);
            }
        } else {
            std::cout << "[connMgr] Docking: Speaker DISCONNECTED" << std::endl;
            if (m_eventBus) {
                SpeakerUndockedEvent event;
                m_eventBus->publish(event);
            }
        }
    }
}