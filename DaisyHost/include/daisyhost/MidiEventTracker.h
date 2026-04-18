#pragma once

#include <mutex>
#include <string>
#include <vector>

#include "daisyhost/HostedAppCore.h"

namespace daisyhost
{
struct MidiEventTrackerSnapshot
{
    int                      availableInputs = 0;
    int                      enabledInputs   = 0;
    std::vector<std::string> recentMessages;
};

class MidiEventTracker
{
  public:
    void SetInputDeviceCounts(int availableInputs, int enabledInputs);
    void Record(const MidiMessageEvent& event);
    MidiEventTrackerSnapshot GetSnapshot() const;

  private:
    std::string FormatEvent(const MidiMessageEvent& event) const;

    mutable std::mutex       mutex_;
    int                      availableInputs_ = 0;
    int                      enabledInputs_   = 0;
    std::vector<std::string> recentMessages_;
};
} // namespace daisyhost
