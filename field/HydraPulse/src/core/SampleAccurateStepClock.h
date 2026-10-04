#pragma once
#include <cmath>
#include <cstdint>

namespace hydrapulse::core
{
// One event per sample maximum. Supported domain ensures this bound.
class SampleAccurateStepClock
{
  public:
    static constexpr std::uint64_t kBpmScale = 1'000'000u;
    explicit SampleAccurateStepClock(std::uint32_t sample_rate = 48'000u, double bpm = 120.0,
                                     std::uint32_t steps_per_beat = 4u) noexcept
    {
        if (sample_rate >= 8'000u && sample_rate <= 192'000u)
            sample_rate_ = sample_rate;
        if (steps_per_beat >= 1u && steps_per_beat <= 64u)
            steps_per_beat_ = steps_per_beat;
        threshold_ = std::uint64_t{sample_rate_} * 60u * kBpmScale;
        increment_ = tempo_ * steps_per_beat_;
        (void)SetTempoBpm(bpm);
    }
    bool SetSampleRate(std::uint32_t rate) noexcept
    {
        if (rate < 8'000u || rate > 192'000u)
            return false;
        sample_rate_ = rate;
        threshold_ = std::uint64_t{rate} * 60u * kBpmScale;
        Reset();
        return true;
    }
    // Tempo changes preserve beat phase. Only an explicit transport reset restarts it.
    bool SetTempoBpm(double bpm) noexcept
    {
        if (!std::isfinite(bpm) || bpm < 1.0 || bpm > 1000.0)
            return false;
        const auto proposed = static_cast<std::uint64_t>(std::llround(bpm * double(kBpmScale)));
        tempo_ = proposed;
        increment_ = tempo_ * steps_per_beat_;
        return true;
    }
    void Reset() noexcept
    {
        phase_ = 0;
    }
    bool ProcessSample() noexcept
    {
        phase_ += increment_;
        if (phase_ < threshold_)
            return false;
        phase_ -= threshold_;
        return true;
    }
    std::uint64_t SamplesUntilNextStep() const noexcept
    {
        return (threshold_ - phase_ + increment_ - 1u) / increment_;
    }
    std::uint32_t SampleRate() const noexcept
    {
        return sample_rate_;
    }
    double TempoBpm() const noexcept
    {
        return double(tempo_) / double(kBpmScale);
    }
    std::uint32_t StepsPerBeat() const noexcept
    {
        return steps_per_beat_;
    }

  private:
    std::uint32_t sample_rate_{48'000u}, steps_per_beat_{4u};
    std::uint64_t tempo_{120u * kBpmScale};
    std::uint64_t threshold_{48'000ull * 60u * kBpmScale};
    std::uint64_t increment_{120u * kBpmScale * 4u}, phase_{0};
};
} // namespace hydrapulse::core
