#include "daisyhost/apps/PolyOscCore.h"

#include <algorithm>
#include <array>
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

std::string MakeMenuRootSectionId(const std::string& nodeId)
{
    return nodeId + "/menu/root";
}

std::string MakeMenuSectionId(const std::string& nodeId,
                              const std::string& suffix)
{
    return nodeId + "/menu/" + suffix;
}

std::string MakeMenuItemId(const std::string& nodeId,
                           const std::string& section,
                           const std::string& item)
{
    return nodeId + "/menu/" + section + "/" + item;
}

std::string FormatPercent(float normalizedValue)
{
    return std::to_string(static_cast<int>(std::round(Clamp01(normalizedValue) * 100.0f)))
           + "%";
}
} // namespace

PolyOscCore::PolyOscCore(const std::string& nodeId) : nodeId_(nodeId)
{
    for(std::size_t i = 0; i < 4; ++i)
    {
        portOutputs_[MakeAudioOutputPortId(nodeId_, i + 1)].type = VirtualPortType::kAudio;
    }

    ResetMenuState();
    sharedCore_.ResetToDefaultState();
    RefreshSnapshots();
}

std::string PolyOscCore::GetAppId() const
{
    return "polyosc";
}

std::string PolyOscCore::GetAppDisplayName() const
{
    return "PolyOsc";
}

HostedAppCapabilities PolyOscCore::GetCapabilities() const
{
    HostedAppCapabilities capabilities;
    capabilities.acceptsAudioInput  = false;
    capabilities.acceptsMidiInput   = false;
    capabilities.producesMidiOutput = false;
    return capabilities;
}

HostedAppPatchBindings PolyOscCore::GetPatchBindings() const
{
    HostedAppPatchBindings bindings;
    bindings.knobControlIds = {MakeControlId(nodeId_, "osc1_freq"),
                               MakeControlId(nodeId_, "osc2_freq"),
                               MakeControlId(nodeId_, "osc3_freq"),
                               MakeControlId(nodeId_, "global_freq")};
    bindings.knobParameterIds = {MakeParameterId(nodeId_, "osc1_freq"),
                                 MakeParameterId(nodeId_, "osc2_freq"),
                                 MakeParameterId(nodeId_, "osc3_freq"),
                                 MakeParameterId(nodeId_, "global_freq")};
    bindings.knobDetailLabels = {"Osc 1", "Osc 2", "Osc 3", "Global"};
    bindings.encoderControlId       = MakeEncoderControlId(nodeId_);
    bindings.encoderButtonControlId = MakeEncoderButtonControlId(nodeId_);
    for(std::size_t i = 0; i < 4; ++i)
    {
        bindings.audioOutputPortIds[i] = MakeAudioOutputPortId(nodeId_, i + 1);
    }
    bindings.mainOutputChannels = {3, 3};
    return bindings;
}

void PolyOscCore::Prepare(double sampleRate, std::size_t maxBlockSize)
{
    sharedCore_.Prepare(sampleRate, maxBlockSize);
    RefreshSnapshots();
}

void PolyOscCore::Process(const AudioBufferView&,
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

    std::array<std::vector<float>, 4> renderedOutputs;
    for(auto& renderedOutput : renderedOutputs)
    {
        renderedOutput.assign(frameCount, 0.0f);
    }

    sharedCore_.Process(renderedOutputs[0].data(),
                        renderedOutputs[1].data(),
                        renderedOutputs[2].data(),
                        renderedOutputs[3].data(),
                        frameCount);

    const auto copyCount = std::min<std::size_t>(output.channelCount,
                                                 renderedOutputs.size());
    for(std::size_t channel = 0; channel < copyCount; ++channel)
    {
        if(output.channels[channel] != nullptr)
        {
            std::copy(renderedOutputs[channel].begin(),
                      renderedOutputs[channel].end(),
                      output.channels[channel]);
        }
    }

    for(std::size_t channel = 0; channel < renderedOutputs.size(); ++channel)
    {
        PortValue outputValue;
        outputValue.type   = VirtualPortType::kAudio;
        outputValue.scalar = PeakForBuffer(renderedOutputs[channel].data(),
                                           frameCount);
        portOutputs_[MakeAudioOutputPortId(nodeId_, channel + 1)] = outputValue;
    }
}

