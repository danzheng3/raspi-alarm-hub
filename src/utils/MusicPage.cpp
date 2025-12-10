#include "utils/MusicPage.h"
#include <algorithm>

MusicPage::MusicPage(audioManager* audioMgr, equalizerManager* eqMgr, EventBus* eventBus)
    : audioMgr(audioMgr), eqMgr(eqMgr), m_eventBus(eventBus) {
    
    titleFont = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 40);
    songFont = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 28);
    buttonFont = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 32);
    smallFont = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 18);
    songs = audioMgr->getSongList();

    initEQSliders();
}

MusicPage::~MusicPage() {
    if (titleFont) TTF_CloseFont(titleFont);
    if (songFont) TTF_CloseFont(songFont);
    if (buttonFont) TTF_CloseFont(buttonFont);
    if (smallFont) TTF_CloseFont(smallFont);
}

void MusicPage::initEQSliders() {
    eqSliders.clear();
    // Position sliders on the RIGHT side of the screen
    int startX = 700; 
    int startY = 150; 
    int spacing = 75;
    int width = 50;
    int height = 300; // Taller sliders

    std::array<std::string, 7> labels = {"62", "160", "400", "1k", "2.5k", "6k", "16k"};

    for(int i = 0; i < 7; i++) {
        EQSlider slider;
        slider.bandIndex = i;
        slider.label = labels[i];
        slider.trackRect = {startX + (i * spacing), startY, width, height};
        eqSliders.push_back(slider);
    }
}

void MusicPage::render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 25, 25, 35, 255);
    SDL_RenderClear(renderer);
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color accent = {100, 150, 255, 255};

    // Header
    renderText(renderer, "Music Player", 50, 20, white, titleFont);
    backButton = {1100, 20, 150, 60}; // Back button top right
    renderButton(renderer, backButton, "Back", accent);

    // --- LEFT SIDE: Song List ---
    int listX = 50;
    int listY = 100;
    int listW = 600;
    int itemH = 60;
    int visibleSongs = 8; // Fits in 720p height comfortably

    renderText(renderer, "Library", listX, listY - 40, white, buttonFont);

    for (int i = 0; i < visibleSongs && (i + scrollOffset) < songs.size(); i++) {
        int idx = i + scrollOffset;
        SDL_Rect itemRect = {listX, listY + (i * itemH), listW, itemH - 5};
        
        if (idx == selectedSong) SDL_SetRenderDrawColor(renderer, 60, 90, 150, 255);
        else SDL_SetRenderDrawColor(renderer, 40, 40, 50, 255);
        
        SDL_RenderFillRect(renderer, &itemRect);
        renderText(renderer, songs[idx].title, itemRect.x + 10, itemRect.y + 15, white, smallFont);
    }

    // --- RIGHT SIDE: Equalizer ---
    renderText(renderer, "Equalizer", 700, listY - 40, white, buttonFont);
    
    // Render sliders (logic remains similar, using new coordinates from initEQSliders)
    for (const auto& slider : eqSliders) {
        // Track
        SDL_SetRenderDrawColor(renderer, 60, 60, 70, 255);
        SDL_RenderFillRect(renderer, &slider.trackRect);
        
        uint8_t val = eqMgr->getBand(slider.bandIndex);
        float percent = (float)val / 255.0f;
        int knobH = 30;
        int knobY = slider.trackRect.y + slider.trackRect.h - (int)(percent * slider.trackRect.h) - (knobH/2);
        
        // Clamp
        if (knobY < slider.trackRect.y) knobY = slider.trackRect.y;
        if (knobY > slider.trackRect.y + slider.trackRect.h - knobH) knobY = slider.trackRect.y + slider.trackRect.h - knobH;

        // Fill
        SDL_Rect fillRect = {slider.trackRect.x, knobY + (knobH/2), slider.trackRect.w, (slider.trackRect.y + slider.trackRect.h) - (knobY + (knobH/2))};
        SDL_SetRenderDrawColor(renderer, 30, 215, 96, 255);
        SDL_RenderFillRect(renderer, &fillRect);

        // Knob
        SDL_Rect knob = {slider.trackRect.x - 5, knobY, slider.trackRect.w + 10, knobH};
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &knob);
        
        // Label
        renderText(renderer, slider.label, slider.trackRect.x + 10, slider.trackRect.y + slider.trackRect.h + 10, white, smallFont);
    }

    // --- BOTTOM: Controls ---
    int btnY = 620; // Near bottom
    int btnW = 100;
    int btnH = 60;
    int btnSpacing = 20;
    
    // Center the controls relative to the Song List (0-650 area)
    int ctrlCenterX = listX + listW/2;
    int startBtnX = ctrlCenterX - ( (4*btnW + 3*btnSpacing) / 2 );

    prevButton = {startBtnX, btnY, btnW, btnH};
    playPauseButton = {startBtnX + btnW + btnSpacing, btnY, btnW, btnH};
    stopButton = {startBtnX + 2*(btnW + btnSpacing), btnY, btnW, btnH};
    nextButton = {startBtnX + 3*(btnW + btnSpacing), btnY, btnW, btnH};
    
    SDL_Color btnColor = {70, 100, 180, 255};
    renderButton(renderer, prevButton, "<<", btnColor);
    renderButton(renderer, playPauseButton, (audioMgr->getState() == audioManager::AudioState::PLAYING ? "||" : ">"), btnColor);
    renderButton(renderer, stopButton, "[]", {180, 60, 60, 255});
    renderButton(renderer, nextButton, ">>", btnColor);

    // SDL_RenderPresent(renderer);
}

