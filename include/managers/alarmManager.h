#include <string>
#include "storageManager.h"
#include "timeManager.h"
#include "events/Events.h"
#include "events/EventBus.h"
#include "devices/LED.h"
#include "devices/strobe.h"
#include "devices/pillbox.h"
#include "hardware_layer/GPIO.h"
#include "connectivityManager.h"
#pragma once

struct AlarmConfig {
    bool soundEnabled = true;
    bool ledEnabled = true;
    bool strobeEnabled = false;
    bool pillboxEnabled = false;
    int ledDayOfWeek = 0;  // 1-7 for Monday-Sunday
    std::string alarmAudioPath = "default";
};


class alarmManager {
    public:
        alarmManager(storageManager& storage, timeManager& timeMgr, connectivityManager& connMgr, EventBus* eventBus);
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
        int lastCheckedLEDDay =0;

        void simulateHardwareReset(); 
        //configuration
        void setAlarmConfig(const AlarmConfig& config);
        AlarmConfig getAlarmConfig() const { return alarmConfig; }

        //DEBUGGING
        void setLEDsToCurrentDay(); // <--- NEW INTERNAL METHOD

        void debugStrobe(bool state);
        int readStrobeState();

        void debugLEDs(bool state); // Controls GPIO 22, 23, 24
        bool getResetButtonState(); // Reads GPIO 16
        bool getEnableSwitchState(); // Reads GPIO 17

    private:
        timeManager& timeMgr;
        storageManager& storage;
        connectivityManager& connMgr;
        EventBus* m_eventBus;

        //alarm state

        bool alarmEnabled;
        bool alarmTriggered;
        bool alarmHandled;
        std::string alarmTime;
        AlarmConfig alarmConfig;

        //hardware devices
        std::unique_ptr<LED> ledController;
        std::unique_ptr<strobe> strobeController;

        //button switch for alarm state
        std::unique_ptr<GPIOPin> resetButton; //16
        std::unique_ptr<GPIOPin> enableSwitch; //17
        bool lastResetState = true;
        
        // Actions
        void triggerAlarmActions();
        void clearAlarmActions();
        

        // event handler
        void onUIStopAlarmPressed(const UIStopAlarmPressedEvent& event);
        void onSpeakerDocked(const SpeakerDockedEvent& event);
};
