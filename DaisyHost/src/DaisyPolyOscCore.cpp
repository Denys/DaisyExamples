#include "daisyhost/DaisyPolyOscCore.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace daisyhost
{
namespace
{
constexpr float kTwoPi = 6.28318530717958647692f;

float Clamp01(float value)
{
    return std::clamp(value, 0.0f, 1.0f);
}

int WaveformIndexFromNormalized(float normalizedValue)
{
    return std::clamp(static_cast<int>(std::round(Clamp01(normalizedValue) * 4.0f)),
                      0,
                      4);
}

float NormalizedFromWaveformIndex(int waveformIndex)
{
    return static_cast<float>(std::clamp(waveformIndex, 0, 4)) / 4.0f;
}

const char* WaveformLabel(int waveformIndex)
{
    switch(waveformIndex)
    {
        case 0:
            return "Sine";
        case 1:
            return "Triangle";
        case 2:
            return "Saw";
        case 3:
            return "Ramp";
        case 4:
            return "Square";
        default:
            return "Sine";
    }
}

float RenderWaveform(int waveformIndex, float phase)
{
    switch(waveformIndex)
    {
        case 0:
            return std::sin(kTwoPi * phase);
        case 1:
        {
            const float t = -1.0f + (2.0f * phase);
            return 2.0f * (std::abs(t) - 0.5f);
        }
        case 2:
            return -1.0f * ((phase * 2.0f) - 1.0f);
        case 3:
            return (phase * 2.0f) - 1.0f;
        case 4:
            return phase < 0.5f ? 1.0f : -1.0f;
        default:
            return 0.0f;
    }
}

std::vector<DaisyPolyOscParameter> MakeDefaultParameters()
{
    return {
        {"osc1_freq", "Osc 1", "Frequency", 0.0f, 0.0f, 0.0f, 0, 0, true, true},
        {"osc2_freq", "Osc 2", "Frequency", 0.0f, 0.0f, 0.0f, 0, 1, true, true},
        {"osc3_freq", "Osc 3", "Frequency", 0.0f, 0.0f, 0.0f, 0, 2, true, true},
        {"global_freq", "Global", "Frequency", 0.5f, 0.5f, 0.5f, 0, 3, true, true},
        {"waveform", "Waveform", "Waveform", 0.0f, 0.0f, 0.0f, 4, 4, true, true},
    };
}
} // namespace

struct DaisyPolyOscCore::Impl
{
    double                                sampleRate   = 48000.0;
    std::size_t                           maxBlockSize = kPreferredBlockSize;
    std::vector<DaisyPolyOscParameter>    parameters   = MakeDefaultParameters();
    std::array<float, kOscillatorCount>   phases{};

    DaisyPolyOscParameter* FindMutable(const std::string& parameterId)
    {
        for(auto& parameter : parameters)
        {
            if(parameter.id == parameterId)
            {
                return &parameter;
            }
        }
        return nullptr;
    }

    const DaisyPolyOscParameter* Find(const std::string& parameterId) const
    {
        for(const auto& parameter : parameters)
        {
            if(parameter.id == parameterId)
            {
                return &parameter;
            }
        }
        return nullptr;
    }

    float Value(const std::string& parameterId) const
    {
        const auto* parameter = Find(parameterId);
        return parameter != nullptr ? parameter->normalizedValue : 0.0f;
    }

    void SetWaveformIndex(int waveformIndex)
    {
        auto* waveform = FindMutable("waveform");
        if(waveform == nullptr)
        {
            return;
        }
        waveform->normalizedValue
            = NormalizedFromWaveformIndex(std::clamp(waveformIndex, 0, 4));
        waveform->effectiveNormalizedValue = waveform->normalizedValue;
    }

    void ResetParameterDefaults()
    {
        parameters = MakeDefaultParameters();
    }
};

DaisyPolyOscCore::DaisyPolyOscCore() : impl_(std::make_unique<Impl>()) {}

DaisyPolyOscCore::~DaisyPolyOscCore() = default;

void DaisyPolyOscCore::Prepare(double sampleRate, std::size_t maxBlockSize)
{
    impl_->sampleRate   = sampleRate > 1.0 ? sampleRate : 48000.0;
    impl_->maxBlockSize = maxBlockSize > 0 ? maxBlockSize : kPreferredBlockSize;
}

void DaisyPolyOscCore::Process(float*      output1,
                               float*      output2,
                               float*      output3,
                               float*      mixOutput,
                               std::size_t frameCount)
{
    if(output1 != nullptr)
    {
        std::fill(output1, output1 + frameCount, 0.0f);
    }
    if(output2 != nullptr)
    {
        std::fill(output2, output2 + frameCount, 0.0f);
    }
    if(output3 != nullptr)
    {
        std::fill(output3, output3 + frameCount, 0.0f);
    }
    if(mixOutput != nullptr)
    {
        std::fill(mixOutput, mixOutput + frameCount, 0.0f);
    }

    const int   waveformIndex = GetWaveformIndex();
    const float sampleRate
        = static_cast<float>(impl_->sampleRate > 1.0 ? impl_->sampleRate : 48000.0);
    std::array<float, kOscillatorCount> frequencies{};
    for(std::size_t index = 0; index < frequencies.size(); ++index)
    {
        frequencies[index] = GetOscillatorFrequencyHz(index);
    }

    for(std::size_t frame = 0; frame < frameCount; ++frame)
    {
        std::array<float, kOscillatorCount> samples{};
        for(std::size_t index = 0; index < samples.size(); ++index)
        {
            samples[index] = RenderWaveform(waveformIndex, impl_->phases[index]) * 0.7f;
            impl_->phases[index] += frequencies[index] / sampleRate;
            impl_->phases[index] -= std::floor(impl_->phases[index]);
        }

        if(output1 != nullptr)
        {
            output1[frame] = samples[0];
        }
        if(output2 != nullptr)
        {
            output2[frame] = samples[1];
        }
        if(output3 != nullptr)
        {
            output3[frame] = samples[2];
        }
        if(mixOutput != nullptr)
        {
            mixOutput[frame] = (samples[0] + samples[1] + samples[2]) * 0.25f;
        }
    }
}

void DaisyPolyOscCore::ResetToDefaultState(std::uint32_t)
{
    impl_->ResetParameterDefaults();
    impl_->phases.fill(0.0f);
}

bool DaisyPolyOscCore::SetParameterValue(const std::string& parameterId,
                                         float              normalizedValue)
{
    auto* parameter = impl_->FindMutable(parameterId);
    if(parameter == nullptr)
    {
        return false;
    }

    parameter->normalizedValue = Clamp01(normalizedValue);
    if(parameter->id == "waveform")
    {
        parameter->normalizedValue
            = NormalizedFromWaveformIndex(WaveformIndexFromNormalized(normalizedValue));
    }
    parameter->effectiveNormalizedValue = parameter->normalizedValue;
    return true;
}

bool DaisyPolyOscCore::GetParameterValue(const std::string& parameterId,
                                         float*             normalizedValue) const
{
    const auto* parameter = impl_->Find(parameterId);
    if(parameter == nullptr || normalizedValue == nullptr)
    {
        return false;
    }
    *normalizedValue = parameter->normalizedValue;
    return true;
}

bool DaisyPolyOscCore::GetEffectiveParameterValue(
    const std::string& parameterId,
    float*             normalizedValue) const
{
    return GetParameterValue(parameterId, normalizedValue);
}

const DaisyPolyOscParameter* DaisyPolyOscCore::FindParameter(
    const std::string& parameterId) const
{
    return impl_->Find(parameterId);
}

const std::vector<DaisyPolyOscParameter>& DaisyPolyOscCore::GetParameters() const
{
    return impl_->parameters;
}

std::unordered_map<std::string, float> DaisyPolyOscCore::CaptureStatefulParameterValues()
    const
{
    std::unordered_map<std::string, float> values;
    for(const auto& parameter : impl_->parameters)
    {
        if(parameter.stateful)
        {
            values.emplace(parameter.id, parameter.normalizedValue);
        }
    }
    return values;
}

void DaisyPolyOscCore::RestoreStatefulParameterValues(
    const std::unordered_map<std::string, float>& values)
{
    for(const auto& [parameterId, value] : values)
    {
        SetParameterValue(parameterId, value);
    }
}

void DaisyPolyOscCore::IncrementWaveform(int delta)
{
    const int next = (GetWaveformIndex() + delta) % 5;
    impl_->SetWaveformIndex(next < 0 ? next + 5 : next);
}

int DaisyPolyOscCore::GetWaveformIndex() const
{
    return WaveformIndexFromNormalized(impl_->Value("waveform"));
}

std::string DaisyPolyOscCore::GetWaveformLabel() const
{
    return WaveformLabel(GetWaveformIndex());
}

float DaisyPolyOscCore::GetOscillatorFrequencyHz(std::size_t zeroBasedIndex) const
{
    if(zeroBasedIndex >= kOscillatorCount)
    {
        return 0.0f;
    }

    const float oscillatorValue
        = impl_->Value("osc" + std::to_string(zeroBasedIndex + 1) + "_freq");
    const float globalValue = impl_->Value("global_freq");
    return std::pow(2.0f, (Clamp01(oscillatorValue) + Clamp01(globalValue)) * 5.0f)
           * 55.0f;
}
} // namespace daisyhost
