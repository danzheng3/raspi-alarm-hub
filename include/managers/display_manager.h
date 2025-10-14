#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <string>
#include <iostream>
#include "utils/page.h"

enum class PageType {
    MAIN,
    SETTINGS,
    ALARMS,
    TIMESET
};

class DisplayManager {
    public:
        DisplayManager();
        ~DisplayManager();

        void run();

    private:
        void changePage(PageType newPage);
        SDL_Window* window = nullptr;
        SDL_Renderer* renderer = nullptr;

        TTF_Font* font;
        SDL_Texture* weatherIcon;

        Page* currentPage = nullptr;
        PageType currentPageType = PageType::MAIN;

        /*
        bool wifiConnected;
        std::string currentTime;
        std::string alarmTime;
        int temperature;
        std::string weatherCondition; */
        
        
};