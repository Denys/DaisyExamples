#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

namespace hydrapulse::dsp
{
constexpr float kPi = 3.14159265358979323846f;
inline float Unit(float x, float fallback = 0.0f) noexcept
{
    return std::isfinite(x) ? std::clamp(x, 0.0f, 1.0f) : fallback;
}

inline float ExpMap(float x, float lo, float hi) noexcept
{
    return lo * std::exp(std::log(hi / lo) * Unit(x));
}

class Ramp
{
  public:
    void Reset(float value) noexcept
    {
        value_ = target_ = std::isfinite(value) ? value : 0;
        left_ = 0;
        delta_ = 0;
    }
    void Set(float target, std::uint32_t samples) noexcept
    {
        if (!std::isfinite(target) || target == target_)
            return;
        target_ = target;
        left_ = std::max<std::uint32_t>(samples, 1u);
        delta_ = (target_ - value_) / float(left_);
    }
    float Process() noexcept
    {
        if (left_ && --left_ == 0)
            value_ = target_;
        else if (left_)
            value_ += delta_;
        return value_;
    }
    float Value() const noexcept
    {
        return value_;
    }
    float Target() const noexcept
    {
        return target_;
    }
    bool Settled() const noexcept
    {
        return left_ == 0;
    }

  private:
    float value_{0}, target_{0}, delta_{0};
    std::uint32_t left_{0};
};

class Noise
{
  public:
    void Seed(std::uint32_t x) noexcept
    {
        state_ = x ? x : 1;
    }
    float Process() noexcept
    {
        state_ ^= state_ << 13;
        state_ ^= state_ >> 17;
        state_ ^= state_ << 5;
        return float(state_ >> 8) * (1.0f / 8388608.0f) - 1.0f;
    }

  private:
    std::uint32_t state_{1};
};

class SineTable
{
  public:
    void Init() noexcept
    {
        for (std::size_t i = 0; i < kSize; ++i)
            table_[i] = std::sin(2.0f * kPi * float(i) / float(kSize));
        table_[kSize] = table_[0];
    }
    // Caller supplies finite cycles in a bounded range. Voices enforce this.
    float Read(float cycles) const noexcept
    {
        if (!std::isfinite(cycles) || std::fabs(cycles) > 32767.0f)
            return 0;
        cycles -= float(static_cast<int>(cycles));
        if (cycles < 0)
            cycles += 1;
        const float position = cycles * float(kSize);
        const auto index = static_cast<std::size_t>(position);
        if (index >= kSize)
            return table_[0];
        const float fraction = position - float(index);
        return table_[index] + fraction * (table_[index + 1] - table_[index]);
    }

  private:
    static constexpr std::size_t kSize = 1024;
    std::array<float, kSize + 1> table_{};
};

class Envelope
{
  public:
    void Init(float sr) noexcept
    {
        sr_ = sr;
        level_ = 0;
        remaining_ = 0;
        releasing_ = false;
        SetDecay(0.2f);
    }
    void SetDecay(float t60) noexcept
    {
        coefficient_ = std::exp(-6.90775527898f / (std::clamp(t60, 0.015f, 3.0f) * sr_));
    }
    void Trigger(float velocity, float attack_seconds = 0.001f) noexcept
    {
        target_ = Unit(velocity);
        if (target_ <= 0)
            return;
        remaining_ = std::max<std::uint32_t>(1u, static_cast<std::uint32_t>(sr_ * attack_seconds));
        delta_ = (target_ - level_) / float(remaining_);
        releasing_ = false;
    }
    void Release(float seconds = 0.005f) noexcept
    {
        if (releasing_ && target_ == 0)
            return;
        target_ = 0;
        remaining_ = std::max<std::uint32_t>(1u, static_cast<std::uint32_t>(seconds * sr_));
        delta_ = -level_ / float(remaining_);
        releasing_ = true;
    }
    float Process() noexcept
    {
        if (remaining_)
        {
            if (--remaining_ == 0)
                level_ = target_;
            else
                level_ += delta_;
        }
        else if (!releasing_)
        {
            level_ *= coefficient_;
            if (level_ < 1.0e-6f)
                level_ = 0;
        }
        return level_;
    }
    float Value() const noexcept
    {
        return level_;
    }
    void Reset() noexcept
    {
        level_ = 0;
        remaining_ = 0;
        releasing_ = false;
    }

  private:
    float sr_{48000}, level_{0}, coefficient_{0.999f}, target_{0}, delta_{0};
    std::uint32_t remaining_{0};
    bool releasing_{false};
};

class DcBlock
{
  public:
    void Init(float sr) noexcept
    {
        r_ = std::exp(-2.0f * kPi * 10.0f / sr);
        Reset();
    }
    void Reset() noexcept
    {
        x_ = y_ = 0;
    }
    float Process(float x) noexcept
    {
        const float y = x - x_ + r_ * y_;
        x_ = x;
        y_ = y;
        if (std::fabs(y_) < 1.0e-12f)
            y_ = 0;
        return y_;
    }

  private:
    float r_{0.9987f}, x_{0}, y_{0};
};

inline float Saturate(float x) noexcept
{
    if (!std::isfinite(x))
        return 0;
    x = std::clamp(x, -3.0f, 3.0f);
    const float xx = x * x;
    return x * (27.0f + xx) / (27.0f + 9.0f * xx);
}
} // namespace hydrapulse::dsp
