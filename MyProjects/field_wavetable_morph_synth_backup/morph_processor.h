#pragma once
#include <stdint.h>

namespace synth
{

enum MorphCurve
{
    MORPH_LINEAR = 0,
    MORPH_EXPONENTIAL,
    MORPH_LOGARITHMIC,
    MORPH_SINE,
    MORPH_TRIANGLE,
    MORPH_STEP,
    MORPH_RANDOM,
    MORPH_CUSTOM,
    MORPH_COUNT
};

class MorphProcessor
{
  public:
    MorphProcessor() {}
    ~MorphProcessor() {}

    void  Init(float sample_rate);
    void  SetCurve(MorphCurve curve);
    void  SetSpeed(float speed); // 0-10 Hz
    void  SetLfoEnabled(bool enabled);
    float Process(float input_position); // Returns modulated position 0-1

  private:
    float      sample_rate_;
    MorphCurve current_curve_;
    float      speed_;
    bool       lfo_enabled_;
    float      lfo_phase_;
    float      lfo_inc_;

    float ApplyCurve(float input, MorphCurve curve);
    float lfo_value_; // Cached LFO value
};

} // namespace synth