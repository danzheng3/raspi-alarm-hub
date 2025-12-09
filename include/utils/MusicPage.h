#pragma once
#include "page.h"
#include "managers/audioManager.h"
#include "managers/equalizerManager.h"

#include "events/EventBus.h"
#include <SDL2/SDL_ttf.h>
#include <vector>

class MusicPage : public Page {
public:
    MusicPage(audioManager* audioMgr, equalizerManager* eqMgr, EventBus* eventBus);
    ~MusicPage();
    
    void render(SDL_Renderer* renderer) override;
    void handleEvent(const SDL_Event& event) override;
    PageType getPageRequest() override { return request; }
    
private:
    PageType request = PageType::NONE;
    audioManager* audioMgr;
    equalizerManager* eqMgr;
    EventBus* m_eventBus;
    
    TTF_Font* titleFont;
    TTF_Font* songFont;
    TTF_Font* buttonFont;
    TTF_Font* smallFont;
    
    std::vector<Song> songs;
    int scrollOffset = 0;
    int selectedSong = -1;
    
    // UI Rects
    SDL_Rect backButton;
    SDL_Rect playPauseButton;
    SDL_Rect stopButton;
    SDL_Rect prevButton;
    SDL_Rect nextButton;
    SDL_Rect scrollUpButton;
    SDL_Rect scrollDownButton;

    struct EQSlider {
        SDL_Rect trackRect;
        SDL_Rect knobRect;
        int bandIndex;
        std::string label;
    };
    std::vector<EQSlider> eqSliders;
    void initEQSliders();
    
    void renderText(SDL_Renderer* renderer, const std::string& text, 
                    int x, int y, SDL_Color color, TTF_Font* font);
    void renderButton(SDL_Renderer* renderer, const SDL_Rect& rect, 
                      const std::string& text, SDL_Color bgColor);
    bool isPointInRect(int x, int y, const SDL_Rect& rect);
};