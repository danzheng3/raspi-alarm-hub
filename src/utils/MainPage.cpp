#include "MainPage.h"
#include <SDL2/SDL.h>
#include <iomanip>

MainPage::MainPage(timeManager* timeMgr, alarmManager* alarmMgr, connectivityManager* connMgr) 
    : timeMgr(timeMgr), alarmMgr(alarmMgr), connMgr(connMgr) 
{
    font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 48);
    smallFont = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 36);
    
    // Initialize adjusted time from current time
    if (timeMgr) {
        std::string currentTime = timeMgr->getCurrentTime();
        // Parse HH:MM format
        size_t colonPos = currentTime.find(':');
        if (colonPos != std::string::npos) {
            adjustedHour = std::stoi(currentTime.substr(0, colonPos));
            adjustedMinute = std::stoi(currentTime.substr(colonPos + 1));
        }
    }
}
MainPage::~MainPage() {
    if (font) {
        TTF_CloseFont(font);
    }
    if (smallFont) {
        TTF_CloseFont(smallFont);
    }   
}

void MainPage::render(SDL_Renderer* renderer) {
    // Background gray
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderClear(renderer);

    SDL_Color black = {0, 0, 0, 255};

    // Display WiFi status
    std::string wifiStatus = connMgr ? (connMgr->isWifiConnected() ? "WiFi: Connected" : "WiFi: Disconnected") : "WiFi: Unknown";
    renderText(renderer, wifiStatus, 50, 50, black, smallFont);

    // Display current time (center) - make it a tappable area
    std::string currentTime = timeMgr ? timeMgr->getCurrentTime() : "--:--";
    int centerX = 720 / 2 - 150;
    int centerY = 1280 / 2 - 80;
    
    // Draw a subtle box around the time to indicate it's tappable
    timeDisplayRect = { centerX - 20, centerY - 20, 340, 120 };
    SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
    SDL_RenderDrawRect(renderer, &timeDisplayRect);
    
    renderText(renderer, currentTime, centerX, centerY, black, font);

    // Display alarm time
    std::string alarmTime = alarmMgr ? (alarmMgr->isAlarmEnabled() ? alarmMgr->getAlarmTime() : "None") : "None";
    //alarmDisplayRect = 
    renderText(renderer, "Alarm: " + alarmTime, centerX, 150, black, smallFont);
    int alarmY = centerY + 120;
    alarmDisplayRect = { centerX - 50, 140, 280, 60 };
    SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
    SDL_RenderDrawRect(renderer, &alarmDisplayRect);

    // Touch feedback - blue circle
    if (touched) {
        SDL_SetRenderDrawColor(renderer, 0, 100, 255, 180);
        drawCircle(renderer, touchX, touchY, 30);
    }

    // Popup for time adjustment
    if (currentMode == AdjustmentMode::ADJUST_TIME) {
        renderAdjustPopup(renderer, "Set Time");
    } else if (currentMode == AdjustmentMode::ADJUST_ALARM) {
        renderAdjustPopup(renderer, "Set Alarm");
    }

    SDL_RenderPresent(renderer);
}

void MainPage::handleEvent(const SDL_Event& e) {
    if (e.type == SDL_FINGERDOWN) {
        touched = true;
        touchX = static_cast<int>(e.tfinger.x * 720); 
        touchY = static_cast<int>(e.tfinger.y * 1280);
    } else if (e.type == SDL_MOUSEBUTTONDOWN) {
        touched = true;
        touchX = e.button.x; 
        touchY = e.button.y;

        if (currentMode == AdjustmentMode::NONE) {
            if (isPointInRect(touchX, touchY, timeDisplayRect)) {
                currentMode = AdjustmentMode::ADJUST_TIME;
                std::string currentTime = timeMgr->getCurrentTime();
                sscanf(currentTime.c_str(), "%d:%d", &adjustedHour, &adjustedMinute);
            } else if (isPointInRect(touchX, touchY, alarmDisplayRect)) {
                currentMode = AdjustmentMode::ADJUST_ALARM;
                std::string alarmTime = alarmMgr->getAlarmTime();
                sscanf(alarmTime.c_str(), "%d:%d", &adjustedHour, &adjustedMinute);
            }
        }
        
        // Check if time display area was touched
        /*if (isPointInRect(touchX, touchY, timeDisplayRect) && !adjustingTime) {
            adjustingTime = true;
            // Reset to current time when opening popup
            if (timeMgr) {
                std::string currentTime = timeMgr->getCurrentTime();
                size_t colonPos = currentTime.find(':');
                if (colonPos != std::string::npos) {
                    adjustedHour = std::stoi(currentTime.substr(0, colonPos));
                    adjustedMinute = std::stoi(currentTime.substr(colonPos + 1));
                }
            }
        }*/
        
        // Handle popup button touches
        else { 
            if (isPointInRect(touchX, touchY, HPlusRect)) {
                adjustedHour = (adjustedHour + 1) % 24;
            } else if (isPointInRect(touchX, touchY, HMinusRect)) {
                adjustedHour = (adjustedHour - 1 + 24) % 24;
            } else if (isPointInRect(touchX, touchY, MPlusRect)) {
                adjustedMinute = (adjustedMinute + 1) % 60;
            } else if (isPointInRect(touchX, touchY, MMinusRect)) {
                adjustedMinute = (adjustedMinute - 1 + 60) % 60;
            } else if (isPointInRect(touchX, touchY, SaveRect)) {
                std::ostringstream oss;
                oss << std::setfill('0') << std::setw(2) << adjustedHour << ":"
                    << std::setfill('0') << std::setw(2) << adjustedMinute;
                
                if (currentMode == AdjustmentMode::ADJUST_TIME) {
                    if (timeMgr) {
                        timeMgr->setTime(oss.str());
                    }
                } else if (currentMode == AdjustmentMode::ADJUST_ALARM) {
                    if (alarmMgr) {
                        alarmMgr->setAlarm(oss.str());
                        alarmMgr->saveToStorage(); 
                    }
                }
                currentMode = AdjustmentMode::NONE; // Close popup
            } else if (isPointInRect(touchX, touchY, CancelRect)) {
                currentMode = AdjustmentMode::NONE; // Close popup
            }
        }
    } else if (e.type == SDL_FINGERUP || e.type == SDL_MOUSEBUTTONUP) {
        touched = false;
    }
}


