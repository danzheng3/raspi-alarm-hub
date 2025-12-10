#include "managers/audioManager.h"
audioManager::audioManager(connectivityManager* connMgr, EventBus* eventBus) : connMgr(connMgr), m_eventBus(eventBus) {
    
    // ADDED TO /ETC/FSTAB FOR AUTO MOUNT
    system("mkdir -p /mnt/sdcard");
    scanForSongs();

    setVolume(20);

    if (m_eventBus) {
        m_eventBus->subscribe<AlarmTriggeredEvent>(this, &audioManager::onAlarmTriggered);
        m_eventBus->subscribe<AlarmClearedEvent>(this, &audioManager::onAlarmCleared);
        m_eventBus->subscribe<UISongSelectedEvent>(this, &audioManager::onSongSelected);
        m_eventBus->subscribe<UIVolumeChanged>(this, &audioManager::onUIVolumeChanged);
        m_eventBus->subscribe<SpeakerDockedEvent>(this, &audioManager::onSpeakerDocked);
        m_eventBus->subscribe<SpeakerUndockedEvent>(this, &audioManager::onSpeakerUndocked);
        m_eventBus->subscribe<BluetoothSpeakerConnectedEvent>(this, &audioManager::onBluetoothConnected);
    }

    std::cout << "AudioManager initialized, " << songList.size() << " songs found." << std::endl;
}

audioManager::~audioManager() {
    stop();
    monitorActive = false;
    if (monitorThread.joinable()) {
        monitorThread.join();
    }
}


void audioManager::scanForSongs() {
    songList.clear();
    std::string directory = "/home/daniel/Music";

    if (!std::filesystem::exists(directory)) {
        std::cerr << "[Error] SD card not mounted at " << directory << std::endl;
        return;
    }

    for (const auto& entry : std::filesystem::recursive_directory_iterator(directory)) {
        if (entry.is_regular_file()) {
            std::string filePath = entry.path().string();
            if (entry.path().extension() == ".mp3" || entry.path().extension() == ".wav") {
                Song song;
                song.title = entry.path().stem().string(); // use title
                song.artist = "Unknown"; // hmm other way to get artist
                song.filePath = filePath;
                songList.push_back(song);
                std::cout << "found song: " << song.title << std::endl;
            }
        }
    }
}


