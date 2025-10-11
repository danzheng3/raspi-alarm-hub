#pragma once
#include <SDL2/SDL.h>

class Page {
    public:
        virtual ~Page() = default;
        virtual void render(SDL_Renderer* renderer) = 0;
        virtual void handleEvent(const SDL_Event& event) = 0;
};