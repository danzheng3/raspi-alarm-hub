#include "managers/alarmManager.h"
#include "hardware_layer/GPIO.h"
#include <iostream>

alarmManager::alarmManager(storageManager& storage, timeManager& timeMgr, connectivityManager& connMgr, EventBus* eventBus) 
    : storage(storage), timeMgr(timeMgr), connMgr(connMgr), m_eventBus(eventBus), alarmEnabled(true), alarmTriggered(false) {
    
    ledController = std::make_unique<LED>();
    strobeController = std::make_unique<strobe>();
    pillboxController = std::make_unique<pillbox>();

    loadFromStorage();

    if (m_eventBus) {
        m_eventBus->subscribe<UIStopAlarmPressedEvent>(this, &alarmManager::onUIStopAlarmPressed);
        m_eventBus->subscribe<SpeakerDockedEvent>(this, &alarmManager::onSpeakerDocked);
    }

    // GPIO 16: Reset Button (Push button, normally open)
    resetButton = std::make_unique<GPIOPin>(16);
    resetButton->pinModeIn(GPIOBias::PULL_UP); // Pressed = LOW
    
    // on = low (assume switch connects to ground?)
    enableSwitch = std::make_unique<GPIOPin>(17);
    enableSwitch->pinModeIn(GPIOBias::PULL_UP);


    std::cout << "alarmManager initialized" << std::endl;
    std::cout << "  Alarm time: " << alarmTime << std::endl;
    std::cout << "  Actions - Sound:" << alarmConfig.soundEnabled 
              << " LED:" << alarmConfig.ledEnabled 
              << " Strobe:" << alarmConfig.strobeEnabled 
              << " Pillbox:" << alarmConfig.pillboxEnabled << std::endl;

}


alarmManager::~alarmManager() { clearAlarmActions(); }

void alarmManager::loadFromStorage() {
    alarmTime = storage.getAlarmTime();
    alarmEnabled = true;
    
    try {
        std::string configStr = storage.get("alarm_actions");
        
        if (configStr.empty()) {
            // Use default
            alarmConfig.soundEnabled = true;
            alarmConfig.ledEnabled = true;
            alarmConfig.strobeEnabled = false;
            alarmConfig.pillboxEnabled = false;
            alarmConfig.ledDayOfWeek = 0;
            std::cout << "Using default alarm action config" << std::endl;
        } else {
            auto json = nlohmann::json::parse(configStr);
            alarmConfig.soundEnabled = json.value("sound", true);
            alarmConfig.ledEnabled = json.value("led", true);
            alarmConfig.strobeEnabled = json.value("strobe", false);
            alarmConfig.pillboxEnabled = json.value("pillbox", false);
            alarmConfig.ledDayOfWeek = json.value("led_day", 0);
            alarmConfig.alarmAudioPath = json.value("alarm_audio_path", "default");
        }
    } catch (const std::exception& e) {
        std::cerr << "Error loading alarm config: " << e.what() << std::endl;
        alarmConfig = AlarmConfig{}; // Use defaults
    }

}

void alarmManager::saveToStorage() {
    storage.setAlarmTime(alarmTime);
    try {
        nlohmann::json json;
        json["sound"] = alarmConfig.soundEnabled;
        json["led"] = alarmConfig.ledEnabled;
        json["strobe"] = alarmConfig.strobeEnabled;
        json["pillbox"] = alarmConfig.pillboxEnabled;
        json["led_day"] = alarmConfig.ledDayOfWeek;
        json["alarm_audio_path"] = alarmConfig.alarmAudioPath;
        std::cout << "saving alarm config: " << json.dump() << std::endl;
        
        storage.set("alarm_actions", json.dump());
    } catch (const std::exception& e) {
        std::cerr << "Error saving alarm config: " << e.what() << std::endl;
    }
    
    storage.save();
    std::cout << "Alarm configuration saved" << std::endl;
}

void alarmManager::setAlarmConfig(const AlarmConfig& config) {
    alarmConfig = config;
    saveToStorage();
    std::cout << "Alarm actions updated" << std::endl;
}

void alarmManager::setAlarm(const std::string& time) {
    alarmTime = time;
    alarmEnabled = true;
    alarmTriggered = false;
    std::cout << "set alarm " << alarmTime << std::endl;

    saveToStorage();

    // publish event
    if (m_eventBus) {
        AlarmSetEvent event;
        event.newTime = time;
        m_eventBus->publish(event);
    }
}

bool alarmManager::isAlarmEnabled() const {
    return alarmEnabled;
}

void alarmManager::resetTrigger() {
    alarmTriggered = false;
}

