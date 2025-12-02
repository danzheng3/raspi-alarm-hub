#include "managers/audioManager.h"
audioManager::audioManager(connectivityManager* connMgr, EventBus* eventBus) : connMgr(connMgr), m_eventBus(eventBus) {
    

    system("mkdir -p /mnt/sdcard");
    playQueue.clear();
    songList.clear();
    scanForSongs();

    if (m_eventBus) {
        m_eventBus->subscribe<AlarmTriggeredEvent>(this, &audioManager::onAlarmTriggered);
        m_eventBus->subscribe<AlarmClearedEvent>(this, &audioManager::onAlarmCleared);
        m_eventBus->subscribe<UIPlaySongPressedEvent>(this, &audioManager::onUIPlaySongPressed);
        m_eventBus->subscribe<UIVolumeChanged>(this, &audioManager::onUIVolumeChanged);
        m_eventBus->subscribe<SpeakerDockedEvent>(this, &audioManager::onSpeakerDocked);
        m_eventBus->subscribe<SpeakerUndockedEvent>(this, &audioManager::onSpeakerUndocked);
        m_eventBus->subscribe<BluetoothSpeakerConnectedEvent>(this, &audioManager::onBluetoothConnected);
    }

    std::cout << "AudioManager initialized, " << songList.size() << " songs found." << std::endl;
}

audioManager::~audioManager() {
    system("sudo umount /mnt/sdcard");
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
    std::string escapedPath = song.filePath;

    size_t pos = 0;
    while ((pos = escapedPath.find("'", pos)) != std::string::npos) {
        escapedPath.replace(pos, 1, "'\\''");
        pos += 4;
    }
    
    std::string command = "mpg123 -q '" + escapedPath + "' > /dev/null 2>&1 &";
    system(command.c_str());
    
    currentState = AudioState::PLAYING;
    std::cout << "Playing: " << song.title << std::endl;
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

void audioManager::alarmRing() {
    stop();
    std::string command = "mpg123 --loop -1 '" + std::string(ALARM_RING_PATH) + "' &"; // NEED TO TEST FILEPATH
    setVolume(100); // max volume for alarm

    system(command.c_str());
    currentState = AudioState::PLAYING;
    std::cout << "alarm ringing!" << std::endl;
}

// event handlers

void audioManager::onAlarmTriggered(const AlarmTriggeredEvent& event) {
    std::cout << "audioManager: alarm triggered event" << std::endl;
    alarmRing();
}

void audioManager::onAlarmCleared(const AlarmClearedEvent& event) {
    std::cout << "audioManager: alarm cleared" << std::endl;
    stop();
    //setVolume to previous
}

void audioManager::onUIPlaySongPressed(const UIPlaySongPressedEvent& event) {
    std::cout << "audioManager: play song button pressed" << std::endl;

    if (!songList.empty()) {
        playSong(songList[0]); // need to update logic here
    }
}

void audioManager::onUIVolumeChanged(const UIVolumeChanged& event) {
    std::cout << "audioManager: Volume changed to " << event.newVolume << std::endl;
    setVolume(event.newVolume);
}

void audioManager::onSpeakerDocked(const SpeakerDockedEvent& event) {
    std::cout << "audioManager: Speaker docked, switching to jack output" << std::endl;
    setOutput(AudioOutput::JACK);
}

void audioManager::onSpeakerUndocked(const SpeakerUndockedEvent& event) {
    std::cout << "audioManager: Speaker undocked, switching to auto output" << std::endl;
    setOutput(AudioOutput::AUTO);
}

void audioManager::onBluetoothConnected(const BluetoothSpeakerConnectedEvent& event) {
    std::cout << "audioManager: Bluetooth speaker connected: " << event.deviceName << std::endl;
    btSink = event.deviceName; // Update sink name
    if (currentOutput == AudioOutput::AUTO || currentOutput == AudioOutput::BLUETOOTH) {
        setOutput(AudioOutput::BLUETOOTH);
    }
}



/*
Play: mpg123 "path/to/song.mp3" & 

Stop: killall mpg123 

Pause: kill -STOP $(pidof mpg123) (uses process id)

Resume: kill -CONT $(pidof mpg123) (resumes process)
*/