void MainPage::renderText(SDL_Renderer* renderer, const std::string& text, int x, int y, SDL_Color color, TTF_Font* fontToUse) {
    if (!fontToUse) return;
    SDL_Surface* surface = TTF_RenderText_Blended(fontToUse, text.c_str(), color);
    if (!surface) return;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect rect = { x, y, surface->w, surface->h };
    SDL_RenderCopy(renderer, texture, nullptr, &rect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void MainPage::renderAdjustPopup(SDL_Renderer* renderer, const std::string& title) {
    // Semi-transparent overlay
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
    SDL_Rect overlay = {0, 0, 720, 1280};
    SDL_RenderFillRect(renderer, &overlay);
    
    // Popup background
    SDL_Rect popup = { 60, 350, 600, 580 };
    SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
    SDL_RenderFillRect(renderer, &popup);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &popup);

    SDL_Color black = {0, 0, 0, 255};
    SDL_Color white = {255, 255, 255, 255};
    
    renderText(renderer, title, 250, 380, black, font);

    // Display current adjusted time in large format
    std::ostringstream timeDisplay;
    timeDisplay << std::setfill('0') << std::setw(2) << adjustedHour << ":"
                << std::setfill('0') << std::setw(2) << adjustedMinute;
    renderText(renderer, timeDisplay.str(), 240, 470, black, font);

    // Define button positions - larger and better spaced
    int btnWidth = 120;
    int btnHeight = 80;
    int btnY1 = 580;
    int centerX = 360;
    int spacing = 140;
    
    HPlusRect = { centerX - spacing - btnWidth/2, btnY1, btnWidth, btnHeight };
    HMinusRect = { centerX - spacing - btnWidth/2, btnY1 + 100, btnWidth, btnHeight };
    MPlusRect = { centerX + spacing - btnWidth/2, btnY1, btnWidth, btnHeight };
    MMinusRect = { centerX + spacing - btnWidth/2, btnY1 + 100, btnWidth, btnHeight };

    // Draw hour buttons
    SDL_SetRenderDrawColor(renderer, 100, 150, 255, 255);
    SDL_RenderFillRect(renderer, &HPlusRect);
    SDL_RenderFillRect(renderer, &HMinusRect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &HPlusRect);
    SDL_RenderDrawRect(renderer, &HMinusRect);

    // Draw minute buttons
    SDL_SetRenderDrawColor(renderer, 100, 255, 150, 255);
    SDL_RenderFillRect(renderer, &MPlusRect);
    SDL_RenderFillRect(renderer, &MMinusRect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &MPlusRect);
    SDL_RenderDrawRect(renderer, &MMinusRect);

    // Button labels
    renderText(renderer, "H+", HPlusRect.x + 35, HPlusRect.y + 20, white, smallFont);
    renderText(renderer, "H-", HMinusRect.x + 35, HMinusRect.y + 20, white, smallFont);
    renderText(renderer, "M+", MPlusRect.x + 35, MPlusRect.y + 20, white, smallFont);
    renderText(renderer, "M-", MMinusRect.x + 35, MMinusRect.y + 20, white, smallFont);
    
    // Labels
    renderText(renderer, "Hour", HPlusRect.x + 25, HPlusRect.y - 35, black, smallFont);
    renderText(renderer, "Minute", MPlusRect.x + 10, MPlusRect.y - 35, black, smallFont);

    // Save and Cancel buttons
    SaveRect = { 120, 820, 200, 70 };
    CancelRect = { 400, 820, 200, 70 };
    
    SDL_SetRenderDrawColor(renderer, 50, 200, 50, 255);
    SDL_RenderFillRect(renderer, &SaveRect);
    SDL_SetRenderDrawColor(renderer, 200, 50, 50, 255);
    SDL_RenderFillRect(renderer, &CancelRect);
    
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &SaveRect);
    SDL_RenderDrawRect(renderer, &CancelRect);
    
    renderText(renderer, "Save", SaveRect.x + 50, SaveRect.y + 15, white, smallFont);
    renderText(renderer, "Cancel", CancelRect.x + 35, CancelRect.y + 15, white, smallFont);
}

bool MainPage::isPointInRect(int x, int y, const SDL_Rect& rect) {
    return x >= rect.x && x <= rect.x + rect.w && y >= rect.y && y <= rect.y + rect.h;
}

void MainPage::drawCircle(SDL_Renderer* renderer, int centerX, int centerY, int radius) {
    for (int w = 0; w < radius * 2; w++) {
        for (int h = 0; h < radius * 2; h++) {
            int dx = radius - w;
            int dy = radius - h;
            if ((dx*dx + dy*dy) <= (radius * radius)) {
                SDL_RenderDrawPoint(renderer, centerX + dx, centerY + dy);
            }
        }
    }
}