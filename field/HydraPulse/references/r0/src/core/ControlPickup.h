#pragma once

#include <algorithm>
#include <cmath>

namespace hydrapulse::core
{

class ControlPickup
{
  public:
    explicit ControlPickup(float tolerance = 0.015f) noexcept
        : tolerance_(std::max(0.0f, tolerance))
    {
    }

    void Reset(float target, float physical) noexcept
    {
        target_       = Clamp(target);
        value_        = target_;
        previous_     = Clamp(physical);
        has_previous_ = true;
        captured_     = std::fabs(previous_ - target_) <= tolerance_;
        if(captured_)
            value_ = previous_;
    }

    [[nodiscard]] bool Update(float physical) noexcept
    {
        const float current = Clamp(physical);

        if(!has_previous_)
        {
            previous_     = current;
            has_previous_ = true;
        }

        if(captured_)
        {
            previous_ = current;
            value_    = current;
            return true;
        }

        const bool inside = std::fabs(current - target_) <= tolerance_;
        const bool crossed = (previous_ <= target_ && current >= target_)
                             || (previous_ >= target_ && current <= target_);

        previous_ = current;
        if(inside || crossed)
        {
            captured_ = true;
            value_    = current;
            return true;
        }

        return false;
    }

    [[nodiscard]] bool Captured() const noexcept { return captured_; }
    [[nodiscard]] float Value() const noexcept { return value_; }
    [[nodiscard]] float Target() const noexcept { return target_; }

  private:
    [[nodiscard]] static float Clamp(float value) noexcept
    {
        return std::clamp(value, 0.0f, 1.0f);
    }

    float tolerance_{0.015f};
    float target_{0.0f};
    float value_{0.0f};
    float previous_{0.0f};
    bool  has_previous_{false};
    bool  captured_{false};
};

} // namespace hydrapulse::core
