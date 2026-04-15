#pragma once

#include <algorithm>

namespace cloudseedbridge
{
/**
 * Control-independent parameter model.
 *
 * Any hardware mapping (Seed ADC, Field knobs, MIDI CC, etc.) writes to this
 * normalized [0..1] domain. DSP/engine mapping consumes this model.
 */
struct ControlModel
{
    float mix            = 0.5f;
    float size           = 0.55f;
    float decay          = 0.65f;
    float diffusion      = 0.60f;
    float pre_delay      = 0.05f;
    float damping        = 0.45f;
    float modulation     = 0.30f;
    float modulationrate = 0.25f;

    void Clamp()
    {
        mix            = std::clamp(mix, 0.0f, 1.0f);
        size           = std::clamp(size, 0.0f, 1.0f);
        decay          = std::clamp(decay, 0.0f, 1.0f);
        diffusion      = std::clamp(diffusion, 0.0f, 1.0f);
        pre_delay      = std::clamp(pre_delay, 0.0f, 1.0f);
        damping        = std::clamp(damping, 0.0f, 1.0f);
        modulation     = std::clamp(modulation, 0.0f, 1.0f);
        modulationrate = std::clamp(modulationrate, 0.0f, 1.0f);
    }
};

} // namespace cloudseedbridge
