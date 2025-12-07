#include "managers/powerManager.h"
#include <iostream>
#include <algorithm>

powerManager::powerManager(std::shared_ptr<MCP3021> adc, EventBus* eventBus)
    : lightSensor(adc), m_eventBus(eventBus) {
    
    lastActivityTime = std::chrono::steady_clock::now();
    currentState = PowerState::ACTIVE;

    if (lightSensor) {
        lightSensor->testConnection();
    }
    fadeActive = true;
    fadeThread = std::thread(&powerManager::fadeLoop, this);
    
    std::cout << "PowerManager initialized" << std::endl;
}

powerManager::~powerManager() {
    stopMonitoring();
    fadeActive = false;
    fadeCv.notify_all();
    if (fadeThread.joinable()) {
        fadeThread.join();
    }
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
            
    }

    setTargetBrightness(currentBrightness);
    
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
    setTargetBrightness(currentBrightness);
    
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
    // observed LDR values:
    // Dark: 0-2   -> Map to 20-40% brightness
    // Dim: 3-10   -> Map to 40-70% brightness
    // Light: 10-20 -> Map to 70-100% brightness
    
    int brightness;
    
    if (lightLevel <= 2) {
        // Dark room (0-2): Map linearly to 20-40%
        // lightLevel=0 -> 20%, lightLevel=2 -> 40%
        brightness = 20 + (lightLevel * 20) / 2;
    } 
    else if (lightLevel <= 10) {
        // Dim room (3-10): Map linearly to 40-70%
        // lightLevel=3 -> 40%, lightLevel=10 -> 70%
        brightness = 40 + ((lightLevel - 3) * 30) / (10 - 3);
    } 
    else {
        // Normal room lights (11-20): Map linearly to 70-100%
        // lightLevel=11 -> 70%, lightLevel=20 -> 100%
        brightness = 70 + ((lightLevel - 11) * 30) / (20 - 11);
    }
    
    // Clamp between 20 (min usable dim) and 100 (full bright)
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
        auto timeSinceCheck = std::chrono::duration_cast<std::chrono::seconds>(now - lastLight);      
        // Check for state transitions based on idle time

        if (state == PowerState::ACTIVE && idleTime >= dimTimeout) {
            transitionTo(PowerState::DIMMED);
        }
        
        // Update brightness based on ambient light (if auto-brightness enabled and active)
        if (autoBrightnessEnabled && currentState == PowerState::ACTIVE) {
            
            if (timeSinceCheck >= std::chrono::seconds(5)) {  // Check every 5 seconds
                int ambientLight = readAmbientLight();
                std::cout << "[PwrMgr] Ambient light: " << ambientLight << std::endl;

                std::lock_guard<std::mutex> lock(stateMutex);
                lastLightCheckTime = std::chrono::steady_clock::now();
                int newBrightness = calculateBrightnessFromLight(ambientLight);
                
                // Only update if changed significantly (>5%)
                if (abs(newBrightness - currentBrightness) > 5) {
                    currentBrightness = newBrightness;
                    std::cout << "[PwrMgr] Light Change. Changing brightness" << std::endl;

                    // CHANGE BRIGHTNESS FX HERE?
                    setTargetBrightness(currentBrightness);
                    
                    if (m_eventBus) {
                        ScreenBrightnessChanged event;
                        event.brightness = currentBrightness;
                        m_eventBus->publish(event);
                    }
                }
                
                lastLightCheckTime = now;
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}


void powerManager::fadeLoop() {
    while (fadeActive) {
        std::unique_lock<std::mutex> lock(fadeMutex);
        
        // Wait until we need to change brightness
        fadeCv.wait(lock, [this] {
            return (currentHardwareBrightness != targetHardwareBrightness) || !fadeActive;
        });

        if (!fadeActive) break;

        // Unlock to perform I/O
        lock.unlock();

        int target = targetHardwareBrightness;
        int current = currentHardwareBrightness;

        // Simple Step Logic: Always move by 1
        if (current < target) {
            current++;
        } else if (current > target) {
            current--;
        }

        // Write to Hardware File
        std::ofstream file(BACKLIGHT_PATH);
        if (file.is_open()) {
            file << current;
            currentHardwareBrightness = current;
        } else {
            // Optional: Print error once if file fails (to avoid spamming logs)
            std::cerr << "Failed to write to backlight file" << std::endl;
        }

        // Sleep 40ms (Constant Rate)
        // 31 steps * 40ms = ~1.2 seconds for full fade
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
    }
}

void powerManager::setTargetBrightness(int percent) {
    // 1. Clamp input 0-100
    percent = std::clamp(percent, 0, 100);

    // 2. Map 0-100% to 0-31 scale
    int hwTarget = (percent * MAX_HW_BRIGHTNESS) / 100;

    // 3. Notify the fade loop
    {
        std::lock_guard<std::mutex> lock(fadeMutex);
        targetHardwareBrightness = hwTarget;
    }
    fadeCv.notify_one(); 
}