void PolyOscCore::SetControl(const std::string& controlId, float normalizedValue)
{
    if(controlId == MakeEncoderButtonControlId(nodeId_))
    {
        SetEncoderPress(normalizedValue >= 0.5f);
        return;
    }

    const auto suffix = StripControlId(controlId);
    if(!suffix.empty() && sharedCore_.SetParameterValue(suffix, normalizedValue))
    {
        RefreshSnapshots();
    }
}

void PolyOscCore::SetEncoderDelta(int delta)
{
    sharedCore_.IncrementWaveform(delta);
    RefreshSnapshots();
}

void PolyOscCore::SetEncoderPress(bool pressed)
{
    if(pressed && !encoderPressed_)
    {
        MenuPress();
    }
    encoderPressed_ = pressed;
}

void PolyOscCore::SetPortInput(const std::string& portId, const PortValue& value)
{
    portInputs_[portId] = value;
}

PortValue PolyOscCore::GetPortOutput(const std::string& portId) const
{
    const auto it = portOutputs_.find(portId);
    return it != portOutputs_.end() ? it->second : PortValue{};
}

void PolyOscCore::TickUi(double)
{
}

bool PolyOscCore::SetParameterValue(const std::string& parameterId,
                                    float              normalizedValue)
{
    const auto suffix = StripParameterId(parameterId);
    if(suffix.empty() || !sharedCore_.SetParameterValue(suffix, normalizedValue))
    {
        return false;
    }

    RefreshSnapshots();
    return true;
}

bool PolyOscCore::SetEffectiveParameterValue(const std::string& parameterId,
                                             float normalizedValue)
{
    const auto suffix = StripParameterId(parameterId);
    if(suffix.empty()
       || !sharedCore_.SetEffectiveParameterValue(suffix, normalizedValue))
    {
        return false;
    }

    RefreshSnapshots();
    return true;
}

ParameterValueLookup PolyOscCore::GetControlValue(const std::string& controlId) const
{
    ParameterValueLookup lookup;
    const auto           suffix = StripControlId(controlId);
    float                value  = 0.0f;
    if(!suffix.empty() && sharedCore_.GetParameterValue(suffix, &value))
    {
        lookup.hasValue = true;
        lookup.value    = value;
    }
    return lookup;
}

ParameterValueLookup PolyOscCore::GetParameterValue(
    const std::string& parameterId) const
{
    ParameterValueLookup lookup;
    const auto           suffix = StripParameterId(parameterId);
    float                value  = 0.0f;
    if(!suffix.empty() && sharedCore_.GetParameterValue(suffix, &value))
    {
        lookup.hasValue = true;
        lookup.value    = value;
    }
    return lookup;
}

ParameterValueLookup PolyOscCore::GetEffectiveParameterValue(
    const std::string& parameterId) const
{
    ParameterValueLookup lookup;
    const auto           suffix = StripParameterId(parameterId);
    float                value  = 0.0f;
    if(!suffix.empty() && sharedCore_.GetEffectiveParameterValue(suffix, &value))
    {
        lookup.hasValue = true;
        lookup.value    = value;
    }
    return lookup;
}

void PolyOscCore::ResetToDefaultState(std::uint32_t seed)
{
    sharedCore_.ResetToDefaultState(seed);
    ResetMenuState();
    RefreshSnapshots();
}

std::unordered_map<std::string, float> PolyOscCore::CaptureStatefulParameterValues()
    const
{
    std::unordered_map<std::string, float> values;
    for(const auto& [parameterId, value] : sharedCore_.CaptureStatefulParameterValues())
    {
        values.emplace(MakeParameterId(nodeId_, parameterId), value);
    }
    return values;
}

void PolyOscCore::RestoreStatefulParameterValues(
    const std::unordered_map<std::string, float>& values)
{
    for(const auto& [parameterId, value] : values)
    {
        const auto suffix = StripParameterId(parameterId);
        if(!suffix.empty())
        {
            sharedCore_.SetParameterValue(suffix, value);
        }
    }
    RefreshSnapshots();
}

const std::vector<ParameterDescriptor>& PolyOscCore::GetParameters() const
{
    return parameters_;
}

const MenuModel& PolyOscCore::GetMenuModel() const
{
    return menu_;
}

