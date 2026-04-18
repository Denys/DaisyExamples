#include "daisyhost/MidiEventTracker.h"

#include <iomanip>
#include <sstream>

namespace daisyhost
{
void MidiEventTracker::SetInputDeviceCounts(int availableInputs, int enabledInputs)
{
    std::lock_guard<std::mutex> lock(mutex_);
    availableInputs_ = availableInputs;
    enabledInputs_   = enabledInputs;
}

void MidiEventTracker::Record(const MidiMessageEvent& event)
{
    const std::string formatted = FormatEvent(event);
    if(formatted.empty())
    {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    recentMessages_.push_back(formatted);
    if(recentMessages_.size() > 8)
    {
        recentMessages_.erase(recentMessages_.begin());
    }
}

MidiEventTrackerSnapshot MidiEventTracker::GetSnapshot() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    MidiEventTrackerSnapshot snapshot;
    snapshot.availableInputs = availableInputs_;
    snapshot.enabledInputs   = enabledInputs_;
    snapshot.recentMessages  = recentMessages_;
    return snapshot;
}

std::string MidiEventTracker::FormatEvent(const MidiMessageEvent& event) const
{
    const std::uint8_t status = static_cast<std::uint8_t>(event.status & 0xF0);
    const int          channel
        = static_cast<int>(event.status & 0x0F) + 1;

    std::ostringstream stream;
    switch(status)
    {
        case 0x80:
            stream << "Note Off ch" << channel << " note " << static_cast<int>(event.data1)
                   << " vel " << static_cast<int>(event.data2);
            return stream.str();

        case 0x90:
            if(event.data2 == 0)
            {
                stream << "Note Off ch" << channel << " note "
                       << static_cast<int>(event.data1);
            }
            else
            {
                stream << "Note On ch" << channel << " note "
                       << static_cast<int>(event.data1) << " vel "
                       << static_cast<int>(event.data2);
            }
            return stream.str();

        case 0xB0:
            stream << "CC " << static_cast<int>(event.data1) << " ch" << channel
                   << " val " << static_cast<int>(event.data2);
            return stream.str();

        case 0xC0:
            stream << "Program ch" << channel << " " << static_cast<int>(event.data1);
            return stream.str();

        default:
            if(event.status >= 0xF8)
            {
                return {};
            }
            stream << "Status 0x" << std::hex << std::uppercase
                   << static_cast<int>(event.status) << std::dec;
            return stream.str();
    }
}
} // namespace daisyhost
