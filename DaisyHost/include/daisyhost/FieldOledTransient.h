#pragma once

#include <string>
#include <vector>

#include "daisyhost/DisplayModel.h"

namespace daisyhost
{
struct FieldOledTransientRequest
{
    std::string              title;
    std::string              value;
    std::vector<std::string> detailLines;
    bool                     held          = false;
    bool                     hasBar        = false;
    float                    barNormalized = 0.0f;
};

class FieldOledTransient
{
  public:
    static constexpr double kDefaultHoldMs = 2000.0;

    void Show(const FieldOledTransientRequest& request,
              double                           holdMs = kDefaultHoldMs);
    void Tick(double deltaMs);
    void SetHeld(bool held);
    bool IsVisible() const;
    void Clear();
    void ApplyToDisplay(DisplayModel& display) const;

  private:
    std::string              title_;
    std::string              value_;
    std::vector<std::string> detailLines_;
    double                   remainingMs_   = 0.0;
    bool                     visible_       = false;
    bool                     held_          = false;
    bool                     hasBar_        = false;
    float                    barNormalized_ = 0.0f;
};
} // namespace daisyhost
