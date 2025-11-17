#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include "managers/connectivityManager.h"
#include "events/EventBus.h"
#include "events/Events.h"

#define ALARM_RING_PATH "../../images/alarm_ring.mp3"

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

        std::vector<Song> getSongList();
        void playSong(const Song& song);

        void stop();
        void pause();
        void resume();
        void setOutput(AudioOutput output);
        AudioState getState();
        void setVolume(int volume); //0-100 percentage

        void alarmRing();

        // need functions for changing volume, and equalizer settings


    private:
        std::string runCommand(const std::string& command);
        connectivityManager* connMgr;
        AudioOutput currentOutput = AudioOutput::AUTO;
        AudioState currentState = AudioState::STOPPED;
        int currentVolume = 50; // default volume 50%
        void scanForSongs();
        std::vector<Song> songList;
        std::string jackSink = "default"; // need to set sink name
        std::string btSink = "bluetooth-default"; // need to set sink name!

        // EVENT-HANDLERS
        void onAlarmTriggered(const AlarmTriggeredEvent& event);
        void onAlarmStopped(const AlarmClearedEvent& event);

        connectivityManager* connMgr;
        EventBus* m_eventBus;
        
};