#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <string>
#include <iostream>
#include "utils/page.h"
#include <atomic>
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
                        connectivityManager* connMgr, EventBus* eventBus);
        ~DisplayManager();

        void run(std::atomic<bool>& running);
        void changePage(PageType newPage);


    private:
        // EVENT HANDLERS
        void onAlarmTriggered(const AlarmTriggeredEvent& event);
        void onAlarmStopped(const AlarmClearedEvent& event);
        void onTimeUpdated(const TimeUpdatedEvent& event);
        void onWifiStatusChanged(const WifiStatusChangedEvent& event);

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

        //event States
        bool m_isAlarmActive = false;
        bool m_isWifiConnected = false; // need something for time as well

        /*
        bool wifiConnected;
        std::string currentTime;
        std::string alarmTime;
        int temperature;
        std::string weatherCondition; */
        
        
};