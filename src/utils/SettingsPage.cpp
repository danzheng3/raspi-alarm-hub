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
    
    renderText(renderer, "Connectivity Settings", 50, 20, white, titleFont);
    backButton = {1100, 20, 150, 60};
    renderButton(renderer, backButton, "Back", accent);

    // --- LEFT SIDE: WiFi ---
    int leftX = 50;
    int leftW = 550;
    
    renderText(renderer, "WiFi Networks", leftX, 100, white, listFont);
    scanButton = {leftX + 350, 95, 150, 40};
    renderButton(renderer, scanButton, scanning ? "..." : "Scan", accent);
    
    // List
    int listY = 160;
    for (size_t i = 0; i < std::min(wifiNetworks.size(), size_t(8)); i++) {
        SDL_Rect netRect = {leftX, listY + i * 60, leftW, 50};
        bool isSelected = (wifiNetworks[i] == selectedSSID);
        
        SDL_SetRenderDrawColor(renderer, isSelected ? 60 : 40, isSelected ? 90 : 45, isSelected ? 150 : 50, 255);
        SDL_RenderFillRect(renderer, &netRect);
        renderText(renderer, wifiNetworks[i], netRect.x + 15, netRect.y + 10, white, listFont);
    }
    
    // --- RIGHT SIDE: Input & Bluetooth ---
    int rightX = 650;
    int rightW = 580;

    // Bluetooth Status Box
    btStatusRect = {rightX, 100, rightW, 80};
    SDL_SetRenderDrawColor(renderer, 40, 45, 50, 255);
    SDL_RenderFillRect(renderer, &btStatusRect);
    std::string btText = "Bluetooth: " + (std::string)(connMgr->isBluetoothConnected() ? "Connected" : "Disconnected");
    renderText(renderer, btText, rightX + 20, 120, connMgr->isBluetoothConnected() ? green : white, listFont);

    // Password Entry
    if (!selectedSSID.empty()) {
        renderText(renderer, "Connect to: " + selectedSSID, rightX, 250, white, listFont);
        renderText(renderer, "Pass: " + passwordInput + "_", rightX, 300, white, listFont);
        
        connectButton = {rightX, 360, 150, 50};
        disconnectButton = {rightX + 170, 360, 150, 50};
        renderButton(renderer, connectButton, "Connect", green);
        renderButton(renderer, disconnectButton, "Cancel", {200, 80, 80, 255});

        // Keyboard rendered below buttons
        renderKeyboard(renderer, rightX, 450); 
    }

    // SDL_RenderPresent(renderer);
}

void SettingsPage::renderKeyboard(SDL_Renderer* renderer, int startX, int startY) {
    keyRects.clear();
    int keySize = 50;
    int gap = 5;
    int keysPerRow = 10;
    
    for (size_t i = 0; i < keyLayout.size(); i++) {
        int row = i / keysPerRow;
        int col = i % keysPerRow;
        SDL_Rect k = {startX + col*(keySize+gap), startY + row*(keySize+gap), keySize, keySize};
        keyRects.push_back(k);
        
        SDL_SetRenderDrawColor(renderer, 60, 70, 90, 255);
        SDL_RenderFillRect(renderer, &k);
        // ... render text char ...
    }
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