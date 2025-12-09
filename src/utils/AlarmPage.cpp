#include "utils/AlarmPage.h"

AlarmsPage::AlarmsPage(alarmManager* alarmMgr, audioManager* audioMgr)
    : alarmMgr(alarmMgr), audioMgr(audioMgr) {
    
    titleFont = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 40);
    font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 28);
    smallFont = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 24);
    
    availableSongs = audioMgr->getSongList();
    
    // Determine selected index from current config
    std::string currentPath = alarmMgr->getAlarmConfig().alarmAudioPath;
    if (currentPath == "default") selectedAudioIndex = -1;
    else {
        for(size_t i=0; i<availableSongs.size(); i++) {
            if(availableSongs[i].filePath == currentPath) {
                selectedAudioIndex = i;
                break;
            }
        }
    }
}

AlarmsPage::~AlarmsPage() {
    TTF_CloseFont(titleFont);
    TTF_CloseFont(font);
    TTF_CloseFont(smallFont);
}

void AlarmsPage::render(SDL_Renderer* renderer) {
    // Dark Background
    SDL_SetRenderDrawColor(renderer, 25, 25, 35, 255);
    SDL_RenderClear(renderer);
    
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color accent = {100, 150, 255, 255};
    
    // Header
    renderText(renderer, "Alarm Settings", 200, 20, white, titleFont);
    backButton = {20, 20, 150, 60};
    renderButton(renderer, backButton, "< Back", accent);
    
    // Toggles Section
    AlarmConfig config = alarmMgr->getAlarmConfig();
    
    int toggleY = 120;
    soundToggle = {550, toggleY, 80, 40};
    renderToggle(renderer, "Play Sound", soundToggle, config.soundEnabled);
    
    ledToggle = {550, toggleY + 60, 80, 40};
    renderToggle(renderer, "Enable LED", ledToggle, config.ledEnabled);
    
    strobeToggle = {550, toggleY + 120, 80, 40};
    renderToggle(renderer, "Enable Strobe", strobeToggle, config.strobeEnabled);
    
    pillboxToggle = {550, toggleY + 180, 80, 40};
    renderToggle(renderer, "Open Pillbox", pillboxToggle, config.pillboxEnabled);
    
    // Audio Selection List
    renderText(renderer, "Alarm Sound:", 50, 400, white, font);
    
    int listY = 450;
    int itemH = 60;
    
    // Default Option
    SDL_Rect defRect = {50, listY, 620, 50};
    if (selectedAudioIndex == -1) SDL_SetRenderDrawColor(renderer, 60, 90, 150, 255);
    else SDL_SetRenderDrawColor(renderer, 40, 40, 50, 255);
    SDL_RenderFillRect(renderer, &defRect);
    renderText(renderer, "Default Alarm (Beep)", 70, listY + 10, white, smallFont);
    
    // Song List
    for(int i=0; i<8 && (i + scrollOffset) < availableSongs.size(); i++) {
        int idx = i + scrollOffset;
        SDL_Rect itemRect = {50, listY + 55 + (i * 55), 620, 50};
        
        if (idx == selectedAudioIndex) SDL_SetRenderDrawColor(renderer, 60, 90, 150, 255);
        else SDL_SetRenderDrawColor(renderer, 40, 40, 50, 255);
        
        SDL_RenderFillRect(renderer, &itemRect);
        renderText(renderer, availableSongs[idx].title, 70, itemRect.y + 10, white, smallFont);
    }
    
    SDL_RenderPresent(renderer);
}

void AlarmsPage::handleEvent(const SDL_Event& e) {
    if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_FINGERDOWN) {
        int x = (e.type == SDL_MOUSEBUTTONDOWN) ? e.button.x : e.tfinger.x * 720;
        int y = (e.type == SDL_MOUSEBUTTONDOWN) ? e.button.y : e.tfinger.y * 1280;
        
        if (isPointInRect(x, y, backButton)) {
            pageRequest = PageType::MAIN;
            return;
        }
        
        AlarmConfig config = alarmMgr->getAlarmConfig();
        bool changed = false;
        
        if (isPointInRect(x, y, soundToggle)) { config.soundEnabled = !config.soundEnabled; changed = true; }
        if (isPointInRect(x, y, ledToggle)) { config.ledEnabled = !config.ledEnabled; changed = true; }
        if (isPointInRect(x, y, strobeToggle)) { config.strobeEnabled = !config.strobeEnabled; changed = true; }
        if (isPointInRect(x, y, pillboxToggle)) { config.pillboxEnabled = !config.pillboxEnabled; changed = true; }
        
        // Audio Selection
        int listY = 450;
        // Check Default
        if (isPointInRect(x, y, {50, listY, 620, 50})) {
            config.alarmAudioPath = "default";
            selectedAudioIndex = -1;
            changed = true;
        }
        
        // Check Songs
        for(int i=0; i<8 && (i + scrollOffset) < availableSongs.size(); i++) {
            SDL_Rect itemRect = {50, listY + 55 + (i * 55), 620, 50};
            if (isPointInRect(x, y, itemRect)) {
                selectedAudioIndex = i + scrollOffset;
                config.alarmAudioPath = availableSongs[selectedAudioIndex].filePath;
                changed = true;
            }
        }
        
        if (changed) alarmMgr->setAlarmConfig(config);
    }
}

void AlarmsPage::renderToggle(SDL_Renderer* renderer, const std::string& label, const SDL_Rect& rect, bool state) {
    SDL_Color white = {255, 255, 255, 255};
    renderText(renderer, label, 50, rect.y + 5, white, font);
    
    SDL_SetRenderDrawColor(renderer, state ? 80 : 80, state ? 200 : 80, 80, 255); // Green if true, Gray/Red if false
    SDL_RenderFillRect(renderer, &rect);
    
    SDL_Color black = {0,0,0,255};
    renderText(renderer, state ? "ON" : "OFF", rect.x + 15, rect.y + 8, black, smallFont);
}

// Helpers (renderText, renderButton, isPointInRect) same as other pages...
void AlarmsPage::renderText(SDL_Renderer* renderer, const std::string& text, int x, int y, SDL_Color color, TTF_Font* f) {
    if (!f) return;
    SDL_Surface* surf = TTF_RenderText_Blended(f, text.c_str(), color);
    if (surf) {
        SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_Rect dst = {x, y, surf->w, surf->h};
        SDL_RenderCopy(renderer, tex, nullptr, &dst);
        SDL_FreeSurface(surf);
        SDL_DestroyTexture(tex);
    }
}

void AlarmsPage::renderButton(SDL_Renderer* renderer, const SDL_Rect& rect, const std::string& text, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
    SDL_RenderFillRect(renderer, &rect);
    SDL_Color white = {255,255,255,255};
    renderText(renderer, text, rect.x + 30, rect.y + 15, white, font);
}

bool AlarmsPage::isPointInRect(int x, int y, const SDL_Rect& r) {
    return x>=r.x && x<=r.x+r.w && y>=r.y && y<=r.y+r.h;
}