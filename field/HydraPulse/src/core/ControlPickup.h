#pragma once
#include <algorithm>
#include <cmath>

namespace hydrapulse::core
{
class ControlPickup
{
  public:
    ControlPickup() noexcept : ControlPickup(0.015f) {}
    explicit ControlPickup(float tolerance) noexcept
        : tolerance_(std::isfinite(tolerance) ? std::clamp(tolerance, 0.0f, 0.1f) : 0.015f)
    {
    }
    void Reset(float target, float physical) noexcept
    {
        target_ = std::isfinite(target) ? std::clamp(target, 0.0f, 1.0f) : 0.0f;
        value_ = target_;
        has_previous_ = std::isfinite(physical);
        previous_ = has_previous_ ? std::clamp(physical, 0.0f, 1.0f) : target_;
        captured_ = has_previous_ && std::fabs(previous_ - target_) <= tolerance_;
        if (captured_)
            value_ = previous_;
    }
    bool Update(float physical) noexcept
    {
        if (!std::isfinite(physical))
            return false;
        const float current = std::clamp(physical, 0.0f, 1.0f);
        if (!has_previous_)
        {
            previous_ = current;
            has_previous_ = true;
        }
        const bool crossed =
            (previous_ <= target_ && current >= target_) || (previous_ >= target_ && current <= target_);
        captured_ = captured_ || crossed || std::fabs(current - target_) <= tolerance_;
        previous_ = current;
        if (captured_)
            value_ = current;
        return captured_;
    }
    bool Captured() const noexcept
    {
        return captured_;
    }
    float Value() const noexcept
    {
        return value_;
    }
    float Target() const noexcept
    {
        return target_;
    }

  private:
    float tolerance_, target_{0}, value_{0}, previous_{0};
    bool has_previous_{false}, captured_{false};
};
} // namespace hydrapulse::core
