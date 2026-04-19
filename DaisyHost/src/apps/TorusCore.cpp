#include "daisyhost/apps/TorusCore.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <utility>

namespace daisyhost
{
namespace apps
{
namespace
{
constexpr float kTwoPi = 6.28318530717958647692f;

std::string MakeParameterId(const std::string& nodeId, const std::string& suffix)
{
    return nodeId + "/param/" + suffix;
}

std::string MakeMenuRootSectionId(const std::string& nodeId)
{
    return nodeId + "/menu/root";
}

std::string MakeMenuSectionId(const std::string& nodeId, const std::string& suffix)
{
    return nodeId + "/menu/" + suffix;
}

std::string MakeMenuItemId(const std::string& nodeId,
                           const std::string& section,
                           const std::string& item)
{
    return nodeId + "/menu/" + section + "/" + item;
}

float Clamp01(float value)
{
    return std::clamp(value, 0.0f, 1.0f);
}

float PeakForBuffer(const float* data, std::size_t frameCount)
{
    float peak = 0.0f;
    if(data == nullptr)
    {
        return peak;
    }

    for(std::size_t i = 0; i < frameCount; ++i)
    {
        peak = std::max(peak, std::abs(data[i]));
    }
    return peak;
}

float QuantizedNormalized(float normalizedValue, int choiceCount)
{
    if(choiceCount <= 1)
    {
        return 0.0f;
    }

    const int index = std::clamp(
        static_cast<int>(std::round(Clamp01(normalizedValue) * static_cast<float>(choiceCount - 1))),
        0,
        choiceCount - 1);
    return static_cast<float>(index) / static_cast<float>(choiceCount - 1);
}

int QuantizeChoice(float normalizedValue, int choiceCount)
{
    if(choiceCount <= 1)
    {
        return 0;
    }

    return std::clamp(
        static_cast<int>(std::round(Clamp01(normalizedValue) * static_cast<float>(choiceCount - 1))),
        0,
        choiceCount - 1);
}

float StepSizeForChoices(int choiceCount)
{
    return choiceCount <= 1 ? 1.0f : 1.0f / static_cast<float>(choiceCount - 1);
}
} // namespace

struct TorusCore::Impl
{
    enum class ParameterIndex : std::size_t
    {
        kFrequency = 0,
        kStructure,
        kBrightness,
        kDamping,
        kPosition,
        kPolyphony,
        kModel,
        kEasterFx,
        kEasterEgg,
        kNormalizeExciter,
        kNormalizeNote,
        kNormalizeStrum,
        kCtrl1Assignment,
        kCtrl2Assignment,
        kCtrl3Assignment,
        kCtrl4Assignment,
        kCount,
    };

    enum class ParameterSource
    {
        kNone,
        kControl,
        kCv,
        kMenu,
        kRestore,
    };

    explicit Impl(std::string nodeId)
    : nodeId(std::move(nodeId))
    {
        parameters = {
            MakeParameterDescriptor("frequency", "Frequency", 0.50f, 0, 1),
            MakeParameterDescriptor("structure", "Structure", 0.50f, 0, 2),
            MakeParameterDescriptor("brightness", "Brightness", 0.50f, 0, 3),
            MakeParameterDescriptor("damping", "Damping", 0.50f, 0, 4),
            MakeParameterDescriptor("position", "Position", 0.50f, 0, 5),
            MakeParameterDescriptor("polyphony", "Polyphony", 0.00f, 3, 6),
            MakeParameterDescriptor("model", "Model", 0.00f, 6, 7),
            MakeParameterDescriptor("easter_fx", "Easter FX", 0.00f, 6, 8),
            MakeParameterDescriptor("easter_egg", "Easter Egg", 0.00f, 2, 9),
            MakeParameterDescriptor("normalize_exciter", "Normalize Exciter", 1.00f, 2, 10),
            MakeParameterDescriptor("normalize_note", "Normalize Note", 1.00f, 2, 11),
            MakeParameterDescriptor("normalize_strum", "Normalize Strum", 1.00f, 2, 12),
            MakeParameterDescriptor("ctrl1_assignment", "CTRL 1 Assignment", 0.00f, 5, 13),
            MakeParameterDescriptor("ctrl2_assignment", "CTRL 2 Assignment", 0.25f, 5, 14),
            MakeParameterDescriptor("ctrl3_assignment", "CTRL 3 Assignment", 0.50f, 5, 15),
            MakeParameterDescriptor("ctrl4_assignment", "CTRL 4 Assignment", 0.75f, 5, 16),
        };

        for(std::size_t i = 0; i < 4; ++i)
        {
            portInputs[TorusCore::MakeCvInputPortId(nodeId, i + 1)] = {VirtualPortType::kCv};
            portInputs[TorusCore::MakeAudioInputPortId(nodeId, i + 1)] = {VirtualPortType::kAudio};
            portOutputs[TorusCore::MakeAudioOutputPortId(nodeId, i + 1)] = {VirtualPortType::kAudio};
        }
        for(std::size_t i = 0; i < 2; ++i)
        {
            portInputs[TorusCore::MakeGateInputPortId(nodeId, i + 1)] = {VirtualPortType::kGate};
        }
        portInputs[TorusCore::MakeMidiInputPortId(nodeId, 1)]   = {VirtualPortType::kMidi};
        portOutputs[TorusCore::MakeMidiOutputPortId(nodeId, 1)] = {VirtualPortType::kMidi};
        portOutputs[TorusCore::MakeGateOutputPortId(nodeId, 1)] = {VirtualPortType::kGate};
    }

    std::string nodeId;
    double sampleRate   = 48000.0;
    std::size_t maxBlockSize = TorusCore::kPreferredBlockSize;
    bool prepared = false;

    std::vector<ParameterDescriptor> parameters;
    std::array<ParameterSource, static_cast<std::size_t>(ParameterIndex::kCount)> parameterSources{};
    std::array<std::uint64_t, static_cast<std::size_t>(ParameterIndex::kCount)> parameterTouchSerials{};
    std::array<float, 4> controlInputs{};
    std::array<bool, 4> controlInputInitialized{};
    std::uint64_t nextTouchSerial = 0;
    std::uint32_t randomSeed = 0;

