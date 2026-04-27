#include "daisyhost/HostModulationUiText.h"

#include <iomanip>
#include <sstream>

namespace daisyhost
{
namespace
{
std::string SourceDisplayName(HostModulationSource source)
{
    switch(source)
    {
        case HostModulationSource::kCv1: return "CV 1";
        case HostModulationSource::kCv2: return "CV 2";
        case HostModulationSource::kCv3: return "CV 3";
        case HostModulationSource::kCv4: return "CV 4";
        case HostModulationSource::kLfo1: return "LFO 1";
        case HostModulationSource::kLfo2: return "LFO 2";
        case HostModulationSource::kLfo3: return "LFO 3";
        case HostModulationSource::kLfo4: return "LFO 4";
        case HostModulationSource::kNone: break;
    }
    return "Empty";
}

std::string FormatFloat(float value)
{
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(2) << value;
    return stream.str();
}

const EffectiveHostModulationLaneSnapshot* FindLaneSnapshot(
    const EffectiveHostModulationDestinationSnapshot* destinationSnapshot,
    std::size_t                                      slotIndex)
{
    if(destinationSnapshot == nullptr)
    {
        return nullptr;
    }
    for(const auto& lane : destinationSnapshot->lanes)
    {
        if(lane.slotIndex == static_cast<int>(slotIndex))
        {
            return &lane;
        }
    }
    return nullptr;
}
} // namespace

HostModulationLaneDisplayText BuildHostModulationLaneDisplayText(
    std::size_t                                    slotIndex,
    const std::string&                            destinationLabel,
    const HostModulationLane&                     lane,
    const EffectiveHostModulationDestinationSnapshot* destinationSnapshot)
{
    HostModulationLaneDisplayText text;
    const auto laneNumber = std::to_string(slotIndex + 1);
    if(lane.source == HostModulationSource::kNone)
    {
        text.title = slotIndex == 0 ? "+ Add lane" : "Lane " + laneNumber;
        text.detail = destinationLabel.empty() ? "Select a destination"
                                               : "Destination " + destinationLabel;
        return text;
    }

    const auto sourceName = SourceDisplayName(lane.source);
    text.title = "Lane " + laneNumber + "  " + sourceName + " -> "
                 + (destinationLabel.empty() ? "Destination" : destinationLabel);

    std::ostringstream detail;
    if(lane.source >= HostModulationSource::kLfo1)
    {
        detail << "Amount " << FormatFloat(lane.bipolarDepth);
    }
    else
    {
        detail << "Range " << FormatFloat(lane.cvTargetMinimum) << ".."
               << FormatFloat(lane.cvTargetMaximum);
    }

    if(destinationSnapshot != nullptr)
    {
        detail << "  Base " << FormatFloat(destinationSnapshot->baseNativeValue)
               << "  Result "
               << FormatFloat(destinationSnapshot->resultNativeValue);
        if(!destinationSnapshot->unitLabel.empty())
        {
            detail << " " << destinationSnapshot->unitLabel;
        }
        if(destinationSnapshot->clamped)
        {
            detail << " clamped";
        }
    }

    if(const auto* laneSnapshot = FindLaneSnapshot(destinationSnapshot, slotIndex))
    {
        detail << "  Live " << FormatFloat(laneSnapshot->liveSourceValue)
               << "  Delta "
               << FormatFloat(laneSnapshot->nativeContribution);
    }

    text.detail = detail.str();
    return text;
}
} // namespace daisyhost
