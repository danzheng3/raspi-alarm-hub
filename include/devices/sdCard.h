#include "hardware_layer/SPIBus.h"
#include <nlohmann/json.hpp>

#pragma once


// operation: reads from sd card over SPI. parses mp3 files and saves them into json storage.
// json storage is called once needed by other modules to play audio files.

// MUST ADD SPI-mmc to boot/config.txt for dtoverlay

#define MOSI 10
#define MISO 9
#define SCLK 11
#define CE 8

class sdCard {
    public:
        sdCard(std::string mountPoint);
        ~sdCard();

        bool readData();
        nlohmann::json getMetadata();
    private:
        std::string rootPath;
        nlohmann::json audioData;
};