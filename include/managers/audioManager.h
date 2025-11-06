#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include "managers/connectivityManager.h"

struct Song {
    std::string title;
    std::string artist;
    std::string filePath;
};

class audioManager {
    public:
        enum class AudioOutput { AUTO, JACK, BLUETOOTH };
        enum class AudioState { STOPPED, PLAYING, PAUSED };
        audioManager(connectivityManager* connMgr);
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

        // need functions for changing volume, and equalizer settings


    private:
        std::string runCommand(const std::string& command);
        connectivityManager* connMgr;
        AudioOutput currentOutput = AudioOutput::AUTO;
        AudioState currentState = AudioState::STOPPED;
        int currentVolume = 50; // default volume 50%
        void scanForSongs();
        std::vector<Song> songList;
        std::string jackSink;
        std::string btSink;

};