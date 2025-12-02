#include "devices/sdCard.h"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

// NOTE: MUST ADD TO DTOVERLAY / BOOT FOR SD CARD KERNEL HANDLING
sdCard::sdCard(std::string mountPoint) : rootPath(mountPoint) {
    //check if mounted

    if (!fs::exists(rootPath)) {
        std::cerr << "sd card mount point does not exist: " << rootPath << std::endl;
        std::cerr << "please add to /etc/fstab or mount manually" << std::endl;
        return;
    }
    std::cout << "sd card mounted at " << rootPath << std::endl;

}

sdCard::~sdCard() {}


bool sdCard::readData() {
    audioData.clear();
    audioData["files"] = nlohmann::json::array();
    int fileCount = 0;

    if (!fs::exists(rootPath) || !fs::is_directory(rootPath)) {
        std::cerr << "sd card path not found: " << rootPath << std::endl;
        return false;
    }

    try {
        for (const auto& entry : fs::recursive_directory_iterator(rootPath)) {
            if (entry.is_regular_file()) {
                std::string path = entry.path().string();

                if (path.find(".mp3") != std::string::npos ||
                    path.find(".MP3") != std::string::npos ||
                    path.find(".wav") != std::string::npos) {
                    
                    nlohmann::json track;
                    track["filename"]=entry.path().filename().string();
                    track["full_path"] = path;
                    track["size"] = entry.file_size();
                    std::string title = entry.path().stem().string();
                    track["title"] = title;
                    audioData["files"].push_back(track);
                    fileCount++;
                }
            }
        }

        std::cout << "Found " << fileCount << " audio files on sd card." << std::endl;

        return fileCount > 0;
        
    } catch (const fs::filesystem_error& e) {
        std::cerr << "filesystem error: " << e.what() << std::endl;
        return false;
    }
    
}

nlohmann::json sdCard::getMetadata() {
    return audioData;
}