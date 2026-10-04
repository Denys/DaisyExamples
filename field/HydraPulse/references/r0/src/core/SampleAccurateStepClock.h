#pragma once

#include <cmath>
#include <cstdint>
#include <limits>

namespace hydrapulse::core
{

class SampleAccurateStepClock
{
  public:
    static constexpr std::uint64_t kBpmScale = 1'000'000u;

    explicit SampleAccurateStepClock(std::uint32_t sample_rate = 48'000u,
                                     double        bpm         = 120.0,
                                     std::uint32_t steps_per_beat = 4u) noexcept
        : sample_rate_(sample_rate), steps_per_beat_(steps_per_beat)
    {
        (void)SetTempoBpm(bpm);
        RecomputeThreshold();
    }

    bool SetSampleRate(std::uint32_t sample_rate) noexcept
    {
        if(sample_rate == 0u)
            return false;
        sample_rate_ = sample_rate;
        RecomputeThreshold();
        Reset();
        return true;
    }

    bool SetTempoBpm(double bpm) noexcept
    {
        if(!std::isfinite(bpm) || bpm <= 0.0 || bpm > 1000.0)
            return false;

        const long double scaled = static_cast<long double>(bpm) * kBpmScale;
        if(scaled > static_cast<long double>(std::numeric_limits<std::uint64_t>::max()))
            return false;

        tempo_u_bpm_ = static_cast<std::uint64_t>(std::llround(scaled));
        RecomputeIncrement();
        Reset();
        return tempo_u_bpm_ != 0u;
    }

    void Reset() noexcept { phase_ = 0u; }

    [[nodiscard]] bool ProcessSample() noexcept
    {
        phase_ += phase_increment_;
        if(phase_ >= threshold_)
        {
            phase_ -= threshold_;
            return true;
        }
        return false;
    }

    [[nodiscard]] std::uint64_t SamplesUntilNextStep() const noexcept
    {
        if(phase_increment_ == 0u || threshold_ <= phase_)
            return 0u;

        const auto remaining = threshold_ - phase_;
        return (remaining + phase_increment_ - 1u) / phase_increment_;
    }

    [[nodiscard]] std::uint32_t SampleRate() const noexcept { return sample_rate_; }

    [[nodiscard]] double TempoBpm() const noexcept
    {
        return static_cast<double>(tempo_u_bpm_) / static_cast<double>(kBpmScale);
    }

    [[nodiscard]] std::uint32_t StepsPerBeat() const noexcept { return steps_per_beat_; }

  private:
    void RecomputeThreshold() noexcept
    {
        threshold_ = static_cast<std::uint64_t>(sample_rate_) * 60u * kBpmScale;
        RecomputeIncrement();
    }

    void RecomputeIncrement() noexcept
    {
        phase_increment_ = tempo_u_bpm_ * static_cast<std::uint64_t>(steps_per_beat_);
    }

    std::uint32_t sample_rate_{48'000u};
    std::uint32_t steps_per_beat_{4u};
    std::uint64_t tempo_u_bpm_{120u * kBpmScale};
    std::uint64_t threshold_{48'000u * 60u * kBpmScale};
    std::uint64_t phase_increment_{120u * kBpmScale * 4u};
    std::uint64_t phase_{0u};
};

} // namespace hydrapulse::core
