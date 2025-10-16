#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <string>
#include <iostream>
#include "utils/page.h"
#include <atomic>

enum class PageType {
    MAIN,
    SETTINGS,
    ALARMS,
    TIMESET
};

class DisplayManager {
    public:
        DisplayManager(timeManager* timeMgr, alarmManager* alarmMgr, connectivityManager* connMgr);
        ~DisplayManager();

        void run(std::atomic<bool>& running);
        void changePage(PageType newPage);


    private:
        SDL_Window* window = nullptr;
        SDL_Renderer* renderer = nullptr;

        TTF_Font* font;
        SDL_Texture* weatherIcon;

        Page* currentPage = nullptr;
        PageType currentPageType = PageType::MAIN;

        timeManager* timeMgr;
        alarmManager* alarmMgr;
        connectivityManager* connMgr;

        /*
        bool wifiConnected;
        std::string currentTime;
        std::string alarmTime;
        int temperature;
        std::string weatherCondition; */
        
        
};