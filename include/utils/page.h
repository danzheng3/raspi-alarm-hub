#pragma once
#include <SDL2/SDL.h>

enum class PageType {
    NONE,
    MAIN,
    MUSIC,
    SETTINGS,
    ALARMS,
    TIMESET
};

class Page {
    public:
        virtual ~Page() = default;
        virtual void render(SDL_Renderer* renderer) = 0;
        virtual void handleEvent(const SDL_Event& event) = 0;
        virtual PageType getPageRequest() { return PageType::NONE; }
};