void PolyOscCore::MenuRotate(int delta)
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
                                            + static_cast<float>(delta) * 0.05f,
                                        0.0f,
                                        1.0f));
        }
        return;
    }

    const int itemCount = static_cast<int>(sectionIt->items.size());
    int       next      = (sectionIt->selectedIndex + delta) % itemCount;
    if(next < 0)
    {
        next += itemCount;
    }
    sectionIt->selectedIndex = next;
    menu_.currentSelection   = next;
    BuildDisplay();
}

void PolyOscCore::MenuPress()
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
        menu_.isEditing        = false;
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
            menu_.currentSelection = 0;
        }
        menu_.isEditing = false;
    }
    else if(item.actionKind == MenuItemActionKind::kValue && item.editable)
    {
        menu_.isEditing = !menu_.isEditing;
    }
    else
    {
        menu_.isOpen = false;
    }
    BuildDisplay();
}

void PolyOscCore::SetMenuItemValue(const std::string& itemId,
                                   float              normalizedValue)
{
    if(itemId == MakeMenuItemId(nodeId_, "params", "osc1_freq"))
    {
        sharedCore_.SetParameterValue("osc1_freq", normalizedValue);
    }
    else if(itemId == MakeMenuItemId(nodeId_, "params", "osc2_freq"))
    {
        sharedCore_.SetParameterValue("osc2_freq", normalizedValue);
    }
    else if(itemId == MakeMenuItemId(nodeId_, "params", "osc3_freq"))
    {
        sharedCore_.SetParameterValue("osc3_freq", normalizedValue);
    }
    else if(itemId == MakeMenuItemId(nodeId_, "params", "global_freq"))
    {
        sharedCore_.SetParameterValue("global_freq", normalizedValue);
    }
    else if(itemId == MakeMenuItemId(nodeId_, "waveform", "waveform"))
    {
        sharedCore_.SetParameterValue("waveform", normalizedValue);
    }
    RefreshSnapshots();
}

const DisplayModel& PolyOscCore::GetDisplayModel() const
{
    return display_;
}

std::string PolyOscCore::MakeParameterId(const std::string& nodeId,
                                         const std::string& suffix)
{
    return nodeId + "/param/" + suffix;
}

std::string PolyOscCore::MakeControlId(const std::string& nodeId,
                                       const std::string& suffix)
{
    return nodeId + "/control/" + suffix;
}

std::string PolyOscCore::MakeEncoderControlId(const std::string& nodeId)
{
    return nodeId + "/control/encoder";
}

std::string PolyOscCore::MakeEncoderButtonControlId(const std::string& nodeId)
{
    return nodeId + "/control/encoder_button";
}

std::string PolyOscCore::MakeAudioOutputPortId(const std::string& nodeId,
                                               std::size_t       oneBasedIndex)
{
    return nodeId + "/port/audio_out_" + std::to_string(oneBasedIndex);
}

void PolyOscCore::RefreshSnapshots()
{
    RefreshParameters();
    BuildMenuModel();
    BuildDisplay();
}

void PolyOscCore::RefreshParameters()
{
    parameters_.clear();
    for(const auto& parameter : sharedCore_.GetParameters())
    {
        ParameterDescriptor descriptor;
        descriptor.id
            = MakeParameterId(nodeId_, parameter.id);
        descriptor.label                    = parameter.label;
        descriptor.normalizedValue          = parameter.normalizedValue;
        descriptor.defaultNormalizedValue   = parameter.defaultNormalizedValue;
        descriptor.effectiveNormalizedValue = parameter.effectiveNormalizedValue;
        descriptor.stepCount                = parameter.stepCount;
        descriptor.importanceRank           = parameter.importanceRank;
        descriptor.automatable              = parameter.automatable;
        descriptor.stateful                 = parameter.stateful;
        descriptor.menuEditable             = true;
        parameters_.push_back(descriptor);
    }
}

