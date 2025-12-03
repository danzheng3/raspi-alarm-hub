#pragma once
#include <memory>
#include <atomic>
#include <thread>
#include <chrono>
#include <mutex>
#include "devices/MCP3021.h"
#include "events/EventBus.h"
#include "events/Events.h"

enum class PowerState {
    ACTIVE,
    DIMMED,
    SLEEP
};

class powerManager {
public:
    powerManager(std::shared_ptr<MCP3021> adc, EventBus* eventBus);
    ~powerManager();
    
    // User activity (resets idle timer)
    void registerActivity();
    
    // Power states
    PowerState getCurrentState() const { return currentState; }
    void wakeUp();
    void sleep();
    
    // Brightness control
    int getBrightness() const { return currentBrightness; }
    void setBrightness(int brightness);  // Manual override 0-100
    void enableAutoBrightness(bool enable);
    
    // Start/stop monitoring
    void startMonitoring();
    void stopMonitoring();

private:
    std::shared_ptr<MCP3021> lightSensor;
    EventBus* m_eventBus;
    
    PowerState currentState = PowerState::ACTIVE;
    
    std::atomic<bool> monitoringActive{false};
    std::thread monitorThread;
    
    // Activity tracking
    std::mutex stateMutex;
    std::chrono::steady_clock::time_point lastActivityTime;
    std::chrono::steady_clock::time_point lastLightCheckTime;
    const std::chrono::seconds dimTimeout{30};     // Dim after 30s
    const std::chrono::seconds sleepTimeout{300};  // Sleep after 5min
    
    // Brightness
    int currentBrightness = 100;
    int dimmedBrightness = 20;
    bool autoBrightnessEnabled = true;
    
    // Monitoring loop
    void monitorLoop();
    
    // Ambient light reading
    int readAmbientLight();
    int calculateBrightnessFromLight(int lightLevel);
    
    // State transitions
    void transitionTo(PowerState newState);
};