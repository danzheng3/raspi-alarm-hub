#pragma once
#include <string>
#include <memory>
#include <thread>
#include <atomic>
#include "events/EventBus.h"
#include "events/Events.h"
#include "storageManager.h"

// Weather code mapping for Open-Meteo
enum class WeatherCode {
    CLEAR_SKY = 0,
    MAINLY_CLEAR = 1,
    PARTLY_CLOUDY = 2,
    OVERCAST = 3,
    FOG = 45,
    DEPOSITING_RIME_FOG = 48,
    DRIZZLE_LIGHT = 51,
    DRIZZLE_MODERATE = 53,
    DRIZZLE_DENSE = 55,
    FREEZING_DRIZZLE_LIGHT = 56,
    FREEZING_DRIZZLE_DENSE = 57,
    RAIN_SLIGHT = 61,
    RAIN_MODERATE = 63,
    RAIN_HEAVY = 65,
    FREEZING_RAIN_LIGHT = 66,
    FREEZING_RAIN_HEAVY = 67,
    SNOW_SLIGHT = 71,
    SNOW_MODERATE = 73,
    SNOW_HEAVY = 75,
    SNOW_GRAINS = 77,
    RAIN_SHOWERS_SLIGHT = 80,
    RAIN_SHOWERS_MODERATE = 81,
    RAIN_SHOWERS_VIOLENT = 82,
    SNOW_SHOWERS_SLIGHT = 85,
    SNOW_SHOWERS_HEAVY = 86,
    THUNDERSTORM = 95,
    THUNDERSTORM_SLIGHT_HAIL = 96,
    THUNDERSTORM_HEAVY_HAIL = 99
};

struct WeatherData {
    double temperature;          // in Fahrenheit
    int weatherCode;            // Open-Meteo weather code
    std::string condition;      // Human-readable condition
    int humidity;               // Percentage
    double windSpeed;           // mph
    double precipitation;       // inches
    bool valid;
};

class weatherManager {
public:
    weatherManager(storageManager& storage, EventBus* eventBus);
    ~weatherManager();
    
    bool fetchWeather(double latitude, double longitude); // API call
    
    bool fetchWeatherByCity(const std::string& city);
    
    // Get cached weather data
    WeatherData getWeatherData() const { return currentWeather; }
    
    // Auto-update weather at regular intervals
    void startAutoUpdate(int intervalMinutes = 30);
    void stopAutoUpdate();

private:
    storageManager& storage;
    EventBus* m_eventBus;
    WeatherData currentWeather;
    
    double cachedLat;
    double cachedLon;
    std::atomic<bool> autoUpdateEnabled;
    std::thread updateThread;
    
    // libcurl callback
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* userp);
    
    // Parse JSON response
    bool parseWeatherResponse(const std::string& response);
    
    // Convert weather code to human-readable string
    std::string weatherCodeToString(int code) const;
    
    // Geocoding helper
    bool geocodeCity(const std::string& city, double& lat, double& lon);
};