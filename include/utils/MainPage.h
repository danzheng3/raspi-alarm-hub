#include "page.h"
#include "managers/timeManager.h"
#include "managers/alarmManager.h"
#include "managers/connectivityManager.h"
#include <SDL2/SDL_ttf.h>


class MainPage : public Page {
public:
    MainPage(timeManager* timeMgr = nullptr, alarmManager* alarmMgr = nullptr, connectivityManager* connMgr = nullptr);
    ~MainPage();
    void render(SDL_Renderer* renderer) override;
    SDL_Rect renderLabeledBox(SDL_Renderer* renderer, const std::string& text, int x, int y, bool drawBox, bool* wasTouched);
    void handleEvent(const SDL_Event& event) override;

private:

    enum class AdjustmentMode {
        NONE,
        ADJUST_TIME,
        ADJUST_ALARM
    };
    timeManager* timeMgr;
    alarmManager* alarmMgr;
    connectivityManager* connMgr;
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

    void renderText(SDL_Renderer* renderer, const std::string& text, int x, int y, SDL_Color color, TTF_Font* fontToUse);
    void renderAdjustPopup(SDL_Renderer* renderer, const std::string& title);


    bool isPointInRect(int x, int y, const SDL_Rect& rect);
    void drawCircle(SDL_Renderer* renderer, int centerX, int centerY, int radius);

    SDL_Rect HPlusRect, HMinusRect, MPlusRect, MMinusRect;
    SDL_Rect SaveRect, CancelRect;
    SDL_Rect timeDisplayRect;
    SDL_Rect alarmDisplayRect;
    SDL_Color Black = {0,0,0,255};

};
