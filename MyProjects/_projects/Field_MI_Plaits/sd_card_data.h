/**
 * @file sd_card_data.h
 * @brief SD Card data loader for Plaits extra engines
 * 
 * This module loads Plaits engine data from SD card to enable additional engines:
 * - Wavetable (requires 256KB wavetable data)
 * - Chord (requires chord ratio data)
 * - Speech (requires LPC speech data)
 * - Modal (requires modal resonator data)
 * 
 * SD Card Structure Required:
 * /plaits/wavetable.bin    - 49,920 bytes (WAV_INTEGRATED_WAVES_SIZE)
 * /plaits/table_fold.bin   - LUT_FOLD_SIZE * 2 bytes  
 * /plaits/table_fold2.bin  - LUT_FOLD_2_SIZE * 2 bytes
 * 
 * Usage:
 *   // At startup, call before voice.Init()
 *   PlaitsLoadSdData();
 *   
 *   // Check loaded
 *   if (PlaitsSdDataLoaded()) {
 *       // Enable extra engines
 *   }
 * 
 * Note: This requires modifying Plaits voice.cc to use 
 * alternative data pointers instead of embedded resources
 */

#ifndef SD_CARD_DATA_H_
#define SD_CARD_DATA_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize and load all Plaits data from SD card
 * @return true if successful
 */
bool PlaitsLoadSdData(void);

/**
 * Check if SD data was loaded successfully  
 * @return true if data is loaded
 */
bool PlaitsSdDataLoaded(void);

/**
 * Get status string for display (OLED)
 * @return static string with status
 */
const char* PlaitsSdStatus(void);

#ifdef __cplusplus
}
#endif

#endif /* SD_CARD_DATA_H_ */