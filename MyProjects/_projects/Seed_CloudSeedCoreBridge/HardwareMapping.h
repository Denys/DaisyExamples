#pragma once

#include "CloudSeedControlModel.h"
#include "CloudSeedInteractiveParameters.h"

#include "daisy_seed.h"

namespace cloudseedbridge
{
/**
 * Daisy Seed hardware adapter.
 *
 * Wiring assumption (adjust to your panel):
 *  A0 mix, A1 size, A2 decay, A3 diffusion,
 *  A4 pre-delay, A5 damping, A6 modulation amount, A7 modulation rate.
 */
class SeedMapping
{
  public:
    void Init(daisy::DaisySeed &seed)
    {
        daisy::AdcChannelConfig adc_cfg[8];
        for(size_t i = 0; i < 8; ++i)
            adc_cfg[i].InitSingle(seed.GetPin(static_cast<int>(15 + i)));
        seed.adc.Init(adc_cfg, 8);
        seed.adc.Start();
    }

    void ReadInto(daisy::DaisySeed &seed, ControlModel &controls)
    {
        controls.mix = seed.adc.GetFloat(CloudSeedPerformance::MIX_PARAM);
        controls.size = seed.adc.GetFloat(CloudSeedPerformance::SIZE_PARAM);
        controls.decay = seed.adc.GetFloat(CloudSeedPerformance::DECAY_PARAM);
        controls.diffusion
            = seed.adc.GetFloat(CloudSeedPerformance::DIFFUSION_PARAM);
        controls.pre_delay
            = seed.adc.GetFloat(CloudSeedPerformance::PRE_DELAY_PARAM);
        controls.damping
            = seed.adc.GetFloat(CloudSeedPerformance::DAMPING_PARAM);
        controls.modulation
            = seed.adc.GetFloat(CloudSeedPerformance::MODULATION_PARAM);
        controls.modulationrate
            = seed.adc.GetFloat(CloudSeedPerformance::MODULATION_RATE_PARAM);
        controls.Clamp();
    }
};

/**
 * Daisy Field mapping helper for future deployment.
 * Not compiled by this Seed example, but keeps controls independent now.
 */
struct FieldMappingLayout
{
    // Field knob plan:
    // K1 mix, K2 size, K3 decay, K4 diffusion,
    // K5 pre-delay, K6 damping, K7 modulation, K8 modulation-rate.
};

} // namespace cloudseedbridge
