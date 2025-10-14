#include "utils/page.h"
#include "utils/MainPage.h"
#include <display_manager.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>


DisplayManager::DisplayManager() {

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

    window = SDL_CreateWindow("Raspi Alarm Hub", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Could not create window: " << SDL_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
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
            currentPage = new MainPage();
            break;
        // Future cases for SETTINGS, ALARMS, TIMESET
        default:
            currentPage = new MainPage(); 
            break;
    }
}

void DisplayManager::run() {
    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (currentPage) {
                currentPage->handleEvent(event);
            }
        }

        if (currentPage) {
            currentPage->render(renderer);
        }

        SDL_Delay(100);  //16 ms ~ 60 FPS
    }
}