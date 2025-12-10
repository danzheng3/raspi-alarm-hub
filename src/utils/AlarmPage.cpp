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
    // 1. Dark Background
    SDL_SetRenderDrawColor(renderer, 25, 25, 35, 255);
    SDL_RenderClear(renderer);
    
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color accent = {100, 150, 255, 255};
    
    // 2. Header
    renderText(renderer, "Alarm Settings", 50, 20, white, titleFont);
    backButton = {1100, 20, 150, 60};
    renderButton(renderer, backButton, "Back", accent);
    
    // --- LEFT SIDE: Toggles (X: 100) ---
    AlarmConfig config = alarmMgr->getAlarmConfig();
    int toggleX = 100;
    int toggleY = 150;
    int gap = 80;
    
    // Toggle Buttons (Draw label to the left, button to the right)
    soundToggle = {toggleX + 300, toggleY, 80, 40};
    renderToggle(renderer, "Play Sound", soundToggle, config.soundEnabled);
    
    ledToggle = {toggleX + 300, toggleY + gap, 80, 40};
    renderToggle(renderer, "Enable LED", ledToggle, config.ledEnabled);
    
    strobeToggle = {toggleX + 300, toggleY + gap*2, 80, 40};
    renderToggle(renderer, "Enable Strobe", strobeToggle, config.strobeEnabled);
    
    pillboxToggle = {toggleX + 300, toggleY + gap*3, 80, 40};
    renderToggle(renderer, "Open Pillbox", pillboxToggle, config.pillboxEnabled);
    
    // --- RIGHT SIDE: Audio List (X: 600) ---
    int listX = 600;
    int listY = 100;
    int listW = 600;
    int itemH = 60;
    
    renderText(renderer, "Select Alarm Sound", listX, listY, white, font);
    
    // 1. Default Option
    int contentStart = listY + 60;
    SDL_Rect defRect = {listX, contentStart, listW, itemH - 10};
    
    if (selectedAudioIndex == -1) SDL_SetRenderDrawColor(renderer, 60, 90, 150, 255); // Highlight
    else SDL_SetRenderDrawColor(renderer, 40, 40, 50, 255);
    
    SDL_RenderFillRect(renderer, &defRect);
    renderText(renderer, "Default Alarm (Beep)", listX + 20, contentStart + 12, white, smallFont);
    
    // 2. Song List
    int songsY = contentStart + itemH;
    for(int i=0; i<8 && (i + scrollOffset) < availableSongs.size(); i++) {
        int idx = i + scrollOffset;
        SDL_Rect itemRect = {listX, songsY + (i * itemH), listW, itemH - 10};
        
        if (idx == selectedAudioIndex) SDL_SetRenderDrawColor(renderer, 60, 90, 150, 255);
        else SDL_SetRenderDrawColor(renderer, 40, 40, 50, 255);
        
        SDL_RenderFillRect(renderer, &itemRect);
        renderText(renderer, availableSongs[idx].title, listX + 20, itemRect.y + 12, white, smallFont);
    }
}

void AlarmsPage::handleEvent(const SDL_Event& e) {
    if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_FINGERDOWN) {
        int x, y;
        
        if (e.type == SDL_MOUSEBUTTONDOWN) {
            x = e.button.x;
            y = e.button.y;
        } else {
            // FIX: Use Landscape dimensions (1280x720)
            x = static_cast<int>(e.tfinger.x * 1280);
            y = static_cast<int>(e.tfinger.y * 720);
        }
        
        if (isPointInRect(x, y, backButton)) {
            pageRequest = PageType::MAIN;
            return;
        }
        
        AlarmConfig config = alarmMgr->getAlarmConfig();
        bool changed = false;
        
        // Check Toggles
        if (isPointInRect(x, y, soundToggle)) { config.soundEnabled = !config.soundEnabled; changed = true; }
        if (isPointInRect(x, y, ledToggle)) { config.ledEnabled = !config.ledEnabled; changed = true; }
        if (isPointInRect(x, y, strobeToggle)) { config.strobeEnabled = !config.strobeEnabled; changed = true; }
        if (isPointInRect(x, y, pillboxToggle)) { config.pillboxEnabled = !config.pillboxEnabled; changed = true; }
        
        // Check Audio Selection
        int listX = 600;
        int listY = 100;
        int itemH = 60;
        int contentStart = listY + 60;

        // 1. Check Default
        SDL_Rect defRect = {listX, contentStart, 600, itemH - 10};
        if (isPointInRect(x, y, defRect)) {
            config.alarmAudioPath = "default";
            selectedAudioIndex = -1;
            changed = true;
        }
        
        // 2. Check Songs
        int songsY = contentStart + itemH;
        for(int i=0; i<8 && (i + scrollOffset) < availableSongs.size(); i++) {
            SDL_Rect itemRect = {listX, songsY + (i * itemH), 600, itemH - 10};
            if (isPointInRect(x, y, itemRect)) {
                selectedAudioIndex = i + scrollOffset;
                config.alarmAudioPath = availableSongs[selectedAudioIndex].filePath;
                changed = true;
            }
        }
        
        if (changed) {
            alarmMgr->setAlarmConfig(config);
        }
    }
}

void AlarmsPage::renderToggle(SDL_Renderer* renderer, const std::string& label, const SDL_Rect& rect, bool state) {
    SDL_Color white = {255, 255, 255, 255};
    // Draw label to the LEFT of the toggle box
    renderText(renderer, label, rect.x - 300, rect.y + 5, white, font);
    
    SDL_SetRenderDrawColor(renderer, state ? 80 : 80, state ? 200 : 80, 80, 255);
    SDL_RenderFillRect(renderer, &rect);
    
    SDL_Color black = {0,0,0,255};
    renderText(renderer, state ? "ON" : "OFF", rect.x + 15, rect.y + 8, black, smallFont);
}

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
    // Center text roughly
    renderText(renderer, text, rect.x + 30, rect.y + 15, white, font);
}

bool AlarmsPage::isPointInRect(int x, int y, const SDL_Rect& r) {
    return x >= r.x && x <= r.x + r.w && y >= r.y && y <= r.y + r.h;
}