void PolyOscCore::BuildMenuModel()
{
    const bool wasOpen    = menu_.isOpen;
    const bool wasEditing = menu_.isEditing;
    const auto stack      = menu_.sectionStack;
    const auto sectionId  = menu_.currentSectionId.empty() ? MakeMenuRootSectionId(nodeId_)
                                                           : menu_.currentSectionId;

    menu_             = {};
    menu_.isOpen      = wasOpen;
    menu_.isEditing   = wasEditing;
    menu_.sectionStack = stack;
    menu_.currentSectionId = sectionId;

    MenuSection root;
    root.id    = MakeMenuRootSectionId(nodeId_);
    root.title = "PolyOsc";
    root.items = {
        {MakeMenuItemId(nodeId_, "root", "params"),
         "Params",
         false,
         MenuItemActionKind::kEnterSection,
         0.0f,
         "",
         MakeMenuSectionId(nodeId_, "params")},
        {MakeMenuItemId(nodeId_, "root", "waveform"),
         "Waveform",
         false,
         MenuItemActionKind::kEnterSection,
         0.0f,
         "",
         MakeMenuSectionId(nodeId_, "waveform")},
        {MakeMenuItemId(nodeId_, "root", "info"),
         "Info",
         false,
         MenuItemActionKind::kEnterSection,
         0.0f,
         "",
         MakeMenuSectionId(nodeId_, "info")},
    };
    menu_.sections.push_back(root);

    MenuSection params;
    params.id    = MakeMenuSectionId(nodeId_, "params");
    params.title = "Params";
    for(const auto& parameterId :
        {"osc1_freq", "osc2_freq", "osc3_freq", "global_freq"})
    {
        float value = 0.0f;
        sharedCore_.GetParameterValue(parameterId, &value);
        const auto* parameter = sharedCore_.FindParameter(parameterId);
        params.items.push_back({MakeMenuItemId(nodeId_, "params", parameterId),
                                parameter != nullptr ? parameter->label : parameterId,
                                true,
                                MenuItemActionKind::kValue,
                                value,
                                FormatPercent(value),
                                ""});
    }
    params.items.push_back({MakeMenuItemId(nodeId_, "params", "back"),
                            "Back",
                            false,
                            MenuItemActionKind::kBack});
    menu_.sections.push_back(params);

    float waveformValue = 0.0f;
    sharedCore_.GetParameterValue("waveform", &waveformValue);
    MenuSection waveform;
    waveform.id    = MakeMenuSectionId(nodeId_, "waveform");
    waveform.title = "Waveform";
    waveform.items = {
        {MakeMenuItemId(nodeId_, "waveform", "waveform"),
         "Waveform",
         true,
         MenuItemActionKind::kValue,
         waveformValue,
         sharedCore_.GetWaveformLabel(),
         ""},
        {MakeMenuItemId(nodeId_, "waveform", "back"),
         "Back",
         false,
         MenuItemActionKind::kBack},
    };
    menu_.sections.push_back(waveform);

    MenuSection info;
    info.id    = MakeMenuSectionId(nodeId_, "info");
    info.title = "Info";
    info.items = {
        {MakeMenuItemId(nodeId_, "info", "source"),
         "Patch PolyOsc",
         false,
         MenuItemActionKind::kReadonly,
         0.0f,
         "3 osc + mix",
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
        sectionIt              = menu_.sections.begin();
    }
    menu_.currentSelection = sectionIt != menu_.sections.end()
                                 ? std::clamp(sectionIt->selectedIndex,
                                              0,
                                              static_cast<int>(sectionIt->items.size()) - 1)
                                 : 0;
}

void PolyOscCore::BuildDisplay()
{
    display_ = {};
    display_.title = "PolyOsc";
    display_.texts.push_back({0, 0, "PolyOsc", false});
    display_.texts.push_back({0, 12, "Wave: " + sharedCore_.GetWaveformLabel(), false});
    display_.texts.push_back({0, 24, "Mix Out 4", false});
}

void PolyOscCore::ResetMenuState()
{
    menu_                  = {};
    menu_.currentSectionId = MakeMenuRootSectionId(nodeId_);
    menu_.currentSelection = 0;
}

std::string PolyOscCore::StripParameterId(const std::string& parameterId) const
{
    const std::string prefix = nodeId_ + "/param/";
    if(parameterId.rfind(prefix, 0) != 0)
    {
        return {};
    }
    return parameterId.substr(prefix.size());
}

std::string PolyOscCore::StripControlId(const std::string& controlId) const
{
    const std::string prefix = nodeId_ + "/control/";
    if(controlId.rfind(prefix, 0) != 0)
    {
        return {};
    }
    return controlId.substr(prefix.size());
}
} // namespace apps
} // namespace daisyhost
