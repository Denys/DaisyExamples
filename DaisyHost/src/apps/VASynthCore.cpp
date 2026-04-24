#include "daisyhost/apps/VASynthCore.h"

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

std::string FormatPageText(DaisyVASynthPage page)
{
    switch(page)
    {
        case DaisyVASynthPage::kFilter:
            return "Filter";
        case DaisyVASynthPage::kMotion:
            return "Motion";
        case DaisyVASynthPage::kOsc:
        default:
            return "Osc";
    }
}

std::string FormatOnOff(bool enabled)
{
    return enabled ? "On" : "Off";
}
} // namespace

VASynthCore::VASynthCore(const std::string& nodeId) : nodeId_(nodeId)
{
    portInputs_[MakeGateInputPortId(nodeId_, 1)].type = VirtualPortType::kGate;
    portInputs_[MakeMidiInputPortId(nodeId_, 1)].type = VirtualPortType::kMidi;
    portOutputs_[MakeAudioOutputPortId(nodeId_, 1)].type = VirtualPortType::kAudio;
    portOutputs_[MakeAudioOutputPortId(nodeId_, 2)].type = VirtualPortType::kAudio;

    ResetMenuState();
    sharedCore_.ResetToDefaultState();
    RefreshSnapshots();
}

std::string VASynthCore::GetAppId() const
{
    return "vasynth";
}

std::string VASynthCore::GetAppDisplayName() const
{
    return "VA Synth";
}

HostedAppCapabilities VASynthCore::GetCapabilities() const
{
    HostedAppCapabilities capabilities;
    capabilities.acceptsAudioInput  = false;
    capabilities.acceptsMidiInput   = true;
    capabilities.producesMidiOutput = false;
    return capabilities;
}

HostedAppPatchBindings VASynthCore::GetPatchBindings() const
{
    HostedAppPatchBindings bindings;
    const auto pageBinding = sharedCore_.GetActivePageBinding();
    for(std::size_t i = 0; i < pageBinding.parameterIds.size(); ++i)
    {
        bindings.knobControlIds[i]   = MakeControlId(nodeId_, pageBinding.parameterIds[i]);
        bindings.knobDetailLabels[i] = pageBinding.parameterLabels[i];
    }
    bindings.encoderControlId       = MakeEncoderControlId(nodeId_);
    bindings.encoderButtonControlId = MakeEncoderButtonControlId(nodeId_);
    bindings.gateInputPortIds[0]    = MakeGateInputPortId(nodeId_, 1);
    bindings.midiInputPortId        = MakeMidiInputPortId(nodeId_, 1);
    bindings.audioOutputPortIds[0]  = MakeAudioOutputPortId(nodeId_, 1);
    bindings.audioOutputPortIds[1]  = MakeAudioOutputPortId(nodeId_, 2);
    bindings.mainOutputChannels     = {0, 1};
    return bindings;
}

void VASynthCore::Prepare(double sampleRate, std::size_t maxBlockSize)
{
    sharedCore_.Prepare(sampleRate, maxBlockSize);
    RefreshSnapshots();
}

void VASynthCore::Process(const AudioBufferView&,
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

    std::vector<float> left(frameCount, 0.0f);
    std::vector<float> right(frameCount, 0.0f);
    sharedCore_.Process(left.data(), right.data(), frameCount);

    if(output.channelCount > 0 && output.channels[0] != nullptr)
    {
        std::copy(left.begin(), left.end(), output.channels[0]);
    }
    if(output.channelCount > 1 && output.channels[1] != nullptr)
    {
        std::copy(right.begin(), right.end(), output.channels[1]);
    }

    PortValue leftOutput;
    leftOutput.type   = VirtualPortType::kAudio;
    leftOutput.scalar = PeakForBuffer(left.data(), frameCount);
    portOutputs_[MakeAudioOutputPortId(nodeId_, 1)] = leftOutput;

    PortValue rightOutput;
    rightOutput.type   = VirtualPortType::kAudio;
    rightOutput.scalar = PeakForBuffer(right.data(), frameCount);
    portOutputs_[MakeAudioOutputPortId(nodeId_, 2)] = rightOutput;
}

void VASynthCore::SetControl(const std::string& controlId, float normalizedValue)
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

void VASynthCore::SetEncoderDelta(int delta)
{
    MenuRotate(delta);
}

void VASynthCore::SetEncoderPress(bool pressed)
{
    if(pressed && !encoderPressed_)
    {
        MenuPress();
    }
    encoderPressed_ = pressed;
}

