#include <string>
#include "storageManager.h"
#include "timeManager.h"
#include "events/Events.h"
#include "events/EventBus.h"
#include "devices/LED.h"
#include "devices/strobe.h"
#include "devices/pillbox.h"
#include "hardware_layer/GPIO.h"
#pragma once

struct AlarmConfig {
    bool soundEnabled = true;
    bool ledEnabled = true;
    bool strobeEnabled = false;
    bool pillboxEnabled = false;
    int ledDayOfWeek = 0;  // 0-6 for Monday-Sunday
};


class alarmManager {
    public:
        alarmManager(storageManager& storage, timeManager& timeMgr, EventBus* eventBus);
        ~alarmManager();

        void setAlarm(const std::string& time);
        std::string getAlarmTime() const { return alarmTime; }
        bool isAlarmEnabled() const;
        void setAlarmEnabled(bool enabled); // DONE VIA GPIO SWITCH

        void checkPhysicalControls();
        
        void loadFromStorage();
        void saveToStorage();

        //triggering
        bool shouldTrigger();
        void resetTrigger();

        //configuration
        void setAlarmConfig(const AlarmConfig& config);
        AlarmConfig getAlarmConfig() const { return alarmConfig; }

    private:
        timeManager& timeMgr;
        storageManager& storage;
        EventBus* m_eventBus;

        //alarm state

        bool alarmEnabled;
        bool alarmTriggered;
        std::string alarmTime;
        AlarmConfig alarmConfig;

        //hardware devices
        std::unique_ptr<LED> ledController;
        std::unique_ptr<strobe> strobeController;
        std::unique_ptr<pillbox> pillboxController;

        //button switch for alarm state
        std::unique_ptr<GPIOPin> resetButton; //16
        std::unique_ptr<GPIOPin> enableSwitch; //17
        bool lastResetState = true;
        
        // Actions
        void triggerAlarmActions();
        void clearAlarmActions();
        

        // event handler
        void onUIStopAlarmPressed(const UIStopAlarmPressedEvent& event);
};
