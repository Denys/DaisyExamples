/**
 * @file sd_card_data.cpp
 * @brief SD Card data loader for Plaits extra engines
 * 
 * Loads wavetables and extra engine data from SD card to enable:
 * - Wavetable engine (256KB)
 * - Chord engine (8KB)
 * - Speech engine (64KB)  
 * - Modal engine (32KB)
 * 
 * SD Card should contain:
 * /plaits/wavetable.bin (256KB)
 * /plaits/chord_data.bin (8KB)
 * /plaits/speech_data.bin (64KB)
 * /plaits/modal_data.bin (32KB)
 */

#include "sd_card_data.h"
#include "daisy_seed.h"
#include <ff.h>

using namespace daisy;

static FATFS fs;
static bool sd_initialized = false;

extern "C" {
// These are referenced by the engines but we provide them from SD
extern char wav_integrated_waves[256 * 1024];
extern char chord_ratios[8 * 1024];
extern char speech_lpc[64 * 1024];
extern char modal_voices[32 * 1024];
}

/**
 * Initialize SD card and load Plaits data
 * Call this once at startup before voice.Init()
 */
bool PlaitsLoadSdData() {
    FRESULT result;
    
    // Mount SD card
    result = f_mount(&fs, "", 1);
    if (result != FR_OK) {
        return false;
    }
    
    FIL fp;
    UINT bytes_read;
    
    // Load wavetable data (256KB)
    result = f_open(&fp, "/plaits/wavetable.bin", FA_READ);
    if (result == FR_OK) {
        f_read(&fp, wav_integrated_waves, 256 * 1024, &bytes_read);
        f_close(&fp);
    }
    
    // Load chord data (8KB)
    result = f_open(&fp, "/plaits/chord_data.bin", FA_READ);
    if (result == FR_OK) {
        f_read(&fp, chord_ratios, 8 * 1024, &bytes_read);
        f_close(&fp);
    }
    
    // Load speech data (64KB)
    result = f_open(&fp, "/plaits/speech_data.bin", FA_READ);
    if (result == FR_OK) {
        f_read(&fp, speech_lpc, 64 * 1024, &bytes_read);
        f_close(&fp);
    }
    
    // Load modal data (32KB)
    result = f_open(&fp, "/plaits/modal_data.bin", FA_READ);
    if (result == FR_OK) {
        f_read(&fp, modal_voices, 32 * 1024, &bytes_read);
        f_close(&fp);
    }
    
    sd_initialized = true;
    return true;
}

/**
 * Check if SD data is loaded
 */
bool PlaitsSdDataLoaded() {
    return sd_initialized;
}

/**
 * Get status message for display
 */
const char* PlaitsSdStatus() {
    if (!sd_initialized) {
        return "SD: Not loaded";
    }
    return "SD: Full data";
}