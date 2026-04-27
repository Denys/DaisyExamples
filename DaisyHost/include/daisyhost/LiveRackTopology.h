#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace daisyhost
{
enum class LiveRackTopologyPreset
{
    kNode0Only,
    kNode1Only,
    kNode0ToNode1,
    kNode1ToNode0,
};

struct LiveRackTopologyRoute
{
    std::string sourcePortId;
    std::string destPortId;
};

struct LiveRackTopologyConfig
{
    std::string                      entryNodeId;
    std::string                      outputNodeId;
    std::vector<LiveRackTopologyRoute> routes;
};

struct LiveRackAudioRouteEndpoint
{
    std::string nodeId;
    std::string portId;
    std::size_t channelIndex = 0;
};

struct LiveRackResolvedAudioRoute
{
    LiveRackAudioRouteEndpoint source;
    LiveRackAudioRouteEndpoint destination;
};

// Shared plan for the current two-node, audio-only live rack contract. Richer
// routing semantics belong in WS9 so render and live paths do not diverge.
struct LiveRackRoutePlan
{
    LiveRackTopologyConfig                  config;
    std::vector<std::string>                processingOrder;
    std::vector<LiveRackResolvedAudioRoute> audioRoutes;
};

LiveRackTopologyConfig BuildLiveRackTopologyConfig(LiveRackTopologyPreset preset);

std::string GetLiveRackTopologyDisplayLabel(LiveRackTopologyPreset preset);

std::string GetLiveRackNodeRoleDisplayLabel(LiveRackTopologyPreset preset,
                                            const std::string&      nodeId,
                                            bool                   selected);

bool ValidateLiveRackTopologyConfig(const LiveRackTopologyConfig& config,
                                    std::string*                  errorMessage = nullptr);

bool TryBuildLiveRackRoutePlan(const LiveRackTopologyConfig& config,
                               LiveRackRoutePlan*           plan,
                               std::string*                 errorMessage = nullptr);

bool TryInferLiveRackTopologyPreset(const LiveRackTopologyConfig& config,
                                    LiveRackTopologyPreset*       preset,
                                    std::string*                  errorMessage = nullptr);
} // namespace daisyhost
