#include "managers/equalizerManager.h"
#include <nlohmann/json.hpp>
#include <iostream>


equalizerManager::equalizerManager(std::shared_ptr<I2CBus> i2cBus, 
                                   storageManager& storage, 
                                   EventBus* eventBus)
    : storage(storage), m_eventBus(eventBus) {
    
    // Verify digital pot addresses
    digipot1 = std::make_shared<MCP44X1>(i2cBus, 0x2C);
    digipot2 = std::make_shared<MCP44X1>(i2cBus, 0x2D); // IS A0 PULLED HIGH?
    
    // Initialize band structure
    bands[0] = {"62Hz", 62, 128};
    bands[1] = {"160Hz", 160, 128};
    bands[2] = {"400Hz", 400, 128};
    bands[3] = {"1kHz", 1000, 128};
    bands[4] = {"2.5kHz", 2500, 128};
    bands[5] = {"6.25kHz", 6250, 128};
    bands[6] = {"16kHz", 16000, 128};


    testHardwareConnection();
    
    // Load saved preset or defaults
    loadCustomPreset();
    
    // Initialize hardware
    updateHardware();
    
    // Subscribe to events
    if (m_eventBus) {
        m_eventBus->subscribe<UIEqualizerBandChanged>(this, &equalizerManager::onEqualizerChanged);
    }
    
    std::cout << "EqualizerManager initialized" << std::endl;
}

equalizerManager::~equalizerManager() {}

void equalizerManager::setBand(int bandIndex, uint8_t value) {
    if (bandIndex < 0 || bandIndex >= 7) {
        std::cerr << "Invalid band index: " << bandIndex << std::endl;
        return;
    }
    
    bands[bandIndex].value = value;
    
    // Update hardware
    // Bands 0-3 -> Digipot1 (W0-W3)
    // Bands 4-6 -> Digipot2 (W0-W2)
    
    if (bandIndex <= 3) {
        MCP44X1::Wiper wiper;
        switch (bandIndex) {
            case 0: wiper = MCP44X1::Wiper::W0; break;
            case 1: wiper = MCP44X1::Wiper::W1; break;
            case 2: wiper = MCP44X1::Wiper::W2; break;
            case 3: wiper = MCP44X1::Wiper::W3; break;
        }
        
        if (digipot1->setWiper(wiper, value)) {
            std::cout << "EQ Band " << bandIndex << " (" << bands[bandIndex].name 
                      << "): " << (int)value << std::endl;
        }
    } else {
        MCP44X1::Wiper wiper;
        switch (bandIndex) {
            case 4: wiper = MCP44X1::Wiper::W0; break;
            case 5: wiper = MCP44X1::Wiper::W1; break;
            case 6: wiper = MCP44X1::Wiper::W2; break;
        }
        
        if (digipot2->setWiper(wiper, value)) {
            std::cout << "EQ Band " << bandIndex << " (" << bands[bandIndex].name 
                      << "): " << (int)value << std::endl;
        }
    }
}

uint8_t equalizerManager::getBand(int bandIndex) const {
    if (bandIndex < 0 || bandIndex >= 7) return 128;
    return bands[bandIndex].value;
}

void equalizerManager::updateHardware() {
    std::cout << "Updating equalizer hardware..." << std::endl;
    
    for (int i = 0; i < 7; i++) {
        setBand(i, bands[i].value);
    }
}


void equalizerManager::loadCustomPreset() {
    try {
        std::string eqStr = storage.get("equalizer_settings");
        
        if (eqStr.empty()) {
            std::cout << "No saved EQ settings, using flat preset" << std::endl;
            for (auto & band : bands) {
                band.value = 30; // MAX VAL = 58
            }
            return;
        }
        
        auto json = nlohmann::json::parse(eqStr);
        
        for (int i = 0; i < 7; i++) {
            if (json.contains(std::to_string(i))) {
                bands[i].value = json[std::to_string(i)].get<uint8_t>();
            }
        }
        
        std::cout << "Loaded custom EQ settings" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "[Error] loading EQ settings: " << e.what() << std::endl;
    }
}

void equalizerManager::onEqualizerChanged(const UIEqualizerBandChanged& event) {
    std::cout << "EQ band " << event.bandIndex << " changed to " << (int)event.value << std::endl;
    setBand(event.bandIndex, event.value);
}

bool equalizerManager::testHardwareConnection() {
    std::cout << "--- Testing Equalizer Hardware (MCP4461) ---" << std::endl;
    bool success = true;

    // Test Device 1 (0x2C)
    uint8_t originalVal, testVal;
    
    // Attempt to read current value of Wiper 0
    if (digipot1->readWiper(MCP44X1::Wiper::W0, originalVal)) {
        std::cout << "[0x2C] Read OK. Current: " << (int)originalVal << std::endl;
        
        // Write test value
        if (digipot1->setWiper(MCP44X1::Wiper::W0, 0x55)) {
            digipot1->readWiper(MCP44X1::Wiper::W0, testVal);
            if (testVal == 0x55) {
                std::cout << "[0x2C] Write/Verify OK." << std::endl;
            } else {
                std::cerr << "[0x2C] Write Failed! Expected 0x55, got " << (int)testVal << std::endl;
                success = false;
            }
            // Restore original
            digipot1->setWiper(MCP44X1::Wiper::W0, originalVal);
        }
    } else {
        std::cerr << "[0x2C] Device not responding! Check wiring/power." << std::endl;
        success = false;
    }

    // Test Device 2 (0x2D)
    if (digipot2->readWiper(MCP44X1::Wiper::W0, originalVal)) {
        std::cout << "[0x2D] Read OK. Current: " << (int)originalVal << std::endl;
         // Write test value
        if (digipot2->setWiper(MCP44X1::Wiper::W0, 0xAA)) {
            digipot2->readWiper(MCP44X1::Wiper::W0, testVal);
            if (testVal == 0xAA) {
                std::cout << "[0x2D] Write/Verify OK." << std::endl;
            } else {
                std::cerr << "[0x2D] Write Failed! Expected 0xAA, got " << (int)testVal << std::endl;
                success = false;
            }
            // Restore original
            digipot2->setWiper(MCP44X1::Wiper::W0, originalVal);
        }
    } else {
        std::cerr << "[0x2D] Device not responding! Check wiring/power." << std::endl;
        success = false;
    }

    std::cout << "MCP4461 test finish. Successful: " << success << std::endl;
    return success;
}