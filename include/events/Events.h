#pragma once
#include <string>

/**
 * triggered by alarmManager
 * subscribed by audioManager (play sound), displayManager (show UI)
 */


 // ALARM EVENTS

struct AlarmTriggeredEvent {};
struct AlarmSetEvent{ std::string newTime; };
struct AlarmClearedEvent {};

// UI/INPUT events

struct UIStopAlarmPressedEvent{};
struct UIVolumeChanged{ int newVolume; };
struct UISongSelectedEvent {
    size_t songIndex;
};
struct UIEqualizerBandChanged {
    int bandIndex;      // 0-6
    uint8_t value;      // 0-255
};

 
//Hardware/Connectivity

struct SpeakerDockedEvent{};
struct SpeakerUndockedEvent{};
struct BluetoothSpeakerConnectedEvent {
    std::string deviceName;
};
struct WifiStatusChangedEvent {
    bool isConnected;
};

//system events
struct TimeUpdatedEvent {
    std::string currentTime;
};
struct WeatherUpdatedEvent {
    int temperature;
    std::string condition;
};

struct SystemWakeEvent {};


struct ScreenBrightnessChanged {
    int brightness;  // 0-100
};