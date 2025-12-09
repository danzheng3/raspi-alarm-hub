#pragma once
#include "page.h"
#include "managers/alarmManager.h"
#include "managers/audioManager.h"
#include <SDL2/SDL_ttf.h>
#include <vector>

class AlarmsPage : public Page {
public:
    AlarmsPage(alarmManager* alarmMgr, audioManager* audioMgr);
    ~AlarmsPage();
    
    void render(SDL_Renderer* renderer) override;
    void handleEvent(const SDL_Event& event) override;
    PageType getPageRequest() override { return pageRequest; }

private:
    alarmManager* alarmMgr;
    audioManager* audioMgr;
    PageType pageRequest = PageType::NONE;
    
    TTF_Font* titleFont;
    TTF_Font* font;
    TTF_Font* smallFont;
    
    // UI Elements
    SDL_Rect backButton;
    
    // Toggles
    SDL_Rect soundToggle;
    SDL_Rect ledToggle;
    SDL_Rect strobeToggle;
    SDL_Rect pillboxToggle;
    
    // Audio Selection
    std::vector<Song> availableSongs;
    int scrollOffset = 0;
    int selectedAudioIndex = -1; // -1 = default
    
    void renderText(SDL_Renderer* renderer, const std::string& text, int x, int y, SDL_Color color, TTF_Font* font);
    void renderToggle(SDL_Renderer* renderer, const std::string& label, const SDL_Rect& rect, bool state);
    void renderButton(SDL_Renderer* renderer, const SDL_Rect& rect, const std::string& text, SDL_Color bgColor);
    bool isPointInRect(int x, int y, const SDL_Rect& rect);
};