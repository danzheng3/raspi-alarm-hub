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
    if (weatherCode >= 51 && weatherCode <= 67) return "/home/daniel/Downloads/raspi-alarm-hub/images/rainy.png";
    if (weatherCode >= 71 && weatherCode <= 77) return "/home/daniel/Downloads/raspi-alarm-hub/images/snow_icon.png";
    if (weatherCode >= 80 && weatherCode <= 82) return "/home/daniel/Downloads/raspi-alarm-hub/images/rainy.png";
    if (weatherCode >= 85 && weatherCode <= 86) return "/home/daniel/Downloads/raspi-alarm-hub/images/snow_icon.png";
    //if (weatherCode >= 95) return "../../images/thunder.png"; // add if needed
    return "~/Downloads/raspi-alarm-hub/images/cloudy.png"; // default
}

void MainPage::render(SDL_Renderer* renderer) {
    // Gradient background (horizontal stretch)
    for (int y = 0; y < 720; y++) {
        int blue = 35 + (y * 15) / 720;
        SDL_SetRenderDrawColor(renderer, 15, 20, blue, 255);
        SDL_RenderDrawLine(renderer, 0, y, 1280, y);
    }
    
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color lightGray = {200, 200, 200, 255};
    SDL_Color accent = {100, 150, 255, 255};
    
    // --- TOP STATUS BAR (Full Width) ---
    SDL_SetRenderDrawColor(renderer, 25, 30, 45, 200);
    SDL_Rect statusBar = {0, 0, 1280, 50};
    SDL_RenderFillRect(renderer, &statusBar);
    
    // WiFi status (Top Left)
    std::string wifiStatus = connMgr ? (connMgr->isWifiConnected() ? "WiFi Connected" : "WiFi Disconnected") : "WiFi ?";
    renderText(renderer, wifiStatus, 20, 10, connMgr && connMgr->isWifiConnected() ? accent : lightGray, tinyFont);
    
    // Date (Top Right of the main area, not the sidebar)
    if (timeMgr) {
        std::string dateStr = timeMgr->getFormattedDate("%b %d, %Y");
        renderText(renderer, dateStr, 750, 10, lightGray, tinyFont);
    }

    // --- LEFT CONTENT AREA (0 to 980) ---
    
    // 1. Large Time Display
    if (timeMgr) {
        std::string timeStr = timeMgr->getFormattedTime();
        // Centered in the left zone (0-980)
        int boxWidth = 500;
        int boxX = (980 - boxWidth) / 2; 
        timeDisplayRect = {boxX, 100, boxWidth, 160};
        
        SDL_SetRenderDrawColor(renderer, 40, 45, 65, 180);
        SDL_RenderFillRect(renderer, &timeDisplayRect);
        
        // Draw Border
        SDL_SetRenderDrawColor(renderer, accent.r, accent.g, accent.b, 255);
        SDL_RenderDrawRect(renderer, &timeDisplayRect);
        
        // Center text in box
        int w, h;
        TTF_SizeText(font, timeStr.c_str(), &w, &h);
        renderText(renderer, timeStr, boxX + (boxWidth - w)/2, timeDisplayRect.y + 30, white, font);
    }
    
    // 2. Weather Display (Below Time)
    if (weatherMgr) {
        WeatherData weather = weatherMgr->getWeatherData();
        if (weather.valid) {
            int weatherBoxWidth = 400;
            int weatherBoxX = (980 - weatherBoxWidth) / 2;
            weatherRect = {weatherBoxX, 300, weatherBoxWidth, 180};
            
            SDL_SetRenderDrawColor(renderer, 40, 45, 65, 180);
            SDL_RenderFillRect(renderer, &weatherRect);
            
            // Icon
            int iconSize = 70;
            std::string iconPath = getWeatherIconPath(weather.weatherCode);
            SDL_Surface* iconSurface = IMG_Load(iconPath.c_str());
            if (iconSurface) {
                SDL_Texture* iconTexture = SDL_CreateTextureFromSurface(renderer, iconSurface);
                SDL_Rect iconRect = {weatherRect.x + 30, weatherRect.y + 55, iconSize, iconSize};
                SDL_RenderCopy(renderer, iconTexture, nullptr, &iconRect);
                SDL_DestroyTexture(iconTexture);
                SDL_FreeSurface(iconSurface);
            }

            // Temp & Condition text next to icon
            std::ostringstream tempStr;
            tempStr << (int)weather.temperature << " F";
            renderText(renderer, tempStr.str(), weatherRect.x + 130, weatherRect.y + 50, white, smallFont);
            renderText(renderer, weather.condition, weatherRect.x + 130, weatherRect.y + 100, lightGray, tinyFont);
        }
    }
    
    // 3. Alarm Status (Bottom Left)
    alarmDisplayRect = {240, 520, 500, 100};
    SDL_SetRenderDrawColor(renderer, 40, 45, 65, 180);
    SDL_RenderFillRect(renderer, &alarmDisplayRect);
    
    std::string alarmLabel = "Next Alarm: ";
    std::string alarmTime = alarmMgr ? (alarmMgr->isAlarmEnabled() ? alarmMgr->getAlarmTime() : "OFF") : "?";
    
    int aw, ah;
    std::string fullAlarmStr = alarmLabel + alarmTime;
    TTF_SizeText(smallFont, fullAlarmStr.c_str(), &aw, &ah);
    renderText(renderer, fullAlarmStr, alarmDisplayRect.x + (500-aw)/2, alarmDisplayRect.y + 30, white, smallFont);


    // --- RIGHT SIDEBAR (Navigation) ---
    int sidebarX = 980;
    int sidebarW = 300;
    
    // Background for sidebar
    SDL_SetRenderDrawColor(renderer, 30, 35, 50, 255);
    SDL_Rect sidebarBg = {sidebarX, 0, sidebarW, 720};
    SDL_RenderFillRect(renderer, &sidebarBg);
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderDrawLine(renderer, sidebarX, 0, sidebarX, 720);

    // Buttons stacked vertically
    int btnH = 120;
    int btnW = 240;
    int btnX = sidebarX + (sidebarW - btnW) / 2;
    int startY = 80;
    int gap = 30;

    musicButton = {btnX, startY, btnW, btnH};
    settingsButton = {btnX, startY + btnH + gap, btnW, btnH};
    alarmButton = {btnX, startY + (btnH + gap) * 2, btnW, btnH};
    wifiButton = {btnX, startY + (btnH + gap) * 3, btnW, btnH};

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
    
}

