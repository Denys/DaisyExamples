#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace hydrapulse::core
{
// Sixteenth clock. A swing pair remains exactly two nominal intervals in rational arithmetic.
// ProcessSample is called once for each sample AFTER the initial step-zero sample.
class GrooveClock
{
  public:
    void Init(std::uint32_t rate) noexcept
    {
        rate_ = (rate >= 8'000 && rate <= 192'000) ? rate : 48'000;
        base_ = std::uint64_t{rate_} * 60ull * 1'000'000ull;
        Reset();
    }
    bool SetTempo(double bpm) noexcept
    {
        if (!std::isfinite(bpm) || bpm < 40.0 || bpm > 240.0)
            return false;
        tempo_ = static_cast<std::uint64_t>(std::llround(bpm * 1'000'000.0));
        increment_ = tempo_ * 4ull * 1000ull;
        return true;
    }
    bool SetSwing(float amount) noexcept
    {
        if (!std::isfinite(amount) || amount < 0 || amount > 1)
            return false;
        pending_swing_ = static_cast<std::uint16_t>(std::lround(amount * 400.0f));
        return true;
    }
    void Reset() noexcept
    {
        phase_ = 0;
        long_interval_ = true;
        swing_ = pending_swing_;
        threshold_ = base_ * (1000ull + swing_);
    }
    bool ProcessSample() noexcept
    {
        phase_ += increment_;
        if (phase_ < threshold_)
            return false;
        phase_ -= threshold_;
        long_interval_ = !long_interval_;
        if (long_interval_)
            swing_ = pending_swing_; // commit swing only at pair boundary
        threshold_ = base_ * (long_interval_ ? 1000ull + swing_ : 1000ull - swing_);
        return true;
    }
    double Tempo() const noexcept
    {
        return double(tempo_) / 1'000'000.0;
    }
    float Swing() const noexcept
    {
        return float(pending_swing_) / 400.0f;
    }

  private:
    std::uint32_t rate_{48'000};
    std::uint64_t base_{48'000ull * 60ull * 1'000'000ull};
    std::uint64_t threshold_{48'000ull * 60ull * 1'000'000ull * 1000ull};
    std::uint64_t tempo_{120'000'000ull}, increment_{480'000'000'000ull}, phase_{0};
    std::uint16_t swing_{0}, pending_swing_{0};
    bool long_interval_{true};
};
} // namespace hydrapulse::core