void audioManager::playSongAtIndex(size_t index) {
    if (index >= songList.size()) {
        std::cerr << "[Error] Invalid song index" << std::endl;
        return;
    }
    
    stop();
    monitorActive = false;

    // 2. Handle the thread
    if (monitorThread.joinable()) {
        // PREVENT DEADLOCK: Check if we are running INSIDE the monitor thread
        if (std::this_thread::get_id() == monitorThread.get_id()) {
            std::cout << "[AudioMgr] Self-call detected, detaching monitor thread instead of joining." << std::endl;
            monitorThread.detach(); 
        } else {
            monitorThread.join();
        }
    }
    std::string stopCommand = "killall -9 mpg123 2>/dev/null";
    system(stopCommand.c_str());
    currentState = AudioState::STOPPED;
    currentIndex = index;
    
    const Song& song = songList[index];
    
    // Escape single quotes in path
    std::string escapedPath = song.filePath;
    size_t pos = 0;
    while ((pos = escapedPath.find("'", pos)) != std::string::npos) {
        escapedPath.replace(pos, 1, "'\\''");
        pos += 4;
    }

    setOutput(AudioOutput::AUTO);
    
    // Play song and automatically continue to next when finished
    std::string command = "setsid sudo -u daniel XDG_RUNTIME_DIR=/run/user/$(id -u daniel) mpg123 -o pulse -q '" + escapedPath + "' < /dev/null && echo 'SONG_FINISHED' > /tmp/mpg123.log 2>&1 &";
    system(command.c_str());
    
    currentState = AudioState::PLAYING;
    monitorActive = true;
    std::cout << "Playing: " << song.title << std::endl;
    
    // Start background thread to monitor song completion
    monitorThread = std::thread([this]() {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        while (currentState == AudioState::PLAYING && monitorActive) {
            std::string result = runCommand("pgrep mpg123");
            if (result.empty()) {
                playNextSong();
                break;
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });
}

void audioManager::playNextSong() {
    if (currentState == AudioState::STOPPED) return;
    
    currentIndex++;
    if (currentIndex >= songList.size()) {
        std::cout << "End of playlist" << std::endl;
        stop();
        return;
    }
    
    playSongAtIndex(currentIndex);
}


void audioManager::pause() {
    if (currentState == AudioState::PLAYING) {
        std::string command = "pkill -STOP -u daniel -x mpg123";
        system(command.c_str());
        
        currentState = AudioState::PAUSED;
        std::cout << "Audio paused" << std::endl;
    }
}

void audioManager::resume() {
    if (currentState == AudioState::PAUSED) {
        std::string command = "killall -CONT mpg123 2>/dev/null";
        system(command.c_str());
        currentState = AudioState::PLAYING;
        std::cout << "Audio resumed" << std::endl;
    }
}

void audioManager::stop() {
    if (currentState != AudioState::STOPPED) {
        std::string command = "killall -9 mpg123 2>/dev/null";
        system(command.c_str());
        currentState = AudioState::STOPPED;
        std::cout << "Audio stopped" << std::endl;
    }
}

void audioManager::setOutput(AudioOutput output) { // NEED TO TEST THIS
    std::string sinkToSet = jackSink;
    bool btConnected = connMgr->isBluetoothConnected() && connMgr;

    if (output == AudioOutput::BLUETOOTH && btConnected) {
        std::string result = runCommand("pactl list short sinks | grep bluez");
        if (!result.empty()) {
            std::istringstream iss(result);
            iss >> btSink;
            sinkToSet = btSink;
        }
    } else if (output == AudioOutput::AUTO) {
        if (btConnected) {
            std::string result = runCommand("pactl list short sinks | grep bluez");
            if (!result.empty()) {
                std::istringstream iss(result);
                iss >> btSink;
                sinkToSet = btSink;
                std::cout << "[audioMgr] set to bt sink" << btSink << std::endl;
            } else {
                std::cout << "[audioMgr] bt sink not found" << std::endl;
            }
        }
    }

    std::string command = "sudo -u daniel XDG_RUNTIME_DIR=/run/user/$(id -u daniel) pactl set-default-sink " + sinkToSet;
    system(command.c_str());
    currentOutput = output;
}

void audioManager::setVolume(int volume) { // based on 0-100 percentage
    if (volume < 0) volume = 0;
    if (volume > 100) volume = 100;

    std::string command = "sudo -u daniel XDG_RUNTIME_DIR=/run/user/$(id -u daniel) pactl set-sink-volume @DEFAULT_SINK@ " + std::to_string(volume) + "%";    system(command.c_str());
    currentVolume = volume;

    std::cout << "audioManager: volume set to " << volume << std::endl;
} 

void audioManager::alarmRing(const std::string& customPath) {

    std::cout << "[AudioMgr] Switching output for Alarm..." << std::endl;
    setOutput(AudioOutput::AUTO);

    stop();
    std::string path = (customPath.empty() || customPath == "default") ? std::string(ALARM_RING_PATH) : customPath;
    std::cout << "path debug " << path << std::endl;
    if (!std::filesystem::exists(path)) {
        std::cout << "[audioMgr] custom audio path DNE" << std::endl;
        path = std::string(ALARM_RING_PATH);
    }
    std::string command = "setsid sudo -u daniel env XDG_RUNTIME_DIR=/run/user/$(id -u daniel) mpg123 -o pulse -q --loop -1 '" + path + "' < /dev/null > /dev/null 2>&1 &";
    setVolume(100); // max volume for alarm

    system(command.c_str());
    currentState = AudioState::PLAYING;
    std::cout << "audioManager: alarm ringing using " << path << std::endl;
}

/*

EVENT HANDLERS

*/

void audioManager::onAlarmTriggered(const AlarmTriggeredEvent& event) {
    std::cout << "audioManager: alarm triggered event" << std::endl;
    if (event.playAudio) {
        alarmRing(event.audioPath);
        return;
    } else {
        std::cout << "[audioMgr]: alarm audio disabled, not ringing" << std::endl;
    }
}

void audioManager::onAlarmCleared(const AlarmClearedEvent& event) {
    std::cout << "audioManager: alarm cleared" << std::endl;
    stop();
    setVolume(50);
    //setVolume to previous
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

void audioManager::onSongSelected(const UISongSelectedEvent& event) {
    std::cout << "Song selected: index " << event.songIndex << std::endl;
    playSongAtIndex(event.songIndex);
}

//run command helper

std::string audioManager::runCommand(const std::string& command) {
    std::array<char, 128> buffer;
    std::string result;
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) return "ERROR";
    
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }
    pclose(pipe);
    return result;
}

audioManager::AudioState audioManager::getState() {
    return currentState;
}



/*
Play: mpg123 "path/to/song.mp3" & 

Stop: killall mpg123 

Pause: kill -STOP $(pidof mpg123) (uses process id)

Resume: kill -CONT $(pidof mpg123) (resumes process)
*/