void MainPage::renderCenteredText(SDL_Renderer* renderer, const std::string& text, int y, SDL_Color color, TTF_Font* font) {
    if (!font || text.empty()) return;
    
    int w, h;
    TTF_SizeText(font, text.c_str(), &w, &h);
    
    // CHANGED: Use 1280 (Logical Landscape Width) instead of 720
    int x = (1280 - w) / 2; 
    
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
    // Draw Button Background
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 200);
    SDL_RenderFillRect(renderer, &rect);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &rect);
    
    SDL_Color white = {255, 255, 255, 255};
    int w, h;

    // Center the Icon (if you add icons later)
    if (!icon.empty()) {
        TTF_SizeText(font, icon.c_str(), &w, &h);
        renderText(renderer, icon, rect.x + (rect.w - w)/2, rect.y + 20, white, font);
    }

    // Center the Label
    // We calculate the width of the text, then subtract half of that from the center of the button
    if (tinyFont && !label.empty()) {
        TTF_SizeText(tinyFont, label.c_str(), &w, &h);
        int labelX = rect.x + (rect.w - w) / 2;
        int labelY = rect.y + (rect.h - h) / 2; // Perfectly vertically centered
        
        // If you have an icon, push the label down slightly
        if (!icon.empty()) labelY = rect.y + 80;

        renderText(renderer, label, labelX, labelY, white, tinyFont);
    }
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
    // 1. DIMENSIONS FOR 720p HEIGHT
    int popupW = 500;
    int popupH = 400; // Much shorter to fit in 720px
    int popupX = (1280 - popupW) / 2; // Center horizontally (Logical width is 1280)
    int popupY = (720 - popupH) / 2;  // Center vertically (Logical height is 720)

    // Semi-transparent overlay
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
    SDL_Rect overlay = {0, 0, 1280, 720};
    SDL_RenderFillRect(renderer, &overlay);
    
    // Popup background
    SDL_Rect popup = { popupX, popupY, popupW, popupH };
    SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
    SDL_RenderFillRect(renderer, &popup);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &popup);

    SDL_Color black = {0, 0, 0, 255};
    SDL_Color white = {255, 255, 255, 255};
    
    // Title
    auto drawCenteredInPopup = [&](const std::string& str, int yOffset, TTF_Font* f) {
    int w, h;
    TTF_SizeText(f, str.c_str(), &w, &h);
    // Center X = PopupX + (PopupWidth - TextWidth) / 2
    int textX = popupX + (popupW - w) / 2; 
    int textY = popupY + yOffset;
    renderText(renderer, str, textX, textY, black, f);
    };

    // Draw Title
    drawCenteredInPopup(title, 20, font);

    // Draw Time
    std::ostringstream timeDisplay;
    timeDisplay << std::setfill('0') << std::setw(2) << adjustedHour << ":"
                << std::setfill('0') << std::setw(2) << adjustedMinute;
    drawCenteredInPopup(timeDisplay.str(), 80, font);

    // Buttons
    int btnSize = 60;
    int spacing = 30;
    int yButtons = popupY + 160;
    
    // Centered group of buttons
    int groupWidth = (btnSize * 4) + (spacing * 3); // Total width of 4 buttons
    int startX = popupX + (popupW - groupWidth) / 2;

    // H+, H-, M+, M-
    HPlusRect  = { startX, yButtons, btnSize, btnSize };
    HMinusRect = { startX + btnSize + spacing, yButtons, btnSize, btnSize };
    MPlusRect  = { startX + 2*(btnSize + spacing), yButtons, btnSize, btnSize };
    MMinusRect = { startX + 3*(btnSize + spacing), yButtons, btnSize, btnSize };

    // Draw adjustment buttons
    auto drawBtn = [&](SDL_Rect r, const char* lbl, SDL_Color c) {
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 255);
        SDL_RenderFillRect(renderer, &r);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &r);
        int w,h; TTF_SizeText(smallFont, lbl, &w, &h);
        renderText(renderer, lbl, r.x + (r.w-w)/2, r.y + (r.h-h)/2, white, smallFont);
    };

    SDL_Color blue = {100, 150, 255, 255};
    SDL_Color green = {100, 200, 150, 255};

    drawBtn(HPlusRect, "H+", blue);
    drawBtn(HMinusRect, "H-", blue);
    drawBtn(MPlusRect, "M+", green);
    drawBtn(MMinusRect, "M-", green);

    // Save / Cancel Buttons at bottom
    int actionBtnW = 150;
    int actionBtnH = 50;
    int actionY = popupY + popupH - 70;

    SaveRect = { popupX + 50, actionY, actionBtnW, actionBtnH };
    CancelRect = { popupX + popupW - 50 - actionBtnW, actionY, actionBtnW, actionBtnH };

    drawBtn(SaveRect, "Save", {50, 200, 50, 255});
    drawBtn(CancelRect, "Cancel", {200, 50, 50, 255});
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