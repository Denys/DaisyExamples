#include "daisyhost/apps/DelayFxAdaptationCore.h"

#include <algorithm>
#include <cmath>

namespace daisyhost
{
namespace apps
{
namespace
{
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

ParameterRole RoleForParameterId(const std::string& id)
{
    if(id == "mix")
    {
        return ParameterRole::kMix;
    }
    if(id == "time")
    {
        return ParameterRole::kPrimaryDelay;
    }
    if(id == "pre_delay")
    {
        return ParameterRole::kTertiaryDelay;
    }
    if(id == "feedback")
    {
        return ParameterRole::kFeedback;
    }
    return ParameterRole::kGeneric;
}

const char* LayerSectionName(int layer)
{
    switch(layer)
    {
        case 1: return "sw1";
        case 2: return "sw2";
        default: return "base";
    }
}

const char* LayerTitle(int layer)
{
    switch(layer)
    {
        case 1: return "SW1 Layer";
        case 2: return "SW2 Layer";
        default: return "Base Layer";
    }
}

float FieldButtonLedValue(int state)
{
    return state == 0 ? 0.0f : (state == 1 ? 0.32f : 0.85f);
}
} // namespace

DelayFxAdaptationCore::DelayFxAdaptationCore(DaisyDelayFxSource source,
                                             const std::string& nodeId,
                                             bool bundleMode)
: source_(source), nodeId_(nodeId), sharedCore_(source), bundleMode_(bundleMode)
{
    portInputs_[MakeAudioInputPortId(nodeId_, 1)].type = VirtualPortType::kAudio;
    portInputs_[MakeAudioInputPortId(nodeId_, 2)].type = VirtualPortType::kAudio;
    portInputs_[MakeMidiInputPortId(nodeId_)].type = VirtualPortType::kMidi;
    portInputs_[MakeGateInputPortId(nodeId_)].type = VirtualPortType::kGate;
    portOutputs_[MakeAudioOutputPortId(nodeId_, 1)].type = VirtualPortType::kAudio;
    portOutputs_[MakeAudioOutputPortId(nodeId_, 2)].type = VirtualPortType::kAudio;
    sharedCore_.SetBundleMode(bundleMode_);
    ResetMenuState();
    sharedCore_.ResetToDefaultState();
    RefreshSnapshots();
}

std::string DelayFxAdaptationCore::GetAppId() const
{
    if(bundleMode_)
    {
        return "field_delay_bundle";
    }
    return sharedCore_.GetProfile().appId;
}

std::string DelayFxAdaptationCore::GetAppDisplayName() const
{
    if(bundleMode_)
    {
        return "Field Delay Bundle";
    }
    return sharedCore_.GetProfile().displayName;
}

HostedAppCapabilities DelayFxAdaptationCore::GetCapabilities() const
{
    HostedAppCapabilities capabilities;
    capabilities.acceptsAudioInput = true;
    capabilities.acceptsMidiInput = true;
    capabilities.producesMidiOutput = false;
    return capabilities;
}

HostedAppPatchBindings DelayFxAdaptationCore::GetPatchBindings() const
{
    HostedAppPatchBindings bindings;
    for(std::size_t i = 0; i < 4; ++i)
    {
        const char* id = sharedCore_.GetParameterForLayerKnob(0, i);
        if(id[0] != '\0')
        {
            bindings.knobControlIds[i] = MakeControlId(nodeId_, id);
            bindings.knobParameterIds[i] = MakeParameterId(nodeId_, id);
            const auto* parameter = sharedCore_.FindParameter(id);
            bindings.knobDetailLabels[i]
                = parameter != nullptr ? parameter->label : id;
        }
    }

    for(std::size_t i = 0; i < 8; ++i)
    {
        const char* id = sharedCore_.GetParameterForLayerKnob(0, i);
        if(id[0] != '\0')
        {
            bindings.fieldKnobControlIds[i] = MakeControlId(nodeId_, id);
            bindings.fieldKnobParameterIds[i] = MakeParameterId(nodeId_, id);
            const auto* parameter = sharedCore_.FindParameter(id);
            bindings.fieldKnobDetailLabels[i]
                = parameter != nullptr ? parameter->label : id;
        }
    }

    for(std::size_t i = 0; i < bindings.fieldKeyMenuItemIds.size(); ++i)
    {
        const char row = i < 8 ? 'a' : 'b';
        const std::size_t key = (i % 8) + 1;
        bindings.fieldKeyMenuItemIds[i]
            = MakeMenuItemId(nodeId_,
                             "field_keys",
                             std::string(1, row) + std::to_string(key));
        bindings.fieldKeyDetailLabels[i]
            = i < 8 ? (std::string("A") + std::to_string(key))
                    : (std::string("B") + std::to_string(key));
    }

    bindings.encoderControlId = MakeControlId(nodeId_, "encoder");
    bindings.encoderButtonControlId = MakeControlId(nodeId_, "encoder_button");
    bindings.audioInputPortIds[0] = MakeAudioInputPortId(nodeId_, 1);
    bindings.audioInputPortIds[1] = MakeAudioInputPortId(nodeId_, 2);
    bindings.audioOutputPortIds[0] = MakeAudioOutputPortId(nodeId_, 1);
    bindings.audioOutputPortIds[1] = MakeAudioOutputPortId(nodeId_, 2);
    bindings.midiInputPortId = MakeMidiInputPortId(nodeId_);
    bindings.gateInputPortIds[0] = MakeGateInputPortId(nodeId_);
    bindings.mainOutputChannels = {0, 1};
    return bindings;
}

void DelayFxAdaptationCore::Prepare(double sampleRate, std::size_t maxBlockSize)
{
    delayStorage_.assign(DaisyDelayFxCore::kDelayLineCount
                             * DaisyDelayFxCore::kMaxDelaySamples,
                         0.0f);
    if(!delayStorage_.empty())
    {
        sharedCore_.AttachDelayStorage(delayStorage_.data(),
                                       DaisyDelayFxCore::kDelayLineCount,
                                       DaisyDelayFxCore::kMaxDelaySamples);
    }
    sharedCore_.Prepare(sampleRate, maxBlockSize);
    scratchLeft_.assign(maxBlockSize, 0.0f);
    scratchRight_.assign(maxBlockSize, 0.0f);
    RefreshSnapshots();
}

void DelayFxAdaptationCore::Process(const AudioBufferView& input,
                                    const AudioBufferWriteView& output,
                                    std::size_t frameCount)
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