void MusicPage::handleEvent(const SDL_Event& e) {
    if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_FINGERDOWN || e.type == SDL_MOUSEMOTION || e.type == SDL_FINGERMOTION) {
        int x, y;
        bool isDown = (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_FINGERDOWN);
        bool isMove = (e.type == SDL_MOUSEMOTION || e.type == SDL_FINGERMOTION);

        if (e.type == SDL_MOUSEBUTTONDOWN) {
            x = e.button.x;
            y = e.button.y;
        } else if (e.type == SDL_FINGERDOWN){
            x = static_cast<int>(e.tfinger.x * 720);
            y = static_cast<int>(e.tfinger.y * 1280);
        } else if (e.type == SDL_MOUSEMOTION && (e.motion.state & SDL_BUTTON_LMASK)) {
            x = e.motion.x;
            y = e.motion.y;
        } else {
            return; // No relevant action
        }
        
        // Back button
        if (isDown) {
            if (isPointInRect(x, y, backButton)) {
                // Signal to return to main page (handled by DisplayManager)
                request = PageType::MAIN;
                return;
            }

            // Control buttons
            if (isPointInRect(x, y, playPauseButton)) {
                if (audioMgr->getState() == audioManager::AudioState::PLAYING) {
                    audioMgr->pause();
                } else if (audioMgr->getState() == audioManager::AudioState::PAUSED) {
                    audioMgr->resume();
                } else if (selectedSong >= 0) {
                    audioMgr->playSongAtIndex(selectedSong);
                }
            } else if (isPointInRect(x, y, stopButton)) {
                audioMgr->stop();
            } else if (isPointInRect(x, y, scrollUpButton)) {
                scrollOffset = std::max(0, scrollOffset - 1);
            } else if (isPointInRect(x, y, scrollDownButton)) {
                int maxScroll = std::max(0, static_cast<int>(songs.size()) - 12);
                scrollOffset = std::min(maxScroll, scrollOffset + 1);
            }

            // Song selection
            int listY = 100;
            int itemHeight = 70;
            if (x >= 50 && x <= 670 && y >= listY && y <= listY + 900) {
                int clickedIndex = (y - listY) / itemHeight + scrollOffset;
                if (clickedIndex < songs.size()) {
                    selectedSong = clickedIndex;
                    
                    // Publish event to play this song
                    if (m_eventBus) {
                        UISongSelectedEvent event;
                        event.songIndex = clickedIndex;
                        m_eventBus->publish(event);
                    }
                }
            }
        }

        // Handle EQ Sliders (Click or Drag)
        for (const auto& slider : eqSliders) {
            // Check if touch is roughly in the column of the slider
            if (x >= slider.trackRect.x && x <= slider.trackRect.x + slider.trackRect.w &&
                y >= slider.trackRect.y - 20 && y <= slider.trackRect.y + slider.trackRect.h + 20) {
                
                // Calculate Value
                int relativeY = y - slider.trackRect.y;
                float pct = 1.0f - ((float)relativeY / (float)slider.trackRect.h);
                if (pct < 0.0f) pct = 0.0f;
                if (pct > 1.0f) pct = 1.0f;
                
                uint8_t newVal = (uint8_t)(pct * 255);
                
                // Update Manager Directly
                eqMgr->setBand(slider.bandIndex, newVal);
                
                // publish ui event here if needed
            }
        }
        
        
    }
}

void MusicPage::renderText(SDL_Renderer* renderer, const std::string& text, 
                           int x, int y, SDL_Color color, TTF_Font* font) {
    if (!font) return;
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), color);
    if (!surface) return;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect rect = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, nullptr, &rect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void MusicPage::renderButton(SDL_Renderer* renderer, const SDL_Rect& rect, 
                             const std::string& text, SDL_Color bgColor) {
    SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    SDL_RenderFillRect(renderer, &rect);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &rect);
    
    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface* surface = TTF_RenderText_Blended(buttonFont, text.c_str(), white);
    if (surface) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_Rect textRect = {
            rect.x + (rect.w - surface->w) / 2,
            rect.y + (rect.h - surface->h) / 2,
            surface->w, surface->h
        };
        SDL_RenderCopy(renderer, texture, nullptr, &textRect);
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);
    }
}

bool MusicPage::isPointInRect(int x, int y, const SDL_Rect& rect) {
    return x >= rect.x && x <= rect.x + rect.w && 
           y >= rect.y && y <= rect.y + rect.h;
}