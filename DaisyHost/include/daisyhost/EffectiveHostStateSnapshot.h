#pragma once

#include <array>
#include <string>
#include <vector>

#include "daisyhost/HostAutomationBridge.h"

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

struct EffectiveHostStateSnapshot
{
    std::string appId;
    std::string appDisplayName;
    std::vector<ParameterDescriptor> parameters;
    std::array<EffectiveHostAutomationSlotSnapshot, kHostAutomationSlotCount>
        automationSlots{};
    std::array<EffectiveHostCvInputSnapshot, 4> cvInputs{};
    std::array<EffectiveHostGateInputSnapshot, 2> gateInputs{};
    EffectiveHostAudioInputSnapshot audioInput;
};

EffectiveHostStateSnapshot BuildEffectiveHostStateSnapshot(
    const std::string&                       appId,
    const std::string&                       appDisplayName,
    const HostedAppPatchBindings&            patchBindings,
    const std::vector<ParameterDescriptor>&  parameters,
    const HostAutomationSlotBindings&        automationSlots,
    const std::array<HostCvInputState, 4>&   cvInputs,
    const std::array<HostGateInputState, 2>& gateInputs,
    const HostAudioInputState&               audioInput);
} // namespace daisyhost
