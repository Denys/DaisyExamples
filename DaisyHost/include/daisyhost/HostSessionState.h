#pragma once

#include <string>
#include <unordered_map>

#include "daisyhost/MidiLearnMap.h"

namespace daisyhost
{
struct HostSessionState
{
    std::string                             appId;
    std::unordered_map<std::string, float> controlValues;
    std::unordered_map<std::string, float> parameterValues;
    std::unordered_map<std::string, float> cvValues;
    std::unordered_map<std::string, bool>  gateValues;
    MidiLearnMap                           midiLearn;
    std::uint32_t                          randomSeed = 0;

    std::string Serialize() const;
    static HostSessionState Deserialize(const std::string& text);
};
} // namespace daisyhost
