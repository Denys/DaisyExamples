#pragma once
#include <stdint.h>
#include "daisy.h"  // For DSY_SDRAM_BSS macro

namespace synth
{

const int WAVETABLE_SIZE          = 2048;
const int NUM_WAVETABLES_PER_BANK = 8;
const int NUM_BANKS               = 8;

// Total wavetable data size
const int TOTAL_WAVETABLE_SIZE
    = NUM_BANKS * NUM_WAVETABLES_PER_BANK * WAVETABLE_SIZE;

// Wavetable data in SDRAM
extern float DSY_SDRAM_BSS wavetable_data[TOTAL_WAVETABLE_SIZE];

// Bank indices
enum WavetableBank
{
    BANK_SINE = 0,
    BANK_SAWTOOTH,
    BANK_SQUARE,
    BANK_TRIANGLE,
    BANK_CUSTOM1,
    BANK_CUSTOM2,
    BANK_CUSTOM3,
    BANK_CUSTOM4
};

// Get pointer to specific bank
const float* GetWavetableBank(WavetableBank bank);

// Explicit initialization function
void InitializeWavetables();

} // namespace synth