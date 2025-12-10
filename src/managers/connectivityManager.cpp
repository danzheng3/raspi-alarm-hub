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
    currentSpeakerID = storage.get("bluetooth_id");
    std::cout << "  BT Speaker: " << (currentSpeakerID.empty() ? "None" : currentSpeakerID) << std::endl;

}

void connectivityManager::saveWifiCredentials(const std::string& ssid, const std::string& password) {
    storage.setWifiCredentials(ssid, password);
    storage.save();
}

void connectivityManager::saveBluetoothSpeakerID(const std::string& speakerID) {
    storage.set("bluetooth_id", speakerID);
    storage.save();
}

void connectivityManager::init() {
    loadCredentials();

    btAdapter.initialize();

    if (wifiAdapter.isConnected()) {
        std::cout << "WiFi is already connected. Skipping configuration." << std::endl;
        
        if (m_eventBus) {
            WifiStatusChangedEvent event;
            event.isConnected = true;
            m_eventBus->publish(event);
        }

    } else if (!currentSSID.empty() && !wifiPassword.empty()) {
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

    if (!currentSpeakerID.empty()) {
        
        // --- NEW LOGIC START ---
        // Check if the sink already exists in PulseAudio
        std::string macForPulse = currentSpeakerID;
        std::replace(macForPulse.begin(), macForPulse.end(), ':', '_'); // Convert XX:XX to XX_XX
        
        std::string checkCmd = "sudo -u daniel XDG_RUNTIME_DIR=/run/user/$(id -u daniel) pactl list short sinks | grep blue";        
        bool alreadyConnected = false;
        FILE* pipe = popen(checkCmd.c_str(), "r");
        if (pipe) {
            char buffer[128];
            if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                alreadyConnected = true;
            }
            pclose(pipe);
        }

        if (alreadyConnected) {
            std::cout << "[ConnMgr] Bluetooth Speaker already connected (PulseAudio sink found)." << std::endl;
            // Optionally publish the event so the UI knows
            if (m_eventBus) {
                BluetoothSpeakerConnectedEvent event;
                event.deviceName = currentSpeakerID; // or the Pulse name
                m_eventBus->publish(event);
            }
        } 
        else {
            // Not found, proceed with connection attempt
            std::cout << "[ConnMgr] Found saved Speaker ID. Connecting..." << std::endl;
            
            std::thread([this]() {
                std::this_thread::sleep_for(std::chrono::seconds(2));
                if (connectBluetooth(currentSpeakerID)) {
                    std::cout << "[ConnMgr] Auto-connected to Bluetooth Speaker." << std::endl;
                } else {
                    std::cerr << "[ConnMgr] Failed to auto-connect to Bluetooth Speaker." << std::endl;
                }
            }).detach();
        }
        // --- NEW LOGIC END ---
    }

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