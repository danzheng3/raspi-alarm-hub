#include "MainPage.h"
#include <SDL2/SDL.h>
#include <iomanip>

MainPage::MainPage(timeManager* timeMgr, alarmManager* alarmMgr, connectivityManager* connMgr,
                   weatherManager* weatherMgr, powerManager* powerMgr) 
    : timeMgr(timeMgr), alarmMgr(alarmMgr), connMgr(connMgr), weatherMgr(weatherMgr), powerMgr(powerMgr) 
{
    font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 48);
    smallFont = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 36);
    tinyFont = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 24);

    if (timeMgr) {
        std::string currentTime = timeMgr->getFormattedTime();
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

std::string MainPage::getWeatherIconPath(int weatherCode) {
    // Map weather codes to icon files
    if (weatherCode == 0 || weatherCode == 1) return "/home/daniel/Downloads/raspi-alarm-hub/images/sunny-weather.png";
    if (weatherCode == 2 || weatherCode == 3) return "/home/daniel/Downloads/raspi-alarm-hub/images/cloudy.png";
    //if (weatherCode >= 45 && weatherCode <= 48) return "../../images/fog.png"; // add if this becomes an issue
    if (weatherCode >= 51 && weatherCode <= 67) return "/home/daniel/raspi-alarm-hub/images/rainy.png";
    if (weatherCode >= 71 && weatherCode <= 77) return "/home/daniel/raspi-alarm-hub/images/snow_icon.png";
    if (weatherCode >= 80 && weatherCode <= 82) return "/home/daniel/raspi-alarm-hub/images/rainy.png";
    if (weatherCode >= 85 && weatherCode <= 86) return "/home/daniel/raspi-alarm-hub/images/snow_icon.png";
    //if (weatherCode >= 95) return "../../images/thunder.png"; // add if needed
    return "~/Downloads/raspi-alarm-hub/images/cloudy.png"; // default
}

void MainPage::render(SDL_Renderer* renderer) {
    // Gradient background (dark blue to darker blue)
    for (int y = 0; y < 1280; y++) {
        int blue = 35 + (y * 15) / 1280;
        SDL_SetRenderDrawColor(renderer, 15, 20, blue, 255);
        SDL_RenderDrawLine(renderer, 0, y, 720, y);
    }
    
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color lightGray = {200, 200, 200, 255};
    SDL_Color dimGray = {150, 150, 150, 255};
    SDL_Color accent = {100, 150, 255, 255};
    
    // Status bar (top)
    SDL_SetRenderDrawColor(renderer, 25, 30, 45, 200);
    SDL_Rect statusBar = {0, 0, 720, 60};
    SDL_RenderFillRect(renderer, &statusBar);
    
    // WiFi status (left side of status bar)
    std::string wifiStatus = connMgr ? 
        (connMgr->isWifiConnected() ? "WiFi Connected" : "WiFi Disconnected") : "WiFi ?";
    renderText(renderer, wifiStatus, 20, 15, 
               connMgr && connMgr->isWifiConnected() ? accent : dimGray, tinyFont);
    
    // Date (right side of status bar)
    if (timeMgr) {
        std::string dateStr = timeMgr->getFormattedDate("%b %d, %Y");
        renderText(renderer, dateStr, 520, 15, lightGray, tinyFont);
    }
    
    // Large time display (center-top)
    if (timeMgr) {
        std::string timeStr = timeMgr->getFormattedTime();
        int boxWidth = 520;
        int boxX = (720 - boxWidth) / 2;
        timeDisplayRect = {boxX, 100, boxWidth, 150};
        
        // Semi-transparent background
        SDL_SetRenderDrawColor(renderer, 40, 45, 65, 180);
        SDL_RenderFillRect(renderer, &timeDisplayRect);
        SDL_SetRenderDrawColor(renderer, accent.r, accent.g, accent.b, 255);
        for (int i = 0; i < 3; i++) {
            SDL_Rect outline = {timeDisplayRect.x - i, timeDisplayRect.y - i, 
                                timeDisplayRect.w + 2*i, timeDisplayRect.h + 2*i};
            SDL_RenderDrawRect(renderer, &outline);
        }
        
        
        renderCenteredText(renderer, timeStr, 130, white, font);    
    }
    
    // Weather display (center)
    if (weatherMgr) {
        WeatherData weather = weatherMgr->getWeatherData();
        if (weather.valid) {
            int weatherBoxWidth = 320;
            int weatherBoxX = (720 - weatherBoxWidth) / 2;
            weatherRect = {weatherBoxX, 300, weatherBoxWidth, 200};
            
            SDL_SetRenderDrawColor(renderer, 40, 45, 65, 180);
            SDL_RenderFillRect(renderer, &weatherRect);
            SDL_SetRenderDrawColor(renderer, 80, 85, 100, 255);
            SDL_RenderDrawRect(renderer, &weatherRect);
            
            // Weather icon
            int iconSize = 80;
            int iconX = weatherRect.x + (weatherRect.w - iconSize) / 2;
            int iconY = weatherRect.y + 15; // 15px padding from top

            std::string iconPath = getWeatherIconPath(weather.weatherCode);
            SDL_Surface* iconSurface = IMG_Load(iconPath.c_str());
            if (iconSurface) {
                SDL_Texture* iconTexture = SDL_CreateTextureFromSurface(renderer, iconSurface);
                SDL_Rect iconRect = {iconX, iconY, iconSize, iconSize};
                SDL_RenderCopy(renderer, iconTexture, nullptr, &iconRect);
                SDL_DestroyTexture(iconTexture);
                SDL_FreeSurface(iconSurface);
            } else {
                std::cerr << "[DisplayMgr] weather icon surface fail: " << IMG_GetError()  << std::endl;
            }
            
            // Temperature
            std::ostringstream tempStr;
            tempStr << (int)weather.temperature << " F";
            renderCenteredText(renderer, tempStr.str(), weatherRect.y + 110, white, smallFont);
            
            // Condition
            renderCenteredText(renderer, weather.condition, weatherRect.y + 160, lightGray, tinyFont);
        }
    }
    
    // Alarm display (below weather)
    alarmDisplayRect = {150, 550, 420, 100};
    SDL_SetRenderDrawColor(renderer, 40, 45, 65, 180);
    SDL_RenderFillRect(renderer, &alarmDisplayRect);
    SDL_SetRenderDrawColor(renderer, 80, 85, 100, 255);
    SDL_RenderDrawRect(renderer, &alarmDisplayRect);
    
    std::string alarmLabel = "Alarm: ";
    std::string alarmTime = alarmMgr ? 
        (alarmMgr->isAlarmEnabled() ? alarmMgr->getAlarmTime() : "Not Set") : "?";
    renderText(renderer, alarmLabel + alarmTime, alarmDisplayRect.x + 50, 
               alarmDisplayRect.y + 30, white, smallFont);
    
    // Navigation buttons (bottom)
    int btnY = 1050;
    int btnW = 150;
    int btnH = 150;
    int spacing = (720 - 4 * btnW) / 5;
    
    musicButton = {spacing, btnY, btnW, btnH};
    settingsButton = {spacing * 2 + btnW, btnY, btnW, btnH};
    alarmButton = {spacing * 3 + btnW * 2, btnY, btnW, btnH};
    wifiButton = {spacing * 4 + btnW * 3, btnY, btnW, btnH};
    
    renderIconButton(renderer, musicButton, "", "Music", accent);
    renderIconButton(renderer, settingsButton, "", "Settings", accent);
    renderIconButton(renderer, alarmButton, "", "Alarm", accent);
    renderIconButton(renderer, wifiButton, "", "WiFi", accent);
    
    // Touch feedback
    if (touched) {
        SDL_SetRenderDrawColor(renderer, 100, 150, 255, 100);
        for (int r = 10; r < 50; r += 5) {
            drawCircle(renderer, touchX, touchY, r);
        }
    }
    
    // Time/Alarm adjustment popup
    if (currentMode == AdjustmentMode::ADJUST_TIME) {
        renderAdjustPopup(renderer, "Set Time");
    } else if (currentMode == AdjustmentMode::ADJUST_ALARM) {
        renderAdjustPopup(renderer, "Set Alarm");
    }
    
    SDL_RenderPresent(renderer);
}

void MainPage::renderCenteredText(SDL_Renderer* renderer, const std::string& text, int y, SDL_Color color, TTF_Font* font) {
    if (!font || text.empty()) return;
    
    int w, h;
    TTF_SizeText(font, text.c_str(), &w, &h);
    
    int x = (720 - w) / 2; // Center based on screen width of 720
    
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), color);
    if (surface) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_Rect rect = {x, y, w, h};
        SDL_RenderCopy(renderer, texture, nullptr, &rect);
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);
    }
}

