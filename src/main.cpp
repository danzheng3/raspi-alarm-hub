#include <iostream>
#include "alarmManager.h"
#include "connectivityManager.h"
#include "timeManager.h"
#include "storageManager.h"
#include "powerManager.h"
#include "display_manager.h"
#include "audioManager.h"
#include "events/EventBus.h"
#include "weatherManager.h"
#include "equalizerManager.h"


#include <thread>
#include <chrono>
#include <atomic>


const std::string I2C_BUS_PATH = "/dev/i2c-1";



std::atomic<bool> running(true);

int main() {
    // if (gpioInitialise() < 0) {
    //     std::cerr << "Failed to initialize GPIO" << std::endl;
    //     return 1;
    // }
    std::cout << "eventBus Created" << std::endl;

    storageManager storage;
    storage.load();


    auto bus = std::make_shared<I2CBus>(I2C_BUS_PATH);
    std::cout << "I2C initialized " << std::endl;

    auto rtc = std::make_shared<MCP7940N>(bus);
    std::cout << "RTC initialized " << std::endl;

    auto adc = std::make_shared<MCP3021>(bus); 
    std::cout << "ADC initialized " << std::endl;


    EventBus eventBus;

    timeManager timeMgr(storage, rtc, &eventBus);
    std::cout << "TimeMgr: " << timeMgr.getFormattedTime() << std::endl;


    WifiAdapter wifiAdapter;
    BluetoothAdapter btAdapter;
    std::cout << "btMgr initialized" << std::endl;
    connectivityManager connMgr(wifiAdapter, btAdapter, storage, &eventBus);
    std::cout << "connMgr initialized" << std::endl;
    std::cout << "WiFi: " << (connMgr.isWifiConnected() ? "Connected" : "Disconnected") << std::endl;

    if (connMgr.isWifiConnected()) {
        std::cout << "WiFi Connected: " << (connMgr.isWifiConnected() ? "Yes" : "No") << std::endl;
        std::cout << "Attempting NTP Sync..." << std::endl;
        timeMgr.trySyncFromNTP();
    }

    alarmManager alarmMgr(storage, timeMgr, connMgr, &eventBus);
    if (alarmMgr.isAlarmEnabled()) {
        std::cout << "Alarm: " << alarmMgr.getAlarmTime() << std::endl;
    } else {
        std::cout << "Alarm: Not set" << std::endl;
    }

    audioManager audioMgr(&connMgr, &eventBus);
    std::cout << "Audio: " << audioMgr.getSongList().size() << " songs" << std::endl;

    powerManager pwrMgr(adc, &eventBus);
    pwrMgr.enableAutoBrightness(true);
    pwrMgr.startMonitoring();
    std::cout << "Power management initialized" << std::endl;

    weatherManager weatherMgr(storage, &eventBus);
    std::cout << "Weather initialized" << std::endl;
    weatherMgr.startAutoUpdate(30);

    std::thread([&weatherMgr]() {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        weatherMgr.fetchWeather(40.4237, -86.9212);
    }).detach();

    equalizerManager eqMgr(bus, storage, &eventBus);
    DisplayManager displayMgr(&timeMgr, &alarmMgr, &connMgr, &pwrMgr, &audioMgr, &weatherMgr, &eqMgr, &eventBus);

    std::cout << "DisplayManager initialized" << std::endl;

   

    std::cout << "========================================" << std::endl;
    std::cout << "System ready!" << std::endl;
    std::cout << "========================================\n" << std::endl;

    std::cout << "\n[TEST] --- STARTING HARDWARE CHECK ---" << std::endl;

    alarmMgr.debugStrobe(true);
    alarmMgr.debugLEDs(true);

    bool strobeStatus = alarmMgr.readStrobeState();
    std::cout << "[TEST] Strobe (GPIO 25) Raw Status: " << strobeStatus;
    
    if (strobeStatus == 1) std::cout << " (HIGH - SUCCESS)" << std::endl;
    else if (strobeStatus == 0) std::cout << " (LOW - FAIL)" << std::endl;
    else if (strobeStatus == -1) std::cout << " (ERROR - GPIOD READ FAILED)" << std::endl;
    else std::cout << " (UNKNOWN ERROR)" << std::endl;

    std::cout << "\n[TEST] Reading Input States:" << std::endl;

    // Reset Button (GPIO 16) - Pull Up (Default HIGH)
    bool resetState = alarmMgr.getResetButtonState();
    std::cout << "  -> Reset Button (GPIO 16): " << resetState 
              << (resetState ? " (HIGH/RELEASED)" : " (LOW/PRESSED)") << std::endl;

    // Enable Switch (GPIO 17) - Pull Up (Default HIGH)
    bool enableState = alarmMgr.getEnableSwitchState();
    std::cout << "  -> Enable Switch (GPIO 17): " << enableState
              << (enableState ? " (HIGH/OFF)" : " (LOW/ON)") << std::endl;

    // Dock Detect (GPIO 27) - Pull Up (Default HIGH)
    bool dockState = connMgr.getDockSwitchState();
    std::cout << "  -> Dock Detect (GPIO 27):  " << dockState
              << (dockState ? " (HIGH/UNDOCKED)" : " (LOW/DOCKED)") << std::endl;

    std::cout << "[TEST] --- HARDWARE CHECK COMPLETE ---\n\n" << std::endl;
    // ==========================================

    



    std::thread logicThread([&]() {
        while (running) {
            timeMgr.checkAndPublishTimeUpdate();

            // -- hardware check --
            connMgr.checkDockStatus();
            //alarmMgr.checkPhysicalControls(); //UNCOMMENTED BC OF PHYSICAL RN
            
            // Check if alarm should trigger
            if (alarmMgr.shouldTrigger()) {
                std::cout << "\n🔔 ALARM! 🔔\n" << std::endl;
            }

            if (pwrMgr.getCurrentState() != PowerState::ACTIVE && 
                audioMgr.getState() == audioManager::AudioState::STOPPED) {
                
                if (connMgr.isBluetoothConnected()) {
                    std::cout << "[System] Idle & Silent -> Disconnecting Bluetooth" << std::endl;
                    connMgr.disconnectBluetooth();
                }

            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    });

    displayMgr.run(running);
    running = false;
    weatherMgr.stopAutoUpdate();
    
    if (logicThread.joinable()) {
        logicThread.join();
    }

    #ifndef TEST_MODE
    // gpioTerminate();
    #endif
    std::cout << "Goodbye!" << std::endl;
    return 0;
}