#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "daisyhost/HostModulation.h"
#include "daisyhost/MidiLearnMap.h"

namespace daisyhost
{
struct HostSessionNodeState
{
    std::string   nodeId;
    std::string   appId;
    std::uint32_t randomSeed = 0;
};

struct HostSessionRoute
{
    std::string sourcePortId;
    std::string destPortId;
};

struct HostSessionModulationLaneState
{
    std::string        nodeId;
    std::string        parameterId;
    int                slotIndex = 0;
    HostModulationLane lane;
};

struct HostSessionState
{
    std::string                             appId;
    std::string                             boardId        = "daisy_patch";
    std::string                             selectedNodeId = "node0";
    std::string                             entryNodeId    = "node0";
    std::string                             outputNodeId   = "node0";
    std::unordered_map<std::string, float> controlValues;
    std::unordered_map<std::string, float> parameterValues;
    std::unordered_map<std::string, float> cvValues;
    std::unordered_map<std::string, bool>  gateValues;
    MidiLearnMap                           midiLearn;
    std::uint32_t                          randomSeed = 0;
    std::vector<HostSessionNodeState>      nodes;
    std::vector<HostSessionRoute>          routes;
    std::vector<HostSessionModulationLaneState> modulationLanes;

    std::string Serialize() const;
    static HostSessionState Deserialize(const std::string& text);
};
} // namespace daisyhost