void MainPage::renderIconButton(SDL_Renderer* renderer, const SDL_Rect& rect, 
                                const std::string& icon, const std::string& label, 
                                SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 200);
    SDL_RenderFillRect(renderer, &rect);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &rect);
    
    SDL_Color white = {255, 255, 255, 255};
    renderText(renderer, icon, rect.x + rect.w/2 - 20, rect.y + 30, white, font);
    renderText(renderer, label, rect.x + rect.w/2 - 40, rect.y + 100, white, tinyFont);
}

void MainPage::handleEvent(const SDL_Event& e) {
    if (e.type == SDL_FINGERDOWN || e.type == SDL_MOUSEBUTTONDOWN) {
        touched = true;
        if (e.type == SDL_MOUSEBUTTONDOWN) {
            touchX = e.button.x;
            touchY = e.button.y;
        } else {
            touchX = static_cast<int>(e.tfinger.x * 720);
            touchY = static_cast<int>(e.tfinger.y * 1280);
        }
        
        if (currentMode == AdjustmentMode::NONE) {
            // Check navigation buttons
            if (isPointInRect(touchX, touchY, musicButton)) {
                pageChangeRequest = PageType::MUSIC;
                return;
            }
            if (isPointInRect(touchX, touchY, wifiButton)) {
                pageChangeRequest = PageType::SETTINGS;
                return;
            }
            
            // Long press on time to adjust
            if (isPointInRect(touchX, touchY, timeDisplayRect)) {
                pressStartTime = std::chrono::steady_clock::now();
                isPressing = true;
            }
            
            // Tap alarm to adjust
            if (isPointInRect(touchX, touchY, alarmDisplayRect)) {
                currentMode = AdjustmentMode::ADJUST_ALARM;
                if (alarmMgr) {
                    std::string alarmTime = alarmMgr->getAlarmTime();
                    sscanf(alarmTime.c_str(), "%d:%d", &adjustedHour, &adjustedMinute);
                }
            }

            if (isPointInRect(touchX, touchY, alarmButton)) {
                pageChangeRequest = PageType::ALARMS; // Navigate to the new page
                return;
            }
        } else {
            // Handle popup buttons (same as before)
            handlePopupButtons(touchX, touchY);
        }
    } else if (e.type == SDL_FINGERUP || e.type == SDL_MOUSEBUTTONUP) {
        if (isPressing) {
            auto pressDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - pressStartTime);
            
            if (pressDuration.count() > 500) {  // Long press (500ms)
                currentMode = AdjustmentMode::ADJUST_TIME;
                if (timeMgr) {
                    std::string currentTime = timeMgr->getFormattedTime();
                    sscanf(currentTime.c_str(), "%d:%d", &adjustedHour, &adjustedMinute);
                }
            }
            isPressing = false;
        }
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
    
    renderCenteredText(renderer, title, 380, black, font);

    // Display current adjusted time in large format
    std::ostringstream timeDisplay;
    timeDisplay << std::setfill('0') << std::setw(2) << adjustedHour << ":"
                << std::setfill('0') << std::setw(2) << adjustedMinute;
    renderCenteredText(renderer, timeDisplay.str(), 470, black, font);

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

    // Button internal labels (H+, M+, etc) - Centering these too
    int w, h;
    
    // Helper to center text inside a rect
    auto renderTextInRect = [&](const std::string& txt, const SDL_Rect& r, SDL_Color c, TTF_Font* f) {
        TTF_SizeText(f, txt.c_str(), &w, &h);
        renderText(renderer, txt, r.x + (r.w - w)/2, r.y + (r.h - h)/2, c, f);
    };

    // Button labels
    renderText(renderer, "H+", HPlusRect.x + 35, HPlusRect.y + 20, white, smallFont);
    renderText(renderer, "H-", HMinusRect.x + 35, HMinusRect.y + 20, white, smallFont);
    renderText(renderer, "M+", MPlusRect.x + 35, MPlusRect.y + 20, white, smallFont);
    renderText(renderer, "M-", MMinusRect.x + 35, MMinusRect.y + 20, white, smallFont);
    
    TTF_SizeText(smallFont, "Hour", &w, &h);
    renderText(renderer, "Hour", HPlusRect.x + (HPlusRect.w - w)/2, HPlusRect.y - 35, black, smallFont);

    TTF_SizeText(smallFont, "Minute", &w, &h);
    renderText(renderer, "Minute", MPlusRect.x + (MPlusRect.w - w)/2, MPlusRect.y - 35, black, smallFont);

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

SDL_Rect MainPage::renderLabeledBox(SDL_Renderer* renderer,
                                    const std:: string& text,
                                    int x, int y,
                                    bool drawBox,
                                    bool* wasTouched)
{
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), Black);
    if (!surface) return {x,y,0,0};

    
}

