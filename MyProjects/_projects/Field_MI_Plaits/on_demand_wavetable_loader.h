/**
 * @file on_demand_wavetable_loader.h
 * @brief On-demand wavetable loader for Plaits extra engines
 * 
 * Loads engine data from SD card ONLY when that engine is selected.
 * Saves RAM by only keeping one dataset loaded at a time.
 * 
 * Usage:
 *   // Initialize at startup
 *   WavetableLoader.Init();
 *   
 *   // When selecting engine:
 *   WavetableLoader.LoadForEngine(engine_slot);
 *   
 *   // When switching to base engine:
 *   WavetableLoader.Unload();
 * 
 * SD Card Structure:
 * /plaits/wavetables/
 *   - wavetable_full.bin    (99KB) - Wavetable, Chord
 *   - waveshape_lut1.bin   (1KB)  - Waveshaping
 *   - waveshape_lut2.bin   (1KB)  - Waveshaping
 */

#ifndef ON_DEMAND_WAVETABLE_LOADER_H_
#define ON_DEMAND_WAVETABLE_LOADER_H_

#include <stdint.h>
#include <stdbool.h>

// Forward declarations - headers included in cpp file
namespace daisy {
    class SdmmcHandler;
    class FatFSInterface;
}

// Engine slots that need external data
enum EngineNeedingData {
    ENGINE_NEED_NONE = -1,
    ENGINE_NEED_WAVETABLE = 5,   // A6 - Wavetable engine
    ENGINE_NEED_CHORD = 6,       // A7 - Chord engine  
    ENGINE_NEED_WAVESHAPE = 1,   // A2 - Waveshaping engine
};

class WavetableLoader {
public:
    /**
     * Initialize the loader
     */
    void Init();

    /**
     * Cleanup resources
     */
    ~WavetableLoader();
    
    /**
     * Load data for specific engine
     * @param engine_slot The key slot (A1-A8, B1-B8 = 0-15)
     * @return true if loaded successfully
     */
    bool LoadForEngine(int engine_slot);
    
    /**
     * Unload current data (free RAM)
     */
    void Unload();
    
    /**
     * Check if data is currently loaded
     */
    bool IsLoaded() const { return loaded_engine_ != ENGINE_NEED_NONE; }
    
    /**
     * Get current loaded engine
     */
    int GetLoadedEngine() const { return loaded_engine_; }
    
    /**
     * Check if loading is in progress
     */
    bool IsLoading() const { return loading_; }
    
    /**
     * Get status message for display
     */
    const char* GetStatus();
    
    /**
     * Check if engine needs external data
     */
    static bool EngineNeedsData(int engine_slot);

private:
    int loaded_engine_ = ENGINE_NEED_NONE;
    bool loading_ = false;
    bool sd_initialized_ = false;
};

// Global instance - use this in your code
extern WavetableLoader g_wavetable_loader;

#endif  // ON_DEMAND_WAVETABLE_LOADER_H_