void VASynthCore::SetPortInput(const std::string& portId, const PortValue& value)
{
    portInputs_[portId] = value;

    if(portId == MakeGateInputPortId(nodeId_, 1) && value.type == VirtualPortType::kGate)
    {
        sharedCore_.TriggerGate(value.gate);
        RefreshSnapshots();
        return;
    }

    if(portId == MakeMidiInputPortId(nodeId_, 1) && value.type == VirtualPortType::kMidi)
    {
        bool sawChange = false;
        for(const auto& event : value.midiEvents)
        {
            if((event.status & 0xF0) == 0x90 && event.data2 > 0)
            {
                sharedCore_.TriggerMidiNote(event.data1, event.data2);
                sawChange = true;
            }
            else if((event.status & 0xF0) == 0x80
                    || ((event.status & 0xF0) == 0x90 && event.data2 == 0))
            {
                sharedCore_.ReleaseMidiNote(event.data1);
                sawChange = true;
            }
        }
        if(sawChange)
        {
            RefreshSnapshots();
        }
    }
}

PortValue VASynthCore::GetPortOutput(const std::string& portId) const
{
    const auto it = portOutputs_.find(portId);
    return it != portOutputs_.end() ? it->second : PortValue{};
}

void VASynthCore::TickUi(double)
{
}

