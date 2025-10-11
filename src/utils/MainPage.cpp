#include "MainPage.h"
#include <SDL2/SDL.h>

MainPage::MainPage() {}

void MainPage::render(SDL_Renderer* renderer) {
    //background
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_Rect timeBox = { 50, 50, 200, 80 };
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &timeBox);

    if (touched) {
        SDL_Rect touchIndicator = { touchX - 10, touchY - 10, 20, 20 };
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &touchIndicator);
    }

    SDL_RenderPresent(renderer);
}

void MainPage::handleEvent(const SDL_Event& e) {
    if (e.type == SDL_FINGERDOWN) {
        touched = true;
        touchX = static_cast<int>(e.tfinger.x * 1280); 
        touchY = static_cast<int>(e.tfinger.y * 720); 
    } else if (e.type == SDL_FINGERUP) {
        touched = false;
    }
}