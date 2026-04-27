#pragma once

#include <cstddef>
#include <string>

#include "daisyhost/EffectiveHostStateSnapshot.h"
#include "daisyhost/HostModulation.h"

namespace daisyhost
{
struct HostModulationLaneDisplayText
{
    std::string title;
    std::string detail;
};

HostModulationLaneDisplayText BuildHostModulationLaneDisplayText(
    std::size_t                                    slotIndex,
    const std::string&                            destinationLabel,
    const HostModulationLane&                     lane,
    const EffectiveHostModulationDestinationSnapshot* destinationSnapshot);
} // namespace daisyhost
