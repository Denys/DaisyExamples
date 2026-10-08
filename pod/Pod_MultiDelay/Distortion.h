#pragma once
#include "DigiMono.h"
namespace poddemo
{
// Bounded cubic shaping law used by the first-party Field drive (Dsp.h, Drive).
// Pod adaptation: no oversampling/alignment delay; smoothed gain, tone and enable.
class Distortion
{
    float gain_ = 12.6f, gainTarget_ = 12.6f;
    float coeff_ = 0, coeffTarget_ = 0, low_ = 0, blend_ = 0, blendTarget_ = 0;

  public:
    void Configure(float drive, float tone, bool enabled)
    {
        gainTarget_ = 1 + 29 * std::clamp(Finite(drive), 0.0f, 1.0f);
        const float hz
            = 1000 * std::pow(12.0f, std::clamp(Finite(tone), 0.0f, 1.0f));
        coeffTarget_ = 1 - std::exp(-6.28318530718f * hz / 48000);
        blendTarget_ = enabled ? 1 : 0;
    }
    void Init(float drive, float tone, bool enabled)
    {
        Configure(drive, tone, enabled);
        gain_  = gainTarget_;
        coeff_ = coeffTarget_;
        low_ = blend_ = 0;
    }
    float Process(float input)
    {
        const float clean = std::clamp(Finite(input), -1.0f, 1.0f);
        gain_ += (gainTarget_ - gain_) * .002f;
        coeff_ += (coeffTarget_ - coeff_) * .002f;
        const float x      = std::clamp(clean * gain_, -1.5f, 1.5f);
        const float shaped = x * (1 - x * x / 6.75f);
        low_ += (shaped - low_) * coeff_;
        const float step = 1.0f / 240;
        if(blend_ < blendTarget_)
            blend_ = std::min(blendTarget_, blend_ + step);
        else if(blend_ > blendTarget_)
            blend_ = std::max(blendTarget_, blend_ - step);
        // Exact clean identity once OFF, including at startup; keep filter warm.
        return blend_ == 0 ? clean : Finite(clean + blend_ * (low_ - clean));
    }
};
} // namespace poddemo
