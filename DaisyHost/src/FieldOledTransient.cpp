#include "daisyhost/FieldOledTransient.h"

#include <algorithm>

namespace daisyhost
{
namespace
{
float Clamp01(float value)
{
    return std::max(0.0f, std::min(1.0f, value));
}
} // namespace

void FieldOledTransient::Show(const FieldOledTransientRequest& request,
                              double                           holdMs)
{
    title_         = request.title;
    value_         = request.value;
    detailLines_   = request.detailLines;
    remainingMs_   = holdMs > 0.0 ? holdMs : kDefaultHoldMs;
    visible_       = true;
    held_          = request.held;
    hasBar_        = request.hasBar;
    barNormalized_ = Clamp01(request.barNormalized);
}

void FieldOledTransient::Tick(double deltaMs)
{
    if(!visible_ || held_)
    {
        return;
    }

    remainingMs_ -= std::max(0.0, deltaMs);
    if(remainingMs_ <= 0.0)
    {
        Clear();
    }
}

void FieldOledTransient::SetHeld(bool held)
{
    const bool wasHeld = held_;
    held_ = held;
    if(visible_ && wasHeld && !held_)
    {
        remainingMs_ = kDefaultHoldMs;
    }
}

bool FieldOledTransient::IsVisible() const
{
    return visible_;
}

void FieldOledTransient::Clear()
{
    title_.clear();
    value_.clear();
    detailLines_.clear();
    remainingMs_   = 0.0;
    visible_       = false;
    held_          = false;
    hasBar_        = false;
    barNormalized_ = 0.0f;
}

void FieldOledTransient::ApplyToDisplay(DisplayModel& display) const
{
    if(!visible_)
    {
        return;
    }

    display.mode  = DisplayMode::kStatus;
    display.title = title_;
    display.texts.clear();
    display.bars.clear();
    display.texts.push_back({0, 0, title_, true});
    display.texts.push_back({0, 16, value_, false});

    int y = hasBar_ ? 38 : 32;
    if(hasBar_)
    {
        display.bars.push_back({0, 32, 118, 6, barNormalized_});
    }

    for(const auto& line : detailLines_)
    {
        display.texts.push_back({0, y, line, false});
        y += 10;
        if(y > 54)
        {
            break;
        }
    }
}
} // namespace daisyhost