    std::unordered_map<std::string, PortValue> portInputs;
    std::unordered_map<std::string, PortValue> portOutputs;
    MenuModel menu;
    DisplayModel display;
    std::unordered_map<std::string, int> menuSelections;
    bool encoderPressed = false;

    std::array<float, 4> phase{};
    std::array<float, 4> excitation{};
    std::array<float, 4> resonator{};
    std::array<float, 4> shimmer{};
    std::uint32_t noiseState = 1u;
    bool previousGate1 = false;
    float previousLeft = 0.0f;
    float previousRight = 0.0f;
    float lfoPhase = 0.0f;
    float smoothedInputPeak = 0.0f;

    ParameterDescriptor MakeParameterDescriptor(const std::string& suffix,
                                                const std::string& label,
                                                float              defaultNormalizedValue,
                                                int                stepCount,
                                                int                importanceRank) const
    {
        ParameterDescriptor descriptor;
        descriptor.id                      = MakeParameterId(nodeId, suffix);
        descriptor.label                   = label;
        descriptor.normalizedValue         = defaultNormalizedValue;
        descriptor.defaultNormalizedValue  = defaultNormalizedValue;
        descriptor.effectiveNormalizedValue = defaultNormalizedValue;
        descriptor.unitLabel               = stepCount == 0 ? "%" : "";
        descriptor.stepCount               = stepCount;
        descriptor.role                    = ParameterRole::kGeneric;
        descriptor.importanceRank          = importanceRank;
        descriptor.automatable             = true;
        descriptor.stateful                = true;
        descriptor.menuEditable            = true;
        return descriptor;
    }
};

namespace
{
using ParameterIndex = TorusCore::Impl::ParameterIndex;
using ParameterSource = TorusCore::Impl::ParameterSource;

std::array<const char*, 5> kSemanticNames = {
    "Frequency", "Structure", "Brightness", "Damping", "Position"};

ParameterIndex AssignmentToParameterIndex(int assignment)
{
    switch(assignment)
    {
        case 0: return ParameterIndex::kFrequency;
        case 1: return ParameterIndex::kStructure;
        case 2: return ParameterIndex::kBrightness;
        case 3: return ParameterIndex::kDamping;
        case 4:
        default: return ParameterIndex::kPosition;
    }
}

std::string ControlNameFromAssignment(int assignment)
{
    const std::size_t index = static_cast<std::size_t>(std::clamp(assignment, 0, 4));
    return kSemanticNames[index];
}

int AssignmentForControl(const TorusCore::Impl& impl, std::size_t controlIndex)
{
    return QuantizeChoice(
        impl.parameters[static_cast<std::size_t>(ParameterIndex::kCtrl1Assignment) + controlIndex]
            .effectiveNormalizedValue,
        5);
}

float AssignedParameterNormalizedValue(const TorusCore::Impl& impl,
                                       std::size_t            controlIndex)
{
    return impl
        .parameters[static_cast<std::size_t>(
            AssignmentToParameterIndex(AssignmentForControl(impl, controlIndex)))]
        .normalizedValue;
}

void SyncControlInputsFromAssignments(TorusCore::Impl& impl)
{
    for(std::size_t i = 0; i < impl.controlInputs.size(); ++i)
    {
        impl.controlInputs[i] = AssignedParameterNormalizedValue(impl, i);
        impl.controlInputInitialized[i] = true;
    }
}

float BaseStepForMenuItem(const std::string& itemId)
{
    if(itemId.find("assignment") != std::string::npos)
    {
        return StepSizeForChoices(5);
    }
    if(itemId.find("polyphony") != std::string::npos)
    {
        return StepSizeForChoices(3);
    }
    if(itemId.find("model") != std::string::npos || itemId.find("easter_fx") != std::string::npos)
    {
        return StepSizeForChoices(6);
    }
    if(itemId.find("normalize") != std::string::npos || itemId.find("enabled") != std::string::npos)
    {
        return 1.0f;
    }
    return 0.02f;
}

} // namespace

TorusCore::TorusCore(const std::string& nodeId)
: nodeId_(nodeId),
  impl_(std::make_unique<Impl>(nodeId))
{
    ResetToDefaultState();
}

TorusCore::~TorusCore() = default;

std::string TorusCore::GetAppId() const
{
    return "torus";
}

std::string TorusCore::GetAppDisplayName() const
{
    return "Torus";
}

HostedAppCapabilities TorusCore::GetCapabilities() const
{
    HostedAppCapabilities capabilities;
    capabilities.acceptsAudioInput  = true;
    capabilities.acceptsMidiInput   = false;
    capabilities.producesMidiOutput = false;
    return capabilities;
}

HostedAppPatchBindings TorusCore::GetPatchBindings() const
{
    HostedAppPatchBindings bindings;
    for(std::size_t i = 0; i < 4; ++i)
    {
        bindings.knobControlIds[i]    = MakeKnobControlId(nodeId_, i + 1);
        bindings.knobDetailLabels[i]  = ControlNameFromAssignment(
            QuantizeChoice(
                impl_->parameters[static_cast<std::size_t>(ParameterIndex::kCtrl1Assignment) + i]
                    .effectiveNormalizedValue,
                5));
        bindings.cvInputPortIds[i]    = MakeCvInputPortId(nodeId_, i + 1);
        bindings.audioInputPortIds[i] = MakeAudioInputPortId(nodeId_, i + 1);
        bindings.audioOutputPortIds[i] = MakeAudioOutputPortId(nodeId_, i + 1);
    }
    bindings.encoderControlId        = MakeEncoderControlId(nodeId_);
    bindings.encoderButtonControlId  = MakeEncoderButtonControlId(nodeId_);
    bindings.gateInputPortIds[0]     = MakeGateInputPortId(nodeId_, 1);
    bindings.gateInputPortIds[1]     = MakeGateInputPortId(nodeId_, 2);
    bindings.gateOutputPortId        = MakeGateOutputPortId(nodeId_, 1);
    bindings.midiInputPortId         = MakeMidiInputPortId(nodeId_, 1);
    bindings.midiOutputPortId        = MakeMidiOutputPortId(nodeId_, 1);
    bindings.mainOutputChannels      = {0, 1};
    return bindings;
}

void TorusCore::Prepare(double sampleRate, std::size_t maxBlockSize)
{
    impl_->sampleRate = sampleRate > 1.0 ? sampleRate : 48000.0;
    impl_->maxBlockSize = std::max<std::size_t>(1, maxBlockSize);
    impl_->prepared = true;
    ResetToDefaultState(impl_->randomSeed);
}

void TorusCore::Process(const AudioBufferView& input,
                        const AudioBufferWriteView& output,
                        std::size_t frameCount)
{
    for(std::size_t channel = 0; channel < output.channelCount; ++channel)
    {
        if(output.channels[channel] != nullptr)
        {
            std::fill(output.channels[channel], output.channels[channel] + frameCount, 0.0f);
        }
    }

    if(!impl_->prepared)
    {
        return;
    }

    const float* inputChannel = (input.channelCount > 0 && input.channels[0] != nullptr)
                                    ? input.channels[0]
                                    : nullptr;
    const bool gate1 = impl_->portInputs[MakeGateInputPortId(nodeId_, 1)].gate;
    const bool gateRisingEdge = gate1 && !impl_->previousGate1;
    impl_->previousGate1 = gate1;

    const auto frequencyNorm
        = impl_->parameters[static_cast<std::size_t>(ParameterIndex::kFrequency)].effectiveNormalizedValue;
    const auto structureNorm
        = impl_->parameters[static_cast<std::size_t>(ParameterIndex::kStructure)].effectiveNormalizedValue;
    const auto brightnessNorm
        = impl_->parameters[static_cast<std::size_t>(ParameterIndex::kBrightness)].effectiveNormalizedValue;
    const auto dampingNorm
        = impl_->parameters[static_cast<std::size_t>(ParameterIndex::kDamping)].effectiveNormalizedValue;
    const auto positionNorm
        = impl_->parameters[static_cast<std::size_t>(ParameterIndex::kPosition)].effectiveNormalizedValue;
    const auto polyphonyChoice
        = QuantizeChoice(impl_->parameters[static_cast<std::size_t>(ParameterIndex::kPolyphony)]
                             .effectiveNormalizedValue,
                         3);
    const auto modelChoice
        = QuantizeChoice(impl_->parameters[static_cast<std::size_t>(ParameterIndex::kModel)]
                             .effectiveNormalizedValue,
                         6);
    const auto fxChoice
        = QuantizeChoice(impl_->parameters[static_cast<std::size_t>(ParameterIndex::kEasterFx)]
                             .effectiveNormalizedValue,
                         6);
    const bool easterEnabled
        = impl_->parameters[static_cast<std::size_t>(ParameterIndex::kEasterEgg)]
              .effectiveNormalizedValue
          >= 0.5f;
    const bool internalExciter
        = impl_->parameters[static_cast<std::size_t>(ParameterIndex::kNormalizeExciter)]
              .effectiveNormalizedValue
          < 0.5f;
    const bool internalStrum
        = impl_->parameters[static_cast<std::size_t>(ParameterIndex::kNormalizeStrum)]
              .effectiveNormalizedValue
          < 0.5f;

    const int voiceCount = polyphonyChoice == 0 ? 1 : (polyphonyChoice == 1 ? 2 : 4);
    const float baseFrequency = 27.5f * std::pow(2.0f, frequencyNorm * 6.0f);
    const float decay = 0.985f - dampingNorm * 0.08f;
    const float structureMix = 0.2f + structureNorm * 1.6f;
    const float brightnessGain = 0.3f + brightnessNorm * 1.8f;
    const float spread = 0.01f + positionNorm * 0.09f;
    const float shimmerAmount = easterEnabled ? (0.05f + 0.25f * (static_cast<float>(fxChoice) / 5.0f)) : 0.0f;

    float inputPeak = PeakForBuffer(inputChannel, frameCount);
    impl_->smoothedInputPeak += (inputPeak - impl_->smoothedInputPeak) * 0.2f;

    const bool quietState = !gate1 && !internalExciter && !internalStrum
                            && inputPeak < 0.0001f
                            && std::abs(impl_->previousLeft) < 0.0001f
                            && std::abs(impl_->previousRight) < 0.0001f
                            && std::all_of(impl_->excitation.begin(),
                                           impl_->excitation.end(),
                                           [](float value) {
                                               return std::abs(value) < 0.0001f;
                                           })
                            && std::all_of(impl_->resonator.begin(),
                                           impl_->resonator.end(),
                                           [](float value) {
                                               return std::abs(value) < 0.0001f;
                                           });

    if(quietState)
    {
        PortValue inputValue;
        inputValue.type = VirtualPortType::kAudio;
        inputValue.scalar = 0.0f;
        impl_->portInputs[MakeAudioInputPortId(nodeId_, 1)] = inputValue;

        PortValue leftValue;
        leftValue.type = VirtualPortType::kAudio;
        leftValue.scalar = 0.0f;
        impl_->portOutputs[MakeAudioOutputPortId(nodeId_, 1)] = leftValue;

        PortValue rightValue;
        rightValue.type = VirtualPortType::kAudio;
        rightValue.scalar = 0.0f;
        impl_->portOutputs[MakeAudioOutputPortId(nodeId_, 2)] = rightValue;

        PortValue gateOut;
        gateOut.type = VirtualPortType::kGate;
        gateOut.gate = false;
        impl_->portOutputs[MakeGateOutputPortId(nodeId_, 1)] = gateOut;
        return;
    }

    PortValue gateOut;
    gateOut.type = VirtualPortType::kGate;
    gateOut.gate = false;

    for(std::size_t sample = 0; sample < frameCount; ++sample)
    {
        float exciterSample = 0.0f;
        const float externalInput = inputChannel != nullptr ? inputChannel[sample] : 0.0f;

        if(gateRisingEdge && sample == 0)
        {
            impl_->excitation[0] = 1.0f;
            gateOut.gate = true;
        }

        if(internalStrum && !gate1 && sample == 0 && impl_->smoothedInputPeak < 0.001f)
        {
            impl_->excitation[0] = 0.7f;
        }

        if(!internalExciter)
        {
            exciterSample += externalInput;
        }

        float left = 0.0f;
        float right = 0.0f;

        for(int voice = 0; voice < voiceCount; ++voice)
        {
            const float voiceSpread = (static_cast<float>(voice) - static_cast<float>(voiceCount - 1) * 0.5f) * spread;
            const float voiceFrequency = baseFrequency * (1.0f + voiceSpread);
            const float phaseIncrement = voiceFrequency / static_cast<float>(impl_->sampleRate);

            impl_->phase[voice] += phaseIncrement;
            if(impl_->phase[voice] >= 1.0f)
            {
                impl_->phase[voice] -= std::floor(impl_->phase[voice]);
            }

            impl_->noiseState = impl_->noiseState * 1664525u + 1013904223u;
            const float noise = static_cast<float>((impl_->noiseState >> 9) & 0x7fffff) / 4194304.0f - 1.0f;

            float voiceExciter = impl_->excitation[voice];
            if(internalExciter)
            {
                voiceExciter += noise * 0.03f;
            }
            voiceExciter += exciterSample * (voice == 0 ? 0.45f : 0.15f);
            const float excitationDrive
                = Clamp01(std::abs(voiceExciter) * 1.8f + std::abs(exciterSample) * 0.35f);

            const float phase = impl_->phase[voice] * kTwoPi;
            float harmonic = std::sin(phase);
            switch(modelChoice)
            {
                case 0: harmonic += structureMix * 0.25f * std::sin(phase * 2.0f); break;
                case 1: harmonic = std::sin(phase) * 0.7f + std::sin(phase * 3.0f) * structureMix * 0.18f; break;
                case 2: harmonic = std::sin(phase) + std::sin(phase * 5.0f) * structureMix * 0.08f; break;
                case 3: harmonic = std::sin(phase + std::sin(phase * 2.0f) * structureMix * 0.7f); break;
                case 4: harmonic = std::copysign(std::sqrt(std::abs(std::sin(phase))), std::sin(phase)); break;
                case 5:
                default: harmonic = std::sin(phase) * 0.5f + noise * structureMix * 0.2f; break;
            }

            impl_->resonator[voice]
                = impl_->resonator[voice] * decay
                  + (voiceExciter
                     + harmonic * brightnessGain * 0.08f * excitationDrive)
                        * (0.15f + structureMix * 0.10f);
            impl_->shimmer[voice] = impl_->shimmer[voice] * 0.92f
                                    + impl_->resonator[voice] * shimmerAmount;
            impl_->excitation[voice] *= 0.86f - dampingNorm * 0.22f;

            float voiceOut = impl_->resonator[voice];
            if(easterEnabled)
            {
                switch(fxChoice)
                {
                    case 0: voiceOut += impl_->shimmer[voice] * 0.25f; break;
                    case 1: voiceOut = std::tanh(voiceOut * 1.4f); break;
                    case 2: voiceOut += std::sin(phase * 0.5f + impl_->lfoPhase) * 0.08f; break;
                    case 3: voiceOut = std::sin(voiceOut * 2.5f); break;
                    case 4: voiceOut = 0.7f * voiceOut + 0.3f * impl_->previousLeft; break;
                    case 5: voiceOut = 0.7f * voiceOut + 0.3f * impl_->previousRight; break;
                    default: break;
                }
            }

            const float pan = voiceCount == 1
                                  ? positionNorm
                                  : Clamp01(0.5f + voiceSpread * 2.5f);
            left += voiceOut * (1.0f - pan);
            right += voiceOut * pan;
        }

        left /= static_cast<float>(voiceCount);
        right /= static_cast<float>(voiceCount);

        impl_->lfoPhase += 0.0008f + 0.0015f * positionNorm;
        if(impl_->lfoPhase > kTwoPi)
        {
            impl_->lfoPhase -= kTwoPi;
        }

        const float stereoTilt = 0.08f + structureNorm * 0.12f;
        const float motionDrive = Clamp01(std::abs(left)
                                          + std::abs(right)
                                          + std::abs(impl_->previousLeft) * 0.5f
                                          + std::abs(impl_->previousRight) * 0.5f
                                          + std::abs(exciterSample));
        left = std::tanh(left * (0.9f + brightnessGain * 0.18f)
                         + std::sin(impl_->lfoPhase) * stereoTilt * motionDrive);
        right = std::tanh(right * (0.9f + brightnessGain * 0.18f)
                          - std::sin(impl_->lfoPhase) * stereoTilt * motionDrive);

        impl_->previousLeft = left;
        impl_->previousRight = right;

        if(output.channelCount > 0 && output.channels[0] != nullptr)
        {
            output.channels[0][sample] = left;
        }
        if(output.channelCount > 1 && output.channels[1] != nullptr)
        {
            output.channels[1][sample] = right;
        }
    }

    for(std::size_t channel = 2; channel < output.channelCount; ++channel)
    {
        if(output.channels[channel] != nullptr)
        {
            std::fill(output.channels[channel], output.channels[channel] + frameCount, 0.0f);
        }
    }

    PortValue inputValue;
    inputValue.type = VirtualPortType::kAudio;
    inputValue.scalar = inputPeak;
    impl_->portInputs[MakeAudioInputPortId(nodeId_, 1)] = inputValue;

    PortValue leftValue;
    leftValue.type = VirtualPortType::kAudio;
    leftValue.scalar = output.channelCount > 0 && output.channels[0] != nullptr
                           ? PeakForBuffer(output.channels[0], frameCount)
                           : 0.0f;
    impl_->portOutputs[MakeAudioOutputPortId(nodeId_, 1)] = leftValue;

    PortValue rightValue;
    rightValue.type = VirtualPortType::kAudio;
    rightValue.scalar = output.channelCount > 1 && output.channels[1] != nullptr
                            ? PeakForBuffer(output.channels[1], frameCount)
                            : 0.0f;
    impl_->portOutputs[MakeAudioOutputPortId(nodeId_, 2)] = rightValue;
    impl_->portOutputs[MakeGateOutputPortId(nodeId_, 1)] = gateOut;
}

void TorusCore::SetControl(const std::string& controlId, float normalizedValue)
{
    for(std::size_t i = 0; i < 4; ++i)
    {
        if(controlId == MakeKnobControlId(nodeId_, i + 1))
        {
            const int assignment = AssignmentForControl(*impl_, i);
            impl_->controlInputs[i] = Clamp01(normalizedValue);
            impl_->controlInputInitialized[i] = true;
            SetParameterValue(
                impl_->parameters[static_cast<std::size_t>(AssignmentToParameterIndex(assignment))].id,
                normalizedValue);
            impl_->parameterSources[static_cast<std::size_t>(AssignmentToParameterIndex(assignment))]
                = ParameterSource::kControl;
            return;
        }
    }
}

void TorusCore::SetEncoderDelta(int delta)
{
    MenuRotate(delta);
}

void TorusCore::SetEncoderPress(bool pressed)
{
    if(pressed && !impl_->encoderPressed)
    {
        MenuPress();
    }
    impl_->encoderPressed = pressed;
}

void TorusCore::SetPortInput(const std::string& portId, const PortValue& value)
{
    impl_->portInputs[portId] = value;

    for(std::size_t i = 0; i < 4; ++i)
    {
        if(portId == MakeCvInputPortId(nodeId_, i + 1) && value.type == VirtualPortType::kCv)
        {
            const int assignment = QuantizeChoice(
                impl_->parameters[static_cast<std::size_t>(ParameterIndex::kCtrl1Assignment) + i]
                    .effectiveNormalizedValue,
                5);
            SetParameterValue(
                impl_->parameters[static_cast<std::size_t>(AssignmentToParameterIndex(assignment))].id,
                value.scalar);
            impl_->parameterSources[static_cast<std::size_t>(AssignmentToParameterIndex(assignment))]
                = ParameterSource::kCv;
            return;
        }
    }
}

PortValue TorusCore::GetPortOutput(const std::string& portId) const
{
    const auto outputIt = impl_->portOutputs.find(portId);
    if(outputIt != impl_->portOutputs.end())
    {
        return outputIt->second;
    }

    const auto inputIt = impl_->portInputs.find(portId);
    if(inputIt != impl_->portInputs.end())
    {
        return inputIt->second;
    }

    return {};
}

void TorusCore::TickUi(double)
{
}

bool TorusCore::SetParameterValue(const std::string& parameterId, float normalizedValue)
{
    const auto it = std::find_if(
        impl_->parameters.begin(),
        impl_->parameters.end(),
        [&parameterId](const ParameterDescriptor& descriptor) { return descriptor.id == parameterId; });
    if(it == impl_->parameters.end())
    {
        return false;
    }

    auto& parameter = *it;
    const std::size_t index = static_cast<std::size_t>(std::distance(impl_->parameters.begin(), it));
    const float clamped = Clamp01(normalizedValue);

    parameter.normalizedValue = parameter.stepCount > 1
                                    ? QuantizedNormalized(clamped, parameter.stepCount)
                                    : clamped;
    parameter.effectiveNormalizedValue = parameter.normalizedValue;
    impl_->parameterSources[index] = ParameterSource::kMenu;
    impl_->parameterTouchSerials[index] = ++impl_->nextTouchSerial;

    if(index >= static_cast<std::size_t>(ParameterIndex::kCtrl1Assignment)
       && index <= static_cast<std::size_t>(ParameterIndex::kCtrl4Assignment))
    {
        SyncControlInputsFromAssignments(*impl_);
    }

    // Keep the status/menu snapshots in sync with canonical parameter state.
    MenuRotate(0);
    return true;
}

ParameterValueLookup TorusCore::GetControlValue(const std::string& controlId) const
{
    for(std::size_t i = 0; i < 4; ++i)
    {
        if(controlId == MakeKnobControlId(nodeId_, i + 1))
        {
            return {true,
                    impl_->controlInputInitialized[i] ? impl_->controlInputs[i]
                                                      : AssignedParameterNormalizedValue(*impl_, i)};
        }
    }
    return {};
}

ParameterValueLookup TorusCore::GetParameterValue(const std::string& parameterId) const
{
    const auto it = std::find_if(
        impl_->parameters.begin(),
        impl_->parameters.end(),
        [&parameterId](const ParameterDescriptor& descriptor) { return descriptor.id == parameterId; });
    return it != impl_->parameters.end() ? ParameterValueLookup{true, it->normalizedValue}
                                         : ParameterValueLookup{};
}

ParameterValueLookup TorusCore::GetEffectiveParameterValue(const std::string& parameterId) const
{
    const auto it = std::find_if(
        impl_->parameters.begin(),
        impl_->parameters.end(),
        [&parameterId](const ParameterDescriptor& descriptor) { return descriptor.id == parameterId; });
    return it != impl_->parameters.end()
               ? ParameterValueLookup{true, it->effectiveNormalizedValue}
               : ParameterValueLookup{};
}

void TorusCore::ResetToDefaultState(std::uint32_t seed)
{
    impl_->randomSeed = seed;
    impl_->noiseState = seed == 0 ? 1u : seed;
    impl_->phase.fill(0.0f);
    impl_->excitation.fill(0.0f);
    impl_->resonator.fill(0.0f);
    impl_->shimmer.fill(0.0f);
    impl_->previousGate1 = false;
    impl_->previousLeft = 0.0f;
    impl_->previousRight = 0.0f;
    impl_->lfoPhase = 0.0f;
    impl_->smoothedInputPeak = 0.0f;
    impl_->nextTouchSerial = 0;
    impl_->parameterTouchSerials.fill(0);
    impl_->parameterSources.fill(ParameterSource::kNone);
    impl_->controlInputs.fill(0.0f);
    impl_->controlInputInitialized.fill(false);
    impl_->encoderPressed = false;

    for(auto& parameter : impl_->parameters)
    {
        parameter.normalizedValue = parameter.defaultNormalizedValue;
        parameter.effectiveNormalizedValue = parameter.defaultNormalizedValue;
    }

    SyncControlInputsFromAssignments(*impl_);

    impl_->menu = {};
    impl_->menu.currentSectionId = MakeMenuRootSectionId(nodeId_);
    impl_->menu.sectionStack = {MakeMenuRootSectionId(nodeId_)};
    impl_->display = {};
    impl_->display.width = 128;
    impl_->display.height = 64;
    impl_->display.title = "Torus";
    impl_->display.mode = DisplayMode::kStatus;
    impl_->display.revision++;
    MenuRotate(0);
}

std::unordered_map<std::string, float> TorusCore::CaptureStatefulParameterValues() const
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

void TorusCore::RestoreStatefulParameterValues(const std::unordered_map<std::string, float>& values)
{
    for(const auto& entry : values)
    {
        if(SetParameterValue(entry.first, entry.second))
        {
            const auto it = std::find_if(
                impl_->parameters.begin(),
                impl_->parameters.end(),
                [&entry](const ParameterDescriptor& descriptor) { return descriptor.id == entry.first; });
            if(it != impl_->parameters.end())
            {
                impl_->parameterSources[static_cast<std::size_t>(
                    std::distance(impl_->parameters.begin(), it))] = ParameterSource::kRestore;
            }
        }
    }
    SyncControlInputsFromAssignments(*impl_);
    MenuRotate(0);
}

const std::vector<ParameterDescriptor>& TorusCore::GetParameters() const
{
    return impl_->parameters;
}

const MenuModel& TorusCore::GetMenuModel() const
{
    return impl_->menu;
}

void TorusCore::MenuRotate(int delta)
{
    auto rebuildMenu = [&]() {
        auto boolText = [](float value) { return value >= 0.5f ? std::string("On") : std::string("Off"); };
        impl_->menu.sections.clear();

        MenuSection root;
        root.id = MakeMenuRootSectionId(nodeId_);
        root.title = "Menu";
        root.items = {
            {MakeMenuItemId(nodeId_, "root", "controls"), "Controls", false, MenuItemActionKind::kEnterSection, 0.0f, "", MakeMenuSectionId(nodeId_, "controls")},
            {MakeMenuItemId(nodeId_, "root", "poly_model"), "Poly/Model", false, MenuItemActionKind::kEnterSection, 0.0f, "", MakeMenuSectionId(nodeId_, "poly_model")},
            {MakeMenuItemId(nodeId_, "root", "normalize"), "Normalize", false, MenuItemActionKind::kEnterSection, 0.0f, "", MakeMenuSectionId(nodeId_, "normalize")},
            {MakeMenuItemId(nodeId_, "root", "easter"), "Easter Egg", false, MenuItemActionKind::kEnterSection, 0.0f, "", MakeMenuSectionId(nodeId_, "easter")},
        };
        root.selectedIndex = impl_->menuSelections[root.id];

        MenuSection controls;
        controls.id = MakeMenuSectionId(nodeId_, "controls");
        controls.title = "Controls";
        for(std::size_t i = 0; i < 4; ++i)
        {
            const auto& assignmentParameter
                = impl_->parameters[static_cast<std::size_t>(ParameterIndex::kCtrl1Assignment) + i];
            controls.items.push_back({MakeMenuItemId(nodeId_, "controls", "ctrl" + std::to_string(i + 1) + "_assignment"),
                                      "CTRL " + std::to_string(i + 1),
                                      true,
                                      MenuItemActionKind::kValue,
                                      assignmentParameter.normalizedValue,
                                      ControlNameFromAssignment(QuantizeChoice(assignmentParameter.normalizedValue, 5)),
                                      ""});
        }
        controls.items.push_back({MakeMenuItemId(nodeId_, "controls", "back"), "Back", false, MenuItemActionKind::kBack, 0.0f, "", ""});
        controls.selectedIndex = impl_->menuSelections[controls.id];

        MenuSection polyModel;
        polyModel.id = MakeMenuSectionId(nodeId_, "poly_model");
        polyModel.title = "Poly/Model";
        polyModel.items = {
            {MakeMenuItemId(nodeId_, "poly_model", "polyphony"),
             "Polyphony",
             true,
             MenuItemActionKind::kValue,
             impl_->parameters[static_cast<std::size_t>(ParameterIndex::kPolyphony)].normalizedValue,
             std::to_string(QuantizeChoice(impl_->parameters[static_cast<std::size_t>(ParameterIndex::kPolyphony)].normalizedValue, 3) == 0 ? 1 : (QuantizeChoice(impl_->parameters[static_cast<std::size_t>(ParameterIndex::kPolyphony)].normalizedValue, 3) == 1 ? 2 : 4)),
             ""},
            {MakeMenuItemId(nodeId_, "poly_model", "model"),
             "Model",
             true,
             MenuItemActionKind::kValue,
             impl_->parameters[static_cast<std::size_t>(ParameterIndex::kModel)].normalizedValue,
             std::to_string(QuantizeChoice(impl_->parameters[static_cast<std::size_t>(ParameterIndex::kModel)].normalizedValue, 6)),
             ""},
            {MakeMenuItemId(nodeId_, "poly_model", "easter_fx"),
             "Easter FX",
             true,
             MenuItemActionKind::kValue,
             impl_->parameters[static_cast<std::size_t>(ParameterIndex::kEasterFx)].normalizedValue,
             std::to_string(QuantizeChoice(impl_->parameters[static_cast<std::size_t>(ParameterIndex::kEasterFx)].normalizedValue, 6)),
             ""},
            {MakeMenuItemId(nodeId_, "poly_model", "back"), "Back", false, MenuItemActionKind::kBack, 0.0f, "", ""},
        };
        polyModel.selectedIndex = impl_->menuSelections[polyModel.id];

        MenuSection normalize;
        normalize.id = MakeMenuSectionId(nodeId_, "normalize");
        normalize.title = "Normalize";
        normalize.items = {
            {MakeMenuItemId(nodeId_, "normalize", "exciter"), "Exciter", true, MenuItemActionKind::kValue, impl_->parameters[static_cast<std::size_t>(ParameterIndex::kNormalizeExciter)].normalizedValue, boolText(impl_->parameters[static_cast<std::size_t>(ParameterIndex::kNormalizeExciter)].normalizedValue), ""},
            {MakeMenuItemId(nodeId_, "normalize", "note"), "Note", true, MenuItemActionKind::kValue, impl_->parameters[static_cast<std::size_t>(ParameterIndex::kNormalizeNote)].normalizedValue, boolText(impl_->parameters[static_cast<std::size_t>(ParameterIndex::kNormalizeNote)].normalizedValue), ""},
            {MakeMenuItemId(nodeId_, "normalize", "strum"), "Strum", true, MenuItemActionKind::kValue, impl_->parameters[static_cast<std::size_t>(ParameterIndex::kNormalizeStrum)].normalizedValue, boolText(impl_->parameters[static_cast<std::size_t>(ParameterIndex::kNormalizeStrum)].normalizedValue), ""},
            {MakeMenuItemId(nodeId_, "normalize", "back"), "Back", false, MenuItemActionKind::kBack, 0.0f, "", ""},
        };
        normalize.selectedIndex = impl_->menuSelections[normalize.id];

        MenuSection easter;
        easter.id = MakeMenuSectionId(nodeId_, "easter");
        easter.title = "Easter Egg";
        easter.items = {
            {MakeMenuItemId(nodeId_, "easter", "enabled"), "Enabled", true, MenuItemActionKind::kValue, impl_->parameters[static_cast<std::size_t>(ParameterIndex::kEasterEgg)].normalizedValue, boolText(impl_->parameters[static_cast<std::size_t>(ParameterIndex::kEasterEgg)].normalizedValue), ""},
            {MakeMenuItemId(nodeId_, "easter", "back"), "Back", false, MenuItemActionKind::kBack, 0.0f, "", ""},
        };
        easter.selectedIndex = impl_->menuSelections[easter.id];

        impl_->menu.sections = {root, controls, polyModel, normalize, easter};
        if(impl_->menu.sectionStack.empty())
        {
            impl_->menu.sectionStack = {root.id};
        }
        if(impl_->menu.currentSectionId.empty())
        {
            impl_->menu.currentSectionId = root.id;
        }
        const auto sectionIt = std::find_if(
            impl_->menu.sections.begin(),
            impl_->menu.sections.end(),
            [this](const MenuSection& section) { return section.id == impl_->menu.currentSectionId; });
        impl_->menu.currentSelection = sectionIt != impl_->menu.sections.end() ? sectionIt->selectedIndex : 0;
    };

    auto updateDisplay = [&]() {
        impl_->display.mode = impl_->menu.isOpen ? DisplayMode::kMenu : DisplayMode::kStatus;
        impl_->display.texts.clear();
        impl_->display.bars.clear();
        impl_->display.revision++;

        if(impl_->menu.isOpen)
        {
            impl_->display.texts.push_back({0, 0, "Torus Menu", false});
            const auto sectionIt = std::find_if(
                impl_->menu.sections.begin(),
                impl_->menu.sections.end(),
                [this](const MenuSection& section) { return section.id == impl_->menu.currentSectionId; });
            if(sectionIt != impl_->menu.sections.end())
            {
                const auto& section = *sectionIt;
                for(int row = 0; row < 3; ++row)
                {
                    const int itemIndex = row + std::max(0, section.selectedIndex - 1);
                    if(itemIndex >= static_cast<int>(section.items.size()))
                    {
                        break;
                    }

                    const auto& item = section.items[static_cast<std::size_t>(itemIndex)];
                    std::string text = (itemIndex == section.selectedIndex ? "> " : "  ") + item.label;
                    if(!item.valueText.empty())
                    {
                        text += " " + item.valueText;
                    }
                    impl_->display.texts.push_back({0, 16 + row * 14, text, false});
                }
            }
            return;
        }

        impl_->display.texts.push_back({0, 0, "Torus", false});
        impl_->display.texts.push_back({0, 16, "Model " + std::to_string(QuantizeChoice(impl_->parameters[static_cast<std::size_t>(ParameterIndex::kModel)].normalizedValue, 6)), false});
        impl_->display.texts.push_back({0, 30, "CTRL1 " + ControlNameFromAssignment(QuantizeChoice(impl_->parameters[static_cast<std::size_t>(ParameterIndex::kCtrl1Assignment)].normalizedValue, 5)), false});
        impl_->display.texts.push_back({0, 44, "CTRL2 " + ControlNameFromAssignment(QuantizeChoice(impl_->parameters[static_cast<std::size_t>(ParameterIndex::kCtrl2Assignment)].normalizedValue, 5)), false});
    };

    rebuildMenu();

    if(!impl_->menu.isOpen || delta == 0)
    {
        updateDisplay();
        return;
    }

    if(impl_->menu.isEditing)
    {
        const auto sectionIt = std::find_if(
            impl_->menu.sections.begin(),
            impl_->menu.sections.end(),
            [this](const MenuSection& section) { return section.id == impl_->menu.currentSectionId; });
        if(sectionIt != impl_->menu.sections.end()
           && sectionIt->selectedIndex >= 0
           && sectionIt->selectedIndex < static_cast<int>(sectionIt->items.size()))
        {
            const auto& item = sectionIt->items[static_cast<std::size_t>(sectionIt->selectedIndex)];
            if(item.editable && item.actionKind == MenuItemActionKind::kValue)
            {
                SetMenuItemValue(item.id, item.normalizedValue + BaseStepForMenuItem(item.id) * static_cast<float>(delta));
                return;
            }
        }
    }

    auto& sectionSelection = impl_->menuSelections[impl_->menu.currentSectionId];
    const auto sectionIt = std::find_if(
        impl_->menu.sections.begin(),
        impl_->menu.sections.end(),
        [this](const MenuSection& section) { return section.id == impl_->menu.currentSectionId; });
    if(sectionIt != impl_->menu.sections.end())
    {
        const int itemCount = static_cast<int>(sectionIt->items.size());
        sectionSelection = std::clamp(sectionSelection + delta, 0, std::max(0, itemCount - 1));
        impl_->menu.currentSelection = sectionSelection;
    }
    rebuildMenu();
    updateDisplay();
}

void TorusCore::MenuPress()
{
    if(!impl_->menu.isOpen)
    {
        impl_->menu.isOpen = true;
        impl_->menu.isEditing = false;
        impl_->menu.currentSectionId = MakeMenuRootSectionId(nodeId_);
        impl_->menu.currentSelection = impl_->menuSelections[impl_->menu.currentSectionId];
        MenuRotate(0);
        return;
    }

    const auto sectionIt = std::find_if(
        impl_->menu.sections.begin(),
        impl_->menu.sections.end(),
        [this](const MenuSection& section) { return section.id == impl_->menu.currentSectionId; });
    if(sectionIt == impl_->menu.sections.end()
       || sectionIt->selectedIndex < 0
       || sectionIt->selectedIndex >= static_cast<int>(sectionIt->items.size()))
    {
        impl_->menu.isOpen = false;
        impl_->menu.isEditing = false;
        MenuRotate(0);
        return;
    }

    const auto& item = sectionIt->items[static_cast<std::size_t>(sectionIt->selectedIndex)];

    if(impl_->menu.isEditing)
    {
        impl_->menu.isEditing = false;
        MenuRotate(0);
        return;
    }

    switch(item.actionKind)
    {
        case MenuItemActionKind::kEnterSection:
            impl_->menu.currentSectionId = item.targetSectionId;
            impl_->menu.sectionStack.push_back(item.targetSectionId);
            impl_->menu.currentSelection = impl_->menuSelections[item.targetSectionId];
            break;
        case MenuItemActionKind::kBack:
            if(impl_->menu.sectionStack.size() > 1)
            {
                impl_->menu.sectionStack.pop_back();
                impl_->menu.currentSectionId = impl_->menu.sectionStack.back();
            }
            else
            {
                impl_->menu.currentSectionId = MakeMenuRootSectionId(nodeId_);
                impl_->menu.sectionStack = {impl_->menu.currentSectionId};
                impl_->menu.isOpen = false;
            }
            impl_->menu.currentSelection = impl_->menuSelections[impl_->menu.currentSectionId];
            break;
        case MenuItemActionKind::kValue:
            impl_->menu.isEditing = true;
            break;
        case MenuItemActionKind::kMomentary:
        case MenuItemActionKind::kReadonly:
        default: break;
    }

    MenuRotate(0);
}

void TorusCore::SetMenuItemValue(const std::string& itemId, float normalizedValue)
{
    const float clamped = Clamp01(normalizedValue);
    if(itemId == MakeMenuItemId(nodeId_, "controls", "ctrl1_assignment"))
    {
        SetParameterValue(MakeParameterId(nodeId_, "ctrl1_assignment"), clamped);
    }
    else if(itemId == MakeMenuItemId(nodeId_, "controls", "ctrl2_assignment"))
    {
        SetParameterValue(MakeParameterId(nodeId_, "ctrl2_assignment"), clamped);
    }
    else if(itemId == MakeMenuItemId(nodeId_, "controls", "ctrl3_assignment"))
    {
        SetParameterValue(MakeParameterId(nodeId_, "ctrl3_assignment"), clamped);
    }
    else if(itemId == MakeMenuItemId(nodeId_, "controls", "ctrl4_assignment"))
    {
        SetParameterValue(MakeParameterId(nodeId_, "ctrl4_assignment"), clamped);
    }
    else if(itemId == MakeMenuItemId(nodeId_, "poly_model", "polyphony"))
    {
        SetParameterValue(MakeParameterId(nodeId_, "polyphony"), clamped);
    }
    else if(itemId == MakeMenuItemId(nodeId_, "poly_model", "model"))
    {
        SetParameterValue(MakeParameterId(nodeId_, "model"), clamped);
    }
    else if(itemId == MakeMenuItemId(nodeId_, "poly_model", "easter_fx"))
    {
        SetParameterValue(MakeParameterId(nodeId_, "easter_fx"), clamped);
    }
    else if(itemId == MakeMenuItemId(nodeId_, "normalize", "exciter"))
    {
        SetParameterValue(MakeParameterId(nodeId_, "normalize_exciter"), clamped);
    }
    else if(itemId == MakeMenuItemId(nodeId_, "normalize", "note"))
    {
        SetParameterValue(MakeParameterId(nodeId_, "normalize_note"), clamped);
    }
    else if(itemId == MakeMenuItemId(nodeId_, "normalize", "strum"))
    {
        SetParameterValue(MakeParameterId(nodeId_, "normalize_strum"), clamped);
    }
    else if(itemId == MakeMenuItemId(nodeId_, "easter", "enabled"))
    {
        SetParameterValue(MakeParameterId(nodeId_, "easter_egg"), clamped);
    }
    else
    {
        return;
    }

    MenuRotate(0);
}

const DisplayModel& TorusCore::GetDisplayModel() const
{
    return impl_->display;
}

std::string TorusCore::MakeKnobControlId(const std::string& nodeId, std::size_t oneBasedIndex)
{
    return nodeId + "/control/ctrl_" + std::to_string(oneBasedIndex);
}

std::string TorusCore::MakeEncoderControlId(const std::string& nodeId)
{
    return nodeId + "/control/encoder";
}

std::string TorusCore::MakeEncoderButtonControlId(const std::string& nodeId)
{
    return nodeId + "/control/encoder_button";
}

std::string TorusCore::MakeAudioInputPortId(const std::string& nodeId, std::size_t oneBasedIndex)
{
    return nodeId + "/port/audio_in_" + std::to_string(oneBasedIndex);
}

std::string TorusCore::MakeAudioOutputPortId(const std::string& nodeId, std::size_t oneBasedIndex)
{
    return nodeId + "/port/audio_out_" + std::to_string(oneBasedIndex);
}

std::string TorusCore::MakeCvInputPortId(const std::string& nodeId, std::size_t oneBasedIndex)
{
    return nodeId + "/port/cv_in_" + std::to_string(oneBasedIndex);
}

std::string TorusCore::MakeGateInputPortId(const std::string& nodeId, std::size_t oneBasedIndex)
{
    return nodeId + "/port/gate_in_" + std::to_string(oneBasedIndex);
}

std::string TorusCore::MakeGateOutputPortId(const std::string& nodeId, std::size_t oneBasedIndex)
{
    return nodeId + "/port/gate_out_" + std::to_string(oneBasedIndex);
}

std::string TorusCore::MakeMidiInputPortId(const std::string& nodeId, std::size_t oneBasedIndex)
{
    return nodeId + "/port/midi_in_" + std::to_string(oneBasedIndex);
}

std::string TorusCore::MakeMidiOutputPortId(const std::string& nodeId, std::size_t oneBasedIndex)
{
    return nodeId + "/port/midi_out_" + std::to_string(oneBasedIndex);
}
} // namespace apps
} // namespace daisyhost
