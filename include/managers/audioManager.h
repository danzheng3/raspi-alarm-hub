#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <atomic>
#include "managers/connectivityManager.h"
#include "events/EventBus.h"
#include "events/Events.h"
#pragma once

#define ALARM_RING_PATH "/home/daniel/Downloads/raspi-alarm-hub/images/alarm.mp3"

struct Song {
    std::string title;
    std::string artist;
    std::string filePath;
};

class audioManager {
    public:
        enum class AudioOutput { AUTO, JACK, BLUETOOTH };
        enum class AudioState { STOPPED, PLAYING, PAUSED };

        audioManager(connectivityManager* connMgr, EventBus* eventBus);
        // may need to add parameters, plus duplicate depending on when sd slot is added.
        ~audioManager();

        std::vector<Song> getSongList() const { return songList; };

        // PLAYBACK CTRL
        void playSongAtIndex(size_t index);
        void stop();
        void pause();
        void resume();


        // PLAYBACK MODE
        void setOutput(AudioOutput output);
        AudioState getState();
        void setVolume(int volume); // 0-100%

        void alarmRing(const std::string& customPath = "");

        std::thread monitorThread;
        std::atomic<bool> monitorActive{false};

    private:
        std::string runCommand(const std::string& command);
        connectivityManager* connMgr;
        EventBus* m_eventBus;

        // audio state
        AudioOutput currentOutput = AudioOutput::AUTO;
        AudioState currentState = AudioState::STOPPED;
        int currentVolume = 50; // default volume 50%

        // playlist state info
        std::vector<Song> songList;
        size_t currentIndex=0;

        std::string jackSink = "default"; // need to set sink name
        std::string btSink = "bluetooth-default"; // need to set sink name!

        //internal helper
        void scanForSongs();
        void playNextSong();

        // EVENT-HANDLERS
        void onAlarmTriggered(const AlarmTriggeredEvent& event);
        void onAlarmCleared(const AlarmClearedEvent& event);
        void onSongSelected(const UISongSelectedEvent& event);
        void onUIVolumeChanged(const UIVolumeChanged& event);
        void onSpeakerDocked(const SpeakerDockedEvent& event);
        void onSpeakerUndocked(const SpeakerUndockedEvent& event);
        void onBluetoothConnected(const BluetoothSpeakerConnectedEvent& event);
        
};