#pragma once

#include <string>
#include <unordered_map>

#include "daisyhost/MidiLearnMap.h"

namespace daisyhost
{
struct HostSessionState
{
    std::unordered_map<std::string, float> controlValues;
    std::unordered_map<std::string, float> cvValues;
    std::unordered_map<std::string, bool>  gateValues;
    MidiLearnMap                           midiLearn;

    std::string Serialize() const;
    static HostSessionState Deserialize(const std::string& text);
};
} // namespace daisyhost
