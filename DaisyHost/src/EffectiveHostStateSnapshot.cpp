#include "daisyhost/EffectiveHostStateSnapshot.h"

#include <algorithm>

#include "daisyhost/TestInputSignal.h"

namespace daisyhost
{
namespace
{
const ParameterDescriptor* FindParameter(const std::vector<ParameterDescriptor>& parameters,
                                         const std::string&                    parameterId)
{
    const auto it = std::find_if(parameters.begin(),
                                 parameters.end(),
                                 [&parameterId](const ParameterDescriptor& parameter) {
                                     return parameter.id == parameterId;
                                 });
    return it != parameters.end() ? &(*it) : nullptr;
}
} // namespace

EffectiveHostStateSnapshot BuildEffectiveHostStateSnapshot(
    const std::string&                       boardId,
    const std::string&                       selectedNodeId,
    std::size_t                              nodeCount,
    const std::string&                       entryNodeId,
    const std::string&                       outputNodeId,
    const std::string&                       appId,
    const std::string&                       appDisplayName,
    const std::vector<EffectiveHostNodeSummary>& nodeSummaries,
    const std::vector<EffectiveHostRouteSnapshot>& routes,
    const HostedAppPatchBindings&            patchBindings,
    const std::vector<ParameterDescriptor>&  parameters,
    const HostAutomationSlotBindings&        automationSlots,
    const std::array<HostCvInputState, 4>&   cvInputs,
    const std::array<HostGateInputState, 2>& gateInputs,
    const HostAudioInputState&               audioInput,
    const std::vector<MetaControllerDescriptor>& metaControllers)
{
    EffectiveHostStateSnapshot snapshot;
    snapshot.boardId        = boardId;
    snapshot.selectedNodeId = selectedNodeId;
    snapshot.nodeCount      = nodeCount;
    snapshot.entryNodeId    = entryNodeId;
    snapshot.outputNodeId   = outputNodeId;
    snapshot.appId          = appId;
    snapshot.appDisplayName = appDisplayName;
    snapshot.nodeSummaries  = nodeSummaries;
    snapshot.routes         = routes;
    snapshot.parameters     = parameters;
    snapshot.metaControllers.reserve(metaControllers.size());
    for(const auto& controller : metaControllers)
    {
        snapshot.metaControllers.push_back({controller.id,
                                            controller.label,
                                            controller.normalizedValue,
                                            controller.defaultNormalizedValue,
                                            controller.stateful});
    }

    for(std::size_t slotIndex = 0; slotIndex < automationSlots.size(); ++slotIndex)
    {
        const auto& binding = automationSlots[slotIndex];
        auto&       slot    = snapshot.automationSlots[slotIndex];
        slot.slotId         = binding.slotId;
        slot.slotName       = binding.slotName;
        slot.available      = binding.available;
        slot.parameterId    = binding.parameterId;
        slot.parameterLabel = binding.parameterLabel;
        slot.unitLabel      = binding.unitLabel;

        if(!binding.available || binding.parameterId.empty())
        {
            continue;
        }

        if(const auto* parameter = FindParameter(parameters, binding.parameterId))
        {
            slot.normalizedValue          = parameter->normalizedValue;
            slot.effectiveNormalizedValue = parameter->effectiveNormalizedValue;
            if(slot.parameterLabel.empty())
            {
                slot.parameterLabel = parameter->label;
            }
            if(slot.unitLabel.empty())
            {
                slot.unitLabel = parameter->unitLabel;
            }
        }
        else
        {
            slot.available = false;
        }
    }

    for(std::size_t index = 0; index < snapshot.cvInputs.size(); ++index)
    {
        snapshot.cvInputs[index].portId          = patchBindings.cvInputPortIds[index];
        snapshot.cvInputs[index].normalizedValue = cvInputs[index].normalizedValue;
        snapshot.cvInputs[index].volts           = cvInputs[index].volts;
        snapshot.cvInputs[index].sourceMode      = cvInputs[index].sourceMode;
        snapshot.cvInputs[index].waveform        = cvInputs[index].waveform;
        snapshot.cvInputs[index].frequencyHz     = cvInputs[index].frequencyHz;
        snapshot.cvInputs[index].amplitudeVolts  = cvInputs[index].amplitudeVolts;
        snapshot.cvInputs[index].biasVolts       = cvInputs[index].biasVolts;
        snapshot.cvInputs[index].manualVolts     = cvInputs[index].manualVolts;
    }

    for(std::size_t index = 0; index < snapshot.gateInputs.size(); ++index)
    {
        snapshot.gateInputs[index].portId = patchBindings.gateInputPortIds[index];
        snapshot.gateInputs[index].value  = gateInputs[index].value;
    }

    snapshot.audioInput.mode        = audioInput.mode;
    snapshot.audioInput.modeName    = GetTestInputSignalModeName(audioInput.mode);
    snapshot.audioInput.level       = audioInput.level;
    snapshot.audioInput.frequencyHz = audioInput.frequencyHz;

    return snapshot;
}
} // namespace daisyhost
