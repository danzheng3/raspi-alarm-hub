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
struct UIPlaySongPressedEvent{};
struct UIVolumeChanged{ int newVolume; };
 
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


