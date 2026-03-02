#pragma once
#include <stdint.h>

namespace synth
{

class WavetableOsc
{
  public:
    WavetableOsc() {}
    ~WavetableOsc() {}

    void  Init(float        sample_rate,
               const float* wavetable_data,
               int          num_tables,
               int          table_size);
    void  SetFrequency(float freq);
    void  SetWavetable(const float* wavetable_data);
    void  SetPosition(float pos); // 0-1, selects which table to morph between
    float Process();

  private:
    float        sample_rate_;
    const float* wavetable_data_; // Pointer to wavetable bank data
    int          num_tables_;
    int          table_size_;
    float        phase_;
    float        phase_inc_;
    float        position_; // 0-1
    int          current_table_idx_;
    float        morph_frac_; // fractional part for interpolation

    float InterpolateTable(int table_idx, float phase);
};

} // namespace synth