bool VASynthCore::SetParameterValue(const std::string& parameterId,
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

ParameterValueLookup VASynthCore::GetControlValue(const std::string& controlId) const
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

ParameterValueLookup VASynthCore::GetParameterValue(const std::string& parameterId) const
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

ParameterValueLookup VASynthCore::GetEffectiveParameterValue(
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

void VASynthCore::ResetToDefaultState(std::uint32_t seed)
{
    sharedCore_.ResetToDefaultState(seed);
    ResetMenuState();
    RefreshSnapshots();
}

std::unordered_map<std::string, float> VASynthCore::CaptureStatefulParameterValues() const
{
    std::unordered_map<std::string, float> values;
    for(const auto& [parameterId, value] : sharedCore_.CaptureStatefulParameterValues())
    {
        values.emplace(MakeParameterId(nodeId_, parameterId), value);
    }
    return values;
}

void VASynthCore::RestoreStatefulParameterValues(
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

const std::vector<ParameterDescriptor>& VASynthCore::GetParameters() const
{
    return parameters_;
}

const MenuModel& VASynthCore::GetMenuModel() const
{
    return menu_;
}

void VASynthCore::MenuRotate(int)
{
    if(!menu_.isOpen)
    {
        menu_.isOpen = true;
        BuildDisplay();
    }
}

void VASynthCore::MenuPress()
{
    menu_.isOpen = !menu_.isOpen;
    BuildDisplay();
}

void VASynthCore::SetMenuItemValue(const std::string& itemId,
                                   float              normalizedValue)
{
    if(itemId == MakeMenuItemId(nodeId_, "pages", "page"))
    {
        sharedCore_.SetActivePage(normalizedValue >= 0.5f ? DaisyVASynthPage::kFilter
                                                          : DaisyVASynthPage::kOsc);
    }
    else if(itemId == MakeMenuItemId(nodeId_, "oscillators", "osc1_wave"))
    {
        sharedCore_.SetParameterValue("osc1_wave", normalizedValue);
    }
    else if(itemId == MakeMenuItemId(nodeId_, "oscillators", "osc2_wave"))
    {
        sharedCore_.SetParameterValue("osc2_wave", normalizedValue);
    }
    else if(itemId == MakeMenuItemId(nodeId_, "utilities", "audition"))
    {
        sharedCore_.TriggerMomentaryAction("audition");
    }
    else if(itemId == MakeMenuItemId(nodeId_, "utilities", "init_patch"))
    {
        sharedCore_.ResetToDefaultState();
    }
    else if(itemId == MakeMenuItemId(nodeId_, "utilities", "stereo_sim"))
    {
        float currentValue = 0.0f;
        sharedCore_.GetParameterValue("stereo_sim", &currentValue);
        sharedCore_.SetParameterValue("stereo_sim", currentValue >= 0.5f ? 0.0f : 1.0f);
    }
    else if(itemId == MakeMenuItemId(nodeId_, "utilities", "panic"))
    {
        sharedCore_.TriggerMomentaryAction("panic");
    }
    RefreshSnapshots();
}

const DisplayModel& VASynthCore::GetDisplayModel() const
{
    return display_;
}

std::string VASynthCore::MakeParameterId(const std::string& nodeId,
                                         const std::string& suffix)
{
    return nodeId + "/param/" + suffix;
}

std::string VASynthCore::MakeControlId(const std::string& nodeId,
                                       const std::string& suffix)
{
    return nodeId + "/control/" + suffix;
}

std::string VASynthCore::MakeEncoderControlId(const std::string& nodeId)
{
    return nodeId + "/control/encoder";
}

std::string VASynthCore::MakeEncoderButtonControlId(const std::string& nodeId)
{
    return nodeId + "/control/encoder_button";
}

std::string VASynthCore::MakeGateInputPortId(const std::string& nodeId,
                                             std::size_t       oneBasedIndex)
{
    return nodeId + "/port/gate_in_" + std::to_string(oneBasedIndex);
}

std::string VASynthCore::MakeMidiInputPortId(const std::string& nodeId,
                                             std::size_t       oneBasedIndex)
{
    return nodeId + "/port/midi_in_" + std::to_string(oneBasedIndex);
}

std::string VASynthCore::MakeAudioOutputPortId(const std::string& nodeId,
                                               std::size_t       oneBasedIndex)
{
    return nodeId + "/port/audio_out_" + std::to_string(oneBasedIndex);
}

void VASynthCore::RefreshSnapshots()
{
    RefreshParameters();
    BuildMenuModel();
    BuildDisplay();
}

void VASynthCore::RefreshParameters()
{
    parameters_.clear();
    for(const auto& parameter : sharedCore_.GetParameters())
    {
        ParameterDescriptor descriptor;
        descriptor.id                       = MakeParameterId(nodeId_, parameter.id);
        descriptor.label                    = parameter.label;
        descriptor.normalizedValue          = parameter.normalizedValue;
        descriptor.defaultNormalizedValue   = parameter.defaultNormalizedValue;
        descriptor.effectiveNormalizedValue = parameter.effectiveNormalizedValue;
        descriptor.stepCount                = parameter.stepCount;
        descriptor.importanceRank           = parameter.importanceRank;
        descriptor.automatable              = parameter.automatable;
        descriptor.stateful                 = parameter.stateful;
        descriptor.menuEditable             = parameter.performanceTier;
        parameters_.push_back(descriptor);
    }
}

void VASynthCore::BuildMenuModel()
{
    menu_.sections.clear();
    menu_.currentSectionId = MakeMenuRootSectionId(nodeId_);

    MenuSection root;
    root.id = MakeMenuRootSectionId(nodeId_);
    root.title = "Menu";
    root.items = {
        {MakeMenuItemId(nodeId_, "root", "pages"),
         "Pages",
         false,
         MenuItemActionKind::kEnterSection,
         0.0f,
         FormatPageText(sharedCore_.GetActivePage()),
         MakeMenuSectionId(nodeId_, "pages")},
        {MakeMenuItemId(nodeId_, "root", "oscillators"),
         "Oscillators",
         false,
         MenuItemActionKind::kEnterSection,
         0.0f,
         "",
         MakeMenuSectionId(nodeId_, "oscillators")},
        {MakeMenuItemId(nodeId_, "root", "utilities"),
         "Utilities",
         false,
         MenuItemActionKind::kEnterSection,
         0.0f,
         "",
         MakeMenuSectionId(nodeId_, "utilities")},
        {MakeMenuItemId(nodeId_, "root", "info"),
         "Info",
         false,
         MenuItemActionKind::kEnterSection,
         0.0f,
         "",
         MakeMenuSectionId(nodeId_, "info")},
    };
    menu_.sections.push_back(root);

    MenuSection pages;
    pages.id = MakeMenuSectionId(nodeId_, "pages");
    pages.title = "Pages";
    pages.items = {
        {MakeMenuItemId(nodeId_, "pages", "page"),
         "Page",
         true,
         MenuItemActionKind::kValue,
         sharedCore_.GetActivePage() == DaisyVASynthPage::kOsc ? 0.0f : 1.0f,
         FormatPageText(sharedCore_.GetActivePage()),
         ""},
    };
    menu_.sections.push_back(pages);

    float osc1Value = 0.0f;
    float osc2Value = 0.0f;
    sharedCore_.GetParameterValue("osc1_wave", &osc1Value);
    sharedCore_.GetParameterValue("osc2_wave", &osc2Value);
    MenuSection oscillators;
    oscillators.id = MakeMenuSectionId(nodeId_, "oscillators");
    oscillators.title = "Oscillators";
    oscillators.items = {
        {MakeMenuItemId(nodeId_, "oscillators", "osc1_wave"),
         "Osc 1",
         true,
         MenuItemActionKind::kValue,
         osc1Value,
         sharedCore_.GetWaveLabel("osc1_wave"),
         ""},
        {MakeMenuItemId(nodeId_, "oscillators", "osc2_wave"),
         "Osc 2",
         true,
         MenuItemActionKind::kValue,
         osc2Value,
         sharedCore_.GetWaveLabel("osc2_wave"),
         ""},
    };
    menu_.sections.push_back(oscillators);

    float stereoSimValue = 0.0f;
    sharedCore_.GetParameterValue("stereo_sim", &stereoSimValue);
    MenuSection utilities;
    utilities.id = MakeMenuSectionId(nodeId_, "utilities");
    utilities.title = "Utilities";
    utilities.items = {
        {MakeMenuItemId(nodeId_, "utilities", "audition"),
         "Audition",
         true,
         MenuItemActionKind::kMomentary,
         0.0f,
         "",
         ""},
        {MakeMenuItemId(nodeId_, "utilities", "init_patch"),
         "Init Patch",
         true,
         MenuItemActionKind::kMomentary,
         0.0f,
         "",
         ""},
        {MakeMenuItemId(nodeId_, "utilities", "stereo_sim"),
         "Stereo Sim",
         true,
         MenuItemActionKind::kMomentary,
         stereoSimValue,
         FormatOnOff(stereoSimValue >= 0.5f),
         ""},
        {MakeMenuItemId(nodeId_, "utilities", "panic"),
         "Panic",
         true,
         MenuItemActionKind::kMomentary,
         0.0f,
         "",
         ""},
    };
    menu_.sections.push_back(utilities);

    MenuSection info;
    info.id = MakeMenuSectionId(nodeId_, "info");
    info.title = "Info";
    info.items = {
        {MakeMenuItemId(nodeId_, "info", "page"),
         "Page",
         false,
         MenuItemActionKind::kReadonly,
         0.0f,
         FormatPageText(sharedCore_.GetActivePage()),
         ""},
        {MakeMenuItemId(nodeId_, "info", "voices"),
         "Voices",
         false,
         MenuItemActionKind::kReadonly,
         0.0f,
         std::to_string(sharedCore_.GetCurrentVoiceCount()),
         ""},
        {MakeMenuItemId(nodeId_, "info", "note"),
         "Note",
         false,
         MenuItemActionKind::kReadonly,
         0.0f,
         std::to_string(sharedCore_.GetLastMidiNote()),
         ""},
    };
    menu_.sections.push_back(info);
}

void VASynthCore::BuildDisplay()
{
    display_.mode = menu_.isOpen ? DisplayMode::kMenu : DisplayMode::kStatus;
    display_.title = "VA Synth";
    display_.texts.clear();
    display_.bars.clear();
    ++display_.revision;

    if(menu_.isOpen)
    {
        int y = 0;
        for(const auto& section : menu_.sections)
        {
            if(section.id == MakeMenuRootSectionId(nodeId_))
            {
                continue;
            }
            display_.texts.push_back({0, y, section.title, false});
            y += 10;
        }
        return;
    }

    float stereoSimValue = 0.0f;
    sharedCore_.GetParameterValue("stereo_sim", &stereoSimValue);
    display_.texts.push_back({0, 0, "VA Synth", false});
    display_.texts.push_back(
        {0, 12, "Page " + FormatPageText(sharedCore_.GetActivePage()), false});
    display_.texts.push_back(
        {0,
         24,
         "Voices " + std::to_string(sharedCore_.GetCurrentVoiceCount()) + " Note "
             + std::to_string(sharedCore_.GetLastMidiNote()),
         false});
    display_.texts.push_back(
        {0,
         36,
         "Osc " + sharedCore_.GetWaveLabel("osc1_wave") + "/"
             + sharedCore_.GetWaveLabel("osc2_wave"),
         false});
    display_.texts.push_back(
        {0, 48, "Stereo " + FormatOnOff(stereoSimValue >= 0.5f), false});
}

void VASynthCore::ResetMenuState()
{
    menu_.isOpen           = false;
    menu_.isEditing        = false;
    menu_.currentSectionId = MakeMenuRootSectionId(nodeId_);
    menu_.currentSelection = 0;
}

std::string VASynthCore::StripParameterId(const std::string& parameterId) const
{
    const std::string prefix = nodeId_ + "/param/";
    if(parameterId.rfind(prefix, 0) == 0)
    {
        return parameterId.substr(prefix.size());
    }
    return {};
}

std::string VASynthCore::StripControlId(const std::string& controlId) const
{
    const std::string prefix = nodeId_ + "/control/";
    if(controlId.rfind(prefix, 0) == 0)
    {
        return controlId.substr(prefix.size());
    }
    return {};
}
} // namespace apps
} // namespace daisyhost
