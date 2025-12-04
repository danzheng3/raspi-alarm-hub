#include "managers/powerManager.h"
#include <iostream>
#include <algorithm>

powerManager::powerManager(std::shared_ptr<MCP3021> adc, EventBus* eventBus)
    : lightSensor(adc), m_eventBus(eventBus) {
    
    lastActivityTime = std::chrono::steady_clock::now();
    currentState = PowerState::ACTIVE;
    
    std::cout << "PowerManager initialized" << std::endl;
}

powerManager::~powerManager() {
    stopMonitoring();
}

void powerManager::registerActivity() {
    std::lock_guard<std::mutex> lock(stateMutex);   
    lastActivityTime = std::chrono::steady_clock::now();
    
    // Wake up if dimmed or sleeping
    if (currentState != PowerState::ACTIVE) {
        wakeUp();
    }
}

void powerManager::wakeUp() {
    transitionTo(PowerState::ACTIVE);
}

void powerManager::sleep() {
    transitionTo(PowerState::SLEEP);
}

void powerManager::transitionTo(PowerState newState) {
    if (currentState == newState) return;
    
    PowerState oldState = currentState;
    currentState = newState;
    
    switch (newState) {
        case PowerState::ACTIVE:
            std::cout << "Power: ACTIVE" << std::endl;
            if (autoBrightnessEnabled) {
                int ambientLight = readAmbientLight();
                currentBrightness = calculateBrightnessFromLight(ambientLight);
            } else {
                currentBrightness = 100;
            }
            break;
            
        case PowerState::DIMMED:
            std::cout << "Power: DIMMED" << std::endl;
            currentBrightness = dimmedBrightness;
            break;
            
        case PowerState::SLEEP:
            std::cout << "Power: SLEEP" << std::endl;
            currentBrightness = 0;
            break;
    }
    
    // Publish brightness change event
    if (m_eventBus) {
        ScreenBrightnessChanged event;
        event.brightness = currentBrightness;
        m_eventBus->publish(event);
    }
}

void powerManager::setBrightness(int brightness) {
    brightness = std::clamp(brightness, 0, 100);
    currentBrightness = brightness;
    autoBrightnessEnabled = false;  // Manual override disables auto
    
    std::cout << "Brightness set to: " << brightness << "%" << std::endl;
    
    if (m_eventBus) {
        ScreenBrightnessChanged event;
        event.brightness = currentBrightness;
        m_eventBus->publish(event);
    }
}

void powerManager::enableAutoBrightness(bool enable) {
    autoBrightnessEnabled = enable;
    std::cout << "Auto-brightness: " << (enable ? "ON" : "OFF") << std::endl;
    
    if (enable && currentState == PowerState::ACTIVE) {
        int ambientLight = readAmbientLight();
        currentBrightness = calculateBrightnessFromLight(ambientLight);
        
        if (m_eventBus) {
            ScreenBrightnessChanged event;
            event.brightness = currentBrightness;
            m_eventBus->publish(event);
        }
    }
}

int powerManager::readAmbientLight() {
    if (!lightSensor) return 512;  // Default mid-range
    
    uint16_t rawValue;
    if (lightSensor->readValue(rawValue)) {
        return rawValue;  // 0-1023 (10-bit ADC)
    }
    
    return 512;  // Default if read fails
}

int powerManager::calculateBrightnessFromLight(int lightLevel) {
    // Map ADC value (0-1023) to brightness (20-100%)
    // Low light = lower brightness, bright light = higher brightness
    
    // Logarithmic curve for more natural feel
    // Dark room (0-200): 20-40% brightness
    // Normal room (200-600): 40-70% brightness  
    // Bright room (600-1023): 70-100% brightness
    
    int brightness;
    
    if (lightLevel < 200) {
        // Dark: 20-40%
        brightness = 20 + (lightLevel * 20) / 200;
    } else if (lightLevel < 600) {
        // Normal: 40-70%
        brightness = 40 + ((lightLevel - 200) * 30) / 400;
    } else {
        // Bright: 70-100%
        brightness = 70 + ((lightLevel - 600) * 30) / 423;
    }
    
    return std::clamp(brightness, 20, 100);
}

void powerManager::startMonitoring() {
    if (monitoringActive) return;
    
    monitoringActive = true;
    monitorThread = std::thread(&powerManager::monitorLoop, this);
    
    std::cout << "Power monitoring started" << std::endl;
}

void powerManager::stopMonitoring() {
    monitoringActive = false;
    if (monitorThread.joinable()) {
        monitorThread.join();
    }
    
    std::cout << "Power monitoring stopped" << std::endl;
}

void powerManager::monitorLoop() {
    while (monitoringActive) {
        auto now = std::chrono::steady_clock::now();
        PowerState state;
        std::chrono::steady_clock::time_point lastActivity;
        std::chrono::steady_clock::time_point lastLight;
        {
            std::lock_guard<std::mutex> lock(stateMutex);
            state = currentState;
            lastActivity = lastActivityTime;
            lastLight = lastLightCheckTime;
        }


        auto idleTime = std::chrono::duration_cast<std::chrono::seconds>(now - lastActivityTime);
        
        // Check for state transitions based on idle time
        if (state == PowerState::ACTIVE && idleTime >= dimTimeout) {
            transitionTo(PowerState::DIMMED);
        } else if (currentState == PowerState::DIMMED && idleTime >= sleepTimeout) {
            transitionTo(PowerState::SLEEP);
        }
        
        // Update brightness based on ambient light (if auto-brightness enabled and active)
        if (autoBrightnessEnabled && currentState == PowerState::ACTIVE) {
            auto timeSinceCheck = std::chrono::duration_cast<std::chrono::seconds>(now - lastLightCheckTime);
            
            if (timeSinceCheck >= std::chrono::seconds(5)) {  // Check every 5 seconds
                stateMutex.unlock();
                int ambientLight = readAmbientLight();
                stateMutex.lock();
                int newBrightness = calculateBrightnessFromLight(ambientLight);
                
                // Only update if changed significantly (>5%)
                if (abs(newBrightness - currentBrightness) > 5) {
                    currentBrightness = newBrightness;
                    
                    if (m_eventBus) {
                        ScreenBrightnessChanged event;
                        event.brightness = currentBrightness;
                        m_eventBus->publish(event);
                    }
                }
                
                lastLightCheckTime = now;
            }
        }
        std::lock_guard<std::mutex> lock(stateMutex);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}