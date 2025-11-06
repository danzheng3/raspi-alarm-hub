#include "managers/audioManager.h"
audioManager::audioManager(connectivityManager* connMgr) : connMgr(connMgr) {
    int rmdir = system("rmdir /mnt/sdcard/audioDataTemp && mkdir /mnt/sdcard/audioDataTemp");
    int mount_sd = system("sudo mount /dev/mmcblk1p1 /mnt/sdcard");

    if (rmdir != 0 || mount_sd != 0) {
        // Handle error
        std::cout << "error mounting sd card" << std::endl; 
    }

    songList.clear();
    scanForSongs();


}

audioManager::~audioManager() {
    system("sudo umount /mnt/sdcard");
}

std::vector<Song> audioManager::getSongList() {
    return songList;
}

void audioManager::scanForSongs() {
    std::string directory = "/mnt/sdcard/";

    for (const auto& entry : std::filesystem::recursive_directory_iterator(directory)) {
        if (entry.is_regular_file()) {
            std::string filePath = entry.path().string();
            if (entry.path().extension() == ".mp3" || entry.path().extension() == ".wav") {
                Song song;
                song.title = entry.path().stem().string(); // use title
                song.artist = "Unknown Artist"; // hmm other way to get artist
                song.filePath = filePath;
                songList.push_back(song);
                std::cout << "found song: " << song.title << std::endl;
            }
        }
    }
}

void audioManager::playSong(const Song& song) {
    stop();
    if (song.filePath.empty()) {
        std::cout << "No song file path provided." << std::endl;
        return;
    }
    std::string filepath = song.filePath;
    std::string command = "mpg123 '" + filepath + "' &"; // NEED TO TEST FILEPATH

    system(command.c_str());
    currentState = AudioState::PLAYING;
    std::cout << "playing song: " << song.title << std::endl;
}

void audioManager::pause() {
    if (currentState == AudioState::PLAYING) {
        std::string command = "kill -STOP $(pidof mpg123)";
        system(command.c_str());
        currentState = AudioState::PAUSED;
        std::cout << "audio paused" << std::endl;
    }
}

void audioManager::resume() {
    if (currentState == AudioState::PAUSED) {
        std::string command = "kill -CONT $(pidof mpg123)";
        system(command.c_str());
        currentState = AudioState::PLAYING;
        std::cout << "audio resumed" << std::endl;
    }
}

void audioManager::stop() {
    if (currentState != AudioState::STOPPED) {
        std::string command = "killall mpg123";
        system(command.c_str());
        currentState = AudioState::STOPPED;
        std::cout << "audio stopped" << std::endl;
    }
}

void audioManager::setOutput(AudioOutput output) {
    std::string sinkToSet = jackSink;
    bool btConnected = connMgr->isBluetoothConnected() && connMgr;

    if (output == AudioOutput::BLUETOOTH && btConnected) {
        // need logic hear to set sink to btSink using MAC ADDRESS / etc.
        // need to TEST THIS
    } else if (output == AudioOutput::AUTO) {
        if (btConnected) {
            sinkToSet = btSink;
        }
    } else if (output == AudioOutput::JACK) {
        sinkToSet = jackSink;
    }

    std::string command = "pactl set-default-sink " + sinkToSet;
    std::cout << "setting audio output: " << sinkToSet << std::endl;
    system(command.c_str());
    currentOutput = output;
}

void audioManager::setVolume(int volume) { // based on 0-100 percentage
    if (volume < 0) volume = 0;
    if (volume > 100) volume = 100;

    std::string command = "pactl set-sink-volume @DEFAULT_SINK@ " + std::to_string(volume) + "%";
    system(command.c_str());
    currentVolume = volume;
} 

/*
Play: mpg123 "path/to/song.mp3" & 

Stop: killall mpg123 

Pause: kill -STOP $(pidof mpg123) (uses process id)

Resume: kill -CONT $(pidof mpg123) (resumes process)
*/


