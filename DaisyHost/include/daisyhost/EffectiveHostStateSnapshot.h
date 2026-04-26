#pragma once

#include <array>
#include <string>
#include <vector>

#include "daisyhost/HostAutomationBridge.h"
#include "daisyhost/BoardControlMapping.h"
#include "daisyhost/HostModulation.h"

namespace daisyhost
{
struct HostCvInputState
{
    float normalizedValue = 0.0f;
    float volts           = 0.0f;
    int   sourceMode      = 0;
    int   waveform        = 0;
    float frequencyHz     = 0.0f;
    float amplitudeVolts  = 0.0f;
    float biasVolts       = 0.0f;
    float manualVolts     = 0.0f;
};

struct HostGateInputState
{
    bool value = false;
};

struct HostAudioInputState
{
    int   mode        = 0;
    float level       = 0.0f;
    float frequencyHz = 0.0f;
};

struct EffectiveHostAutomationSlotSnapshot
{
    std::string slotId;
    std::string slotName;
    bool        available = false;
    std::string parameterId;
    std::string parameterLabel;
    std::string unitLabel;
    float       normalizedValue          = 0.0f;
    float       effectiveNormalizedValue = 0.0f;
};

struct EffectiveHostCvInputSnapshot
{
    std::string portId;
    float       normalizedValue = 0.0f;
    float       volts           = 0.0f;
    int         sourceMode      = 0;
    int         waveform        = 0;
    float       frequencyHz     = 0.0f;
    float       amplitudeVolts  = 0.0f;
    float       biasVolts       = 0.0f;
    float       manualVolts     = 0.0f;
};

struct EffectiveHostGateInputSnapshot
{
    std::string portId;
    bool        value = false;
};

struct EffectiveHostAudioInputSnapshot
{
    int         mode        = 0;
    std::string modeName;
    float       level       = 0.0f;
    float       frequencyHz = 0.0f;
};

struct EffectiveHostMetaControllerSnapshot
{
    std::string id;
    std::string label;
    float       normalizedValue        = 0.0f;
    float       defaultNormalizedValue = 0.0f;
    bool        stateful               = false;
};

struct EffectiveHostRouteSnapshot
{
    std::string sourcePortId;
    std::string destPortId;
};

struct EffectiveHostModulationLaneSnapshot
{
    int                  slotIndex = 0;
    bool                 enabled = false;
    HostModulationSource source = HostModulationSource::kNone;
    float                cvTargetMinimum = 0.0f;
    float                cvTargetMaximum = 1.0f;
    float                bipolarDepth = 0.0f;
    float                liveSourceValue = 0.0f;
    float                nativeContribution = 0.0f;
};

struct EffectiveHostModulationDestinationSnapshot
{
    std::string nodeId;
    std::string parameterId;
    std::string parameterLabel;
    std::string unitLabel;
    float       baseNativeValue = 0.0f;
    float       resultNativeValue = 0.0f;
    float       resultNormalizedValue = 0.0f;
    bool        clamped = false;
    std::vector<EffectiveHostModulationLaneSnapshot> lanes;
};

struct EffectiveHostFieldCvOutputSnapshot
{
    std::string id;
    std::string label;
    bool        available       = false;
    float       normalizedValue = 0.0f;
    float       volts           = 0.0f;
};

struct EffectiveHostFieldSwitchSnapshot
{
    std::string id;
    std::string label;
    std::string detailLabel;
    bool        available = false;
    bool        pressed   = false;
};

struct EffectiveHostFieldLedSnapshot
{
    std::string id;
    std::string label;
    float       normalizedValue = 0.0f;
};

struct EffectiveHostFieldSurfaceSnapshot
{
    std::array<EffectiveHostFieldCvOutputSnapshot, kDaisyFieldCvOutputCount> cvOutputs{};
    std::array<EffectiveHostFieldSwitchSnapshot, kDaisyFieldSwitchCount> switches{};
    std::array<EffectiveHostFieldLedSnapshot, kDaisyFieldLedCount> leds{};
};

struct EffectiveHostNodeSummary
{
    std::string nodeId;
    std::string appId;
    std::string appDisplayName;
    bool        selected  = false;
    bool        entryNode = false;
    bool        outputNode = false;
};

struct EffectiveHostStateSnapshot
{
    std::string boardId;
    std::string selectedNodeId;
    std::size_t nodeCount = 0;
    std::string entryNodeId;
    std::string outputNodeId;
    std::string appId;
    std::string appDisplayName;
    std::vector<EffectiveHostNodeSummary> nodeSummaries;
    std::vector<EffectiveHostRouteSnapshot> routes;
    std::vector<ParameterDescriptor> parameters;
    std::vector<EffectiveHostMetaControllerSnapshot> metaControllers;
    std::array<EffectiveHostAutomationSlotSnapshot, kHostAutomationSlotCount>
        automationSlots{};
    std::array<EffectiveHostCvInputSnapshot, 4> cvInputs{};
    std::array<EffectiveHostGateInputSnapshot, 2> gateInputs{};
    EffectiveHostAudioInputSnapshot audioInput;
    EffectiveHostFieldSurfaceSnapshot fieldSurface;
    std::string selectedModulationDestinationId;
    std::vector<EffectiveHostModulationDestinationSnapshot> modulationDestinations;
};

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
    const std::vector<MetaControllerDescriptor>& metaControllers = {},
    const EffectiveHostFieldSurfaceSnapshot& fieldSurface = {},
    const std::string& selectedModulationDestinationId = {},
    const std::vector<EffectiveHostModulationDestinationSnapshot>&
        modulationDestinations = {});
} // namespace daisyhost
