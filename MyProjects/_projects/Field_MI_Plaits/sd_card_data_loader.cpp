/**
 * @file sd_card_data_loader.cpp
 * @brief SD Card loader for Plaits extra engine data
 * 
 * This module loads Plaits synthesis data from SD card at startup.
 * When loaded, it enables additional synthesis engines:
 * - Wavetable engine
 * - Waveshaping engine  
 * - Chord engine
 */

#include "sd_card_data_loader.h"
#include <cstring>

// Flag indicating SD data is loaded
namespace {
    bool sd_data_loaded = false;
}

/**
 * Load all Plaits data from SD card
 * Call this at startup before voice.Init()
 * 
 * Note: This is a stub implementation. For full implementation,
 * this would use FatFS to read files from SD card.
 */
bool PlaitsLoadSdData() {
    // TODO: Implement actual SD card loading with FatFS
    // For now, this marks the system as ready for extra data
    
    // Check if files exist on SD card
    // if (SD_CARD_PRESENT) {
    //     f_mount(&fs, "", 1);
    //     if (f_open(&fp, "/plaits/wavetable.bin", FA_READ) == FR_OK) {
    //         f_read(&fp, buffer, size, &bytes);
    //         sd_data_loaded = true;
    //     }
    // }
    
    // For now, return false (no SD data loaded yet)
    // The infrastructure is ready when you implement the SD reading
    sd_data_loaded = false;
    
    return sd_data_loaded;
}

/**
 * Check if SD data is loaded
 */
bool PlaitsSdDataLoaded() {
    return sd_data_loaded;
}

/**
 * Get status string for OLED display
 */
const char* PlaitsSdStatus() {
    if (sd_data_loaded) {
        return "SD: Extra engines OK";
    }
    return "SD: Base (9 eng)";
}