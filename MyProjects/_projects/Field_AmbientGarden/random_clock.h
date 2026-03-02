#pragma once
#include <cstdint>
#include <cstdlib>

/**
 * RandomClock - Sample-accurate trigger generator with variable density.
 *
 * Generates trigger events at randomized intervals. Density controls
 * the average event rate. Each interval has +/- 20% jitter for
 * humanized timing.
 */
class RandomClock
{
  public:
    void Init(float sample_rate)
    {
        sample_rate_ = sample_rate;
        density_     = 0.4f;
        counter_     = 0;
        triggered_   = false;
        frozen_      = false;
        next_trigger_ = CalculateInterval();
    }

    /** Call once per sample in audio callback. Returns true on trigger. */
    bool Process()
    {
        triggered_ = false;
        if(frozen_)
            return false;

        counter_++;
        if(counter_ >= next_trigger_)
        {
            counter_      = 0;
            next_trigger_ = CalculateInterval();
            triggered_    = true;
        }
        return triggered_;
    }

    bool HasTriggered() const { return triggered_; }

    /** Set density 0.0 (very sparse ~2s) to 1.0 (very dense ~50ms). */
    void SetDensity(float density)
    {
        density_ = (density < 0.0f) ? 0.0f : (density > 1.0f) ? 1.0f : density;
    }

    void SetFreeze(bool freeze) { frozen_ = freeze; }
    bool IsFrozen() const { return frozen_; }

  private:
    uint32_t CalculateInterval()
    {
        // Exponential mapping: sparse at low density, rapid at high
        float inv = 1.0f - density_;
        float ms  = 50.0f + inv * inv * inv * 1950.0f;

        // Humanize: +/- 20% jitter
        float jitter = 0.8f + (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 0.4f;
        ms *= jitter;

        uint32_t samples = static_cast<uint32_t>(ms * sample_rate_ / 1000.0f);
        return (samples < 1) ? 1 : samples;
    }

    float    sample_rate_;
    float    density_;
    uint32_t counter_;
    uint32_t next_trigger_;
    bool     triggered_;
    bool     frozen_;
};
