#include <iostream>
#include "alarmManager.h"
#include "connectivityManager.h"
#include "timeManager.h"
#include "storageManager.h"
#include "display_manager.h"

#include <thread>
#include <chrono>
#include <atomic>



std::atomic<bool> running(true);

int main() {
    storageManager storage;
    storage.load();

    timeManager timeMgr(storage);
    
    alarmManager alarmMgr(storage, timeMgr);
    WifiAdapter wifiAdapter;
    BluetoothAdapter btAdapter;
    connectivityManager connMgr(wifiAdapter, btAdapter, storage);
    connMgr.init();
    std::cout << "Number of touch devices: " << SDL_GetNumTouchDevices() << std::endl;
    DisplayManager displayMgr(&timeMgr, &alarmMgr, &connMgr);

    std::cout << "=== Initialization Complete ===" << std::endl;
    std::cout << "Initial time: " << timeMgr.getCurrentTime() << std::endl;
    std::cout << "Alarm enabled: " << (alarmMgr.isAlarmEnabled() ? "Yes" : "No") << std::endl;
    std::cout << "WiFi connected: " << (connMgr.isWifiConnected() ? "Yes" : "No") << std::endl;
    std::cout << "===============================" << std::endl;

    std::thread logicThread([&]() {
        while (running) {
            std::string currentTime = timeMgr.getCurrentTime();
            if (alarmMgr.isAlarmEnabled() && alarmMgr.shouldTrigger()) {
                std::cout << "Alarm Triggered at " << timeMgr.getCurrentTime() << "!" << std::endl;
                alarmMgr.resetTrigger();
            }
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    });

    displayMgr.run(running);
    running = false;
    logicThread.join();
    std::cout << "Shutting down..." << std::endl;


    return 0;
}