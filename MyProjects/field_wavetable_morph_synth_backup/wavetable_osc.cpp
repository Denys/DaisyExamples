#include "wavetable_osc.h"
#include <cmath>

namespace synth
{

void WavetableOsc::Init(float        sample_rate,
                        const float* wavetable_data,
                        int          num_tables,
                        int          table_size)
{
    sample_rate_       = sample_rate;
    wavetable_data_    = wavetable_data;
    num_tables_        = num_tables;
    table_size_        = table_size;
    phase_             = 0.0f;
    phase_inc_         = 0.0f;
    position_          = 0.0f;
    current_table_idx_ = 0;
    morph_frac_        = 0.0f;
}

void WavetableOsc::SetFrequency(float freq)
{
    phase_inc_ = freq / sample_rate_;
}

void WavetableOsc::SetWavetable(const float* wavetable_data)
{
    wavetable_data_ = wavetable_data;
}

void WavetableOsc::SetPosition(float pos)
{
    position_ = pos;
    // Calculate which tables to interpolate between
    float table_pos    = pos * (num_tables_ - 1);
    current_table_idx_ = static_cast<int>(table_pos);
    morph_frac_        = table_pos - current_table_idx_;

    // Clamp to valid range
    if(current_table_idx_ >= num_tables_ - 1)
    {
        current_table_idx_ = num_tables_ - 2;
        morph_frac_        = 1.0f;
    }
}

float WavetableOsc::Process()
{
    // Get samples from two adjacent tables
    float sample1 = InterpolateTable(current_table_idx_, phase_);
    float sample2 = InterpolateTable(current_table_idx_ + 1, phase_);

    // Linear interpolation between tables
    float output = sample1 * (1.0f - morph_frac_) + sample2 * morph_frac_;

    // Update phase
    phase_ += phase_inc_;
    if(phase_ >= 1.0f)
    {
        phase_ -= 1.0f;
    }

    return output;
}

float WavetableOsc::InterpolateTable(int table_idx, float phase)
{
    // Linear interpolation within table
    float index = phase * (table_size_ - 1);
    int   idx   = static_cast<int>(index);
    float frac  = index - idx;

    // Get table offset
    // Get table offset
    const float* table = wavetable_data_ + (table_idx * table_size_);

    // Linear interpolation
    float a = table[idx];
    float b = table[(idx + 1) % table_size_]; // Wrap around for safety
    return a + (b - a) * frac;
}

} // namespace synth