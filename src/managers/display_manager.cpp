#include "utils/page.h"
#include "utils/MainPage.h"
#include "utils/MusicPage.h"
#include "utils/SettingsPage.h"
#include <managers/display_manager.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>


DisplayManager::DisplayManager(timeManager* timeMgr, alarmManager* alarmMgr, 
                                connectivityManager* connMgr, powerManager* pwrMgr, 
                                audioManager* audioMgr, weatherManager* weatherMgr,
                                EventBus* eventBus)
    : timeMgr(timeMgr), alarmMgr(alarmMgr), connMgr(connMgr), 
    pwrMgr(pwrMgr), audioMgr(audioMgr), weatherMgr(weatherMgr),
    window(nullptr), renderer(nullptr), font(nullptr), 
    weatherIcon(nullptr), currentPage(nullptr), m_eventBus(eventBus)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) < 0) {
        std::cerr << "Could not initialize SDL: " << SDL_GetError() << std::endl;
        return;
    }

    if (TTF_Init() == -1) {
        std::cerr << "Could not initialize TTF: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return;
    }

    if (IMG_Init(IMG_INIT_PNG) == 0) {
        std::cerr << "Could not initialize SDL_image: " << IMG_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return;
    }

    window = SDL_CreateWindow("Raspi Alarm Hub", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 720, 1280, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Could not create window: " << SDL_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    std::cout << "displaymanager initialized" << std::endl;

    if (m_eventBus) {
        m_eventBus->subscribe<AlarmTriggeredEvent>(this, &DisplayManager::onAlarmTriggered);
        m_eventBus->subscribe<AlarmClearedEvent>(this, &DisplayManager::onAlarmCleared);
        m_eventBus->subscribe<AlarmSetEvent>(this, &DisplayManager::onAlarmSet);
        m_eventBus->subscribe<TimeUpdatedEvent>(this, &DisplayManager::onTimeUpdated);
        m_eventBus->subscribe<WifiStatusChangedEvent>(this, &DisplayManager::onWifiStatusChanged);
        m_eventBus->subscribe<WeatherUpdatedEvent>(this, &DisplayManager::onWeatherUpdated);
        m_eventBus->subscribe<ScreenBrightnessChanged>(this, &DisplayManager::onBrightnessChanged);

    }

    if (timeMgr) {
        std::lock_guard<std::mutex> lock(stateMutex);
        m_currentTime = timeMgr->getFormattedTime();
    }
    changePage(PageType::MAIN);

}

DisplayManager::~DisplayManager() {
    if (currentPage) {
        delete currentPage;
    }
    if (weatherIcon) {
        SDL_DestroyTexture(weatherIcon);
    }
    if (font) {
        TTF_CloseFont(font);
    }
    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }
    if (window) {
        SDL_DestroyWindow(window);
    }
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
}

void DisplayManager::changePage(PageType newPage) {
    if (currentPage) {
        delete currentPage;
        currentPage = nullptr;
    }

    currentPageType = newPage;

    switch (newPage) {
        case PageType::MAIN:
            currentPage = new MainPage(timeMgr, alarmMgr, connMgr, weatherMgr);
            break;
        case PageType::MUSIC:
            currentPage = new MusicPage(audioMgr, m_eventBus);
            break;
        case PageType::SETTINGS:
            currentPage = new SettingsPage(connMgr);
            break;
        default:
            currentPage = new MainPage(timeMgr, alarmMgr, connMgr, weatherMgr);
            break;
    }
}

void DisplayManager::run(std::atomic<bool>& running) {
    SDL_Event event;
    Uint32 lastUpdateTime = SDL_GetTicks();
    const Uint32 updateInterval = 1000; // update every second

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_FINGERDOWN || event.type == SDL_MOUSEBUTTONDOWN) {
                if (pwrMgr) {
                    pwrMgr->registerActivity();
                }
                std::cout << "Touch detected at (" << event.tfinger.x << ", " << event.tfinger.y << ")" << std::endl;
            }
            
            if (currentPage) {
                currentPage->handleEvent(event);
            }
        }

        if (currentPage) {
            currentPage->render(renderer);
        }

        SDL_Delay(100);  // ~ 10 FPS
    }
}

// EVENT HANDLRS

void DisplayManager::onAlarmTriggered(const AlarmTriggeredEvent& event) {
    std::cout << "displaymanager: alarm triggered, show UI" << std::endl;
    setState(m_isAlarmActive, true);

    // change UI, trigger
}

void DisplayManager::onAlarmCleared(const AlarmClearedEvent& event) {
    std::cout << "Displaymanager: alarm cleared, returning to normal UI" << std::endl;
    setState(m_isAlarmActive, false);
}

void DisplayManager::onAlarmSet(const AlarmSetEvent& event) {
    std::cout << "Displaymanager: alarm set to " << event.newTime << std::endl;
    setState(m_alarmTime, event.newTime);

    //update display
}

void DisplayManager::onTimeUpdated(const TimeUpdatedEvent& event) {
    setState(m_currentTime, event.currentTime);
}

void DisplayManager::onWifiStatusChanged(const WifiStatusChangedEvent& event) {
    std::cout << "DisplayManager: WiFi status changed: " 
              << (event.isConnected ? "Connected" : "Disconnected") << std::endl;
    setState(m_isWifiConnected, event.isConnected);
    // Update WiFi icon in UI
}

void DisplayManager::onWeatherUpdated(const WeatherUpdatedEvent& event) {
    std::cout << "DisplayManager: Weather updated: " << event.temperature 
              << "°, " << event.condition << std::endl;
    setState(m_temperature, event.temperature);
    setState(m_weatherCondition, event.condition);
    // Update weather display
}

void DisplayManager::onBrightnessChanged(const ScreenBrightnessChanged& event) {
    std::cout << "DisplayManager: Brightness changed to " << event.brightness << "%" << std::endl;
    //setState(m_screenBrightness, event.brightness);
}

