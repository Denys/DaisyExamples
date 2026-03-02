#include "morph_processor.h"
#include <cmath>
#include <cstdlib> // for rand()

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace synth
{

void MorphProcessor::Init(float sample_rate)
{
    sample_rate_   = sample_rate;
    current_curve_ = MORPH_LINEAR;
    speed_         = 1.0f;
    lfo_enabled_   = false;
    lfo_phase_     = 0.0f;
    lfo_inc_       = 0.0f;
    lfo_value_     = 0.0f;
}

void MorphProcessor::SetCurve(MorphCurve curve)
{
    current_curve_ = curve;
}

void MorphProcessor::SetSpeed(float speed)
{
    speed_   = speed;
    lfo_inc_ = speed_ / sample_rate_;
}

void MorphProcessor::SetLfoEnabled(bool enabled)
{
    lfo_enabled_ = enabled;
}

float MorphProcessor::Process(float input_position)
{
    float modulated_pos = input_position;

    if(lfo_enabled_)
    {
        // Update LFO
        lfo_phase_ += lfo_inc_;
        if(lfo_phase_ >= 1.0f)
        {
            lfo_phase_ -= 1.0f;
        }

        // Generate LFO value (triangle wave for now)
        if(lfo_phase_ < 0.5f)
        {
            lfo_value_ = lfo_phase_ * 2.0f;
        }
        else
        {
            lfo_value_ = 2.0f - lfo_phase_ * 2.0f;
        }

        // Modulate position with LFO
        modulated_pos
            = input_position + lfo_value_ * 0.5f; // Scale LFO influence
        modulated_pos = fmaxf(0.0f, fminf(1.0f, modulated_pos)); // Clamp
    }

    // Apply morphing curve
    return ApplyCurve(modulated_pos, current_curve_);
}

float MorphProcessor::ApplyCurve(float input, MorphCurve curve)
{
    switch(curve)
    {
        case MORPH_LINEAR: return input;

        case MORPH_EXPONENTIAL: return input * input; // Simple exponential

        case MORPH_LOGARITHMIC: return sqrtf(input); // Simple logarithmic

        case MORPH_SINE: return sinf(input * M_PI * 2.0f) * 0.5f + 0.5f;

        case MORPH_TRIANGLE:
            if(input < 0.5f)
            {
                return input * 2.0f;
            }
            else
            {
                return 2.0f - input * 2.0f;
            }

        case MORPH_STEP: return (input > 0.5f) ? 1.0f : 0.0f;

        case MORPH_RANDOM:
            // Simple pseudo-random based on input
            return static_cast<float>(rand()) / RAND_MAX;

        case MORPH_CUSTOM:
            // Placeholder for custom curve
            return input;

        default: return input;
    }
}

} // namespace synth