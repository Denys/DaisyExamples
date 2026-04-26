#include "daisyhost/apps/MultiDelayCore.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace daisyhost
{
namespace apps
{
namespace
{
constexpr float       kTouchEpsilon    = 0.0005f;
constexpr std::size_t kVisibleMenuRows = 4;

const char* kRootSectionId    = "root";
const char* kParamsSectionId  = "params";
const char* kMacrosSectionId  = "macros";
const char* kInputSectionId   = "input";
const char* kMidiSectionId    = "midi";
const char* kTrackerSectionId = "tracker";
const char* kAboutSectionId   = "about";

std::string MakeIndexedId(const std::string& nodeId,
                          const std::string& section,
                          const std::string& stem,
                          std::size_t        oneBasedIndex)
{
    return nodeId + "/" + section + "/" + stem + std::to_string(oneBasedIndex);
}

std::string MakeParameterId(const std::string& nodeId, const std::string& name)
{
    return nodeId + "/param/" + name;
}

std::string MakeMetaControllerId(const std::string& nodeId,
                                 const std::string& name)
{
    return nodeId + "/meta/" + name;
}

std::string MakeMenuItemId(const std::string& nodeId,
                           const std::string& section,
                           const std::string& name)
{
    return nodeId + "/menu/" + section + "/" + name;
}

float AbsMax(float currentPeak, float sample)
{
    const float magnitude = std::fabs(sample);
    return magnitude > currentPeak ? magnitude : currentPeak;
}

int ClampInt(int value, int minValue, int maxValue)
{
    return value < minValue ? minValue
                            : (value > maxValue ? maxValue : value);
}

std::string FormatString(const char* format, int value0)
{
    char buffer[48];
    std::snprintf(buffer, sizeof(buffer), format, value0);
    return std::string(buffer);
}

std::string FormatString(const char* format, int value0, int value1)
{
    char buffer[48];
    std::snprintf(buffer, sizeof(buffer), format, value0, value1);
    return std::string(buffer);
}

std::string FormatString(const char* format, int value0, int value1, int value2)
{
    char buffer[64];
    std::snprintf(buffer, sizeof(buffer), format, value0, value1, value2);
    return std::string(buffer);
}

std::string FormatMidiEvent(const MidiMessageEvent& event)
{
    const std::uint8_t statusNibble = event.status & 0xF0;
    const int          channel      = (event.status & 0x0F) + 1;

    switch(statusNibble)
    {
        case 0x80:
            return FormatString(
                "NoteOff ch%d %d", channel, static_cast<int>(event.data1));
        case 0x90:
            return FormatString("NoteOn ch%d %d vel %d",
                                channel,
                                static_cast<int>(event.data1),
                                static_cast<int>(event.data2));
        case 0xB0:
            return FormatString("CC ch%d %d = %d",
                                channel,
                                static_cast<int>(event.data1),
                                static_cast<int>(event.data2));
        case 0xC0:
            return FormatString(
                "Prog ch%d %d", channel, static_cast<int>(event.data1));
        default:
            return FormatString("MIDI 0x%02X", static_cast<int>(event.status));
    }
}
} // namespace

MultiDelayCore::MultiDelayCore(const std::string& nodeId)
: nodeId_(nodeId)
{
    controlInputs_.fill(0.0f);
    controlInputInitialized_.fill(false);
    cvInputs_.fill(0.0f);
    cvInputInitialized_.fill(false);
    parameterTouchSerials_.fill(0);
    parameterSources_.fill(ParameterSource::kNone);
    trackerLines_.fill("");
    aboutLines_
        = {{"Version 0.2.0", "Shared menu editing", "Patch UI refresh"}};

    parameters_ = {
        ParameterDescriptor{MakeParameterId(nodeId_, "dry_wet"),
                            "Dry/Wet",
                            0.50f,
                            0.50f,
                            0.50f,
                            "%",
                            0,
                            ParameterRole::kMix,
                            0,
                            true,
                            true,
                            true},
        ParameterDescriptor{MakeParameterId(nodeId_, "delay_primary"),
                            "Delay 1",
                            0.0f,
                            0.0f,
                            0.0f,
                            "samples",
                            0,
                            ParameterRole::kPrimaryDelay,
                            1,
                            true,
                            true,
                            true},
        ParameterDescriptor{MakeParameterId(nodeId_, "delay_secondary"),
                            "Delay 2",
                            0.0f,
                            0.0f,
                            0.0f,
                            "samples",
                            0,
                            ParameterRole::kSecondaryDelay,
                            2,
                            true,
                            true,
                            true},
        ParameterDescriptor{MakeParameterId(nodeId_, "feedback"),
                            "Feedback",
                            0.0f,
                            0.0f,
                            0.0f,
                            "%",
                            0,
                            ParameterRole::kFeedback,
                            3,
                            true,
                            true,
                            true},
        ParameterDescriptor{MakeParameterId(nodeId_, "delay_tertiary"),
                            "Delay 3",
                            0.0f,
                            0.0f,
                            0.0f,
                            "samples",
                            0,
                            ParameterRole::kTertiaryDelay,
                            4,
                            true,
                            true,
                            true},
    };
    parameters_[static_cast<std::size_t>(ParameterIndex::kDryWet)]
        .nativeMaximum = 100.0f;
    parameters_[static_cast<std::size_t>(ParameterIndex::kDryWet)]
        .nativeDefault = 50.0f;
    parameters_[static_cast<std::size_t>(ParameterIndex::kDryWet)]
        .nativePrecision = 0;
    for(const auto index : {ParameterIndex::kDelayPrimary,
                           ParameterIndex::kDelaySecondary,
                           ParameterIndex::kDelayTertiary})
    {
        auto& parameter = parameters_[static_cast<std::size_t>(index)];
        parameter.nativeMaximum = static_cast<float>(kMaxDelaySamples);
        parameter.nativePrecision = 0;
    }
    parameters_[static_cast<std::size_t>(ParameterIndex::kFeedback)]
        .nativeMaximum = 100.0f;
    parameters_[static_cast<std::size_t>(ParameterIndex::kFeedback)]
        .nativePrecision = 0;
    const float defaultRegen = std::max(
        0.0f,
        std::min((parameters_[static_cast<std::size_t>(ParameterIndex::kFeedback)]
                      .defaultNormalizedValue
                  - 0.05f)
                     / 0.85f,
                 1.0f));
    metaControllers_ = {
        MetaControllerDescriptor{MakeMetaControllerId(nodeId_, "blend"),
                                 "Blend",
                                 0.0f,
                                 parameters_[static_cast<std::size_t>(
                                     ParameterIndex::kDryWet)]
                                     .defaultNormalizedValue,
                                 true},
        MetaControllerDescriptor{MakeMetaControllerId(nodeId_, "space"),
                                 "Space",
                                 0.0f,
                                 parameters_[static_cast<std::size_t>(
                                     ParameterIndex::kDelayPrimary)]
                                     .defaultNormalizedValue,
                                 true},
        MetaControllerDescriptor{MakeMetaControllerId(nodeId_, "regen"),
                                 "Regen",
                                 0.0f,
                                 defaultRegen,
                                 true},
    };

    menuSelections_[kRootSectionId]    = 0;
    menuSelections_[kParamsSectionId]  = 0;
    menuSelections_[kMacrosSectionId]  = 0;
    menuSelections_[kInputSectionId]   = 0;
    menuSelections_[kMidiSectionId]    = 0;
    menuSelections_[kTrackerSectionId] = 0;
    menuSelections_[kAboutSectionId]   = 0;
    menu_.currentSectionId             = kRootSectionId;

    for(std::size_t i = 1; i <= 4; ++i)
    {
        portInputs_[MakeAudioInputPortId(nodeId_, i)].type = VirtualPortType::kAudio;
        portOutputs_[MakeAudioOutputPortId(nodeId_, i)].type = VirtualPortType::kAudio;
        portInputs_[MakeCvInputPortId(nodeId_, i)].type = VirtualPortType::kCv;
    }

    for(std::size_t i = 1; i <= 2; ++i)
    {
        portInputs_[MakeGateInputPortId(nodeId_, i)].type = VirtualPortType::kGate;
    }

    portOutputs_[MakeGateOutputPortId(nodeId_, 1)].type = VirtualPortType::kGate;
    portInputs_[MakeMidiInputPortId(nodeId_, 1)].type   = VirtualPortType::kMidi;
    portOutputs_[MakeMidiOutputPortId(nodeId_, 1)].type = VirtualPortType::kMidi;

    UpdateMappedStateFromParameters();
    SyncMenuModel();
    UpdateDisplay();
}

std::string MultiDelayCore::GetAppId() const
{
    return "multidelay";
}

std::string MultiDelayCore::GetAppDisplayName() const
{
    return "Multi Delay";
}

HostedAppCapabilities MultiDelayCore::GetCapabilities() const
{
    HostedAppCapabilities capabilities;
    capabilities.acceptsAudioInput  = true;
    capabilities.acceptsMidiInput   = true;
    capabilities.producesMidiOutput = true;
    return capabilities;
}

HostedAppPatchBindings MultiDelayCore::GetPatchBindings() const
{
    HostedAppPatchBindings bindings;
    bindings.knobControlIds = {MakeKnobControlId(nodeId_, 1),
                               MakeKnobControlId(nodeId_, 2),
                               MakeKnobControlId(nodeId_, 3),
                               MakeKnobControlId(nodeId_, 4)};
    bindings.knobParameterIds = {MakeParameterId(nodeId_, "dry_wet"),
                                 MakeParameterId(nodeId_, "delay_primary"),
                                 MakeParameterId(nodeId_, "delay_secondary"),
                                 MakeParameterId(nodeId_, "feedback")};
    bindings.knobDetailLabels = {"Mix", "Delay 1", "Delay 2", "Feedback"};
    bindings.encoderControlId = MakeEncoderControlId(nodeId_);
    bindings.encoderButtonControlId = MakeEncoderButtonControlId(nodeId_);
    for(std::size_t i = 0; i < 4; ++i)
    {
        bindings.cvInputPortIds[i]    = MakeCvInputPortId(nodeId_, i + 1);
        bindings.audioInputPortIds[i] = MakeAudioInputPortId(nodeId_, i + 1);
        bindings.audioOutputPortIds[i] = MakeAudioOutputPortId(nodeId_, i + 1);
    }
    for(std::size_t i = 0; i < 2; ++i)
    {
        bindings.gateInputPortIds[i] = MakeGateInputPortId(nodeId_, i + 1);
    }
    bindings.gateOutputPortId   = MakeGateOutputPortId(nodeId_, 1);
    bindings.midiInputPortId    = MakeMidiInputPortId(nodeId_, 1);
    bindings.midiOutputPortId   = MakeMidiOutputPortId(nodeId_, 1);
    bindings.mainOutputChannels = {3, 3};
    return bindings;
}

void MultiDelayCore::AttachDelayStorage(DelayLineType* delayLines, std::size_t count)
{
    delayLineStorage_ = delayLines;
    delayLineCount_   = count;
}

void MultiDelayCore::Prepare(double sampleRate, std::size_t maxBlockSize)
{
    sampleRate_   = sampleRate > 1000.0 ? sampleRate : 48000.0;
    maxBlockSize_ = maxBlockSize > 0 ? maxBlockSize : 48;

    if(delayLineStorage_ == nullptr || delayLineCount_ < kDelayCount)
    {
        ownedDelayLines_.reset(new DelayLineType[kDelayCount]);
        delayLineStorage_ = ownedDelayLines_.get();
        delayLineCount_   = kDelayCount;
    }

    for(std::size_t i = 0; i < kDelayCount; ++i)
    {
        delayLineStorage_[i].Init();
        delays_[i].line         = &delayLineStorage_[i];
        delays_[i].currentDelay = 0.0f;
        delays_[i].targetDelay  = 0.0f;
    }

    UpdateMappedStateFromParameters();
    SyncMenuModel();
    UpdateDisplay();
}

void MultiDelayCore::Process(const AudioBufferView&      input,
                             const AudioBufferWriteView& output,
                             std::size_t                 frameCount)
{
    for(std::size_t channel = 0; channel < output.channelCount; ++channel)
    {
        if(output.channels[channel] != nullptr)
        {
            std::fill(output.channels[channel],
                      output.channels[channel] + frameCount,
                      0.0f);
        }
    }

    PortValue inputPeak;
    inputPeak.type = VirtualPortType::kAudio;

    std::array<float, 4> outputPeaks = {{0.0f, 0.0f, 0.0f, 0.0f}};
    const float          dryWet      = static_cast<float>(dryWetPercent_) / 100.0f;

    for(std::size_t frame = 0; frame < frameCount; ++frame)
    {
        const float inSample
            = (input.channelCount > 0 && input.channels[0] != nullptr)
                  ? input.channels[0][frame]
                  : 0.0f;

        inputPeak.scalar = AbsMax(inputPeak.scalar, inSample);

        float mix = 0.0f;
        for(std::size_t delayIndex = 0; delayIndex < kDelayCount; ++delayIndex)
        {
            daisysp::fonepole(delays_[delayIndex].currentDelay,
                              delays_[delayIndex].targetDelay,
                              0.0002f);
            delays_[delayIndex].line->SetDelay(delays_[delayIndex].currentDelay);
            const float read = delays_[delayIndex].line->Read();
            delays_[delayIndex].line->Write((feedback_ * read) + inSample);
            mix += read;

            outputPeaks[delayIndex] = AbsMax(outputPeaks[delayIndex], read);
            if(delayIndex < output.channelCount
               && output.channels[delayIndex] != nullptr)
            {
                output.channels[delayIndex][frame] = read;
            }
        }

        const float mixOut = dryWet * mix * 0.3f + (1.0f - dryWet) * inSample;
        outputPeaks[3]     = AbsMax(outputPeaks[3], mixOut);

        if(output.channelCount > 3 && output.channels[3] != nullptr)
        {
            output.channels[3][frame] = mixOut;
        }
    }

    portInputs_[MakeAudioInputPortId(nodeId_, 1)] = inputPeak;
    for(std::size_t i = 0; i < 4; ++i)
    {
        PortValue outputValue;
        outputValue.type   = VirtualPortType::kAudio;
        outputValue.scalar = outputPeaks[i];
        portOutputs_[MakeAudioOutputPortId(nodeId_, i + 1)] = outputValue;
    }
}

void MultiDelayCore::SetControl(const std::string& controlId, float normalizedValue)
{
    const float value = Clamp01(normalizedValue);

    for(std::size_t i = 0; i < 4; ++i)
    {
        if(controlId == MakeKnobControlId(nodeId_, i + 1))
        {
            if(!HasMeaningfulChange(
                   value, controlInputs_[i], controlInputInitialized_[i]))
            {
                return;
            }

            controlInputs_[i]           = value;
            controlInputInitialized_[i] = true;

            switch(i)
            {
                case 0:
                    ApplyParameterValue(ParameterIndex::kDryWet,
                                        value,
                                        ParameterSource::kKnob);
                    return;
                case 1:
                    ApplyParameterValue(ParameterIndex::kDelayPrimary,
                                        value,
                                        ParameterSource::kKnob);
                    return;
                case 2:
                    ApplyParameterValue(ParameterIndex::kDelaySecondary,
                                        value,
                                        ParameterSource::kKnob);
                    return;
                case 3:
                    ApplyParameterValue(ParameterIndex::kFeedback,
                                        value,
                                        ParameterSource::kKnob);
                    return;
                default:
                    break;
            }
        }
    }

    if(controlId == MakeDryWetControlId(nodeId_))
    {
        ApplyParameterValue(ParameterIndex::kDryWet,
                            value,
                            ParameterSource::kKnob);
        return;
    }

    if(controlId == MakeEncoderButtonControlId(nodeId_))
    {
        SetEncoderPress(value >= 0.5f);
    }
}

void MultiDelayCore::SetEncoderDelta(int delta)
{
    MenuRotate(delta);
}

void MultiDelayCore::SetEncoderPress(bool pressed)
{
    if(pressed && !encoderPressed_)
    {
        MenuPress();
    }
    encoderPressed_ = pressed;
}

void MultiDelayCore::SetPortInput(const std::string& portId, const PortValue& value)
{
    portInputs_[portId] = value;

    if(portId == MakeCvInputPortId(nodeId_, 1) && value.type == VirtualPortType::kCv)
    {
        const float normalized = Clamp01(value.scalar);
        if(HasMeaningfulChange(
               normalized, cvInputs_[0], cvInputInitialized_[0]))
        {
            cvInputs_[0]           = normalized;
            cvInputInitialized_[0] = true;
            ApplyParameterValue(ParameterIndex::kDelayTertiary,
                                normalized,
                                ParameterSource::kCv);
        }
        return;
    }

    if(portId == MakeMidiInputPortId(nodeId_, 1) && value.type == VirtualPortType::kMidi)
    {
        for(const auto& event : value.midiEvents)
        {
            PushTrackerLine(FormatMidiEvent(event));
        }
        SyncMenuModel();
        UpdateDisplay();
    }
}

PortValue MultiDelayCore::GetPortOutput(const std::string& portId) const
{
    const auto outputIt = portOutputs_.find(portId);
    if(outputIt != portOutputs_.end())
    {
        return outputIt->second;
    }

    const auto inputIt = portInputs_.find(portId);
    if(inputIt != portInputs_.end())
    {
        return inputIt->second;
    }

    return PortValue();
}

void MultiDelayCore::TickUi(double)
{
}

bool MultiDelayCore::SetParameterValue(const std::string& parameterId,
                                       float              normalizedValue)
{
    const auto* parameter = FindParameterById(parameterId);
    if(parameter == nullptr)
    {
        return false;
    }

    ApplyParameterValue(
        static_cast<ParameterIndex>(parameter - parameters_.data()),
        normalizedValue,
        ParameterSource::kMenu);
    return true;
}

bool MultiDelayCore::SetEffectiveParameterValue(const std::string& parameterId,
                                                float normalizedValue)
{
    auto* parameter = FindParameterById(parameterId);
    if(parameter == nullptr)
    {
        return false;
    }

    parameter->effectiveNormalizedValue = Clamp01(normalizedValue);
    UpdateMappedStateFromEffectiveParameters();
    UpdateDisplay();
    return true;
}

void MultiDelayCore::ClearEffectiveParameterOverrides()
{
    for(auto& parameter : parameters_)
    {
        parameter.effectiveNormalizedValue = parameter.normalizedValue;
    }
    UpdateMappedStateFromEffectiveParameters();
    UpdateDisplay();
}

ParameterValueLookup MultiDelayCore::GetControlValue(
    const std::string& controlId) const
{
    if(controlId == MakeKnobControlId(nodeId_, 1))
    {
        return {true, controlInputInitialized_[0] ? controlInputs_[0]
                                                  : parameters_[static_cast<std::size_t>(
                                                        ParameterIndex::kDryWet)]
                                                        .normalizedValue};
    }
    if(controlId == MakeKnobControlId(nodeId_, 2))
    {
        return {true, controlInputInitialized_[1] ? controlInputs_[1]
                                                  : parameters_[static_cast<std::size_t>(
                                                        ParameterIndex::kDelayPrimary)]
                                                        .normalizedValue};
    }
    if(controlId == MakeKnobControlId(nodeId_, 3))
    {
        return {true, controlInputInitialized_[2] ? controlInputs_[2]
                                                  : parameters_[static_cast<std::size_t>(
                                                        ParameterIndex::kDelaySecondary)]
                                                        .normalizedValue};
    }
    if(controlId == MakeKnobControlId(nodeId_, 4))
    {
        return {true, controlInputInitialized_[3] ? controlInputs_[3]
                                                  : parameters_[static_cast<std::size_t>(
                                                        ParameterIndex::kFeedback)]
                                                        .normalizedValue};
    }
    if(controlId == MakeDryWetControlId(nodeId_))
    {
        return {true, controlInputInitialized_[0] ? controlInputs_[0]
                                                  : parameters_[static_cast<std::size_t>(
                                                        ParameterIndex::kDryWet)]
                                                        .normalizedValue};
    }
    return {};
}

ParameterValueLookup MultiDelayCore::GetParameterValue(
    const std::string& parameterId) const
{
    const auto* parameter = FindParameterById(parameterId);
    if(parameter == nullptr)
    {
        return {};
    }

    return {true, parameter->normalizedValue};
}

ParameterValueLookup MultiDelayCore::GetEffectiveParameterValue(
    const std::string& parameterId) const
{
    const auto* parameter = FindParameterById(parameterId);
    if(parameter == nullptr)
    {
        return {};
    }

    return {true, parameter->effectiveNormalizedValue};
}

const std::vector<MetaControllerDescriptor>& MultiDelayCore::GetMetaControllers() const
{
    return metaControllers_;
}

bool MultiDelayCore::SetMetaControllerValue(const std::string& controllerId,
                                            float              normalizedValue)
{
    const float value = Clamp01(normalizedValue);

    if(controllerId == MakeMetaControllerId(nodeId_, "blend"))
    {
        ApplyParameterValue(
            ParameterIndex::kDryWet, value, ParameterSource::kMacro);
        return true;
    }

    if(controllerId == MakeMetaControllerId(nodeId_, "space"))
    {
        ApplyParameterValue(
            ParameterIndex::kDelayPrimary, value, ParameterSource::kMacro);
        ApplyParameterValue(ParameterIndex::kDelaySecondary,
                            0.05f + (0.90f * value),
                            ParameterSource::kMacro);
        ApplyParameterValue(ParameterIndex::kDelayTertiary,
                            0.10f + (0.80f * value),
                            ParameterSource::kMacro);
        return true;
    }

    if(controllerId == MakeMetaControllerId(nodeId_, "regen"))
    {
        ApplyParameterValue(ParameterIndex::kFeedback,
                            0.05f + (0.85f * value),
                            ParameterSource::kMacro);
        return true;
    }

    return false;
}

ParameterValueLookup MultiDelayCore::GetMetaControllerValue(
    const std::string& controllerId) const
{
    const auto* controller = FindMetaControllerById(controllerId);
    if(controller == nullptr)
    {
        return {};
    }

    return {true, controller->normalizedValue};
}

void MultiDelayCore::ResetToDefaultState(std::uint32_t seed)
{
    randomSeed_ = seed;
    controlInputs_.fill(0.0f);
    controlInputInitialized_.fill(false);
    cvInputs_.fill(0.0f);
    cvInputInitialized_.fill(false);
    parameterTouchSerials_.fill(0);
    parameterSources_.fill(ParameterSource::kNone);
    nextTouchSerial_         = 0;
    encoderPressed_          = false;
    inputSourceNormalized_   = 0.0f;
    inputLevelNormalized_    = 1.0f;
    computerKeysNormalized_  = 1.0f;
    midiOctaveNormalized_    = 0.5f;
    trackerLines_.fill("");
    menu_.isOpen           = false;
    menu_.isEditing        = false;
    menu_.sectionStack.clear();
    menu_.currentSectionId = kRootSectionId;
    menu_.currentSelection = 0;
    menuSelections_[kRootSectionId]    = 0;
    menuSelections_[kParamsSectionId]  = 0;
    menuSelections_[kMacrosSectionId]  = 0;
    menuSelections_[kInputSectionId]   = 0;
    menuSelections_[kMidiSectionId]    = 0;
    menuSelections_[kTrackerSectionId] = 0;
    menuSelections_[kAboutSectionId]   = 0;

    for(auto& input : portInputs_)
    {
        input.second.scalar = 0.0f;
        input.second.gate   = false;
        input.second.midiEvents.clear();
    }

    for(auto& output : portOutputs_)
    {
        output.second.scalar = 0.0f;
        output.second.gate   = false;
        output.second.midiEvents.clear();
    }

    for(auto& parameter : parameters_)
    {
        parameter.normalizedValue = parameter.defaultNormalizedValue;
    }

    for(std::size_t i = 0; i < kDelayCount; ++i)
    {
        if(delayLineStorage_ != nullptr && i < delayLineCount_)
        {
            delayLineStorage_[i].Init();
            delays_[i].line = &delayLineStorage_[i];
        }
        delays_[i].currentDelay = 0.0f;
        delays_[i].targetDelay  = 0.0f;
    }

    UpdateMappedStateFromParameters();
    SyncMenuModel();
    UpdateDisplay();
}

std::unordered_map<std::string, float>
MultiDelayCore::CaptureStatefulParameterValues() const
{
    std::unordered_map<std::string, float> values;
    for(const auto& parameter : parameters_)
    {
        if(parameter.stateful)
        {
            values[parameter.id] = parameter.normalizedValue;
        }
    }
    return values;
}

void MultiDelayCore::RestoreStatefulParameterValues(
    const std::unordered_map<std::string, float>& values)
{
    for(const auto& parameter : parameters_)
    {
        if(!parameter.stateful)
        {
            continue;
        }

        const auto it = values.find(parameter.id);
        if(it != values.end())
        {
            SetParameterValue(parameter.id, it->second);
        }
    }
}

const std::vector<ParameterDescriptor>& MultiDelayCore::GetParameters() const
{
    return parameters_;
}

const MenuModel& MultiDelayCore::GetMenuModel() const
{
    return menu_;
}

void MultiDelayCore::MenuRotate(int delta)
{
    if(delta == 0 || !menu_.isOpen)
    {
        return;
    }

    if(menu_.isEditing)
    {
        const MenuItem* item = GetSelectedMenuItem();
        if(item == nullptr || item->actionKind != MenuItemActionKind::kValue)
        {
            return;
        }

        const float nextValue = GetCurrentMenuItemValue(item->id)
                                + (static_cast<float>(delta) * GetMenuStepSize(item->id));
        SetMenuItemValue(item->id, nextValue);
        return;
    }

    MoveSelection(delta);
    SyncMenuModel();
    UpdateDisplay();
}

void MultiDelayCore::MenuPress()
{
    if(menu_.isEditing)
    {
        menu_.isEditing = false;
        SyncMenuModel();
        UpdateDisplay();
        return;
    }

    if(!menu_.isOpen)
    {
        menu_.isOpen = true;
        menu_.sectionStack.clear();
        menu_.sectionStack.push_back(kRootSectionId);
        SetCurrentSection(kRootSectionId);
        SyncMenuModel();
        UpdateDisplay();
        return;
    }

    const MenuItem* item = GetSelectedMenuItem();
    if(item == nullptr)
    {
        return;
    }

    switch(item->actionKind)
    {
        case MenuItemActionKind::kEnterSection:
            menu_.sectionStack.push_back(item->targetSectionId);
            SetCurrentSection(item->targetSectionId);
            break;
        case MenuItemActionKind::kBack:
            if(menu_.currentSectionId == kRootSectionId)
            {
                menu_.isOpen = false;
                menu_.sectionStack.clear();
                SetCurrentSection(kRootSectionId);
            }
            else
            {
                if(!menu_.sectionStack.empty())
                {
                    menu_.sectionStack.pop_back();
                }

                const std::string nextSection
                    = menu_.sectionStack.empty() ? kRootSectionId : menu_.sectionStack.back();
                SetCurrentSection(nextSection);
            }
            break;
        case MenuItemActionKind::kValue:
            menu_.isEditing = true;
            break;
        case MenuItemActionKind::kMomentary:
            SetMenuItemValue(item->id, 1.0f);
            break;
        case MenuItemActionKind::kReadonly:
            break;
    }

    SyncMenuModel();
    UpdateDisplay();
}

void MultiDelayCore::SetMenuItemValue(const std::string& itemId, float normalizedValue)
{
    const float value = Clamp01(normalizedValue);

    if(itemId == MakeMenuItemId(nodeId_, "macros", "blend"))
    {
        SetMetaControllerValue(MakeMetaControllerId(nodeId_, "blend"), value);
        return;
    }

    if(itemId == MakeMenuItemId(nodeId_, "macros", "space"))
    {
        SetMetaControllerValue(MakeMetaControllerId(nodeId_, "space"), value);
        return;
    }

    if(itemId == MakeMenuItemId(nodeId_, "macros", "regen"))
    {
        SetMetaControllerValue(MakeMetaControllerId(nodeId_, "regen"), value);
        return;
    }

    if(itemId == MakeMenuItemId(nodeId_, "params", "dry_wet"))
    {
        ApplyParameterValue(
            ParameterIndex::kDryWet, value, ParameterSource::kMenu);
        return;
    }

    if(itemId == MakeMenuItemId(nodeId_, "params", "delay_primary"))
    {
        ApplyParameterValue(
            ParameterIndex::kDelayPrimary, value, ParameterSource::kMenu);
        return;
    }

    if(itemId == MakeMenuItemId(nodeId_, "params", "delay_secondary"))
    {
        ApplyParameterValue(
            ParameterIndex::kDelaySecondary, value, ParameterSource::kMenu);
        return;
    }

    if(itemId == MakeMenuItemId(nodeId_, "params", "feedback"))
    {
        ApplyParameterValue(
            ParameterIndex::kFeedback, value, ParameterSource::kMenu);
        return;
    }

    if(itemId == MakeMenuItemId(nodeId_, "params", "delay_tertiary"))
    {
        ApplyParameterValue(
            ParameterIndex::kDelayTertiary, value, ParameterSource::kMenu);
        return;
    }

    if(itemId == MakeMenuItemId(nodeId_, "input", "source"))
    {
        inputSourceNormalized_ = value;
    }
    else if(itemId == MakeMenuItemId(nodeId_, "input", "level"))
    {
        inputLevelNormalized_ = value;
    }
    else if(itemId == MakeMenuItemId(nodeId_, "input", "fire_impulse"))
    {
        PortValue gate;
        gate.type = VirtualPortType::kGate;
        gate.gate = value >= 0.5f;
        portOutputs_[MakeGateOutputPortId(nodeId_, 1)] = gate;
        if(gate.gate)
        {
            PushTrackerLine("Impulse fired");
        }
    }
    else if(itemId == MakeMenuItemId(nodeId_, "midi", "computer_keys"))
    {
        computerKeysNormalized_ = value >= 0.5f ? 1.0f : 0.0f;
    }
    else if(itemId == MakeMenuItemId(nodeId_, "midi", "octave"))
    {
        midiOctaveNormalized_ = value;
    }

    SyncMenuModel();
    UpdateDisplay();
}

const DisplayModel& MultiDelayCore::GetDisplayModel() const
{
    return display_;
}

int MultiDelayCore::GetDryWetPercent() const
{
    return dryWetPercent_;
}

float MultiDelayCore::GetFeedback() const
{
    return feedback_;
}

float MultiDelayCore::GetDelayTargetSamples(std::size_t index) const
{
    if(index >= kDelayCount)
    {
        return 0.0f;
    }
    return delays_[index].targetDelay;
}

std::string MultiDelayCore::MakeKnobControlId(const std::string& nodeId,
                                              std::size_t       oneBasedIndex)
{
    return MakeIndexedId(nodeId, "control", "ctrl_", oneBasedIndex);
}

std::string MultiDelayCore::MakeEncoderControlId(const std::string& nodeId)
{
    return nodeId + "/control/encoder";
}

std::string MultiDelayCore::MakeEncoderButtonControlId(const std::string& nodeId)
{
    return nodeId + "/control/encoder_button";
}

std::string MultiDelayCore::MakeDryWetControlId(const std::string& nodeId)
{
    return nodeId + "/control/drywet";
}

std::string MultiDelayCore::MakeAudioInputPortId(const std::string& nodeId,
                                                 std::size_t       oneBasedIndex)
{
    return MakeIndexedId(nodeId, "port", "audio_in_", oneBasedIndex);
}

std::string MultiDelayCore::MakeAudioOutputPortId(const std::string& nodeId,
                                                  std::size_t       oneBasedIndex)
{
    return MakeIndexedId(nodeId, "port", "audio_out_", oneBasedIndex);
}

std::string MultiDelayCore::MakeCvInputPortId(const std::string& nodeId,
                                              std::size_t       oneBasedIndex)
{
    return MakeIndexedId(nodeId, "port", "cv_in_", oneBasedIndex);
}

std::string MultiDelayCore::MakeGateInputPortId(const std::string& nodeId,
                                                std::size_t       oneBasedIndex)
{
    return MakeIndexedId(nodeId, "port", "gate_in_", oneBasedIndex);
}

std::string MultiDelayCore::MakeGateOutputPortId(const std::string& nodeId,
                                                 std::size_t       oneBasedIndex)
{
    return MakeIndexedId(nodeId, "port", "gate_out_", oneBasedIndex);
}

std::string MultiDelayCore::MakeMidiInputPortId(const std::string& nodeId,
                                                std::size_t       oneBasedIndex)
{
    return MakeIndexedId(nodeId, "port", "midi_in_", oneBasedIndex);
}

std::string MultiDelayCore::MakeMidiOutputPortId(const std::string& nodeId,
                                                 std::size_t       oneBasedIndex)
{
    return MakeIndexedId(nodeId, "port", "midi_out_", oneBasedIndex);
}

float MultiDelayCore::Clamp01(float value) const
{
    if(value < 0.0f)
    {
        return 0.0f;
    }
    if(value > 1.0f)
    {
        return 1.0f;
    }
    return value;
}

float MultiDelayCore::MapLogControl(float normalized, float min, float max) const
{
    const float clamped = Clamp01(normalized);
    const float safeMin = min < 0.0000001f ? 0.0000001f : min;
    const float lmin    = std::log(safeMin);
    const float lmax    = std::log(max);
    return std::exp((clamped * (lmax - lmin)) + lmin);
}

bool MultiDelayCore::HasMeaningfulChange(float value,
                                         float previous,
                                         bool  initialized) const
{
    return !initialized || std::fabs(value - previous) > kTouchEpsilon;
}

ParameterDescriptor* MultiDelayCore::FindParameterById(
    const std::string& parameterId)
{
    const auto self = const_cast<const MultiDelayCore*>(this);
    return const_cast<ParameterDescriptor*>(self->FindParameterById(parameterId));
}

const ParameterDescriptor* MultiDelayCore::FindParameterById(
    const std::string& parameterId) const
{
    const auto it = std::find_if(parameters_.begin(),
                                 parameters_.end(),
                                 [&parameterId](const ParameterDescriptor& parameter) {
                                     return parameter.id == parameterId;
                                 });
    return it != parameters_.end() ? &(*it) : nullptr;
}

MetaControllerDescriptor* MultiDelayCore::FindMetaControllerById(
    const std::string& controllerId)
{
    const auto self = const_cast<const MultiDelayCore*>(this);
    return const_cast<MetaControllerDescriptor*>(
        self->FindMetaControllerById(controllerId));
}

const MetaControllerDescriptor* MultiDelayCore::FindMetaControllerById(
    const std::string& controllerId) const
{
    const auto it = std::find_if(metaControllers_.begin(),
                                 metaControllers_.end(),
                                 [&controllerId](const MetaControllerDescriptor& controller) {
                                     return controller.id == controllerId;
                                 });
    return it != metaControllers_.end() ? &(*it) : nullptr;
}

void MultiDelayCore::ApplyParameterValue(ParameterIndex index,
                                         float          normalizedValue,
                                         ParameterSource source)
{
    ParameterDescriptor& parameter = parameters_[static_cast<std::size_t>(index)];
    const float          clamped   = Clamp01(normalizedValue);

    if(!HasMeaningfulChange(clamped, parameter.normalizedValue, true))
    {
        return;
    }

    parameter.normalizedValue = clamped;
    parameterTouchSerials_[static_cast<std::size_t>(index)] = ++nextTouchSerial_;
    parameterSources_[static_cast<std::size_t>(index)]      = source;

    UpdateMappedStateFromParameters();
    SyncMenuModel();
    UpdateDisplay();
}

void MultiDelayCore::UpdateMappedStateFromParameters()
{
    for(auto& parameter : parameters_)
    {
        parameter.effectiveNormalizedValue = parameter.normalizedValue;
    }

    UpdateMappedStateFromEffectiveParameters();
    UpdateMetaControllersFromParameters();
}

void MultiDelayCore::UpdateMappedStateFromEffectiveParameters()
{
    delays_[0].targetDelay = MapLogControl(
        parameters_[static_cast<std::size_t>(ParameterIndex::kDelayPrimary)]
            .effectiveNormalizedValue,
        static_cast<float>(sampleRate_ * 0.05),
        static_cast<float>(kMaxDelaySamples));
    delays_[1].targetDelay = MapLogControl(
        parameters_[static_cast<std::size_t>(ParameterIndex::kDelaySecondary)]
            .effectiveNormalizedValue,
        static_cast<float>(sampleRate_ * 0.05),
        static_cast<float>(kMaxDelaySamples));
    delays_[2].targetDelay = MapLogControl(
        parameters_[static_cast<std::size_t>(ParameterIndex::kDelayTertiary)]
            .effectiveNormalizedValue,
        static_cast<float>(sampleRate_ * 0.05),
        static_cast<float>(kMaxDelaySamples));

    feedback_ = parameters_[static_cast<std::size_t>(ParameterIndex::kFeedback)]
                    .effectiveNormalizedValue;
    dryWetPercent_ = static_cast<int>(std::round(
        parameters_[static_cast<std::size_t>(ParameterIndex::kDryWet)]
            .effectiveNormalizedValue
        * 100.0f));
}

void MultiDelayCore::UpdateMetaControllersFromParameters()
{
    for(std::size_t index = 0; index < metaControllers_.size(); ++index)
    {
        metaControllers_[index].normalizedValue = DeriveMetaControllerValue(
            static_cast<MetaControllerIndex>(index));
    }
}

float MultiDelayCore::DeriveMetaControllerValue(MetaControllerIndex index) const
{
    switch(index)
    {
        case MetaControllerIndex::kBlend:
            return parameters_[static_cast<std::size_t>(ParameterIndex::kDryWet)]
                .normalizedValue;
        case MetaControllerIndex::kSpace:
            return parameters_[static_cast<std::size_t>(
                                   ParameterIndex::kDelayPrimary)]
                .normalizedValue;
        case MetaControllerIndex::kRegen:
            return Clamp01(
                (parameters_[static_cast<std::size_t>(ParameterIndex::kFeedback)]
                     .normalizedValue
                 - 0.05f)
                / 0.85f);
        case MetaControllerIndex::kCount: break;
    }

    return 0.0f;
}

void MultiDelayCore::SyncMenuModel()
{
    menu_.sections.clear();

    auto addSection = [this](const std::string& id, const std::string& title) -> MenuSection& {
        menu_.sections.push_back(MenuSection{id, title, {}, menuSelections_[id]});
        return menu_.sections.back();
    };

    MenuSection& root = addSection(kRootSectionId, "Menu");
    root.items.push_back({MakeMenuItemId(nodeId_, "root", "params"),
                          "Params",
                          false,
                          MenuItemActionKind::kEnterSection,
                          0.0f,
                          "",
                          kParamsSectionId});
    root.items.push_back({MakeMenuItemId(nodeId_, "root", "macros"),
                          "Macros",
                          false,
                          MenuItemActionKind::kEnterSection,
                          0.0f,
                          "",
                          kMacrosSectionId});
    root.items.push_back({MakeMenuItemId(nodeId_, "root", "input"),
                          "Input",
                          false,
                          MenuItemActionKind::kEnterSection,
                          0.0f,
                          "",
                          kInputSectionId});
    root.items.push_back({MakeMenuItemId(nodeId_, "root", "midi"),
                          "MIDI",
                          false,
                          MenuItemActionKind::kEnterSection,
                          0.0f,
                          "",
                          kMidiSectionId});
    root.items.push_back({MakeMenuItemId(nodeId_, "root", "tracker"),
                          "Tracker",
                          false,
                          MenuItemActionKind::kEnterSection,
                          0.0f,
                          "",
                          kTrackerSectionId});
    root.items.push_back({MakeMenuItemId(nodeId_, "root", "about"),
                          "About",
                          false,
                          MenuItemActionKind::kEnterSection,
                          0.0f,
                          "",
                          kAboutSectionId});

    MenuSection& params = addSection(kParamsSectionId, "Params");
    params.items.push_back({MakeMenuItemId(nodeId_, "params", "back"),
                            "Back",
                            false,
                            MenuItemActionKind::kBack});
    params.items.push_back({MakeMenuItemId(nodeId_, "params", "dry_wet"),
                            "Dry/Wet",
                            true,
                            MenuItemActionKind::kValue,
                            parameters_[0].normalizedValue,
                            FormatPercent(parameters_[0].normalizedValue)});
    params.items.push_back({MakeMenuItemId(nodeId_, "params", "delay_primary"),
                            "Delay 1",
                            true,
                            MenuItemActionKind::kValue,
                            parameters_[1].normalizedValue,
                            FormatDelaySamples(parameters_[1].normalizedValue)});
    params.items.push_back({MakeMenuItemId(nodeId_, "params", "delay_secondary"),
                            "Delay 2",
                            true,
                            MenuItemActionKind::kValue,
                            parameters_[2].normalizedValue,
                            FormatDelaySamples(parameters_[2].normalizedValue)});
    params.items.push_back({MakeMenuItemId(nodeId_, "params", "feedback"),
                            "Feedback",
                            true,
                            MenuItemActionKind::kValue,
                            parameters_[3].normalizedValue,
                            FormatFeedback(parameters_[3].normalizedValue)});
    params.items.push_back({MakeMenuItemId(nodeId_, "params", "delay_tertiary"),
                            "Delay 3",
                            true,
                            MenuItemActionKind::kValue,
                            parameters_[4].normalizedValue,
                            FormatDelaySamples(parameters_[4].normalizedValue)});

    MenuSection& macros = addSection(kMacrosSectionId, "Macros");
    macros.items.push_back({MakeMenuItemId(nodeId_, "macros", "back"),
                            "Back",
                            false,
                            MenuItemActionKind::kBack});
    macros.items.push_back({MakeMenuItemId(nodeId_, "macros", "blend"),
                            "Blend",
                            true,
                            MenuItemActionKind::kValue,
                            metaControllers_[0].normalizedValue,
                            FormatPercent(metaControllers_[0].normalizedValue)});
    macros.items.push_back({MakeMenuItemId(nodeId_, "macros", "space"),
                            "Space",
                            true,
                            MenuItemActionKind::kValue,
                            metaControllers_[1].normalizedValue,
                            FormatPercent(metaControllers_[1].normalizedValue)});
    macros.items.push_back({MakeMenuItemId(nodeId_, "macros", "regen"),
                            "Regen",
                            true,
                            MenuItemActionKind::kValue,
                            metaControllers_[2].normalizedValue,
                            FormatPercent(metaControllers_[2].normalizedValue)});

    MenuSection& input = addSection(kInputSectionId, "Input");
    input.items.push_back({MakeMenuItemId(nodeId_, "input", "back"),
                           "Back",
                           false,
                           MenuItemActionKind::kBack});
    input.items.push_back({MakeMenuItemId(nodeId_, "input", "source"),
                           "Source",
                           true,
                           MenuItemActionKind::kValue,
                           inputSourceNormalized_,
                           FormatInputSource(inputSourceNormalized_)});
    input.items.push_back({MakeMenuItemId(nodeId_, "input", "level"),
                           "Level",
                           true,
                           MenuItemActionKind::kValue,
                           inputLevelNormalized_,
                           FormatPercent(inputLevelNormalized_)});
    input.items.push_back({MakeMenuItemId(nodeId_, "input", "fire_impulse"),
                           "Fire Impulse",
                           false,
                           MenuItemActionKind::kMomentary,
                           0.0f,
                           "Press"});

    MenuSection& midi = addSection(kMidiSectionId, "MIDI");
    midi.items.push_back({MakeMenuItemId(nodeId_, "midi", "back"),
                          "Back",
                          false,
                          MenuItemActionKind::kBack});
    midi.items.push_back({MakeMenuItemId(nodeId_, "midi", "computer_keys"),
                          "Computer Keys",
                          true,
                          MenuItemActionKind::kValue,
                          computerKeysNormalized_,
                          computerKeysNormalized_ >= 0.5f ? "On" : "Off"});
    midi.items.push_back({MakeMenuItemId(nodeId_, "midi", "octave"),
                          "Octave",
                          true,
                          MenuItemActionKind::kValue,
                          midiOctaveNormalized_,
                          FormatOctave(midiOctaveNormalized_)});
    midi.items.push_back({MakeMenuItemId(nodeId_, "midi", "input_status"),
                          "Input Status",
                          false,
                          MenuItemActionKind::kReadonly,
                          0.0f,
                          FormatMidiStatus()});

    MenuSection& tracker = addSection(kTrackerSectionId, "Tracker");
    tracker.items.push_back({MakeMenuItemId(nodeId_, "tracker", "back"),
                             "Back",
                             false,
                             MenuItemActionKind::kBack});
    for(std::size_t i = 0; i < trackerLines_.size(); ++i)
    {
        tracker.items.push_back({MakeMenuItemId(nodeId_,
                                                "tracker",
                                                "line_" + std::to_string(i + 1)),
                                 "Event " + std::to_string(i + 1),
                                 false,
                                 MenuItemActionKind::kReadonly,
                                 0.0f,
                                 FormatTrackerLine(i)});
    }

    MenuSection& about = addSection(kAboutSectionId, "About");
    about.items.push_back({MakeMenuItemId(nodeId_, "about", "back"),
                           "Back",
                           false,
                           MenuItemActionKind::kBack});
    for(std::size_t i = 0; i < aboutLines_.size(); ++i)
    {
        about.items.push_back({MakeMenuItemId(nodeId_,
                                              "about",
                                              "line_" + std::to_string(i + 1)),
                               i == 0 ? "Info" : ("Info " + std::to_string(i + 1)),
                               false,
                               MenuItemActionKind::kReadonly,
                               0.0f,
                               FormatAboutLine(i)});
    }

    for(auto& section : menu_.sections)
    {
        if(section.items.empty())
        {
            section.selectedIndex = 0;
            continue;
        }

        const int maxIndex = static_cast<int>(section.items.size()) - 1;
        section.selectedIndex = ClampInt(menuSelections_[section.id], 0, maxIndex);
        menuSelections_[section.id] = section.selectedIndex;
    }

    auto currentIt
        = std::find_if(menu_.sections.begin(),
                       menu_.sections.end(),
                       [this](const MenuSection& section) {
                           return section.id == menu_.currentSectionId;
                       });
    if(currentIt == menu_.sections.end())
    {
        SetCurrentSection(kRootSectionId);
        currentIt = std::find_if(menu_.sections.begin(),
                                 menu_.sections.end(),
                                 [this](const MenuSection& section) {
                                     return section.id == menu_.currentSectionId;
                                 });
    }

    menu_.currentSelection
        = currentIt != menu_.sections.end() ? currentIt->selectedIndex : 0;
}

void MultiDelayCore::UpdateDisplay()
{
    ++display_.revision;
    display_.texts.clear();
    display_.bars.clear();

    if(!menu_.isOpen)
    {
        display_.mode  = DisplayMode::kStatus;
        display_.title = "Multi Delay";
        display_.texts.push_back({0, 0, "Multi Delay", false});
        display_.texts.push_back(
            {0, 14, "Mix " + FormatPercent(parameters_[0].normalizedValue), false});
        display_.texts.push_back(
            {0, 26, "D1 " + FormatDelaySamples(parameters_[1].normalizedValue), false});
        display_.texts.push_back(
            {0, 38, "D2 " + FormatDelaySamples(parameters_[2].normalizedValue), false});
        display_.texts.push_back(
            {0, 50, "Fb " + FormatFeedback(parameters_[3].normalizedValue)
                        + " D3 "
                        + FormatDelaySamples(parameters_[4].normalizedValue),
             false});

        for(std::size_t i = 0; i < parameters_.size(); ++i)
        {
            display_.bars.push_back({static_cast<int>(i * 25),
                                     56,
                                     20,
                                     6,
                                     parameters_[i].normalizedValue});
        }
        return;
    }

    display_.mode = DisplayMode::kMenu;

    const auto sectionIt
        = std::find_if(menu_.sections.begin(),
                       menu_.sections.end(),
                       [this](const MenuSection& section) {
                           return section.id == menu_.currentSectionId;
                       });
    if(sectionIt == menu_.sections.end())
    {
        display_.title = "Menu";
        display_.texts.push_back({0, 0, "Menu", false});
        return;
    }

    display_.title = sectionIt->title;
    display_.texts.push_back({0, 0, sectionIt->title, false});

    const int itemCount = static_cast<int>(sectionIt->items.size());
    const int maxStart
        = std::max(0, itemCount - static_cast<int>(kVisibleMenuRows));
    const int startIndex = ClampInt(menu_.currentSelection - 1, 0, maxStart);
    const int endIndex   = std::min(itemCount, startIndex + static_cast<int>(kVisibleMenuRows));

    for(int itemIndex = startIndex; itemIndex < endIndex; ++itemIndex)
    {
        const MenuItem& item = sectionIt->items[static_cast<std::size_t>(itemIndex)];
        const bool      selected = itemIndex == menu_.currentSelection;

        std::string line = selected ? (menu_.isEditing ? "* " : "> ") : "  ";
        line += item.label;
        if(!item.valueText.empty())
        {
            line += ": " + item.valueText;
        }

        display_.texts.push_back({0,
                                  14 + ((itemIndex - startIndex) * 12),
                                  line,
                                  false});
    }

    const MenuItem* selectedItem = GetSelectedMenuItem();
    if(selectedItem != nullptr
       && selectedItem->actionKind == MenuItemActionKind::kValue)
    {
        display_.bars.push_back({0, 58, 128, 6, selectedItem->normalizedValue});
    }
}

std::string MultiDelayCore::FormatPercent(float normalized) const
{
    return FormatString("%d%%",
                        static_cast<int>(std::round(Clamp01(normalized) * 100.0f)));
}

std::string MultiDelayCore::FormatDelaySamples(float normalized) const
{
    return FormatString("%d",
                        static_cast<int>(std::round(MapLogControl(
                            normalized,
                            static_cast<float>(sampleRate_ * 0.05),
                            static_cast<float>(kMaxDelaySamples)))));
}

std::string MultiDelayCore::FormatFeedback(float normalized) const
{
    return FormatPercent(normalized);
}

std::string MultiDelayCore::FormatInputSource(float normalized) const
{
    static const std::array<const char*, 5> kInputSources
        = {{"Host In", "Sine", "Saw", "Noise", "Impulse"}};
    const std::size_t index = static_cast<std::size_t>(
        std::round(Clamp01(normalized) * static_cast<float>(kInputSources.size() - 1)));
    return kInputSources[index];
}

std::string MultiDelayCore::FormatOctave(float normalized) const
{
    static const std::array<const char*, 5> kOctaves = {{"C1", "C2", "C3", "C4", "C5"}};
    const std::size_t index = static_cast<std::size_t>(
        std::round(Clamp01(normalized) * static_cast<float>(kOctaves.size() - 1)));
    return kOctaves[index];
}

std::string MultiDelayCore::FormatMidiStatus() const
{
    return trackerLines_[0].empty() ? "Waiting" : trackerLines_[0];
}

std::string MultiDelayCore::FormatTrackerLine(std::size_t index) const
{
    if(index >= trackerLines_.size() || trackerLines_[index].empty())
    {
        return index == 0 ? "No MIDI events" : "-";
    }
    return trackerLines_[index];
}

std::string MultiDelayCore::FormatAboutLine(std::size_t index) const
{
    if(index >= aboutLines_.size())
    {
        return "-";
    }
    return aboutLines_[index];
}

float MultiDelayCore::GetMenuStepSize(const std::string& itemId) const
{
    if(itemId == MakeMenuItemId(nodeId_, "input", "source")
       || itemId == MakeMenuItemId(nodeId_, "midi", "octave"))
    {
        return 0.25f;
    }

    if(itemId == MakeMenuItemId(nodeId_, "midi", "computer_keys"))
    {
        return 1.0f;
    }

    return 0.05f;
}

float MultiDelayCore::GetCurrentMenuItemValue(const std::string& itemId) const
{
    if(itemId == MakeMenuItemId(nodeId_, "macros", "blend"))
    {
        return metaControllers_[static_cast<std::size_t>(
            MetaControllerIndex::kBlend)]
            .normalizedValue;
    }
    if(itemId == MakeMenuItemId(nodeId_, "macros", "space"))
    {
        return metaControllers_[static_cast<std::size_t>(
            MetaControllerIndex::kSpace)]
            .normalizedValue;
    }
    if(itemId == MakeMenuItemId(nodeId_, "macros", "regen"))
    {
        return metaControllers_[static_cast<std::size_t>(
            MetaControllerIndex::kRegen)]
            .normalizedValue;
    }
    if(itemId == MakeMenuItemId(nodeId_, "params", "dry_wet"))
    {
        return parameters_[0].normalizedValue;
    }
    if(itemId == MakeMenuItemId(nodeId_, "params", "delay_primary"))
    {
        return parameters_[1].normalizedValue;
    }
    if(itemId == MakeMenuItemId(nodeId_, "params", "delay_secondary"))
    {
        return parameters_[2].normalizedValue;
    }
    if(itemId == MakeMenuItemId(nodeId_, "params", "feedback"))
    {
        return parameters_[3].normalizedValue;
    }
    if(itemId == MakeMenuItemId(nodeId_, "params", "delay_tertiary"))
    {
        return parameters_[4].normalizedValue;
    }
    if(itemId == MakeMenuItemId(nodeId_, "input", "source"))
    {
        return inputSourceNormalized_;
    }
    if(itemId == MakeMenuItemId(nodeId_, "input", "level"))
    {
        return inputLevelNormalized_;
    }
    if(itemId == MakeMenuItemId(nodeId_, "midi", "computer_keys"))
    {
        return computerKeysNormalized_;
    }
    if(itemId == MakeMenuItemId(nodeId_, "midi", "octave"))
    {
        return midiOctaveNormalized_;
    }
    return 0.0f;
}

void MultiDelayCore::SetCurrentSection(const std::string& sectionId)
{
    menu_.currentSectionId = sectionId;
    menu_.currentSelection = menuSelections_[sectionId];
    menu_.isEditing        = false;
}

void MultiDelayCore::MoveSelection(int delta)
{
    auto sectionIt
        = std::find_if(menu_.sections.begin(),
                       menu_.sections.end(),
                       [this](const MenuSection& section) {
                           return section.id == menu_.currentSectionId;
                       });
    if(sectionIt == menu_.sections.end() || sectionIt->items.empty())
    {
        return;
    }

    const int itemCount = static_cast<int>(sectionIt->items.size());
    int       next      = menuSelections_[menu_.currentSectionId] + delta;

    next %= itemCount;
    if(next < 0)
    {
        next += itemCount;
    }

    menuSelections_[menu_.currentSectionId] = next;
    menu_.currentSelection                  = next;
}

const MenuItem* MultiDelayCore::GetSelectedMenuItem() const
{
    const auto sectionIt
        = std::find_if(menu_.sections.begin(),
                       menu_.sections.end(),
                       [this](const MenuSection& section) {
                           return section.id == menu_.currentSectionId;
                       });
    if(sectionIt == menu_.sections.end() || sectionIt->items.empty())
    {
        return nullptr;
    }

    const int index = ClampInt(menu_.currentSelection,
                               0,
                               static_cast<int>(sectionIt->items.size()) - 1);
    return &sectionIt->items[static_cast<std::size_t>(index)];
}

MenuItem* MultiDelayCore::GetSelectedMenuItem()
{
    const auto self = const_cast<const MultiDelayCore*>(this);
    return const_cast<MenuItem*>(self->GetSelectedMenuItem());
}

void MultiDelayCore::PushTrackerLine(const std::string& line)
{
    if(line.empty())
    {
        return;
    }

    for(std::size_t i = trackerLines_.size() - 1; i > 0; --i)
    {
        trackerLines_[i] = trackerLines_[i - 1];
    }
    trackerLines_[0] = line;
}
} // namespace apps
} // namespace daisyhost
