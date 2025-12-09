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
    int startX = 50;
    int startY = 850; // Position below song list
    int spacing = 95;
    int width = 60;
    int height = 200;

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
    // Dark background
    SDL_SetRenderDrawColor(renderer, 25, 25, 35, 255);
    SDL_RenderClear(renderer);
    
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color gray = {150, 150, 150, 255};
    SDL_Color accent = {100, 150, 255, 255};
    
    // Title
    renderText(renderer, "Music Player", 250, 20, white, titleFont);
    
    // Back button (top left)
    backButton = {20, 20, 150, 60};
    renderButton(renderer, backButton, "< Back", accent);
    
    // Song list area
    int listY = 100;
    int listHeight = 900;
    int itemHeight = 60;
    int visibleSongs = 8;
    
    // Render songs
    for (int i = 0; i < visibleSongs && (i + scrollOffset) < songs.size(); i++) {
        int songIndex = i + scrollOffset;
        SDL_Rect songRect = {50, listY + i * itemHeight, 620, itemHeight - 5};
        
        // Highlight if playing or selected
        if (songIndex == selectedSong) {
            SDL_SetRenderDrawColor(renderer, 60, 90, 150, 255);
            SDL_RenderFillRect(renderer, &songRect);
        } else {
            SDL_SetRenderDrawColor(renderer, 40, 40, 50, 255);
            SDL_RenderFillRect(renderer, &songRect);
        }
        
        SDL_SetRenderDrawColor(renderer, 80, 80, 90, 255);
        SDL_RenderDrawRect(renderer, &songRect);
        
        renderText(renderer, songs[songIndex].title, 
                   songRect.x + 15, songRect.y + 18, white, songFont);
    }
    
    // REMOVED SCROLL BUTTON FEATURE FOR SIMPLICITY
    
    // Control buttons at bottom
    int btnY = 1100;
    int btnW = 130;
    int btnH = 80;
    int btnSpacing = 20;
    int totalWidth = 4 * btnW + 3 * btnSpacing;
    int startX = (720 - totalWidth) / 2;
    
    prevButton = {startX, btnY, btnW, btnH};
    playPauseButton = {startX + btnW + btnSpacing, btnY, btnW, btnH};
    stopButton = {startX + 2 * (btnW + btnSpacing), btnY, btnW, btnH};
    nextButton = {startX + 3 * (btnW + btnSpacing), btnY, btnW, btnH};
    
    SDL_Color btnColor = {70, 100, 180, 255};
    SDL_Color stopColor = {180, 60, 60, 255};
    
    renderButton(renderer, prevButton, "<<", btnColor);
    
    if (audioMgr->getState() == audioManager::AudioState::PLAYING) {
        renderButton(renderer, playPauseButton, "||", btnColor);
    } else {
        renderButton(renderer, playPauseButton, ">", btnColor);
    }
    
    renderButton(renderer, stopButton, "STOP", stopColor);
    renderButton(renderer, nextButton, ">>", btnColor);

    renderText(renderer, "Equalizer", 50, 800, white, buttonFont);

    for (const auto& slider : eqSliders) {
        // 1. Draw Track Background
        SDL_SetRenderDrawColor(renderer, 60, 60, 70, 255);
        SDL_RenderFillRect(renderer, &slider.trackRect);
        
        // 2. Calculate Knob Position based on current Value
        uint8_t val = eqMgr->getBand(slider.bandIndex); // 0-255
        // Invert Y because 255 is TOP, 0 is BOTTOM
        float percent = (float)val / 255.0f;
        int knobH = 20;
        int knobY = slider.trackRect.y + slider.trackRect.h - (int)(percent * slider.trackRect.h) - (knobH/2);
        
        // Clamp knobY
        if (knobY < slider.trackRect.y) knobY = slider.trackRect.y;
        if (knobY > slider.trackRect.y + slider.trackRect.h - knobH) knobY = slider.trackRect.y + slider.trackRect.h - knobH;

        // 3. Draw Filled Bar (Green spotify style)
        SDL_Rect fillRect = {
            slider.trackRect.x, 
            knobY + (knobH/2), 
            slider.trackRect.w, 
            (slider.trackRect.y + slider.trackRect.h) - (knobY + (knobH/2))
        };
        SDL_SetRenderDrawColor(renderer, 30, 215, 96, 255); // Spotify Green
        SDL_RenderFillRect(renderer, &fillRect);

        // 4. Draw Knob
        SDL_Rect knob = {slider.trackRect.x - 5, knobY, slider.trackRect.w + 10, knobH};
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &knob);

        // 5. Label
        renderText(renderer, slider.label, slider.trackRect.x + 10, slider.trackRect.y + slider.trackRect.h + 5, white, smallFont);
    }
    
    SDL_RenderPresent(renderer);
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