void MainPage::handlePopupButtons(int x, int y) {
    // Hour Adjustment
    if (isPointInRect(x, y, HPlusRect)) {
        adjustedHour = (adjustedHour + 1) % 24;
    } else if (isPointInRect(x, y, HMinusRect)) {
        adjustedHour = (adjustedHour - 1 + 24) % 24;
    }
    
    // Minute Adjustment
    if (isPointInRect(x, y, MPlusRect)) {
        adjustedMinute = (adjustedMinute + 1) % 60;
    } else if (isPointInRect(x, y, MMinusRect)) {
        adjustedMinute = (adjustedMinute - 1 + 60) % 60;
    }

    // Save Action
    if (isPointInRect(x, y, SaveRect)) {
        if (currentMode == AdjustmentMode::ADJUST_TIME && timeMgr) {
            // Preserve current date, only update HH:MM
            struct tm newTime = timeMgr->getCurrentTime();
            newTime.tm_hour = adjustedHour;
            newTime.tm_min = adjustedMinute;
            newTime.tm_sec = 0;
            timeMgr->setTime(newTime);
        } 
        else if (currentMode == AdjustmentMode::ADJUST_ALARM && alarmMgr) {
            char buffer[6];
            snprintf(buffer, sizeof(buffer), "%02d:%02d", adjustedHour, adjustedMinute);
            alarmMgr->setAlarm(std::string(buffer));
        }
        currentMode = AdjustmentMode::NONE;
    }

    // Cancel Action
    if (isPointInRect(x, y, CancelRect)) {
        currentMode = AdjustmentMode::NONE;
    }
}