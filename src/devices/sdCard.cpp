#include "devices/sdCard.h"
#include <filesystem>
#include <iostream>


// NOTE: MUST ADD TO DTOVERLAY / BOOT FOR SD CARD KERNEL HANDLING
sdCard::sdCard(std::string mountPoint) : rootPath(mountPoint) {

}

sdCard::~sdCard() {}

namespace fs = std::filesystem;

bool sdCard::readData() {
    audioData.clear();
    audioData["files"] = nlohmann::json::array();

    if (!fs::exists(rootPath) || !fs::is_directory(rootPath)) {
        std::cerr << "sd card path not found: " << rootPath << std::endl;
        return false;
    }

    try {
        for (const auto& entry : fs::recursive_directory_iterator(rootPath)) {
            if (entry.is_regular_file()) {
                std::string path = entry.path().string();

                if (path.find(".mp3") != std::string::npos ||
                    path.find(".MP3") != std::string::npos) {
                    
                    nlohmann::json track;
                    track["filename"]=entry.path().filename().string();
                    track["full_path"] = path;
                    track["size"] = entry.file_size();
                    audioData["files"].push_back(track);
                }
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "filesystem error: " << e.what() << std::endl;
        return false;
    }

    return true;
}

nlohmann::json sdCard::getMetadata() {
    return audioData;
}