#include "managers/alarmManager.h"
#include <iostream>

alarmManager::alarmManager(storageManager& storage, timeManager& timeMgr, EventBus* eventBus) 
    : storage(storage), timeMgr(timeMgr), m_eventBus(eventBus), alarmEnabled(true), alarmTriggered(false) {
    
    ledController = std::make_unique<LED>();
    strobeController = std::make_unique<strobe>();
    pillboxController = std::make_unique<pillbox>();

    loadFromStorage();

    if (m_eventBus) {
        m_eventBus->subscribe<UIStopAlarmPressedEvent>(this, &alarmManager::onUIStopAlarmPressed);
    }

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
        alarmTriggered = true;
        std::cout << "Alarm triggered at " << currentTime << std::endl;

        triggerAlarmActions();

        if (alarmConfig.soundEnabled || alarmConfig.ledEnabled || alarmConfig.strobeEnabled || alarmConfig.pillboxEnabled) {
            if (m_eventBus) {
                AlarmTriggeredEvent event;
                m_eventBus->publish(event);
            }
        }
        return true;
    }
    return false;
}

void alarmManager::onUIStopAlarmPressed(const UIStopAlarmPressedEvent& event) {
    std::cout << "alarmManager: stop alarm button pressed" << std::endl;
    alarmTriggered = false;

    if (m_eventBus) {
        AlarmClearedEvent clearEvent;
        m_eventBus->publish(clearEvent);
    }
}

void alarmManager::triggerAlarmActions() {
    std::cout << "Triggering alarm actions:" << std::endl;

    if (alarmConfig.ledEnabled && ledController) {
        GPIOPin A0Pin(A0);
        GPIOPin A1Pin(A1);
        GPIOPin A2Pin(A2);
        
        if (ledController->setLED(A0Pin, A1Pin, A2Pin, alarmConfig.ledDayOfWeek)) {
            std::cout << "Alarm: LED activated (day " << alarmConfig.ledDayOfWeek << ")" << std::endl;
        }
    }

    if (alarmConfig.strobeEnabled && strobeController) {
        if (strobeController->strobeActivate()) {
            std::cout << "Alarm: Strobe activated" << std::endl;
        }
    }
    
    // Pillbox
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
        GPIOPin A0Pin(A0);
        GPIOPin A1Pin(A1);
        GPIOPin A2Pin(A2);
        ledController->turnOff(A0Pin, A1Pin, A2Pin);
    }
    
    // Turn off strobe
    if (strobeController) {
        strobeController->strobeToNormal();
    }
    
    // Close pillbox
    if (pillboxController) {
        pillboxController->closePillbox();
    }
}

void alarmManager::onUIStopAlarmPressed(const UIStopAlarmPressedEvent& event) {
    std::cout << "Stop alarm button pressed" << std::endl;
    
    alarmTriggered = false;
    clearAlarmActions();

    if (m_eventBus) {
        AlarmClearedEvent clearEvent;
        m_eventBus->publish(clearEvent);
    }
}


