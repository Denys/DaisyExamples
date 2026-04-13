/**
 * @file on_demand_wavetable_loader.cpp
 * @brief On-demand wavetable loader implementation
 * 
 * This is a TEMPLATE implementation showing the on-demand loading concept.
 * For full functionality, this would integrate with FatFS to read from SD card.
 * 
 * Currently provides the framework - SD reading code needs to be added
 * when you have FatFS properly integrated in the project.
 */

#include "on_demand_wavetable_loader.h"
#include <cstring>

// Data buffers in RAM - loaded on demand
// These replace the embedded data when engine is selected
static int16_t s_wavetable_buffer[49920];  // 99KB
static int16_t s_waveshape_lut1[516];       // 1KB
static int16_t s_waveshape_lut2[516];       // 1KB

// Current status
static bool s_initialized = false;
static char s_status_message[32] = "WT: Ready";

WavetableLoader g_wavetable_loader;

void WavetableLoader::Init() {
    loaded_engine_ = ENGINE_NEED_NONE;
    loading_ = false;
    sd_initialized_ = false;

    // For now, mark as not initialized - SD card support needs to be added to main firmware
    // TODO: Integrate with main SD card initialization
    s_initialized = false;
    strcpy(s_status_message, "WT: SD Not Ready");
}

bool WavetableLoader::EngineNeedsData(int engine_slot) {
    // These engines need external data loaded from SD
    return engine_slot == ENGINE_NEED_WAVETABLE ||  // A6 - Wavetable
           engine_slot == ENGINE_NEED_CHORD ||     // A7 - Chord  
           engine_slot == ENGINE_NEED_WAVESHAPE;   // A2 - Waveshaping
}

bool WavetableLoader::LoadForEngine(int engine_slot) {
    if (!s_initialized) {
        strcpy(s_status_message, "WT: Not init");
        return false;
    }

    // Check if already loaded for this engine
    if (loaded_engine_ == engine_slot && IsLoaded()) {
        return true;  // Already loaded
    }

    // Unload any existing data first
    Unload();

    loading_ = true;
    strcpy(s_status_message, "WT: Loading...");

    // TODO: Implement actual SD card file reading here
    // For now, simulate loading success

    loading_ = false;
    loaded_engine_ = engine_slot;

    // Update status based on engine
    if (engine_slot == ENGINE_NEED_WAVETABLE || engine_slot == ENGINE_NEED_CHORD) {
        strcpy(s_status_message, "WT: Wavetable");
    } else if (engine_slot == ENGINE_NEED_WAVESHAPE) {
        strcpy(s_status_message, "WT: Waveshape");
    }

    // TODO: Point Plaits engines to use these buffers instead of embedded data
    // This requires modifying voice.cc to check for external data pointers

    return true;  // Simulate success for now
}

void WavetableLoader::Unload() {
    if (loaded_engine_ != ENGINE_NEED_NONE) {
        loaded_engine_ = ENGINE_NEED_NONE;
        strcpy(s_status_message, "WT: Unloaded");
    }
    loading_ = false;
}

const char* WavetableLoader::GetStatus() {
    if (loading_) {
        return "WT: Loading...";
    }
    if (!IsLoaded()) {
        return "WT: Ready";
    }
    return s_status_message;
}