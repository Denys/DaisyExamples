#include "daisyhost/apps/BraidsCore.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>

namespace daisyhost
{
namespace apps
{
namespace
{
constexpr std::size_t kVisibleMenuRows = 4;

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
    char buffer[16];
    std::snprintf(
        buffer, sizeof(buffer), "%d%%", static_cast<int>(std::round(Clamp01(normalizedValue) * 100.0f)));
    return std::string(buffer);
}

std::string FormatTune(int semitoneOffset)
{
    char buffer[16];
    std::snprintf(buffer, sizeof(buffer), "%+dst", semitoneOffset);
    return std::string(buffer);
}

std::string FormatPageText(DaisyBraidsPage page)
{
    return page == DaisyBraidsPage::kFinish ? "Finish" : "Drum";
}
} // namespace

BraidsCore::BraidsCore(const std::string& nodeId)
: nodeId_(nodeId)
{
    portInputs_[MakeGateInputPortId(nodeId_, 1)].type = VirtualPortType::kGate;
    portInputs_[MakeMidiInputPortId(nodeId_, 1)].type = VirtualPortType::kMidi;
    portOutputs_[MakeAudioOutputPortId(nodeId_, 1)].type = VirtualPortType::kAudio;
    portOutputs_[MakeAudioOutputPortId(nodeId_, 2)].type = VirtualPortType::kAudio;

    ResetMenuState();
    sharedCore_.ResetToDefaultState();
    RefreshSnapshots();
}

std::string BraidsCore::GetAppId() const
{
    return "braids";
}

std::string BraidsCore::GetAppDisplayName() const
{
    return "Braids";
}

HostedAppCapabilities BraidsCore::GetCapabilities() const
{
    HostedAppCapabilities capabilities;
    capabilities.acceptsAudioInput  = false;
    capabilities.acceptsMidiInput   = true;
    capabilities.producesMidiOutput = false;
    return capabilities;
}

HostedAppPatchBindings BraidsCore::GetPatchBindings() const
{
    HostedAppPatchBindings bindings;
    const auto pageBinding = sharedCore_.GetActivePageBinding();
    for(std::size_t i = 0; i < pageBinding.parameterIds.size(); ++i)
    {
        bindings.knobControlIds[i] = MakeControlId(nodeId_, pageBinding.parameterIds[i]);
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

void BraidsCore::Prepare(double sampleRate, std::size_t maxBlockSize)
{
    sharedCore_.Prepare(sampleRate, maxBlockSize);
    RefreshSnapshots();
}

void BraidsCore::Process(const AudioBufferView&,
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

void BraidsCore::SetControl(const std::string& controlId, float normalizedValue)
{
    if(controlId == MakeEncoderButtonControlId(nodeId_))
    {
        SetEncoderPress(normalizedValue >= 0.5f);
        return;
    }

    const std::string suffix = StripControlId(controlId);
    if(suffix.empty())
    {
        return;
    }

    if(sharedCore_.SetParameterValue(suffix, normalizedValue))
    {
        RefreshSnapshots();
    }
}

void BraidsCore::SetEncoderDelta(int delta)
{
    MenuRotate(delta);
}

void BraidsCore::SetEncoderPress(bool pressed)
{
    if(pressed && !encoderPressed_)
    {
        MenuPress();
    }
    encoderPressed_ = pressed;
}

void BraidsCore::SetPortInput(const std::string& portId, const PortValue& value)
{
    portInputs_[portId] = value;

    if(portId == MakeGateInputPortId(nodeId_, 1) && value.type == VirtualPortType::kGate)
    {
        sharedCore_.TriggerGate(value.gate);
        BuildMenuModel();
        BuildDisplay();
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
        }
        if(sawChange)
        {
            BuildMenuModel();
            BuildDisplay();
        }
    }
}

PortValue BraidsCore::GetPortOutput(const std::string& portId) const
{
    if(const auto outputIt = portOutputs_.find(portId); outputIt != portOutputs_.end())
    {
        return outputIt->second;
    }
    if(const auto inputIt = portInputs_.find(portId); inputIt != portInputs_.end())
    {
        return inputIt->second;
    }
    return PortValue();
}

void BraidsCore::TickUi(double)
{
}

bool BraidsCore::SetParameterValue(const std::string& parameterId,
                                   float              normalizedValue)
{
    const std::string suffix = StripParameterId(parameterId);
    if(suffix.empty())
    {
        return false;
    }
    const bool applied = sharedCore_.SetParameterValue(suffix, normalizedValue);
    if(applied)
    {
        RefreshSnapshots();
    }
    return applied;
}

ParameterValueLookup BraidsCore::GetControlValue(const std::string& controlId) const
{
    const auto pageBinding = sharedCore_.GetActivePageBinding();
    for(const auto& parameterId : pageBinding.parameterIds)
    {
        if(controlId == MakeControlId(nodeId_, parameterId))
        {
            float value = 0.0f;
            if(sharedCore_.GetParameterValue(parameterId, &value))
            {
                return {true, value};
            }
        }
    }
    return {};
}

ParameterValueLookup BraidsCore::GetParameterValue(const std::string& parameterId) const
{
    float value = 0.0f;
    if(!sharedCore_.GetParameterValue(StripParameterId(parameterId), &value))
    {
        return {};
    }
    return {true, value};
}

ParameterValueLookup BraidsCore::GetEffectiveParameterValue(
    const std::string& parameterId) const
{
    float value = 0.0f;
    if(!sharedCore_.GetEffectiveParameterValue(StripParameterId(parameterId), &value))
    {
        return {};
    }
    return {true, value};
}

void BraidsCore::ResetToDefaultState(std::uint32_t seed)
{
    sharedCore_.ResetToDefaultState(seed);
    ResetMenuState();
    RefreshSnapshots();
}

std::unordered_map<std::string, float> BraidsCore::CaptureStatefulParameterValues() const
{
    std::unordered_map<std::string, float> values;
    for(const auto& entry : sharedCore_.CaptureStatefulParameterValues())
    {
        values.emplace(MakeParameterId(nodeId_, entry.first), entry.second);
    }
    return values;
}

void BraidsCore::RestoreStatefulParameterValues(
    const std::unordered_map<std::string, float>& values)
{
    std::unordered_map<std::string, float> stripped;
    for(const auto& entry : values)
    {
        const std::string suffix = StripParameterId(entry.first);
        if(!suffix.empty())
        {
            stripped[suffix] = entry.second;
        }
    }
    sharedCore_.RestoreStatefulParameterValues(stripped);
    RefreshSnapshots();
}

const std::vector<ParameterDescriptor>& BraidsCore::GetParameters() const
{
    return parameters_;
}

const MenuModel& BraidsCore::GetMenuModel() const
{
    return menu_;
}

void BraidsCore::MenuRotate(int delta)
{
    if(delta == 0 || !menu_.isOpen)
    {
        return;
    }

    if(menu_.isEditing)
    {
        if(MenuItem* item = GetSelectedMenuItem())
        {
            item->normalizedValue
                = Clamp01(item->normalizedValue + (GetMenuStepSize(item->id) * static_cast<float>(delta)));
            SetMenuItemValue(item->id, item->normalizedValue);
        }
        return;
    }

    MoveSelection(delta);
    BuildDisplay();
}

void BraidsCore::MenuPress()
{
    if(!menu_.isOpen)
    {
        menu_.isOpen = true;
        BuildDisplay();
        return;
    }

    MenuItem* item = GetSelectedMenuItem();
    if(item == nullptr)
    {
        return;
    }

    switch(item->actionKind)
    {
        case MenuItemActionKind::kEnterSection:
            menu_.sectionStack.push_back(menu_.currentSectionId);
            menu_.currentSectionId = item->targetSectionId;
            menu_.currentSelection = menuSelections_[menu_.currentSectionId];
            break;
        case MenuItemActionKind::kBack:
            if(menu_.sectionStack.empty())
            {
                menu_.isOpen = false;
            }
            else
            {
                menu_.currentSectionId = menu_.sectionStack.back();
                menu_.sectionStack.pop_back();
                menu_.currentSelection = menuSelections_[menu_.currentSectionId];
            }
            break;
        case MenuItemActionKind::kValue:
            menu_.isEditing = !menu_.isEditing;
            break;
        case MenuItemActionKind::kMomentary:
            SetMenuItemValue(item->id, 1.0f);
            break;
        case MenuItemActionKind::kReadonly:
            menu_.isOpen = false;
            break;
    }
    BuildDisplay();
}

void BraidsCore::SetMenuItemValue(const std::string& itemId, float normalizedValue)
{
    const float value = Clamp01(normalizedValue);
    if(itemId == MakeMenuItemId(nodeId_, "pages", "page"))
    {
        sharedCore_.SetActivePage(value >= 0.5f ? DaisyBraidsPage::kFinish
                                                : DaisyBraidsPage::kDrum);
        RefreshSnapshots();
        return;
    }
    if(itemId == MakeMenuItemId(nodeId_, "model", "model"))
    {
        sharedCore_.SetParameterValue("model", value);
        RefreshSnapshots();
        return;
    }
    if(itemId == MakeMenuItemId(nodeId_, "utilities", "audition") && value >= 0.5f)
    {
        sharedCore_.TriggerMomentaryAction("audition");
        BuildMenuModel();
        BuildDisplay();
        return;
    }
    if(itemId == MakeMenuItemId(nodeId_, "utilities", "randomize_model")
       && value >= 0.5f)
    {
        sharedCore_.TriggerMomentaryAction("randomize_model");
        RefreshSnapshots();
        return;
    }
    if(itemId == MakeMenuItemId(nodeId_, "utilities", "panic") && value >= 0.5f)
    {
        sharedCore_.TriggerMomentaryAction("panic");
        BuildMenuModel();
        BuildDisplay();
        return;
    }
}

const DisplayModel& BraidsCore::GetDisplayModel() const
{
    return display_;
}

std::string BraidsCore::MakeParameterId(const std::string& nodeId,
                                        const std::string& suffix)
{
    return nodeId + "/param/" + suffix;
}

std::string BraidsCore::MakeControlId(const std::string& nodeId,
                                      const std::string& suffix)
{
    return nodeId + "/control/" + suffix;
}

std::string BraidsCore::MakeEncoderControlId(const std::string& nodeId)
{
    return nodeId + "/control/encoder";
}

std::string BraidsCore::MakeEncoderButtonControlId(const std::string& nodeId)
{
    return nodeId + "/control/encoder_button";
}

std::string BraidsCore::MakeGateInputPortId(const std::string& nodeId,
                                            std::size_t       oneBasedIndex)
{
    return nodeId + "/port/gate_in_" + std::to_string(oneBasedIndex);
}

std::string BraidsCore::MakeMidiInputPortId(const std::string& nodeId,
                                            std::size_t       oneBasedIndex)
{
    return nodeId + "/port/midi_in_" + std::to_string(oneBasedIndex);
}

std::string BraidsCore::MakeAudioOutputPortId(const std::string& nodeId,
                                              std::size_t       oneBasedIndex)
{
    return nodeId + "/port/audio_out_" + std::to_string(oneBasedIndex);
}

void BraidsCore::RefreshSnapshots()
{
    RefreshParameters();
    BuildMenuModel();
    BuildDisplay();
}

void BraidsCore::RefreshParameters()
{
    parameters_.clear();
    for(const auto& parameter : sharedCore_.GetParameters())
    {
        ParameterDescriptor descriptor;
        descriptor.id                     = MakeParameterId(nodeId_, parameter.id);
        descriptor.label                  = parameter.label;
        descriptor.normalizedValue        = parameter.normalizedValue;
        descriptor.defaultNormalizedValue = parameter.defaultNormalizedValue;
        descriptor.effectiveNormalizedValue = parameter.effectiveNormalizedValue;
        descriptor.stepCount              = parameter.stepCount;
        descriptor.importanceRank         = parameter.importanceRank;
        descriptor.automatable            = parameter.automatable;
        descriptor.stateful               = parameter.stateful;
        descriptor.menuEditable           = true;
        descriptor.unitLabel = "";
        parameters_.push_back(descriptor);
    }
}

void BraidsCore::BuildMenuModel()
{
    menu_.sections.clear();

    MenuSection root;
    root.id = MakeMenuRootSectionId(nodeId_);
    root.title = "Braids";
    root.selectedIndex = menuSelections_[root.id];
    root.items = {
        {MakeMenuItemId(nodeId_, "root", "pages"),
         "Pages",
         false,
         MenuItemActionKind::kEnterSection,
         0.0f,
         FormatPageText(sharedCore_.GetActivePage()),
         MakeMenuSectionId(nodeId_, "pages")},
        {MakeMenuItemId(nodeId_, "root", "model"),
         "Model",
         false,
         MenuItemActionKind::kEnterSection,
         0.0f,
         sharedCore_.GetCurrentModelLabel(),
         MakeMenuSectionId(nodeId_, "model")},
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
    pages.selectedIndex = menuSelections_[pages.id];
    pages.items = {
        {MakeMenuItemId(nodeId_, "pages", "back"),
         "Back",
         false,
         MenuItemActionKind::kBack,
         0.0f,
         "",
         ""},
        {MakeMenuItemId(nodeId_, "pages", "page"),
         "Page",
         true,
         MenuItemActionKind::kValue,
         sharedCore_.GetActivePage() == DaisyBraidsPage::kFinish ? 1.0f : 0.0f,
         FormatPageText(sharedCore_.GetActivePage()),
         ""},
    };
    menu_.sections.push_back(pages);

    float modelValue = 0.0f;
    sharedCore_.GetParameterValue("model", &modelValue);
    MenuSection model;
    model.id = MakeMenuSectionId(nodeId_, "model");
    model.title = "Model";
    model.selectedIndex = menuSelections_[model.id];
    model.items = {
        {MakeMenuItemId(nodeId_, "model", "back"),
         "Back",
         false,
         MenuItemActionKind::kBack,
         0.0f,
         "",
         ""},
        {MakeMenuItemId(nodeId_, "model", "model"),
         "Model",
         true,
         MenuItemActionKind::kValue,
         modelValue,
         sharedCore_.GetCurrentModelLabel(),
         ""},
    };
    menu_.sections.push_back(model);

    MenuSection utilities;
    utilities.id = MakeMenuSectionId(nodeId_, "utilities");
    utilities.title = "Utilities";
    utilities.selectedIndex = menuSelections_[utilities.id];
    utilities.items = {
        {MakeMenuItemId(nodeId_, "utilities", "back"),
         "Back",
         false,
         MenuItemActionKind::kBack,
         0.0f,
         "",
         ""},
        {MakeMenuItemId(nodeId_, "utilities", "audition"),
         "Audition",
         false,
         MenuItemActionKind::kMomentary,
         0.0f,
         "Strike",
         ""},
        {MakeMenuItemId(nodeId_, "utilities", "randomize_model"),
         "Randomize Model",
         false,
         MenuItemActionKind::kMomentary,
         0.0f,
         "Subset",
         ""},
        {MakeMenuItemId(nodeId_, "utilities", "panic"),
         "Panic",
         false,
         MenuItemActionKind::kMomentary,
         0.0f,
         "Clear",
         ""},
    };
    menu_.sections.push_back(utilities);

    MenuSection info;
    info.id = MakeMenuSectionId(nodeId_, "info");
    info.title = "Info";
    info.selectedIndex = menuSelections_[info.id];
    info.items = {
        {MakeMenuItemId(nodeId_, "info", "back"),
         "Back",
         false,
         MenuItemActionKind::kBack,
         0.0f,
         "",
         ""},
        {MakeMenuItemId(nodeId_, "info", "model"),
         "Model",
         false,
         MenuItemActionKind::kReadonly,
         0.0f,
         sharedCore_.GetCurrentModelLabel(),
         ""},
        {MakeMenuItemId(nodeId_, "info", "note"),
         "Note",
         false,
         MenuItemActionKind::kReadonly,
         0.0f,
         std::to_string(sharedCore_.GetCurrentMidiNote()),
         ""},
        {MakeMenuItemId(nodeId_, "info", "page"),
         "Page",
         false,
         MenuItemActionKind::kReadonly,
         0.0f,
         FormatPageText(sharedCore_.GetActivePage()),
         ""},
        {MakeMenuItemId(nodeId_, "info", "finish"),
         "Finish",
         false,
         MenuItemActionKind::kReadonly,
         0.0f,
         sharedCore_.GetResolutionLabel() + " / " + sharedCore_.GetSampleRateLabel(),
         ""},
    };
    menu_.sections.push_back(info);

    if(menu_.currentSectionId.empty())
    {
        menu_.currentSectionId = root.id;
    }
    menu_.currentSelection = menuSelections_[menu_.currentSectionId];
}

void BraidsCore::BuildDisplay()
{
    display_.title = "Braids";
    display_.texts.clear();
    display_.bars.clear();
    display_.mode = menu_.isOpen ? DisplayMode::kMenu : DisplayMode::kStatus;
    ++display_.revision;

    if(menu_.isOpen)
    {
        const MenuSection* currentSection = nullptr;
        for(const auto& section : menu_.sections)
        {
            if(section.id == menu_.currentSectionId)
            {
                currentSection = &section;
                break;
            }
        }
        if(currentSection == nullptr)
        {
            return;
        }

        display_.texts.push_back({0, 0, currentSection->title, false});
        const int startIndex = std::max(0, currentSection->selectedIndex);
        for(std::size_t row = 0; row < kVisibleMenuRows; ++row)
        {
            const int itemIndex = startIndex + static_cast<int>(row);
            if(itemIndex >= static_cast<int>(currentSection->items.size()))
            {
                break;
            }
            const auto& item = currentSection->items[static_cast<std::size_t>(itemIndex)];
            std::string line = item.label;
            if(!item.valueText.empty())
            {
                line += " " + item.valueText;
            }
            display_.texts.push_back(
                {0, 12 + static_cast<int>(row * 12), line, itemIndex == currentSection->selectedIndex});
        }
        return;
    }

    display_.texts.push_back({0, 0, "Braids", false});
    display_.texts.push_back(
        {0, 12, "Model " + sharedCore_.GetCurrentModelLabel(), false});
    display_.texts.push_back(
        {0,
         24,
         "Page " + FormatPageText(sharedCore_.GetActivePage()) + " Note "
             + std::to_string(sharedCore_.GetCurrentMidiNote()),
         false});
    display_.texts.push_back(
        {0,
         36,
         "Tune " + FormatTune(sharedCore_.GetCurrentTuneSemitoneOffset()) + " Sig "
             + FormatPercent(GetParameterValue(MakeParameterId(nodeId_, "signature")).value),
         false});
    display_.texts.push_back(
        {0,
         48,
         "Fin " + sharedCore_.GetResolutionLabel() + " " + sharedCore_.GetSampleRateLabel(),
         false});
}

void BraidsCore::ResetMenuState()
{
    menu_.isOpen = false;
    menu_.isEditing = false;
    menu_.sectionStack.clear();
    menu_.currentSectionId = MakeMenuRootSectionId(nodeId_);
    menu_.currentSelection = 0;
    menuSelections_[MakeMenuRootSectionId(nodeId_)] = 0;
    menuSelections_[MakeMenuSectionId(nodeId_, "pages")] = 0;
    menuSelections_[MakeMenuSectionId(nodeId_, "model")] = 0;
    menuSelections_[MakeMenuSectionId(nodeId_, "utilities")] = 0;
    menuSelections_[MakeMenuSectionId(nodeId_, "info")] = 0;
}

float BraidsCore::GetMenuStepSize(const std::string& itemId) const
{
    if(itemId == MakeMenuItemId(nodeId_, "pages", "page"))
    {
        return 1.0f;
    }
    if(itemId == MakeMenuItemId(nodeId_, "model", "model"))
    {
        return 1.0f / 5.0f;
    }
    return 0.05f;
}

void BraidsCore::MoveSelection(int delta)
{
    for(auto& section : menu_.sections)
    {
        if(section.id == menu_.currentSectionId)
        {
            const int maxIndex = static_cast<int>(section.items.size()) - 1;
            section.selectedIndex = std::clamp(section.selectedIndex + delta, 0, maxIndex);
            menu_.currentSelection = section.selectedIndex;
            menuSelections_[section.id] = section.selectedIndex;
            break;
        }
    }
}

const MenuItem* BraidsCore::GetSelectedMenuItem() const
{
    for(const auto& section : menu_.sections)
    {
        if(section.id == menu_.currentSectionId
           && section.selectedIndex >= 0
           && section.selectedIndex < static_cast<int>(section.items.size()))
        {
            return &section.items[static_cast<std::size_t>(section.selectedIndex)];
        }
    }
    return nullptr;
}

MenuItem* BraidsCore::GetSelectedMenuItem()
{
    for(auto& section : menu_.sections)
    {
        if(section.id == menu_.currentSectionId
           && section.selectedIndex >= 0
           && section.selectedIndex < static_cast<int>(section.items.size()))
        {
            return &section.items[static_cast<std::size_t>(section.selectedIndex)];
        }
    }
    return nullptr;
}

std::string BraidsCore::StripParameterId(const std::string& parameterId) const
{
    const std::string prefix = nodeId_ + "/param/";
    if(parameterId.rfind(prefix, 0) != 0)
    {
        return {};
    }
    return parameterId.substr(prefix.size());
}

std::string BraidsCore::StripControlId(const std::string& controlId) const
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