    ConsumeMidiInput();
    if(scratchLeft_.size() < frameCount)
    {
        scratchLeft_.assign(frameCount, 0.0f);
        scratchRight_.assign(frameCount, 0.0f);
    }

    const float* inputLeft = (input.channelCount > 0 && input.channels[0] != nullptr)
                                 ? input.channels[0]
                                 : nullptr;
    const float* inputRight = (input.channelCount > 1 && input.channels[1] != nullptr)
                                  ? input.channels[1]
                                  : inputLeft;

    sharedCore_.Process(inputLeft,
                        inputRight,
                        scratchLeft_.data(),
                        scratchRight_.data(),
                        frameCount);

    if(output.channelCount > 0 && output.channels[0] != nullptr)
    {
        std::copy(scratchLeft_.begin(),
                  scratchLeft_.begin() + static_cast<std::ptrdiff_t>(frameCount),
                  output.channels[0]);
    }
    if(output.channelCount > 1 && output.channels[1] != nullptr)
    {
        std::copy(scratchRight_.begin(),
                  scratchRight_.begin() + static_cast<std::ptrdiff_t>(frameCount),
                  output.channels[1]);
    }

    PortValue leftOutput;
    leftOutput.type = VirtualPortType::kAudio;
    leftOutput.scalar = PeakForBuffer(scratchLeft_.data(), frameCount);
    portOutputs_[MakeAudioOutputPortId(nodeId_, 1)] = leftOutput;

