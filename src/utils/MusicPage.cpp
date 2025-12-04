#include "utils/MusicPage.h"
#include <algorithm>

MusicPage::MusicPage(audioManager* audioMgr, EventBus* eventBus)
    : audioMgr(audioMgr), m_eventBus(eventBus) {
    
    titleFont = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 40);
    songFont = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 28);
    buttonFont = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 32);
    
    songs = audioMgr->getSongList();
}

MusicPage::~MusicPage() {
    if (titleFont) TTF_CloseFont(titleFont);
    if (songFont) TTF_CloseFont(songFont);
    if (buttonFont) TTF_CloseFont(buttonFont);
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
    int itemHeight = 70;
    int visibleSongs = listHeight / itemHeight;
    
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
    
    // Scroll buttons
    scrollUpButton = {680, 100, 30, 100};
    scrollDownButton = {680, listHeight - 100, 30, 100};
    
    SDL_SetRenderDrawColor(renderer, 80, 80, 90, 255);
    SDL_RenderFillRect(renderer, &scrollUpButton);
    SDL_RenderFillRect(renderer, &scrollDownButton);
    renderText(renderer, "^", scrollUpButton.x + 8, scrollUpButton.y + 30, white, buttonFont);
    renderText(renderer, "v", scrollDownButton.x + 8, scrollDownButton.y + 30, white, buttonFont);
    
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
    
    renderButton(renderer, stopButton, "□", stopColor);
    renderButton(renderer, nextButton, ">>", btnColor);
    
    SDL_RenderPresent(renderer);
}

void MusicPage::handleEvent(const SDL_Event& e) {
    if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_FINGERDOWN) {
        int x, y;
        if (e.type == SDL_MOUSEBUTTONDOWN) {
            x = e.button.x;
            y = e.button.y;
        } else {
            x = static_cast<int>(e.tfinger.x * 720);
            y = static_cast<int>(e.tfinger.y * 1280);
        }
        
        // Back button
        if (isPointInRect(x, y, backButton)) {
            // Signal to return to main page (handled by DisplayManager)
            return;
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