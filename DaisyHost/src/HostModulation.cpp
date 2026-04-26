#include "daisyhost/HostModulation.h"

#include <algorithm>
#include <cmath>

namespace daisyhost
{
namespace
{
float Clamp01(float value)
{
    return std::clamp(value, 0.0f, 1.0f);
}

bool IsCvSource(HostModulationSource source)
{
    return source >= HostModulationSource::kCv1
           && source <= HostModulationSource::kCv4;
}

bool IsLfoSource(HostModulationSource source)
{
    return source >= HostModulationSource::kLfo1
           && source <= HostModulationSource::kLfo4;
}

std::size_t SourceIndex(HostModulationSource source)
{
    if(IsCvSource(source))
    {
        return static_cast<std::size_t>(source)
               - static_cast<std::size_t>(HostModulationSource::kCv1);
    }
    if(IsLfoSource(source))
    {
        return static_cast<std::size_t>(source)
               - static_cast<std::size_t>(HostModulationSource::kLfo1);
    }
    return 0;
}
} // namespace

bool IsHostModulationTargetEligible(const ParameterDescriptor& parameter)
{
    return parameter.stepCount <= 0
           && parameter.nativeMaximum > parameter.nativeMinimum
           && !parameter.id.empty();
}

const char* HostModulationSourceToString(HostModulationSource source)
{
    switch(source)
    {
        case HostModulationSource::kNone: return "none";
        case HostModulationSource::kCv1: return "cv1";
        case HostModulationSource::kCv2: return "cv2";
        case HostModulationSource::kCv3: return "cv3";
        case HostModulationSource::kCv4: return "cv4";
        case HostModulationSource::kLfo1: return "lfo1";
        case HostModulationSource::kLfo2: return "lfo2";
        case HostModulationSource::kLfo3: return "lfo3";
        case HostModulationSource::kLfo4: return "lfo4";
    }
    return "none";
}

HostModulationSource HostModulationSourceFromString(const std::string& text)
{
    if(text == "cv1") { return HostModulationSource::kCv1; }
    if(text == "cv2") { return HostModulationSource::kCv2; }
    if(text == "cv3") { return HostModulationSource::kCv3; }
    if(text == "cv4") { return HostModulationSource::kCv4; }
    if(text == "lfo1") { return HostModulationSource::kLfo1; }
    if(text == "lfo2") { return HostModulationSource::kLfo2; }
    if(text == "lfo3") { return HostModulationSource::kLfo3; }
    if(text == "lfo4") { return HostModulationSource::kLfo4; }
    return HostModulationSource::kNone;
}

float ParameterNormalizedToNative(const ParameterDescriptor& parameter,
                                  float normalizedValue)
{
    const float range = parameter.nativeMaximum - parameter.nativeMinimum;
    if(range <= 0.0f)
    {
        return parameter.nativeMinimum;
    }
    return parameter.nativeMinimum + Clamp01(normalizedValue) * range;
}

float ParameterNativeToNormalized(const ParameterDescriptor& parameter,
                                  float nativeValue)
{
    const float range = parameter.nativeMaximum - parameter.nativeMinimum;
    if(range <= 0.0f)
    {
        return 0.0f;
    }
    return Clamp01((nativeValue - parameter.nativeMinimum) / range);
}

HostModulationEvaluation EvaluateHostModulation(
    const ParameterDescriptor& parameter,
    const std::array<HostModulationLane, kHostModulationLaneCount>& lanes,
    const HostModulationSourceValues& sourceValues)
{
    HostModulationEvaluation result;
    result.baseNativeValue
        = ParameterNormalizedToNative(parameter, parameter.normalizedValue);

    float summed = result.baseNativeValue;
    for(std::size_t index = 0; index < lanes.size(); ++index)
    {
        const auto& lane = lanes[index];
        auto&       laneResult = result.lanes[index];
        laneResult.source = lane.source;
        if(!lane.enabled || lane.source == HostModulationSource::kNone)
        {
            continue;
        }

        laneResult.active = true;
        const auto sourceIndex = SourceIndex(lane.source);
        if(IsCvSource(lane.source))
        {
            const float source = Clamp01(sourceValues.cvNormalized[sourceIndex]);
            laneResult.sourceValue = source;
            laneResult.nativeContribution
                = lane.cvTargetMinimum
                  + source * (lane.cvTargetMaximum - lane.cvTargetMinimum);
        }
        else if(IsLfoSource(lane.source))
        {
            const float source
                = std::clamp(sourceValues.lfoBipolar[sourceIndex], -1.0f, 1.0f);
            laneResult.sourceValue = source;
            laneResult.nativeContribution = source * lane.bipolarDepth;
        }

        summed += laneResult.nativeContribution;
    }

    result.resultNativeValue = std::clamp(
        summed, parameter.nativeMinimum, parameter.nativeMaximum);
    result.resultNormalizedValue
        = ParameterNativeToNormalized(parameter, result.resultNativeValue);
    result.clamped = std::abs(result.resultNativeValue - summed) > 0.0001f;
    return result;
}
} // namespace daisyhost
