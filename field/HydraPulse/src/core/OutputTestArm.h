#pragma once
namespace hydrapulse::core
{
// A control held during boot must first be released before outputs can be tested.
class OutputTestArm
{
  public:
    bool Update(bool hold, bool other_switch) noexcept
    {
        if (!hold)
            released_ = true;
        return released_ && hold && !other_switch;
    }

  private:
    bool released_{false};
};
} // namespace hydrapulse::core