    PortValue rightOutput;
    rightOutput.type = VirtualPortType::kAudio;
    rightOutput.scalar = PeakForBuffer(scratchRight_.data(), frameCount);
    portOutputs_[MakeAudioOutputPortId(nodeId_, 2)] = rightOutput;
}

void DelayFxAdaptationCore::SetControl(const std::string& controlId,
                                       float normalizedValue)
{
    const auto suffix = StripControlId(controlId);
    if(suffix == "encoder")
    {
        MenuRotate(normalizedValue >= 0.5f ? 1 : -1);
        return;
    }
    if(suffix == "encoder_button")
    {
        SetEncoderPress(normalizedValue >= 0.5f);
        return;
    }
    if(!suffix.empty())
    {
        sharedCore_.SetParameterValue(suffix, normalizedValue);
        RefreshSnapshots();
    }
}

void DelayFxAdaptationCore::SetEncoderDelta(int delta)
{
    MenuRotate(delta);
}

void DelayFxAdaptationCore::SetEncoderPress(bool pressed)
{
    if(pressed && !encoderPressed_)
    {
        MenuPress();
    }
    encoderPressed_ = pressed;
}

void DelayFxAdaptationCore::SetPortInput(const std::string& portId,
                                         const PortValue& value)
{
    portInputs_[portId] = value;
}

PortValue DelayFxAdaptationCore::GetPortOutput(const std::string& portId) const
{
    const auto it = portOutputs_.find(portId);
    return it != portOutputs_.end() ? it->second : PortValue{};
}

void DelayFxAdaptationCore::TickUi(double)
{
    BuildDisplay();
}

bool DelayFxAdaptationCore::SetParameterValue(const std::string& parameterId,
                                              float normalizedValue)
{
    const auto suffix = StripParameterId(parameterId);
    if(IsBundleAlgorithmParameter(suffix))
    {
        return SetBundleAlgorithmParameter(normalizedValue);
    }
    if(suffix.empty() || !sharedCore_.SetParameterValue(suffix, normalizedValue))
    {
        return false;
    }
    RefreshSnapshots();
    return true;
}

bool DelayFxAdaptationCore::SetEffectiveParameterValue(
    const std::string& parameterId, float normalizedValue)
{
    const auto suffix = StripParameterId(parameterId);
    if(IsBundleAlgorithmParameter(suffix))
    {
        return SetBundleAlgorithmParameter(normalizedValue);
    }
    if(suffix.empty()
       || !sharedCore_.SetEffectiveParameterValue(suffix, normalizedValue))
    {
        return false;
    }
    RefreshSnapshots();
    return true;
}

ParameterValueLookup DelayFxAdaptationCore::GetControlValue(
    const std::string& controlId) const
{
    ParameterValueLookup lookup;
    const auto suffix = StripControlId(controlId);
    float value = 0.0f;
    if(!suffix.empty() && sharedCore_.GetParameterValue(suffix, &value))
    {
        lookup.hasValue = true;
        lookup.value = value;
    }
    return lookup;
}

ParameterValueLookup DelayFxAdaptationCore::GetParameterValue(
    const std::string& parameterId) const
{
    ParameterValueLookup lookup;
    const auto suffix = StripParameterId(parameterId);
    if(IsBundleAlgorithmParameter(suffix))
    {
        lookup.hasValue = true;
        lookup.value = CurrentAlgorithmNormalized();
        return lookup;
    }
    float value = 0.0f;
    if(!suffix.empty() && sharedCore_.GetParameterValue(suffix, &value))
    {
        lookup.hasValue = true;
        lookup.value = value;
    }
    return lookup;
}

ParameterValueLookup DelayFxAdaptationCore::GetEffectiveParameterValue(
    const std::string& parameterId) const
{
    ParameterValueLookup lookup;
    const auto suffix = StripParameterId(parameterId);
    if(IsBundleAlgorithmParameter(suffix))
    {
        lookup.hasValue = true;
        lookup.value = CurrentAlgorithmNormalized();
        return lookup;
    }
    float value = 0.0f;
    if(!suffix.empty() && sharedCore_.GetEffectiveParameterValue(suffix, &value))
    {
        lookup.hasValue = true;
        lookup.value = value;
    }
    return lookup;
}

std::array<float, 16> DelayFxAdaptationCore::GetFieldKeyLedValues() const
{
    if(bundleMode_)
    {
        auto values = sharedCore_.GetFieldKeyLedValues();
        for(std::size_t i = 0; i < 4; ++i)
        {
            values[i] = i == CurrentAlgorithmIndex() ? 0.85f : 0.03f;
        }
        values[4] = FieldButtonLedValue(sharedCore_.GetInternalSynthMode());
        values[5] = FieldButtonLedValue(sharedCore_.GetInternalSynthHoldMode());
        return values;
    }
    return sharedCore_.GetFieldKeyLedValues();
}

void DelayFxAdaptationCore::ResetToDefaultState(std::uint32_t seed)
{
    algorithmSnapshots_ = {};
    algorithmSnapshotValid_ = {};
    sharedCore_.ResetToDefaultState(seed);
    ResetMenuState();
    RefreshSnapshots();
}

std::unordered_map<std::string, float>
DelayFxAdaptationCore::CaptureStatefulParameterValues() const
{
    std::unordered_map<std::string, float> values;
    if(bundleMode_)
    {
        auto snapshots = algorithmSnapshots_;
        auto valid = algorithmSnapshotValid_;
        const std::size_t activeIndex = CurrentAlgorithmIndex();
        snapshots[activeIndex] = sharedCore_.CaptureStatefulParameterValues();
        valid[activeIndex] = true;

        values.emplace(MakeParameterId(nodeId_, "algorithm"),
                       CurrentAlgorithmNormalized());
        for(std::size_t i = 0; i < snapshots.size(); ++i)
        {
            if(!valid[i])
            {
                continue;
            }
            for(const auto& pair : snapshots[i])
            {
                values.emplace(MakeParameterId(nodeId_,
                                               MakeBundleStateId(i, pair.first)),
                               pair.second);
            }
        }
        return values;
    }
    for(const auto& pair : sharedCore_.CaptureStatefulParameterValues())
    {
        values.emplace(MakeParameterId(nodeId_, pair.first), pair.second);
    }
    return values;
}

void DelayFxAdaptationCore::RestoreStatefulParameterValues(
    const std::unordered_map<std::string, float>& values)
{
    if(bundleMode_)
    {
        std::size_t requestedIndex = CurrentAlgorithmIndex();
        algorithmSnapshots_ = {};
        algorithmSnapshotValid_ = {};

        for(const auto& pair : values)
        {
            const auto suffix = StripParameterId(pair.first);
            if(IsBundleAlgorithmParameter(suffix))
            {
                requestedIndex = static_cast<std::size_t>(
                    std::round(Clamp01(pair.second) * 3.0f));
                continue;
            }

            const auto& descriptors = GetDaisyDelayFxAlgorithmDescriptors();
            for(std::size_t i = 0; i < descriptors.size(); ++i)
            {
                const std::string prefix = std::string(descriptors[i].statePrefix)
                                           + ".";
                if(suffix.rfind(prefix, 0) == 0)
                {
                    algorithmSnapshots_[i].emplace(suffix.substr(prefix.size()),
                                                   pair.second);
                    algorithmSnapshotValid_[i] = true;
                    break;
                }
            }
        }

        SelectAlgorithmIndex(requestedIndex, false);
        return;
    }

    std::unordered_map<std::string, float> stripped;
    for(const auto& pair : values)
    {
        const auto suffix = StripParameterId(pair.first);
        if(!suffix.empty())
        {
            stripped.emplace(suffix, pair.second);
        }
    }
    sharedCore_.RestoreStatefulParameterValues(stripped);
    RefreshSnapshots();
}

const std::vector<ParameterDescriptor>& DelayFxAdaptationCore::GetParameters()
    const
{
    return parameters_;
}

const MenuModel& DelayFxAdaptationCore::GetMenuModel() const
{
    return menu_;
}

void DelayFxAdaptationCore::MenuRotate(int delta)
{
    if(!menu_.isOpen)
    {
        menu_.isOpen = true;
        BuildDisplay();
        return;
    }

    auto sectionIt = std::find_if(menu_.sections.begin(),
                                  menu_.sections.end(),
                                  [this](const MenuSection& section) {
                                      return section.id == menu_.currentSectionId;
                                  });
    if(sectionIt == menu_.sections.end() || sectionIt->items.empty())
    {
        return;
    }

    if(menu_.isEditing)
    {
        auto& item = sectionIt->items[sectionIt->selectedIndex];
        if(item.actionKind == MenuItemActionKind::kValue)
        {
            SetMenuItemValue(item.id,
                             std::clamp(item.normalizedValue
                                            + static_cast<float>(delta) * 0.03f,
                                        0.0f,
                                        1.0f));
        }
        return;
    }

    MoveSelection(delta);
}

void DelayFxAdaptationCore::MenuPress()
{
    if(!menu_.isOpen)
    {
        menu_.isOpen = true;
        BuildDisplay();
        return;
    }
    PressSelectedItem();
}

void DelayFxAdaptationCore::SetMenuItemValue(const std::string& itemId,
                                             float normalizedValue)
{
    if(itemId == nodeId_ + "/menu/navigation/back")
    {
        MenuRotate(-1);
        return;
    }
    if(itemId == nodeId_ + "/menu/navigation/forward")
    {
        MenuRotate(1);
        return;
    }

    const auto fieldKey = StripMenuItemPrefix(itemId, "field_keys");
    if(!fieldKey.empty())
    {
        const char row = fieldKey[0];
        const int key = fieldKey.size() > 1 ? fieldKey[1] - '1' : -1;
        if((row == 'a' || row == 'b') && key >= 0 && key < 8)
        {
            const bool pressed = normalizedValue >= 0.5f;
            if(bundleMode_ && row == 'a')
            {
                if(pressed && key >= 0 && key < 4)
                {
                    SelectAlgorithmIndex(static_cast<std::size_t>(key));
                }
                else if(key == 4)
                {
                    if(pressed)
                    {
                        sharedCore_.SetInternalSynthMode(
                            (sharedCore_.GetInternalSynthMode() + 1) % 3);
                    }
                }
                else if(key == 5)
                {
                    if(pressed)
                    {
                        sharedCore_.SetInternalSynthHoldMode(
                            (sharedCore_.GetInternalSynthHoldMode() + 1) % 3);
                    }
                }
                else
                {
                    sharedCore_.TriggerFieldKeyAction(
                        static_cast<std::size_t>(key), pressed);
                }
            }
            else
            {
                const std::size_t index = (row == 'a' ? 0 : 8)
                                          + static_cast<std::size_t>(key);
                sharedCore_.TriggerFieldKeyAction(index, pressed);
            }
        }
        RefreshSnapshots();
        return;
    }

    if(bundleMode_)
    {
        const auto algorithm = StripMenuItemPrefix(itemId, "algorithm");
        if(!algorithm.empty())
        {
            if(algorithm == "back")
            {
                return;
            }
            if(normalizedValue >= 0.5f)
            {
                const auto& descriptors = GetDaisyDelayFxAlgorithmDescriptors();
                for(std::size_t i = 0; i < descriptors.size(); ++i)
                {
                    if(algorithm == descriptors[i].statePrefix)
                    {
                        SelectAlgorithmIndex(i);
                        return;
                    }
                }
            }
            RefreshSnapshots();
            return;
        }
    }

    for(int layer = 0; layer < 3; ++layer)
    {
        const auto suffix = StripMenuItemPrefix(itemId, LayerSectionName(layer));
        if(!suffix.empty())
        {
            sharedCore_.SetParameterValue(suffix, normalizedValue);
            RefreshSnapshots();
            return;
        }
    }
}

const DisplayModel& DelayFxAdaptationCore::GetDisplayModel() const
{
    return display_;
}

std::string DelayFxAdaptationCore::MakeParameterId(const std::string& nodeId,
                                                   const std::string& suffix)
{
    return nodeId + "/param/" + suffix;
}

std::string DelayFxAdaptationCore::MakeControlId(const std::string& nodeId,
                                                 const std::string& suffix)
{
    return nodeId + "/control/" + suffix;
}

std::string DelayFxAdaptationCore::MakeMenuRootSectionId(
    const std::string& nodeId)
{
    return nodeId + "/menu/root";
}

std::string DelayFxAdaptationCore::MakeMenuSectionId(
    const std::string& nodeId, const std::string& suffix)
{
    return nodeId + "/menu/" + suffix;
}

std::string DelayFxAdaptationCore::MakeMenuItemId(
    const std::string& nodeId,
    const std::string& section,
    const std::string& item)
{
    return nodeId + "/menu/" + section + "/" + item;
}

std::string DelayFxAdaptationCore::MakeAudioInputPortId(
    const std::string& nodeId, std::size_t oneBasedIndex)
{
    return nodeId + "/port/audio_in_" + std::to_string(oneBasedIndex);
}

std::string DelayFxAdaptationCore::MakeAudioOutputPortId(
    const std::string& nodeId, std::size_t oneBasedIndex)
{
    return nodeId + "/port/audio_out_" + std::to_string(oneBasedIndex);
}

std::string DelayFxAdaptationCore::MakeMidiInputPortId(
    const std::string& nodeId)
{
    return nodeId + "/port/midi_in";
}

std::string DelayFxAdaptationCore::MakeGateInputPortId(
    const std::string& nodeId)
{
    return nodeId + "/port/gate_in_1";
}

void DelayFxAdaptationCore::RefreshSnapshots()
{
    RefreshParameters();
    BuildMenuModel();
    BuildDisplay();
}

void DelayFxAdaptationCore::RefreshParameters()
{
    parameters_.clear();
    if(bundleMode_)
    {
        ParameterDescriptor descriptor;
        descriptor.id = MakeParameterId(nodeId_, "algorithm");
        descriptor.label = "Algorithm";
        descriptor.normalizedValue = CurrentAlgorithmNormalized();
        descriptor.defaultNormalizedValue = 0.0f;
        descriptor.effectiveNormalizedValue = descriptor.normalizedValue;
        descriptor.unitLabel = "";
        descriptor.stepCount = static_cast<int>(
            GetDaisyDelayFxAlgorithmDescriptors().size());
        descriptor.role = ParameterRole::kGeneric;
        descriptor.importanceRank = 0;
        descriptor.automatable = false;
        descriptor.stateful = true;
        descriptor.menuEditable = true;
        descriptor.nativeMinimum = 0.0f;
        descriptor.nativeMaximum = static_cast<float>(
            GetDaisyDelayFxAlgorithmDescriptors().size() - 1);
        descriptor.nativeDefault = 0.0f;
        descriptor.nativePrecision = 0;
        parameters_.push_back(descriptor);
    }
    for(const auto& parameter : sharedCore_.GetParameters())
    {
        ParameterDescriptor descriptor;
        descriptor.id = MakeParameterId(nodeId_, parameter.id);
        descriptor.label = parameter.label;
        descriptor.normalizedValue = parameter.normalizedValue;
        descriptor.defaultNormalizedValue = parameter.defaultNormalizedValue;
        descriptor.effectiveNormalizedValue = parameter.effectiveNormalizedValue;
        descriptor.unitLabel = parameter.unitLabel;
        descriptor.stepCount = parameter.stepCount;
        descriptor.role = RoleForParameterId(parameter.id);
        descriptor.importanceRank = parameter.importanceRank;
        descriptor.automatable = parameter.automatable;
        descriptor.stateful = parameter.stateful;
        descriptor.menuEditable = parameter.menuEditable;
        descriptor.nativeMinimum = parameter.nativeMinimum;
        descriptor.nativeMaximum = parameter.nativeMaximum;
        descriptor.nativeDefault = parameter.nativeDefault;
        descriptor.nativePrecision = parameter.nativePrecision;
        parameters_.push_back(descriptor);
    }
}

void DelayFxAdaptationCore::BuildMenuModel()
{
    const bool wasOpen = menu_.isOpen;
    const bool wasEditing = menu_.isEditing;
    const auto stack = menu_.sectionStack;
    const auto sectionId = menu_.currentSectionId.empty()
                               ? MakeMenuRootSectionId(nodeId_)
                               : menu_.currentSectionId;

    menu_ = {};
    menu_.isOpen = wasOpen;
    menu_.isEditing = wasEditing;
    menu_.sectionStack = stack;
    menu_.currentSectionId = sectionId;

    MenuSection root;
    root.id = MakeMenuRootSectionId(nodeId_);
    root.title = GetAppDisplayName();
    if(bundleMode_)
    {
        root.items.push_back({MakeMenuItemId(nodeId_, "root", "algorithm"),
                              "Algorithm",
                              false,
                              MenuItemActionKind::kEnterSection,
                              CurrentAlgorithmNormalized(),
                              CurrentAlgorithmLabel(),
                              MakeMenuSectionId(nodeId_, "algorithm")});
    }
    for(int layer = 0; layer < 3; ++layer)
    {
        root.items.push_back({MakeMenuItemId(nodeId_, "root", LayerSectionName(layer)),
                              LayerTitle(layer),
                              false,
                              MenuItemActionKind::kEnterSection,
                              0.0f,
                              "",
                              MakeMenuSectionId(nodeId_, LayerSectionName(layer))});
    }
    root.items.push_back({MakeMenuItemId(nodeId_, "root", "field_keys"),
                          "Field Keys",
                          false,
                          MenuItemActionKind::kEnterSection,
                          0.0f,
                          "",
                          MakeMenuSectionId(nodeId_, "field_keys")});
    root.items.push_back({MakeMenuItemId(nodeId_, "root", "info"),
                          "Info",
                          false,
                          MenuItemActionKind::kEnterSection,
                          0.0f,
                          "",
                          MakeMenuSectionId(nodeId_, "info")});
    menu_.sections.push_back(root);

    if(bundleMode_)
    {
        MenuSection algorithms;
        algorithms.id = MakeMenuSectionId(nodeId_, "algorithm");
        algorithms.title = "Algorithm";
        const auto& descriptors = GetDaisyDelayFxAlgorithmDescriptors();
        for(std::size_t i = 0; i < descriptors.size(); ++i)
        {
            algorithms.items.push_back(
                {MakeMenuItemId(nodeId_,
                                "algorithm",
                                descriptors[i].statePrefix),
                 descriptors[i].label,
                 false,
                 MenuItemActionKind::kMomentary,
                 i == CurrentAlgorithmIndex() ? 1.0f : 0.0f,
                 i == CurrentAlgorithmIndex() ? "selected" : "",
                 ""});
        }
        algorithms.items.push_back({MakeMenuItemId(nodeId_, "algorithm", "back"),
                                    "Back",
                                    false,
                                    MenuItemActionKind::kBack});
        menu_.sections.push_back(algorithms);
    }

    for(int layer = 0; layer < 3; ++layer)
    {
        MenuSection section;
        section.id = MakeMenuSectionId(nodeId_, LayerSectionName(layer));
        section.title = LayerTitle(layer);
        for(std::size_t knob = 0; knob < DaisyDelayFxCore::kKnobCount; ++knob)
        {
            const char* parameterId = sharedCore_.GetParameterForLayerKnob(
                static_cast<std::size_t>(layer), knob);
            if(parameterId[0] == '\0')
            {
                continue;
            }
            float value = 0.0f;
            sharedCore_.GetParameterValue(parameterId, &value);
            const auto* parameter = sharedCore_.FindParameter(parameterId);
            section.items.push_back({MakeMenuItemId(nodeId_,
                                                    LayerSectionName(layer),
                                                    parameterId),
                                     parameter != nullptr ? parameter->label
                                                          : parameterId,
                                     true,
                                     MenuItemActionKind::kValue,
                                     value,
                                     sharedCore_.FormatParameterValue(parameterId),
                                     ""});
        }
        section.items.push_back({MakeMenuItemId(nodeId_,
                                                LayerSectionName(layer),
                                                "back"),
                                 "Back",
                                 false,
                                 MenuItemActionKind::kBack});
        menu_.sections.push_back(section);
    }

    MenuSection keys;
    keys.id = MakeMenuSectionId(nodeId_, "field_keys");
    keys.title = "Field Keys";
    static constexpr std::array<const char*, 8> kSourceALabels = {{
        "A1 Bypass",
        "A2 Freeze",
        "A3 Reverse",
        "A4 Sync",
        "A5 Diffuse",
        "A6 Synth",
        "A7 Oct -",
        "A8 Oct +",
    }};
    static constexpr std::array<const char*, 8> kBundleALabels = {{
        "A1 Tape [multifx]",
        "A2 Tank [reverb]",
        "A3 Texture [FunBox]",
        "A4 Long [sdram]",
        "A5 Synth Off/Pluck/Pad",
        "A6 Hold Moment/Latch/Drone",
        "A7 Oct -",
        "A8 Oct +",
    }};
    const auto& aLabels = bundleMode_ ? kBundleALabels : kSourceALabels;
    for(std::size_t i = 0; i < aLabels.size(); ++i)
    {
        const std::string key = "a" + std::to_string(i + 1);
        keys.items.push_back({MakeMenuItemId(nodeId_, "field_keys", key),
                              aLabels[i],
                              false,
                              MenuItemActionKind::kMomentary,
                              0.0f,
                              "",
                              ""});
    }
    keys.items.push_back({MakeMenuItemId(nodeId_, "field_keys", "back"),
                          "Back",
                          false,
                          MenuItemActionKind::kBack});
    menu_.sections.push_back(keys);

    MenuSection info;
    info.id = MakeMenuSectionId(nodeId_, "info");
    info.title = "Info";
    info.items = {
        {MakeMenuItemId(nodeId_, "info", "source"),
         sharedCore_.GetProfile().sourceRepo,
         false,
         MenuItemActionKind::kReadonly,
         0.0f,
         "",
         ""},
        {MakeMenuItemId(nodeId_, "info", "summary"),
         "Source adaptation",
         false,
         MenuItemActionKind::kReadonly,
         0.0f,
         "Field/DaisyHost",
         ""},
        {MakeMenuItemId(nodeId_, "info", "back"),
         "Back",
         false,
         MenuItemActionKind::kBack},
    };
    menu_.sections.push_back(info);

    auto sectionIt = std::find_if(menu_.sections.begin(),
                                  menu_.sections.end(),
                                  [&sectionId](const MenuSection& section) {
                                      return section.id == sectionId;
                                  });
    if(sectionIt == menu_.sections.end())
    {
        menu_.currentSectionId = MakeMenuRootSectionId(nodeId_);
        sectionIt = menu_.sections.begin();
    }
    if(sectionIt != menu_.sections.end() && !sectionIt->items.empty())
    {
        sectionIt->selectedIndex = std::clamp(sectionIt->selectedIndex,
                                              0,
                                              static_cast<int>(sectionIt->items.size()) - 1);
        menu_.currentSelection = sectionIt->selectedIndex;
    }
}

void DelayFxAdaptationCore::BuildDisplay()
{
    display_ = {};
    display_.title = GetAppDisplayName();
    display_.texts.push_back(
        {0, 0, bundleMode_ ? CurrentAlgorithmLabel() : GetAppDisplayName(), false});

    int y = 12;
    for(const char* parameterId : {"mix", "time", "feedback", "texture"})
    {
        const auto* parameter = sharedCore_.FindParameter(parameterId);
        if(parameter == nullptr)
        {
            continue;
        }
        display_.texts.push_back({0,
                                  y,
                                  parameter->label + ": "
                                      + sharedCore_.FormatParameterValue(parameterId),
                                  false});
        y += 10;
    }
    display_.texts.push_back(
        {0,
         54,
         bundleMode_ ? "A5 synth A6 hold B keys" : "A: modes  B: C4-C5",
         false});
}

void DelayFxAdaptationCore::ResetMenuState()
{
    menu_ = {};
    menu_.currentSectionId = MakeMenuRootSectionId(nodeId_);
    menu_.currentSelection = 0;
}

void DelayFxAdaptationCore::MoveSelection(int delta)
{
    auto sectionIt = std::find_if(menu_.sections.begin(),
                                  menu_.sections.end(),
                                  [this](const MenuSection& section) {
                                      return section.id == menu_.currentSectionId;
                                  });
    if(sectionIt == menu_.sections.end() || sectionIt->items.empty())
    {
        return;
    }

    const int itemCount = static_cast<int>(sectionIt->items.size());
    int next = (sectionIt->selectedIndex + delta) % itemCount;
    if(next < 0)
    {
        next += itemCount;
    }
    sectionIt->selectedIndex = next;
    menu_.currentSelection = next;
    BuildDisplay();
}

void DelayFxAdaptationCore::PressSelectedItem()
{
    auto sectionIt = std::find_if(menu_.sections.begin(),
                                  menu_.sections.end(),
                                  [this](const MenuSection& section) {
                                      return section.id == menu_.currentSectionId;
                                  });
    if(sectionIt == menu_.sections.end() || sectionIt->items.empty())
    {
        menu_.isOpen = false;
        BuildDisplay();
        return;
    }

    auto& item = sectionIt->items[sectionIt->selectedIndex];
    if(item.actionKind == MenuItemActionKind::kEnterSection)
    {
        menu_.sectionStack.push_back(menu_.currentSectionId);
        menu_.currentSectionId = item.targetSectionId;
        menu_.currentSelection = 0;
        menu_.isEditing = false;
    }
    else if(item.actionKind == MenuItemActionKind::kBack)
    {
        if(menu_.sectionStack.empty())
        {
            menu_.isOpen = false;
        }
        else
        {
            menu_.currentSectionId = menu_.sectionStack.back();
            menu_.sectionStack.pop_back();
        }
        menu_.currentSelection = 0;
        menu_.isEditing = false;
    }
    else if(item.actionKind == MenuItemActionKind::kValue && item.editable)
    {
        menu_.isEditing = !menu_.isEditing;
    }
    else if(item.actionKind == MenuItemActionKind::kMomentary)
    {
        SetMenuItemValue(item.id, 1.0f);
        SetMenuItemValue(item.id, 0.0f);
    }
    else
    {
        menu_.isOpen = false;
    }
    BuildMenuModel();
    BuildDisplay();
}

void DelayFxAdaptationCore::SaveCurrentAlgorithmSnapshot()
{
    if(!bundleMode_)
    {
        return;
    }
    const std::size_t index = CurrentAlgorithmIndex();
    algorithmSnapshots_[index] = sharedCore_.CaptureStatefulParameterValues();
    algorithmSnapshotValid_[index] = true;
}

void DelayFxAdaptationCore::SelectAlgorithmIndex(std::size_t index,
                                                 bool saveCurrent)
{
    const auto& descriptors = GetDaisyDelayFxAlgorithmDescriptors();
    if(index >= descriptors.size())
    {
        index = descriptors.size() - 1;
    }

    if(saveCurrent)
    {
        SaveCurrentAlgorithmSnapshot();
    }

    source_ = descriptors[index].source;
    sharedCore_.SetSource(source_);
    if(algorithmSnapshotValid_[index])
    {
        sharedCore_.RestoreStatefulParameterValues(algorithmSnapshots_[index]);
    }
    ResetMenuState();
    RefreshSnapshots();
}

std::size_t DelayFxAdaptationCore::CurrentAlgorithmIndex() const
{
    return DaisyDelayFxAlgorithmIndex(sharedCore_.GetSource());
}

float DelayFxAdaptationCore::CurrentAlgorithmNormalized() const
{
    const auto& descriptors = GetDaisyDelayFxAlgorithmDescriptors();
    if(descriptors.size() <= 1)
    {
        return 0.0f;
    }
    return static_cast<float>(CurrentAlgorithmIndex())
           / static_cast<float>(descriptors.size() - 1);
}

std::string DelayFxAdaptationCore::CurrentAlgorithmLabel() const
{
    return GetDaisyDelayFxAlgorithmDescriptor(sharedCore_.GetSource()).label;
}

bool DelayFxAdaptationCore::SetBundleAlgorithmParameter(float normalizedValue)
{
    if(!bundleMode_)
    {
        return false;
    }
    const auto& descriptors = GetDaisyDelayFxAlgorithmDescriptors();
    const std::size_t index = static_cast<std::size_t>(
        std::round(Clamp01(normalizedValue)
                   * static_cast<float>(descriptors.size() - 1)));
    SelectAlgorithmIndex(index);
    return true;
}

bool DelayFxAdaptationCore::IsBundleAlgorithmParameter(
    const std::string& parameterId) const
{
    return bundleMode_ && parameterId == "algorithm";
}

std::string DelayFxAdaptationCore::MakeBundleStateId(
    std::size_t index, const std::string& parameterId) const
{
    const auto& descriptors = GetDaisyDelayFxAlgorithmDescriptors();
    if(index >= descriptors.size())
    {
        index = descriptors.size() - 1;
    }
    return std::string(descriptors[index].statePrefix) + "." + parameterId;
}

std::string DelayFxAdaptationCore::StripParameterId(
    const std::string& parameterId) const
{
    const std::string prefix = nodeId_ + "/param/";
    if(parameterId.rfind(prefix, 0) != 0)
    {
        return {};
    }
    return parameterId.substr(prefix.size());
}

std::string DelayFxAdaptationCore::StripControlId(
    const std::string& controlId) const
{
    const std::string prefix = nodeId_ + "/control/";
    if(controlId.rfind(prefix, 0) != 0)
    {
        return {};
    }
    return controlId.substr(prefix.size());
}

std::string DelayFxAdaptationCore::StripMenuItemPrefix(
    const std::string& itemId, const char* section) const
{
    const std::string prefix = nodeId_ + "/menu/" + section + "/";
    if(itemId.rfind(prefix, 0) != 0)
    {
        return {};
    }
    return itemId.substr(prefix.size());
}

void DelayFxAdaptationCore::ConsumeMidiInput()
{
    auto midiIt = portInputs_.find(MakeMidiInputPortId(nodeId_));
    if(midiIt == portInputs_.end())
    {
        return;
    }
    for(const auto& event : midiIt->second.midiEvents)
    {
        sharedCore_.HandleMidiEvent(event.status, event.data1, event.data2);
    }
    midiIt->second.midiEvents.clear();
}
} // namespace apps
} // namespace daisyhost
