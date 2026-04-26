#pragma once

#include <array>
#include <cstddef>
#include <string>

#include "daisyhost/HostedAppCore.h"

namespace daisyhost
{
inline constexpr std::size_t kHostModulationLaneCount = 4;
inline constexpr std::size_t kHostModulationSourceCount = 4;

enum class HostModulationSource
{
    kNone = 0,
    kCv1,
    kCv2,
    kCv3,
    kCv4,
    kLfo1,
    kLfo2,
    kLfo3,
    kLfo4,
};

struct HostModulationLane
{
    bool                 enabled = false;
    HostModulationSource source  = HostModulationSource::kNone;
    float                cvTargetMinimum = 0.0f;
    float                cvTargetMaximum = 1.0f;
    float                bipolarDepth    = 0.0f;
};

struct HostModulationSourceValues
{
    std::array<float, kHostModulationSourceCount> cvNormalized{};
    std::array<float, kHostModulationSourceCount> lfoBipolar{};
};

struct HostModulationLaneResult
{
    bool                 active = false;
    HostModulationSource source = HostModulationSource::kNone;
    float                sourceValue = 0.0f;
    float                nativeContribution = 0.0f;
};

struct HostModulationEvaluation
{
    float baseNativeValue      = 0.0f;
    float resultNativeValue    = 0.0f;
    float resultNormalizedValue = 0.0f;
    bool  clamped              = false;
    std::array<HostModulationLaneResult, kHostModulationLaneCount> lanes{};
};

bool IsHostModulationTargetEligible(const ParameterDescriptor& parameter);
const char* HostModulationSourceToString(HostModulationSource source);
HostModulationSource HostModulationSourceFromString(const std::string& text);
float ParameterNormalizedToNative(const ParameterDescriptor& parameter,
                                  float normalizedValue);
float ParameterNativeToNormalized(const ParameterDescriptor& parameter,
                                  float nativeValue);
HostModulationEvaluation EvaluateHostModulation(
    const ParameterDescriptor& parameter,
    const std::array<HostModulationLane, kHostModulationLaneCount>& lanes,
    const HostModulationSourceValues& sourceValues);
} // namespace daisyhost
