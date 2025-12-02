#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <string>
#include <iostream>
#include "utils/page.h"
#include <atomic>
#include <mutex>
#include "events/EventBus.h"
#include "events/Events.h"


enum class PageType {
    MAIN,
    SETTINGS,
    ALARMS,
    TIMESET
};

class DisplayManager {
    public:
        DisplayManager(timeManager* timeMgr, alarmManager* alarmMgr, 
                        connectivityManager* connMgr, powerManager* pwrMgr, EventBus* eventBus);
        ~DisplayManager();

        void run(std::atomic<bool>& running);
        void changePage(PageType newPage);


    private:
        // EVENT HANDLERS
        void onAlarmTriggered(const AlarmTriggeredEvent& event);
        void onAlarmCleared(const AlarmClearedEvent& event);
        void onTimeUpdated(const TimeUpdatedEvent& event);
        void onWifiStatusChanged(const WifiStatusChangedEvent& event);
        void onWeatherUpdated(const WeatherUpdatedEvent& event);
        void onAlarmSet(const AlarmSetEvent& event);
        void onBrightnessChanged(const ScreenBrightnessChanged& event);

        SDL_Window* window = nullptr;
        SDL_Renderer* renderer = nullptr;

        TTF_Font* font;
        SDL_Texture* weatherIcon;

        Page* currentPage = nullptr;
        PageType currentPageType = PageType::MAIN;

        timeManager* timeMgr;
        alarmManager* alarmMgr;
        connectivityManager* connMgr;
        EventBus* m_eventBus;
        powerManager* pwrMgr;

        //event States. thread-safe
        std::mutex stateMutex;
        bool m_isAlarmActive = false;
        bool m_isWifiConnected = false; // need something for time as well
        std::string m_currentTime;
        std::string m_alarmTime;
        int m_temperature = 0;
        std::string m_weatherCondition;

        template<typename T>
        T getState(T& variable) {
            std::lock_guard<std::mutex> lock(stateMutex);
            return variable;
        }

        template<typename T>
        void setState(T& variable, const T& value) {
            std::lock_guard<std::mutex> lock(stateMutex);
            variable = value;
        }



        /*
        bool wifiConnected;
        std::string currentTime;
        std::string alarmTime;
        int temperature;
        std::string weatherCondition; */
        
        
};