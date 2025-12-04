#include "managers/weatherManager.h"
#include <curl/curl.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <nlohmann/json.hpp>

weatherManager::weatherManager(storageManager& storage, EventBus* eventBus)
    : storage(storage), m_eventBus(eventBus), autoUpdateEnabled(false) {

    currentWeather.valid = false;
    currentWeather.temperature = 0;

    cachedLat = 40.4237; // WL latitude / longitude
    cachedLon = -86.9212;
    
    std::cout << "Weathermanager initialized using WL coordinates" << std::endl;

}

weatherManager::~weatherManager() {
    stopAutoUpdate();
}

size_t weatherManager::writeCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t totalSize = size * nmemb;
    userp->append((char*)contents, totalSize);
    return totalSize;
}

bool weatherManager::fetchWeather(double latitude, double longitude) {
    CURL* curl = curl_easy_init();

    if (!curl) {
        std::cerr << "Failed to initialize CURL" << std::endl;
        return false;
    } else {
        std::ostringstream urlStream;
        urlStream << "https://api.open-meteo.com/v1/forecast?"
                << "latitude=" << std::fixed << std::setprecision(4) << latitude
                << "&longitude=" << std::fixed << std::setprecision(4) << longitude
                << "&current=temperature_2m,relative_humidity_2m,precipitation,weather_code,wind_speed_10m"
                << "&temperature_unit=fahrenheit"
                << "&wind_speed_unit=mph"
                << "&precipitation_unit=inch";
        
        std::string url = urlStream.str();
        std::string response;
        
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);


        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            return false;
        }

        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        if (response_code != 200) {
            std::cerr << "Error: Received HTTP response code " << response_code << std::endl;
            return false;
        }

        if (parseWeatherResponse(response)) {
            cachedLat = latitude;
            cachedLon = longitude;
            return true;
        } else {
            return false;
        }
    }
}

bool weatherManager::parseWeatherResponse(const std::string& response) {
    try {
        auto json = nlohmann::json::parse(response);
        if (!json.contains("current")) {
            std::cerr << "[Error] missing 'current' field in weather response" << std::endl;
            return false;
        }
        auto current = json["current"];

        currentWeather.temperature = current["temperature"].get<double>();
        currentWeather.weatherCode = current["weathercode"].get<int>(); //MAY NEED TO FIX THIS
        currentWeather.condition = weatherCodeToString(currentWeather.weatherCode);
        currentWeather.humidity = current["relative_humidity_2m"].get<int>();
        currentWeather.windSpeed = current["wind_speed_10m"].get<double>();
        currentWeather.precipitation = current["precipitation"].get<double>();
        currentWeather.valid = true;

        if (m_eventBus) {
            WeatherUpdatedEvent event;
            event.temperature = currentWeather.temperature;
            event.condition = currentWeather.condition;
            m_eventBus->publish(event);
        }

        std::cout << "Weather updated: " << currentWeather.temperature << "F, " 
                  << currentWeather.condition << std::endl;

        return true;

    } catch (const std::exception& e) {
        std::cerr << "[Error] parsing weather response: " << e.what() << std::endl;
        return false;
    }
}

// auto-update
void weatherManager::startAutoUpdate(int intervalMinutes) {
    autoUpdateEnabled = true;
    intervalMinutes = 1; // TESTING PURPOSES ONLY - CHANGE BACK TO 30
    updateThread = std::thread([this, intervalMinutes]() {
        while (autoUpdateEnabled) {
            fetchWeather(cachedLat, cachedLon);
            
            // Sleep for specified interval
            for (int i = 0; i < intervalMinutes * 60 && autoUpdateEnabled; i++) {
                std::this_thread::sleep_for(std::chrono::seconds(10));
            }
        }
    });
    
    std::cout << "Weather auto-update started (interval: " << intervalMinutes << " minutes)" << std::endl;
}

void weatherManager::stopAutoUpdate() {
    autoUpdateEnabled = false;
    if (updateThread.joinable()) {
        updateThread.join();
    }
}


// weather opcode info

std::string weatherManager::weatherCodeToString(int code) const {
    WeatherCode wCode = static_cast<WeatherCode>(code);
    switch (wCode) {
        case WeatherCode::CLEAR_SKY:
            return "CLEAR_SKY";
        case WeatherCode::MAINLY_CLEAR:
            return "MAINLY_CLEAR";
        case WeatherCode::PARTLY_CLOUDY:
            return "PARTLY_CLOUDY";
        case WeatherCode::OVERCAST:
            return "OVERCAST";
        case WeatherCode::FOG:
            return "FOG";
        case WeatherCode::DEPOSITING_RIME_FOG:
            return "DEPOSITING_RIME_FOG";
        case WeatherCode::DRIZZLE_LIGHT:
            return "DRIZZLE_LIGHT";
        case WeatherCode::DRIZZLE_MODERATE:
            return "DRIZZLE_MODERATE";
        case WeatherCode::DRIZZLE_DENSE:
            return "DRIZZLE_DENSE";
        case WeatherCode::FREEZING_DRIZZLE_LIGHT:
            return "FREEZING_DRIZZLE_LIGHT";
        case WeatherCode::FREEZING_DRIZZLE_DENSE:
            return "FREEZING_DRIZZLE_DENSE";
        case WeatherCode::RAIN_SLIGHT:
            return "RAIN_SLIGHT";
        case WeatherCode::RAIN_MODERATE:
            return "RAIN_MODERATE";
        case WeatherCode::RAIN_HEAVY:
            return "RAIN_HEAVY";
        case WeatherCode::FREEZING_RAIN_LIGHT:
            return "FREEZING_RAIN_LIGHT";
        case WeatherCode::FREEZING_RAIN_HEAVY:
            return "FREEZING_RAIN_HEAVY";
        case WeatherCode::SNOW_SLIGHT:
            return "SNOW_SLIGHT";
        case WeatherCode::SNOW_MODERATE:
            return "SNOW_MODERATE";
        case WeatherCode::SNOW_HEAVY:
            return "SNOW_HEAVY";
        case WeatherCode::SNOW_GRAINS:
            return "SNOW_GRAINS";
        case WeatherCode::RAIN_SHOWERS_SLIGHT:
            return "RAIN_SHOWERS_SLIGHT";
        case WeatherCode::RAIN_SHOWERS_MODERATE:
            return "RAIN_SHOWERS_MODERATE";
        case WeatherCode::RAIN_SHOWERS_VIOLENT:
            return "RAIN_SHOWERS_VIOLENT";
        case WeatherCode::SNOW_SHOWERS_SLIGHT:
            return "SNOW_SHOWERS_SLIGHT";
        case WeatherCode::SNOW_SHOWERS_HEAVY:
            return "SNOW_SHOWERS_HEAVY";
        case WeatherCode::THUNDERSTORM:
            return "THUNDERSTORM";
        case WeatherCode::THUNDERSTORM_SLIGHT_HAIL:
            return "THUNDERSTORM_SLIGHT_HAIL";
        case WeatherCode::THUNDERSTORM_HEAVY_HAIL:
            return "THUNDERSTORM_HEAVY_HAIL";
        default:
            return "UNKNOWN";
    }
}