bool alarmManager::shouldTrigger() {
    if (!alarmEnabled || alarmTriggered) {
        return false;
    }

    std::string currentTime = timeMgr.getFormattedTime();
    if (currentTime == alarmTime) {
        std::cout << "Alarm triggered at " << currentTime << std::endl;

        alarmTriggered = true;

        std::thread([this]() {
            triggerAlarmActions();
        }).detach();

        if (alarmConfig.soundEnabled || alarmConfig.ledEnabled || alarmConfig.strobeEnabled || alarmConfig.pillboxEnabled) {
            if (m_eventBus) {
                AlarmTriggeredEvent event;
                event.playAudio = alarmConfig.soundEnabled;
                if (alarmConfig.soundEnabled) {
                    event.audioPath = alarmConfig.alarmAudioPath;
                }
                m_eventBus->publish(event);
            }
        }
        return true;
    }
    return false;
}


void alarmManager::triggerAlarmActions() {
    std::cout << "Triggering alarm actions:" << std::endl;
    
    if (alarmConfig.ledEnabled && ledController) {
        
        if (ledController->setLED(alarmConfig.ledDayOfWeek)) {
            std::cout << "Alarm: LED activated (day " << alarmConfig.ledDayOfWeek << ")" << std::endl;
        }
    }

    if (alarmConfig.strobeEnabled && strobeController) {
        if (strobeController->strobeActivate()) {
            std::cout << "Alarm: Strobe activated" << std::endl;
        }
    }
    
    
    if (alarmConfig.pillboxEnabled && pillboxController) {
        if (pillboxController->openPillbox()) {
            std::cout << "Alarm: Pillbox open" << std::endl;
        }
    }
}

void alarmManager::clearAlarmActions() {
    std::cout << "Clearing alarm hardware actions..." << std::endl;
    
    // Turn off LED
    if (ledController) {
        ledController->turnOff();
    }
    
    // Turn off strobe
    if (strobeController) {
        strobeController->strobeToNormal();
    }
    
    // Close pillbox
    
    if (pillboxController) {
        std::thread([this]() {
            pillboxController->closePillbox();
        }).detach();
    } 
}

void alarmManager::checkPhysicalControls() {
    bool currentResetState = resetButton->pinRead();

    if (lastResetState == true && currentResetState == false) {
        std::cout << "[Hardware] Alarm Reset Button Pressed" << std::endl;

        if (connMgr.isBluetoothConnected()) {
            std::cout << "[AlarmMgr] button ignored (speaker connected)" << std::endl;
        } else {
            std::cout << "[AlarmMgr] processing alarm reset" << std::endl;
            if (alarmTriggered) {
                UIStopAlarmPressedEvent e; 
                onUIStopAlarmPressed(e);
            }
        }
    
    }
    lastResetState = currentResetState;

    //switch control
    bool switchState = enableSwitch->pinRead();
    bool shouldBeEnabled = (switchState == 0); 
    
    if (alarmEnabled != shouldBeEnabled) {
        alarmEnabled = shouldBeEnabled;
        std::cout << "[Hardware] Alarm Switch changed: " 
                  << (alarmEnabled ? "ENABLED" : "DISABLED") << std::endl;
                  
        // [LATER] Publish event if UI needs to update switch icon
    }
}

// EVENT HANDLERS

void alarmManager::onUIStopAlarmPressed(const UIStopAlarmPressedEvent& event) {
    std::cout << "Stop alarm button pressed" << std::endl;
    
    alarmTriggered = false;
    clearAlarmActions();

    if (m_eventBus) {
        AlarmClearedEvent clearEvent;
        m_eventBus->publish(clearEvent);
    }
}

void alarmManager::setAlarmEnabled(bool enabled) {
    if (alarmEnabled != enabled) {
        alarmEnabled = enabled;
        std::cout << "Alarm manually " << (enabled ? "ENABLED" : "DISABLED") << " via switch." << std::endl;
        
        // If disabled, ensure any currently ringing alarm is cleared
        if (!enabled && alarmTriggered) {
            onUIStopAlarmPressed(UIStopAlarmPressedEvent{});
        }
    }
}

void alarmManager::onSpeakerDocked(const SpeakerDockedEvent& event) {
    if (alarmTriggered) {
        std::cout << "[Alarm] Speaker Docked -> Stopping Alarm." << std::endl;
        // Trigger the stop sequence
        UIStopAlarmPressedEvent e; 
        onUIStopAlarmPressed(e);
    }
}

void alarmManager::simulateHardwareReset() {
    std::cout << "[Simulation] Reset Button Signal Received via Software" << std::endl;
    
    // Reuse the existing internal logic for stopping the alarm
    UIStopAlarmPressedEvent e; 
    onUIStopAlarmPressed(e);
}
