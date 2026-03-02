#include "wavetables.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace synth
{

// Generate basic wavetables
void GenerateSineWave(float* table, int size)
{
    for(int i = 0; i < size; i++)
    {
        table[i] = sinf(2.0f * M_PI * i / size);
    }
}

void GenerateSawtoothWave(float* table, int size)
{
    for(int i = 0; i < size; i++)
    {
        table[i] = 2.0f * (i / (float)size) - 1.0f;
    }
}

void GenerateSquareWave(float* table, int size)
{
    for(int i = 0; i < size; i++)
    {
        table[i] = (i < size / 2) ? 1.0f : -1.0f;
    }
}

void GenerateTriangleWave(float* table, int size)
{
    for(int i = 0; i < size; i++)
    {
        if(i < size / 2)
        {
            table[i] = 4.0f * (i / (float)size) - 1.0f;
        }
        else
        {
            table[i] = 3.0f - 4.0f * (i / (float)size);
        }
    }
}

// Variations for morphing
// Variations for morphing
void GenerateSineVariation(float* table, int size, float phase_offset)
{
    for(int i = 0; i < size; i++)
    {
        table[i] = sinf(2.0f * M_PI * i / size + phase_offset);
    }
}

void GenerateSawtoothVariation(float* table, int size, float slope)
{
    for(int i = 0; i < size; i++)
    {
        table[i] = slope * (2.0f * (i / (float)size) - 1.0f);
    }
}

// Wavetable data storage
// Wavetable data storage in SDRAM (defined in header with DSY_SDRAM_BSS)
// Wavetable data storage in SDRAM (defined in header with DSY_SDRAM_BSS)
float DSY_SDRAM_BSS wavetable_data[TOTAL_WAVETABLE_SIZE];

void InitializeWavetables()
{
    // Bank 0: Sine waves
    for(int i = 0; i < NUM_WAVETABLES_PER_BANK; i++)
    {
        float* table
            = wavetable_data
              + (BANK_SINE * NUM_WAVETABLES_PER_BANK + i) * WAVETABLE_SIZE;
        GenerateSineVariation(table, WAVETABLE_SIZE, i * M_PI / 4.0f);
    }

    // Bank 1: Sawtooth waves
    for(int i = 0; i < NUM_WAVETABLES_PER_BANK; i++)
    {
        float* table
            = wavetable_data
              + (BANK_SAWTOOTH * NUM_WAVETABLES_PER_BANK + i) * WAVETABLE_SIZE;
        GenerateSawtoothVariation(table, WAVETABLE_SIZE, 1.0f + i * 0.2f);
    }

    // Bank 2: Square waves (variations with different duty cycles)
    for(int i = 0; i < NUM_WAVETABLES_PER_BANK; i++)
    {
        float* table
            = wavetable_data
              + (BANK_SQUARE * NUM_WAVETABLES_PER_BANK + i) * WAVETABLE_SIZE;
        int duty = WAVETABLE_SIZE / 2 + i * 20;
        for(int j = 0; j < WAVETABLE_SIZE; j++)
        {
            table[j] = (j < duty) ? 1.0f : -1.0f;
        }
    }

    // Bank 3: Triangle waves
    for(int i = 0; i < NUM_WAVETABLES_PER_BANK; i++)
    {
        float* table
            = wavetable_data
              + (BANK_TRIANGLE * NUM_WAVETABLES_PER_BANK + i) * WAVETABLE_SIZE;
        GenerateTriangleWave(table, WAVETABLE_SIZE);
        // Add some variation
        for(int j = 0; j < WAVETABLE_SIZE; j++)
        {
            table[j] *= (1.0f + i * 0.1f);
        }
    }

    // Banks 4-7: Custom/Placeholder (use sine for now)
    for(int bank = BANK_CUSTOM1; bank <= BANK_CUSTOM4; bank++)
    {
        for(int i = 0; i < NUM_WAVETABLES_PER_BANK; i++)
        {
            float* table
                = wavetable_data
                  + (bank * NUM_WAVETABLES_PER_BANK + i) * WAVETABLE_SIZE;
            GenerateSineVariation(
                table, WAVETABLE_SIZE, i * M_PI / 8.0f + bank * M_PI / 16.0f);
        }
    }
}

const float* GetWavetableBank(WavetableBank bank)
{
    return wavetable_data + (bank * NUM_WAVETABLES_PER_BANK * WAVETABLE_SIZE);
}

} // namespace synth