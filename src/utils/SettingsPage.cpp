#include "utils/SettingsPage.h"

SettingsPage::SettingsPage(connectivityManager* connMgr)
    : connMgr(connMgr) {
    
    titleFont = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 40);
    listFont = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 28);
    buttonFont = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 28);
}

SettingsPage::~SettingsPage() {
    if (titleFont) TTF_CloseFont(titleFont);
    if (listFont) TTF_CloseFont(listFont);
    if (buttonFont) TTF_CloseFont(buttonFont);
}

void SettingsPage::render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 25, 25, 35, 255);
    SDL_RenderClear(renderer);
    
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color accent = {100, 150, 255, 255};
    SDL_Color green = {80, 200, 120, 255};
    SDL_Color red = {200, 80, 80, 255};
    
    // Title & Back
    renderText(renderer, "Connectivity", 200, 20, white, titleFont);
    backButton = {20, 20, 150, 60};
    renderButton(renderer, backButton, "< Back", accent);
    
    // WiFi Section
    renderText(renderer, "WiFi Networks", 50, 100, white, listFont);
    
    // Scan button
    scanButton = {520, 100, 180, 50};
    renderButton(renderer, scanButton, scanning ? "Scanning..." : "Scan", accent);
    
    // Current status
    std::string status = connMgr && connMgr->isWifiConnected() ? 
        "Connected " : "Not Connected";
    renderText(renderer, status, 50, 160, 
               connMgr && connMgr->isWifiConnected() ? green : red, listFont);
    
    // Network list
    int listY = 220;
    for (size_t i = 0; i < std::min(wifiNetworks.size(), size_t(8)); i++) {
        SDL_Rect netRect = {50, listY + i * 60, 620, 55};
        
        bool isSelected = (wifiNetworks[i] == selectedSSID);
        SDL_SetRenderDrawColor(renderer, isSelected ? 60 : 40, 
                               isSelected ? 90 : 45, 
                               isSelected ? 150 : 50, 255);
        SDL_RenderFillRect(renderer, &netRect);
        SDL_SetRenderDrawColor(renderer, 80, 80, 90, 255);
        SDL_RenderDrawRect(renderer, &netRect);
        
        renderText(renderer, wifiNetworks[i], netRect.x + 15, netRect.y + 12, 
                   white, listFont);
    }
    
    // Password input & keyboard (if network selected)
    if (!selectedSSID.empty()) {
        renderText(renderer, "Password: " + passwordInput + "_", 50, 750, white, listFont);
        
        connectButton = {50, 800, 200, 60};
        disconnectButton = {270, 800, 200, 60};
        
        renderButton(renderer, connectButton, "Connect", green);
        renderButton(renderer, disconnectButton, "Cancel", red);
        
        renderKeyboard(renderer);
    }
    
    // Bluetooth section (simple status)
    btStatusRect = {50, 950, 620, 100};
    SDL_SetRenderDrawColor(renderer, 40, 45, 50, 255);
    SDL_RenderFillRect(renderer, &btStatusRect);
    SDL_SetRenderDrawColor(renderer, 80, 80, 90, 255);
    SDL_RenderDrawRect(renderer, &btStatusRect);
    
    std::string btStatus = "Bluetooth: ";
    btStatus += connMgr && connMgr->isBluetoothConnected() ? 
        "Speaker Connected ✓" : "Not Connected";
    renderText(renderer, btStatus, 70, 985, 
               connMgr && connMgr->isBluetoothConnected() ? green : red, listFont);
    
    SDL_RenderPresent(renderer);
}

void SettingsPage::renderKeyboard(SDL_Renderer* renderer) {
    keyRects.clear();
    int keyW = 60;
    int keyH = 60;
    int startY = 1100;
    int keysPerRow = 10;
    
    SDL_Color keyColor = {60, 70, 90, 255};
    SDL_Color white = {255, 255, 255, 255};
    
    for (size_t i = 0; i < keyLayout.size(); i++) {
        int row = i / keysPerRow;
        int col = i % keysPerRow;
        int x = 50 + col * (keyW + 5);
        int y = startY + row * (keyH + 5);
        
        SDL_Rect keyRect = {x, y, keyW, keyH};
        keyRects.push_back(keyRect);
        
        SDL_SetRenderDrawColor(renderer, keyColor.r, keyColor.g, keyColor.b, 255);
        SDL_RenderFillRect(renderer, &keyRect);
        SDL_SetRenderDrawColor(renderer, 100, 100, 110, 255);
        SDL_RenderDrawRect(renderer, &keyRect);
        
        std::string keyStr(1, keyLayout[i]);
        if (keyLayout[i] == ' ') keyStr = "___";
        renderText(renderer, keyStr, x + 18, y + 15, white, buttonFont);
    }
    
    // Backspace key
    SDL_Rect backspaceKey = {570, startY, 120, keyH};
    keyRects.push_back(backspaceKey);
    renderButton(renderer, backspaceKey, "<--", {180, 60, 60, 255});
}

void SettingsPage::handleEvent(const SDL_Event& e) {
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
            // Return to main
            request = PageType::MAIN;
            return;
        }
        
        // Scan button
        if (isPointInRect(x, y, scanButton) && !scanning) {
            scanWifi();
        }
        
        // Network selection
        int listY = 220;
        for (size_t i = 0; i < std::min(wifiNetworks.size(), size_t(8)); i++) {
            SDL_Rect netRect = {50, listY + i * 60, 620, 55};
            if (isPointInRect(x, y, netRect)) {
                selectedSSID = wifiNetworks[i];
                passwordInput.clear();
            }
        }
        
        // Connect button
        if (!selectedSSID.empty() && isPointInRect(x, y, connectButton)) {
            if (connMgr) {
                connMgr->connectToWifi(selectedSSID, passwordInput);
            }
            selectedSSID.clear();
            passwordInput.clear();
        }
        
        // Disconnect/Cancel
        if (isPointInRect(x, y, disconnectButton)) {
            selectedSSID.clear();
            passwordInput.clear();
        }
        
        // Keyboard
        for (size_t i = 0; i < keyRects.size(); i++) {
            if (isPointInRect(x, y, keyRects[i])) {
                if (i < keyLayout.size()) {
                    passwordInput += keyLayout[i];
                } else {
                    // Backspace
                    if (!passwordInput.empty()) {
                        passwordInput.pop_back();
                    }
                }
            }
        }
    }
}

void SettingsPage::scanWifi() {
    scanning = true;
    // Scan in background thread
    std::thread([this]() {
        wifiNetworks.clear();
        WifiAdapter tempAdapter;
        tempAdapter.scan(wifiNetworks);
        scanning = false;
    }).detach();
}

// Helper implementations...
void SettingsPage::renderText(SDL_Renderer* renderer, const std::string& text, 
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

void SettingsPage::renderButton(SDL_Renderer* renderer, const SDL_Rect& rect, 
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

bool SettingsPage::isPointInRect(int x, int y, const SDL_Rect& rect) {
    return x >= rect.x && x <= rect.x + rect.w && 
           y >= rect.y && y <= rect.y + rect.h;
}