#pragma once
#include <memory>
#include <array>
#include "devices/MCP44X1.h"
#include "storageManager.h"
#include "events/EventBus.h"
#include "events/Events.h"

// 7-band equalizer frequencies
// 62Hz, 160Hz, 400Hz, 1kHz, 2.5kHz, 6.25kHz, 16kHz

struct EqualizerBand {
    std::string name;
    int frequency;
    uint8_t value;  // 0-255 for digital pot
};

class equalizerManager {
public:
    equalizerManager(std::shared_ptr<I2CBus> i2cBus, 
                     storageManager& storage, 
                     EventBus* eventBus);
    ~equalizerManager();
    
    // Get/Set individual band
    void setBand(int bandIndex, uint8_t value);
    uint8_t getBand(int bandIndex) const;
    
    // Presets
    void applyPreset(const std::string& presetName);
    void loadCustomPreset();
    void saveCustomPreset();
    
    // Get all bands
    std::array<EqualizerBand, 7> getAllBands() const { return bands; }

private:
    std::shared_ptr<MCP44X1> digipot1;  // Bands 0-3
    std::shared_ptr<MCP44X1> digipot2;  // Bands 4-6 (W0-W2 used)
    
    storageManager& storage;
    EventBus* m_eventBus;
    
    std::array<EqualizerBand, 7> bands;
    
    // Initialize default values
    void initializeDefaults();
    void updateHardware();
    
    // Event handler
    void onEqualizerChanged(const UIEqualizerBandChanged& event);
};