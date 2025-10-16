#include "utils/page.h"
#include "utils/MainPage.h"
#include <managers/display_manager.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>


DisplayManager::DisplayManager(timeManager* timeMgr, alarmManager* alarmMgr, connectivityManager* connMgr)
    : timeMgr(timeMgr), alarmMgr(alarmMgr), connMgr(connMgr), window(nullptr), renderer(nullptr), font(nullptr), weatherIcon(nullptr), currentPage(nullptr)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) < 0) {
        std::cerr << "Could not initialize SDL: " << SDL_GetError() << std::endl;
        return;
    }

    if (TTF_Init() == -1) {
        std::cerr << "Could not initialize TTF: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return;
    }

    if (IMG_Init(IMG_INIT_PNG) == 0) {
        std::cerr << "Could not initialize SDL_image: " << IMG_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return;
    }

    window = SDL_CreateWindow("Raspi Alarm Hub", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 720, 1280, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Could not create window: " << SDL_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    std::cout << "displaymanager initialized" << std::endl;
    changePage(PageType::MAIN);

}

DisplayManager::~DisplayManager() {
    if (currentPage) {
        delete currentPage;
    }
    if (weatherIcon) {
        SDL_DestroyTexture(weatherIcon);
    }
    if (font) {
        TTF_CloseFont(font);
    }
    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }
    if (window) {
        SDL_DestroyWindow(window);
    }
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
}

void DisplayManager::changePage(PageType newPage) {
    if (currentPage) {
        delete currentPage;
        currentPage = nullptr;
    }

    currentPageType = newPage;

    switch (newPage) {
        case PageType::MAIN:
            currentPage = new MainPage(timeMgr, alarmMgr, connMgr);
            break;
        // Future cases for SETTINGS, ALARMS, TIMESET
        default:
            currentPage = new MainPage(timeMgr, alarmMgr, connMgr); 
            break;
    }
}

void DisplayManager::run(std::atomic<bool>& running) {
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_FINGERDOWN) {
                std::cout << "Touch detected at (" << event.tfinger.x << ", " << event.tfinger.y << ")" << std::endl;
            } else if (currentPage) {
                currentPage->handleEvent(event);
            }
        }

        if (currentPage) {
            currentPage->render(renderer);
        }

        SDL_Delay(16);  // ~ 60 FPS
    }
}