#include "daisyhost/LiveRackTopology.h"

#include <algorithm>
#include <set>
#include <utility>

namespace daisyhost
{
namespace
{
constexpr const char* kNode0Id = "node0";
constexpr const char* kNode1Id = "node1";
constexpr const char* kUnsupportedCurrentContractError
    = "unsupported live rack topology shape for the current two-node audio contract";

using CanonicalRoute = std::pair<std::string, std::string>;

bool IsSupportedNodeId(const std::string& nodeId)
{
    return nodeId == kNode0Id || nodeId == kNode1Id;
}

bool TryParseAudioSourcePort(const std::string& portId,
                             std::string*       nodeId,
                             int*               channel)
{
    if(portId == "node0/port/audio_out_1")
    {
        if(nodeId != nullptr)
        {
            *nodeId = kNode0Id;
        }
        if(channel != nullptr)
        {
            *channel = 1;
        }
        return true;
    }

    if(portId == "node0/port/audio_out_2")
    {
        if(nodeId != nullptr)
        {
            *nodeId = kNode0Id;
        }
        if(channel != nullptr)
        {
            *channel = 2;
        }
        return true;
    }

    if(portId == "node1/port/audio_out_1")
    {
        if(nodeId != nullptr)
        {
            *nodeId = kNode1Id;
        }
        if(channel != nullptr)
        {
            *channel = 1;
        }
        return true;
    }

    if(portId == "node1/port/audio_out_2")
    {
        if(nodeId != nullptr)
        {
            *nodeId = kNode1Id;
        }
        if(channel != nullptr)
        {
            *channel = 2;
        }
        return true;
    }

    return false;
}

bool TryParseAudioDestPort(const std::string& portId,
                           std::string*       nodeId,
                           int*               channel)
{
    if(portId == "node0/port/audio_in_1")
    {
        if(nodeId != nullptr)
        {
            *nodeId = kNode0Id;
        }
        if(channel != nullptr)
        {
            *channel = 1;
        }
        return true;
    }

    if(portId == "node0/port/audio_in_2")
    {
        if(nodeId != nullptr)
        {
            *nodeId = kNode0Id;
        }
        if(channel != nullptr)
        {
            *channel = 2;
        }
        return true;
    }

    if(portId == "node1/port/audio_in_1")
    {
        if(nodeId != nullptr)
        {
            *nodeId = kNode1Id;
        }
        if(channel != nullptr)
        {
            *channel = 1;
        }
        return true;
    }

    if(portId == "node1/port/audio_in_2")
    {
        if(nodeId != nullptr)
        {
            *nodeId = kNode1Id;
        }
        if(channel != nullptr)
        {
            *channel = 2;
        }
        return true;
    }

    return false;
}

std::set<CanonicalRoute> MakeCanonicalRoutes(
    const std::vector<LiveRackTopologyRoute>& routes)
{
    std::set<CanonicalRoute> canonicalRoutes;
    for(const auto& route : routes)
    {
        canonicalRoutes.insert({route.sourcePortId, route.destPortId});
    }
    return canonicalRoutes;
}

bool TryMatchSerialPreset(const LiveRackTopologyConfig& config,
                          LiveRackTopologyPreset        preset)
{
    const auto expected = BuildLiveRackTopologyConfig(preset);
    return config.entryNodeId == expected.entryNodeId
           && config.outputNodeId == expected.outputNodeId
           && MakeCanonicalRoutes(config.routes) == MakeCanonicalRoutes(expected.routes);
}

LiveRackAudioRouteEndpoint MakeEndpoint(const std::string& nodeId,
                                        const std::string& portId,
                                        int                channel)
{
    LiveRackAudioRouteEndpoint endpoint;
    endpoint.nodeId       = nodeId;
    endpoint.portId       = portId;
    endpoint.channelIndex = static_cast<std::size_t>(channel - 1);
    return endpoint;
}
} // namespace

LiveRackTopologyConfig BuildLiveRackTopologyConfig(LiveRackTopologyPreset preset)
{
    LiveRackTopologyConfig config;
    switch(preset)
    {
        case LiveRackTopologyPreset::kNode0Only:
            config.entryNodeId  = kNode0Id;
            config.outputNodeId = kNode0Id;
            break;

        case LiveRackTopologyPreset::kNode1Only:
            config.entryNodeId  = kNode1Id;
            config.outputNodeId = kNode1Id;
            break;

        case LiveRackTopologyPreset::kNode0ToNode1:
            config.entryNodeId  = kNode0Id;
            config.outputNodeId = kNode1Id;
            config.routes.push_back({"node0/port/audio_out_1", "node1/port/audio_in_1"});
            config.routes.push_back({"node0/port/audio_out_2", "node1/port/audio_in_2"});
            break;

        case LiveRackTopologyPreset::kNode1ToNode0:
            config.entryNodeId  = kNode1Id;
            config.outputNodeId = kNode0Id;
            config.routes.push_back({"node1/port/audio_out_1", "node0/port/audio_in_1"});
            config.routes.push_back({"node1/port/audio_out_2", "node0/port/audio_in_2"});
            break;
    }

    return config;
}

std::string GetLiveRackTopologyDisplayLabel(LiveRackTopologyPreset preset)
{
    switch(preset)
    {
        case LiveRackTopologyPreset::kNode0Only: return "Node 0 only";
        case LiveRackTopologyPreset::kNode1Only: return "Node 1 only";
        case LiveRackTopologyPreset::kNode0ToNode1: return "Node 0 feeds Node 1";
        case LiveRackTopologyPreset::kNode1ToNode0: return "Node 1 feeds Node 0";
    }

    return "Node 0 only";
}

std::string GetLiveRackNodeRoleDisplayLabel(LiveRackTopologyPreset preset,
                                            const std::string&      nodeId,
                                            bool                   selected)
{
    const auto config = BuildLiveRackTopologyConfig(preset);
    std::string role;
    if(nodeId == config.entryNodeId && nodeId == config.outputNodeId)
    {
        role = "Entry + output";
    }
    else if(nodeId == config.entryNodeId)
    {
        role = "Audio entry";
    }
    else if(nodeId == config.outputNodeId)
    {
        role = "Audio output";
    }
    else
    {
        role = "Not in route";
    }

    return selected ? "Selected - " + role : role;
}

bool ValidateLiveRackTopologyConfig(const LiveRackTopologyConfig& config,
                                    std::string*                  errorMessage)
{
    return TryBuildLiveRackRoutePlan(config, nullptr, errorMessage);
}

bool TryBuildLiveRackRoutePlan(const LiveRackTopologyConfig& config,
                               LiveRackRoutePlan*           plan,
                               std::string*                 errorMessage)
{
    if(!IsSupportedNodeId(config.entryNodeId) || !IsSupportedNodeId(config.outputNodeId))
    {
        if(errorMessage != nullptr)
        {
            *errorMessage
                = "Live rack currently supports exactly two nodes: node0 and node1";
        }
        return false;
    }

    LiveRackRoutePlan candidate;
    candidate.config = config;

    if(config.routes.empty())
    {
        if(config.entryNodeId == config.outputNodeId)
        {
            candidate.processingOrder.push_back(config.entryNodeId);
            if(plan != nullptr)
            {
                *plan = std::move(candidate);
            }
            return true;
        }

        if(errorMessage != nullptr)
        {
            *errorMessage = kUnsupportedCurrentContractError;
        }
        return false;
    }

    if(config.routes.size() != 2u || MakeCanonicalRoutes(config.routes).size() != 2u)
    {
        if(errorMessage != nullptr)
        {
            *errorMessage
                = "Live rack serial topologies require exactly two distinct stereo audio routes";
        }
        return false;
    }

    for(const auto& route : config.routes)
    {
        std::string sourceNodeId;
        std::string destNodeId;
        int         sourceChannel = 0;
        int         destChannel   = 0;
        if(!TryParseAudioSourcePort(route.sourcePortId, &sourceNodeId, &sourceChannel)
           || !TryParseAudioDestPort(route.destPortId, &destNodeId, &destChannel))
        {
            if(errorMessage != nullptr)
            {
                *errorMessage = "Live rack currently supports audio routes only";
            }
            return false;
        }

        if(sourceNodeId == destNodeId || sourceChannel != destChannel)
        {
            if(errorMessage != nullptr)
            {
                *errorMessage = kUnsupportedCurrentContractError;
            }
            return false;
        }

        LiveRackResolvedAudioRoute resolvedRoute;
        resolvedRoute.source
            = MakeEndpoint(sourceNodeId, route.sourcePortId, sourceChannel);
        resolvedRoute.destination
            = MakeEndpoint(destNodeId, route.destPortId, destChannel);
        candidate.audioRoutes.push_back(std::move(resolvedRoute));
    }

    if(TryMatchSerialPreset(config, LiveRackTopologyPreset::kNode0ToNode1)
       || TryMatchSerialPreset(config, LiveRackTopologyPreset::kNode1ToNode0))
    {
        candidate.processingOrder.push_back(config.entryNodeId);
        candidate.processingOrder.push_back(config.outputNodeId);
        std::sort(candidate.audioRoutes.begin(),
                  candidate.audioRoutes.end(),
                  [](const LiveRackResolvedAudioRoute& left,
                     const LiveRackResolvedAudioRoute& right) {
                      if(left.destination.channelIndex
                         != right.destination.channelIndex)
                      {
                          return left.destination.channelIndex
                                 < right.destination.channelIndex;
                      }
                      return left.destination.portId < right.destination.portId;
                  });
        if(plan != nullptr)
        {
            *plan = std::move(candidate);
        }
        return true;
    }

    if(errorMessage != nullptr)
    {
        *errorMessage = kUnsupportedCurrentContractError;
    }
    return false;
}

bool TryInferLiveRackTopologyPreset(const LiveRackTopologyConfig& config,
                                    LiveRackTopologyPreset*       preset,
                                    std::string*                  errorMessage)
{
    if(!ValidateLiveRackTopologyConfig(config, errorMessage))
    {
        return false;
    }

    if(config.routes.empty())
    {
        if(config.entryNodeId == kNode0Id)
        {
            if(preset != nullptr)
            {
                *preset = LiveRackTopologyPreset::kNode0Only;
            }
            return true;
        }

        if(preset != nullptr)
        {
            *preset = LiveRackTopologyPreset::kNode1Only;
        }
        return true;
    }

    if(TryMatchSerialPreset(config, LiveRackTopologyPreset::kNode0ToNode1))
    {
        if(preset != nullptr)
        {
            *preset = LiveRackTopologyPreset::kNode0ToNode1;
        }
        return true;
    }

    if(preset != nullptr)
    {
        *preset = LiveRackTopologyPreset::kNode1ToNode0;
    }
    return true;
}
} // namespace daisyhost
