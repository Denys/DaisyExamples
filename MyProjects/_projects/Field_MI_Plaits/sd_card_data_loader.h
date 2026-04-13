/**
 * @file sd_card_data_loader.h
 * @brief SD Card loader for Plaits extra engine data
 * 
 * This module loads Plaits synthesis data from SD card at startup.
 * When loaded, it enables additional synthesis engines.
 * 
 * Usage:
 *   // At startup, before voice.Init()
 *   PlaitsLoadSdData();
 *   
 *   // Check status
 *   if (PlaitsSdDataLoaded()) {
 *       // Extra engines available
 *   }
 * 
 * SD Card Files (in /plaits/ folder):
 * - wavetable.bin    (49,920 bytes)
 * - table_fold.bin  (1,032 bytes)
 * - table_fold2.bin (1,032 bytes)
 */

#ifndef SD_CARD_DATA_LOADER_H_
#define SD_CARD_DATA_LOADER_H_

#include <stdint.h>
#include <stdbool.h>

// Data sizes from Plaits resources
#define WAV_TABLE_SIZE      49920    // wav_integrated_waves size in int16_t
#define LUT_FOLD_SIZE       516      // lut_fold size in int16_t  
#define LUT_FOLD_2_SIZE     516      // lut_fold_2 size in int16_t

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Load all Plaits data from SD card
 * Call at startup BEFORE voice.Init()
 * @return true if data loaded successfully
 */
bool PlaitsLoadSdData(void);

/**
 * Check if SD data was loaded
 * @return true if SD data is available
 */
bool PlaitsSdDataLoaded(void);

/**
 * Get status string for display
 * @return static string with status message
 */
const char* PlaitsSdStatus(void);

#ifdef __cplusplus
}
#endif

#endif  // SD_CARD_DATA_LOADER_H_