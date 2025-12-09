#pragma once
#include "page.h"
#include "managers/connectivityManager.h"
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <string>
#include <thread>

class SettingsPage : public Page {
public:
    SettingsPage(connectivityManager* connMgr);
    ~SettingsPage();
    
    void render(SDL_Renderer* renderer) override;
    void handleEvent(const SDL_Event& event) override;
    PageType getPageRequest() override { return request; }

    

private:
    PageType request = PageType::NONE;

    connectivityManager* connMgr;
    
    TTF_Font* titleFont;
    TTF_Font* listFont;
    TTF_Font* buttonFont;
    
    std::vector<std::string> wifiNetworks;
    std::string selectedSSID;
    std::string passwordInput;
    bool showingKeyboard = false;
    bool scanning = false;
    
    // UI Rects
    SDL_Rect backButton;
    SDL_Rect scanButton;
    SDL_Rect connectButton;
    SDL_Rect disconnectButton;
    SDL_Rect btStatusRect;
    
    // Keyboard
    std::vector<SDL_Rect> keyRects;
    const std::string keyLayout = "1234567890qwertyuiopasdfghjklzxcvbnm ";
    
    void renderText(SDL_Renderer* renderer, const std::string& text, 
                    int x, int y, SDL_Color color, TTF_Font* font);
    void renderButton(SDL_Renderer* renderer, const SDL_Rect& rect, 
                      const std::string& text, SDL_Color bgColor);
    void renderKeyboard(SDL_Renderer* renderer);
    void scanWifi();
    bool isPointInRect(int x, int y, const SDL_Rect& rect);
};