#include "page.h"
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <sstream>
#include "managers/timeManager.h"
#include "managers/alarmManager.h"
#include "managers/connectivityManager.h"
#include "managers/powerManager.h"
#include "managers/weatherManager.h"
#include "managers/display_manager.h"

#pragma once



class MainPage : public Page {
public:
    MainPage(timeManager* timeMgr = nullptr, alarmManager* alarmMgr = nullptr, connectivityManager* connMgr = nullptr, weatherManager* weatherMgr = nullptr, powerManager* powerMgr = nullptr);
    ~MainPage();
    virtual PageType getPageRequest() override { return pageChangeRequest; }
    void render(SDL_Renderer* renderer) override;
    SDL_Rect renderLabeledBox(SDL_Renderer* renderer, const std::string& text, int x, int y, bool drawBox, bool* wasTouched);
    void handleEvent(const SDL_Event& event) override;

private:

    enum class AdjustmentMode {
        NONE,
        ADJUST_TIME,
        ADJUST_ALARM
    };

    PageType pageChangeRequest = PageType::NONE;

    timeManager* timeMgr;
    alarmManager* alarmMgr;
    connectivityManager* connMgr;
    weatherManager* weatherMgr;
    powerManager* powerMgr;

    AdjustmentMode currentMode = AdjustmentMode::NONE;

    int adjustedHour =0;
    int adjustedMinute =0;

    bool touched = false;
    int touchX = 0;
    int touchY = 0;

    bool adjustingTime = false;
    bool adjustingAlarm = false;
    TTF_Font* font = nullptr;
    TTF_Font* smallFont = nullptr;
    TTF_Font* tinyFont = nullptr;

    void renderText(SDL_Renderer* renderer, const std::string& text, int x, int y, SDL_Color color, TTF_Font* fontToUse);
    void renderAdjustPopup(SDL_Renderer* renderer, const std::string& title);
    void renderCenteredText(SDL_Renderer* renderer, const std::string& text, int y, SDL_Color color, TTF_Font* font);

    bool isPointInRect(int x, int y, const SDL_Rect& rect);
    void drawCircle(SDL_Renderer* renderer, int centerX, int centerY, int radius);

    // Navigation buttons
    SDL_Rect musicButton, settingsButton, alarmButton, wifiButton;
    SDL_Rect weatherRect;
    
    // Long press detection
    bool isPressing = false;
    std::chrono::steady_clock::time_point pressStartTime;

    void renderIconButton(SDL_Renderer* renderer, const SDL_Rect& rect, 
                         const std::string& icon, const std::string& label, 
                         SDL_Color color);
    void handlePopupButtons(int x, int y);
    std::string getWeatherIconPath(int weatherCode);


    SDL_Rect HPlusRect, HMinusRect, MPlusRect, MMinusRect;
    SDL_Rect SaveRect, CancelRect;
    SDL_Rect timeDisplayRect;
    SDL_Rect alarmDisplayRect;
    SDL_Color Black = {0,0,0,255};

};
