#pragma once

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

LiveRackTopologyConfig BuildLiveRackTopologyConfig(LiveRackTopologyPreset preset);

bool ValidateLiveRackTopologyConfig(const LiveRackTopologyConfig& config,
                                    std::string*                  errorMessage = nullptr);

bool TryInferLiveRackTopologyPreset(const LiveRackTopologyConfig& config,
                                    LiveRackTopologyPreset*       preset,
                                    std::string*                  errorMessage = nullptr);
} // namespace daisyhost
