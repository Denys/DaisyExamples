#include "daisyhost/apps/CloudSeedCore.h"

#include <algorithm>
#include <array>
#include <cmath>

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

int ClampInt(int value, int minValue, int maxValue)
{
    return std::clamp(value, minValue, maxValue);
}

float PeakForBuffer(const float* data, std::size_t frameCount)
{
    float peak = 0.0f;
    if(data == nullptr)
    {
        return peak;
    }

    for(std::size_t index = 0; index < frameCount; ++index)
    {
        peak = std::max(peak, std::abs(data[index]));
    }
    return peak;
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

std::string MakeMetaControllerId(const std::string& nodeId,
                                 const std::string& suffix)
{
    return nodeId + "/meta/" + suffix;
}

std::string OnOffText(float normalizedValue)
{
    return normalizedValue >= 0.5f ? "On" : "Off";
}

std::string FormatPageText(DaisyCloudSeedPage page)
{
    switch(page)
    {
        case DaisyCloudSeedPage::kSpace: return "Space";
        case DaisyCloudSeedPage::kMotion: return "Motion";
        case DaisyCloudSeedPage::kArp: return "Arp";
        case DaisyCloudSeedPage::kAdvanced: return "Advanced";
    }
    return "Space";
}

int PageIndex(DaisyCloudSeedPage page)
{
    return static_cast<int>(page);
}

DaisyCloudSeedPage PageFromNormalized(float normalizedValue)
{
    return static_cast<DaisyCloudSeedPage>(
        ClampInt(static_cast<int>(std::round(Clamp01(normalizedValue) * 3.0f)),
                 0,
                 3));
}

std::string FormatPerformanceValue(float normalizedValue)
{
    return std::to_string(static_cast<int>(std::round(Clamp01(normalizedValue) * 100.0f)))
           + "%";
}

int QuantizedIndex(float normalizedValue, int stepCount)
{
    if(stepCount <= 1)
    {
        return 0;
    }
    return ClampInt(static_cast<int>(std::round(Clamp01(normalizedValue)
                                               * static_cast<float>(stepCount - 1))),
                    0,
                    stepCount - 1);
}

std::string FormatArpRateText(float normalizedValue)
{
    static constexpr std::array<const char*, 4> kLabels
        = {{"1/32", "1/16", "1/8", "1/4"}};
    return kLabels[static_cast<std::size_t>(QuantizedIndex(normalizedValue, 4))];
}

std::string FormatArpPatternText(float normalizedValue)
{
    static constexpr std::array<const char*, 3> kLabels = {{"Up", "Down", "Pend"}};
    return kLabels[static_cast<std::size_t>(QuantizedIndex(normalizedValue, 3))];
}

std::string FormatArpTargetText(float normalizedValue)
{
    return normalizedValue >= 0.5f ? "Motion" : "Space";
}
} // namespace

CloudSeedCore::CloudSeedCore(const std::string& nodeId)
: nodeId_(nodeId)
{
    portInputs_[MakeAudioInputPortId(nodeId_, 1)].type  = VirtualPortType::kAudio;
    portInputs_[MakeAudioInputPortId(nodeId_, 2)].type  = VirtualPortType::kAudio;
    portOutputs_[MakeAudioOutputPortId(nodeId_, 1)].type = VirtualPortType::kAudio;
    portOutputs_[MakeAudioOutputPortId(nodeId_, 2)].type = VirtualPortType::kAudio;

    ResetMenuState();
    sharedCore_.ResetToDefaultState();
    RefreshSnapshots();
}

std::string CloudSeedCore::GetAppId() const
{
    return "cloudseed";
}

std::string CloudSeedCore::GetAppDisplayName() const
{
    return "CloudSeed";
}

HostedAppCapabilities CloudSeedCore::GetCapabilities() const
{
    HostedAppCapabilities capabilities;
    capabilities.acceptsAudioInput  = true;
    capabilities.acceptsMidiInput   = false;
    capabilities.producesMidiOutput = false;
    return capabilities;
}

HostedAppPatchBindings CloudSeedCore::GetPatchBindings() const
{
    HostedAppPatchBindings bindings;
    const auto pageBinding = sharedCore_.GetActivePageBinding();
    for(std::size_t i = 0; i < pageBinding.parameterIds.size(); ++i)
    {
        if(pageBinding.parameterIds[i].empty())
        {
            continue;
        }
        bindings.knobControlIds[i]
            = MakeControlId(nodeId_, pageBinding.parameterIds[i]);
        bindings.knobParameterIds[i]
            = MakeParameterId(nodeId_, pageBinding.parameterIds[i]);
        bindings.knobDetailLabels[i] = pageBinding.parameterLabels[i];
    }
    for(std::size_t i = 0; i < pageBinding.fieldParameterIds.size(); ++i)
    {
        if(pageBinding.fieldParameterIds[i].empty())
        {
            continue;
        }
        bindings.fieldKnobControlIds[i]
            = MakeControlId(nodeId_, pageBinding.fieldParameterIds[i]);
        bindings.fieldKnobParameterIds[i]
            = MakeParameterId(nodeId_, pageBinding.fieldParameterIds[i]);
        bindings.fieldKnobDetailLabels[i] = pageBinding.fieldParameterLabels[i];
    }
    bindings.encoderControlId       = MakeEncoderControlId(nodeId_);
    bindings.encoderButtonControlId = MakeEncoderButtonControlId(nodeId_);
    bindings.audioInputPortIds[0]   = MakeAudioInputPortId(nodeId_, 1);
    bindings.audioInputPortIds[1]   = MakeAudioInputPortId(nodeId_, 2);
    bindings.audioOutputPortIds[0]  = MakeAudioOutputPortId(nodeId_, 1);
    bindings.audioOutputPortIds[1]  = MakeAudioOutputPortId(nodeId_, 2);
    bindings.mainOutputChannels     = {0, 1};
    return bindings;
}

void CloudSeedCore::Prepare(double sampleRate, std::size_t maxBlockSize)
{
    sharedCore_.Prepare(sampleRate, maxBlockSize);
    RefreshSnapshots();
}

void CloudSeedCore::Process(const AudioBufferView&      input,
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

    const float* inputLeft
        = (input.channelCount > 0 && input.channels[0] != nullptr)
              ? input.channels[0]
              : nullptr;
    const float* inputRight
        = (input.channelCount > 1 && input.channels[1] != nullptr)
              ? input.channels[1]
              : inputLeft;

    std::vector<float> outputLeft(frameCount, 0.0f);
    std::vector<float> outputRight(frameCount, 0.0f);
    sharedCore_.Process(inputLeft,
                        inputRight,
                        outputLeft.data(),
                        outputRight.data(),
                        frameCount);

    if(output.channelCount > 0 && output.channels[0] != nullptr)
    {
        std::copy(outputLeft.begin(), outputLeft.end(), output.channels[0]);
    }
    if(output.channelCount > 1 && output.channels[1] != nullptr)
    {
        std::copy(outputRight.begin(), outputRight.end(), output.channels[1]);
    }

    PortValue leftInput;
    leftInput.type   = VirtualPortType::kAudio;
    leftInput.scalar = PeakForBuffer(inputLeft, frameCount);
    portInputs_[MakeAudioInputPortId(nodeId_, 1)] = leftInput;

    PortValue rightInput;
    rightInput.type   = VirtualPortType::kAudio;
    rightInput.scalar = PeakForBuffer(inputRight, frameCount);
    portInputs_[MakeAudioInputPortId(nodeId_, 2)] = rightInput;

    PortValue leftOutput;
    leftOutput.type   = VirtualPortType::kAudio;
    leftOutput.scalar = PeakForBuffer(outputLeft.data(), frameCount);
    portOutputs_[MakeAudioOutputPortId(nodeId_, 1)] = leftOutput;

    PortValue rightOutput;
    rightOutput.type   = VirtualPortType::kAudio;
    rightOutput.scalar = PeakForBuffer(outputRight.data(), frameCount);
    portOutputs_[MakeAudioOutputPortId(nodeId_, 2)] = rightOutput;
}

void CloudSeedCore::SetControl(const std::string& controlId, float normalizedValue)
{
    if(controlId == MakeEncoderButtonControlId(nodeId_))
    {
        SetEncoderPress(normalizedValue >= 0.5f);
        return;
    }

    const std::string parameterSuffix = StripControlId(controlId);
    if(parameterSuffix.empty())
    {
        return;
    }

    if(sharedCore_.SetParameterValue(parameterSuffix, normalizedValue))
    {
        RefreshSnapshots();
    }
}

void CloudSeedCore::SetEncoderDelta(int delta)
{
    MenuRotate(delta);
}

void CloudSeedCore::SetEncoderPress(bool pressed)
{
    if(pressed && !encoderPressed_)
    {
        MenuPress();
    }
    encoderPressed_ = pressed;
}

void CloudSeedCore::SetPortInput(const std::string& portId, const PortValue& value)
{
    portInputs_[portId] = value;
}

PortValue CloudSeedCore::GetPortOutput(const std::string& portId) const
{
    if(const auto outputIt = portOutputs_.find(portId);
       outputIt != portOutputs_.end())
    {
        return outputIt->second;
    }
    if(const auto inputIt = portInputs_.find(portId); inputIt != portInputs_.end())
    {
        return inputIt->second;
    }
    return PortValue();
}

void CloudSeedCore::TickUi(double)
{
}

bool CloudSeedCore::SetParameterValue(const std::string& parameterId,
                                      float              normalizedValue)
{
    const std::string suffix = StripParameterId(parameterId);
    if(suffix.empty())
    {
        return false;
    }

    const bool changed = sharedCore_.SetParameterValue(suffix, normalizedValue);
    if(changed)
    {
        RefreshSnapshots();
    }
    return changed;
}

bool CloudSeedCore::SetEffectiveParameterValue(const std::string& parameterId,
                                               float normalizedValue)
{
    const std::string suffix = StripParameterId(parameterId);
    if(suffix.empty())
    {
        return false;
    }

    const bool changed
        = sharedCore_.SetEffectiveParameterValue(suffix, normalizedValue);
    if(changed)
    {
        RefreshSnapshots();
    }
    return changed;
}

ParameterValueLookup CloudSeedCore::GetControlValue(const std::string& controlId) const
{
    const std::string suffix = StripControlId(controlId);
    if(suffix.empty())
    {
        return {};
    }

    float value = 0.0f;
    if(!sharedCore_.GetParameterValue(suffix, &value))
    {
        return {};
    }
    return {true, value};
}

ParameterValueLookup CloudSeedCore::GetParameterValue(
    const std::string& parameterId) const
{
    const std::string suffix = StripParameterId(parameterId);
    if(suffix.empty())
    {
        return {};
    }

    float value = 0.0f;
    if(!sharedCore_.GetParameterValue(suffix, &value))
    {
        return {};
    }
    return {true, value};
}

ParameterValueLookup CloudSeedCore::GetEffectiveParameterValue(
    const std::string& parameterId) const
{
    const std::string suffix = StripParameterId(parameterId);
    if(suffix.empty())
    {
        return {};
    }

    float value = 0.0f;
    if(!sharedCore_.GetEffectiveParameterValue(suffix, &value))
    {
        return {};
    }
    return {true, value};
}

const std::vector<MetaControllerDescriptor>& CloudSeedCore::GetMetaControllers() const
{
    return metaControllers_;
}

bool CloudSeedCore::SetMetaControllerValue(const std::string& controllerId,
                                           float              normalizedValue)
{
    const float value = Clamp01(normalizedValue);

    if(controllerId == MakeMetaControllerId(nodeId_, "blend"))
    {
        sharedCore_.SetParameterValue("mix", value);
    }
    else if(controllerId == MakeMetaControllerId(nodeId_, "space"))
    {
        sharedCore_.SetParameterValue("size", 0.10f + (0.85f * value));
        sharedCore_.SetParameterValue("decay", 0.05f + (0.90f * value));
        sharedCore_.SetParameterValue("pre_delay", 0.10f + (0.70f * value));
    }
    else if(controllerId == MakeMetaControllerId(nodeId_, "motion"))
    {
        sharedCore_.SetParameterValue("mod_amount", value);
        sharedCore_.SetParameterValue("mod_rate", 0.10f + (0.80f * value));
    }
    else if(controllerId == MakeMetaControllerId(nodeId_, "tone"))
    {
        sharedCore_.SetParameterValue("diffusion", 0.10f + (0.85f * value));
        sharedCore_.SetParameterValue("damping", 0.90f - (0.80f * value));
    }
    else
    {
        return false;
    }

    RefreshSnapshots();
    return true;
}

ParameterValueLookup CloudSeedCore::GetMetaControllerValue(
    const std::string& controllerId) const
{
    const auto it = std::find_if(metaControllers_.begin(),
                                 metaControllers_.end(),
                                 [&controllerId](const MetaControllerDescriptor& controller) {
                                     return controller.id == controllerId;
                                 });
    if(it == metaControllers_.end())
    {
        return {};
    }

    return {true, it->normalizedValue};
}

void CloudSeedCore::ResetToDefaultState(std::uint32_t seed)
{
    sharedCore_.ResetToDefaultState(seed);
    ResetMenuState();
    RefreshSnapshots();
}

std::unordered_map<std::string, float>
CloudSeedCore::CaptureStatefulParameterValues() const
{
    std::unordered_map<std::string, float> values;
    for(const auto& entry : sharedCore_.CaptureStatefulParameterValues())
    {
        values.emplace(MakeParameterId(nodeId_, entry.first), entry.second);
    }
    return values;
}

void CloudSeedCore::RestoreStatefulParameterValues(
    const std::unordered_map<std::string, float>& values)
{
    std::unordered_map<std::string, float> strippedValues;
    for(const auto& entry : values)
    {
        const std::string suffix = StripParameterId(entry.first);
        if(!suffix.empty())
        {
            strippedValues.emplace(suffix, entry.second);
        }
    }
    sharedCore_.RestoreStatefulParameterValues(strippedValues);
    RefreshSnapshots();
}

const std::vector<ParameterDescriptor>& CloudSeedCore::GetParameters() const
{
    return parameters_;
}

const MenuModel& CloudSeedCore::GetMenuModel() const
{
    return menu_;
}

void CloudSeedCore::MenuRotate(int delta)
{
    if(delta == 0)
    {
        RefreshSnapshots();
        return;
    }

    if(!menu_.isOpen)
    {
        return;
    }

    if(menu_.isEditing)
    {
        const MenuItem* selected = GetSelectedMenuItem();
        if(selected != nullptr && selected->editable
           && selected->actionKind == MenuItemActionKind::kValue)
        {
            SetMenuItemValue(selected->id,
                             selected->normalizedValue
                                 + GetMenuStepSize(selected->id)
                                       * static_cast<float>(delta));
        }
        return;
    }

    MoveSelection(delta);
    RefreshSnapshots();
}

void CloudSeedCore::MenuPress()
{
    if(!menu_.isOpen)
    {
        menu_.isOpen          = true;
        menu_.isEditing       = false;
        menu_.currentSectionId = MakeMenuRootSectionId(nodeId_);
        menu_.sectionStack     = {MakeMenuRootSectionId(nodeId_)};
        menu_.currentSelection = menuSelections_[menu_.currentSectionId];
        RefreshSnapshots();
        return;
    }

    const MenuItem* item = GetSelectedMenuItem();
    if(item == nullptr)
    {
        menu_.isOpen    = false;
        menu_.isEditing = false;
        RefreshSnapshots();
        return;
    }

    if(menu_.isEditing)
    {
        menu_.isEditing = false;
        RefreshSnapshots();
        return;
    }

    switch(item->actionKind)
    {
        case MenuItemActionKind::kEnterSection:
            menu_.currentSectionId = item->targetSectionId;
            menu_.sectionStack.push_back(item->targetSectionId);
            menu_.currentSelection = menuSelections_[item->targetSectionId];
            break;

        case MenuItemActionKind::kBack:
            if(menu_.sectionStack.size() > 1)
            {
                menu_.sectionStack.pop_back();
                menu_.currentSectionId = menu_.sectionStack.back();
                menu_.currentSelection = menuSelections_[menu_.currentSectionId];
            }
            else
            {
                menu_.isOpen    = false;
                menu_.isEditing = false;
            }
            break;

        case MenuItemActionKind::kValue:
            menu_.isEditing = true;
            break;

        case MenuItemActionKind::kMomentary:
            SetMenuItemValue(item->id, 1.0f);
            return;

        case MenuItemActionKind::kReadonly:
        default: break;
    }

    RefreshSnapshots();
}

void CloudSeedCore::SetMenuItemValue(const std::string& itemId,
                                     float              normalizedValue)
{
    const float clamped = Clamp01(normalizedValue);
    if(itemId == MakeMenuItemId(nodeId_, "macros", "blend"))
    {
        SetMetaControllerValue(MakeMetaControllerId(nodeId_, "blend"), clamped);
        return;
    }
    else if(itemId == MakeMenuItemId(nodeId_, "macros", "space"))
    {
        SetMetaControllerValue(MakeMetaControllerId(nodeId_, "space"), clamped);
        return;
    }
    else if(itemId == MakeMenuItemId(nodeId_, "macros", "motion"))
    {
        SetMetaControllerValue(MakeMetaControllerId(nodeId_, "motion"), clamped);
        return;
    }
    else if(itemId == MakeMenuItemId(nodeId_, "macros", "tone"))
    {
        SetMetaControllerValue(MakeMetaControllerId(nodeId_, "tone"), clamped);
        return;
    }
    else if(itemId == MakeMenuItemId(nodeId_, "pages", "page"))
    {
        sharedCore_.SetActivePage(PageFromNormalized(clamped));
    }
    else if(itemId == MakeMenuItemId(nodeId_, "arp", "enabled"))
    {
        sharedCore_.SetParameterValue("arp_enabled", clamped);
    }
    else if(itemId == MakeMenuItemId(nodeId_, "arp", "rate"))
    {
        sharedCore_.SetParameterValue("arp_rate", clamped);
    }
    else if(itemId == MakeMenuItemId(nodeId_, "arp", "pattern"))
    {
        sharedCore_.SetParameterValue("arp_pattern", clamped);
    }
    else if(itemId == MakeMenuItemId(nodeId_, "arp", "target"))
    {
        sharedCore_.SetParameterValue("arp_target", clamped);
    }
    else if(itemId == MakeMenuItemId(nodeId_, "arp", "depth"))
    {
        sharedCore_.SetParameterValue("arp_depth", clamped);
    }
    else if(itemId == MakeMenuItemId(nodeId_, "program", "program"))
    {
        sharedCore_.SetParameterValue("program", clamped);
    }
    else if(itemId == MakeMenuItemId(nodeId_, "utilities", "bypass"))
    {
        sharedCore_.SetParameterValue("bypass", clamped);
    }
    else if(itemId == MakeMenuItemId(nodeId_, "utilities", "clear_tails"))
    {
        sharedCore_.TriggerMomentaryAction("clear_tails");
    }
    else if(itemId == MakeMenuItemId(nodeId_, "utilities", "randomize_seeds"))
    {
        sharedCore_.TriggerMomentaryAction("randomize_seeds");
    }
    else if(itemId == MakeMenuItemId(nodeId_, "utilities", "interpolation"))
    {
        sharedCore_.SetParameterValue("global_interpolation", clamped);
    }
    else
    {
        return;
    }

    RefreshSnapshots();
}

const DisplayModel& CloudSeedCore::GetDisplayModel() const
{
    return display_;
}

std::string CloudSeedCore::MakeParameterId(const std::string& nodeId,
                                           const std::string& suffix)
{
    return nodeId + "/param/" + suffix;
}

std::string CloudSeedCore::MakeControlId(const std::string& nodeId,
                                         const std::string& suffix)
{
    return nodeId + "/control/" + suffix;
}

std::string CloudSeedCore::MakeEncoderControlId(const std::string& nodeId)
{
    return nodeId + "/control/encoder";
}

std::string CloudSeedCore::MakeEncoderButtonControlId(const std::string& nodeId)
{
    return nodeId + "/control/encoder_button";
}

std::string CloudSeedCore::MakeAudioInputPortId(const std::string& nodeId,
                                                std::size_t       oneBasedIndex)
{
    return nodeId + "/port/audio_in_" + std::to_string(oneBasedIndex);
}

std::string CloudSeedCore::MakeAudioOutputPortId(const std::string& nodeId,
                                                 std::size_t       oneBasedIndex)
{
    return nodeId + "/port/audio_out_" + std::to_string(oneBasedIndex);
}

void CloudSeedCore::RefreshSnapshots()
{
    RefreshParameters();
    RefreshMetaControllers();
    BuildMenuModel();
    BuildDisplay();
}

void CloudSeedCore::RefreshParameters()
{
    parameters_.clear();
    parameters_.reserve(sharedCore_.GetParameters().size());

    for(const auto& parameter : sharedCore_.GetParameters())
    {
        ParameterDescriptor descriptor;
        descriptor.id                      = MakeParameterId(nodeId_, parameter.id);
        descriptor.label                   = parameter.label;
        descriptor.normalizedValue         = parameter.normalizedValue;
        descriptor.defaultNormalizedValue  = parameter.defaultNormalizedValue;
        descriptor.effectiveNormalizedValue = parameter.effectiveNormalizedValue;
        descriptor.stepCount               = parameter.stepCount;
        descriptor.importanceRank          = parameter.importanceRank;
        descriptor.automatable             = parameter.automatable;
        descriptor.stateful                = parameter.stateful;
        descriptor.menuEditable            = false;
        descriptor.role = parameter.id == "mix" ? ParameterRole::kMix
                                                 : ParameterRole::kGeneric;
        parameters_.push_back(std::move(descriptor));
    }
}

void CloudSeedCore::RefreshMetaControllers()
{
    auto getParameterValue = [this](const char* parameterId) {
        float value = 0.0f;
        sharedCore_.GetParameterValue(parameterId, &value);
        return value;
    };
    auto getDefaultValue = [this](const char* parameterId) {
        const auto* parameter = sharedCore_.FindParameter(parameterId);
        return parameter != nullptr ? parameter->defaultNormalizedValue : 0.0f;
    };

    metaControllers_.clear();
    metaControllers_.reserve(4);
    metaControllers_.push_back(
        {MakeMetaControllerId(nodeId_, "blend"),
         "Blend",
         getParameterValue("mix"),
         getDefaultValue("mix"),
         true});
    metaControllers_.push_back(
        {MakeMetaControllerId(nodeId_, "space"),
         "Space",
         Clamp01((getParameterValue("size") - 0.10f) / 0.85f),
         Clamp01((getDefaultValue("size") - 0.10f) / 0.85f),
         true});
    metaControllers_.push_back(
        {MakeMetaControllerId(nodeId_, "motion"),
         "Motion",
         getParameterValue("mod_amount"),
         getDefaultValue("mod_amount"),
         true});
    metaControllers_.push_back(
        {MakeMetaControllerId(nodeId_, "tone"),
         "Tone",
         Clamp01((getParameterValue("diffusion") - 0.10f) / 0.85f),
         Clamp01((getDefaultValue("diffusion") - 0.10f) / 0.85f),
         true});
}

void CloudSeedCore::BuildMenuModel()
{
    menu_.sections.clear();

    auto addSection = [this](const std::string& id,
                             const std::string& title) -> MenuSection& {
        menu_.sections.push_back(MenuSection{id, title, {}, menuSelections_[id]});
        return menu_.sections.back();
    };

    const auto rootId      = MakeMenuRootSectionId(nodeId_);
    const auto pagesId     = MakeMenuSectionId(nodeId_, "pages");
    const auto macrosId    = MakeMenuSectionId(nodeId_, "macros");
    const auto arpId       = MakeMenuSectionId(nodeId_, "arp");
    const auto programId   = MakeMenuSectionId(nodeId_, "program");
    const auto utilitiesId = MakeMenuSectionId(nodeId_, "utilities");
    const auto infoId      = MakeMenuSectionId(nodeId_, "info");

    MenuSection& root = addSection(rootId, "Menu");
    root.items = {
        {MakeMenuItemId(nodeId_, "root", "pages"),
         "Pages",
         false,
         MenuItemActionKind::kEnterSection,
         0.0f,
         "",
         pagesId},
        {MakeMenuItemId(nodeId_, "root", "macros"),
         "Macros",
         false,
         MenuItemActionKind::kEnterSection,
         0.0f,
         "",
         macrosId},
        {MakeMenuItemId(nodeId_, "root", "arp"),
         "Arp",
         false,
         MenuItemActionKind::kEnterSection,
         0.0f,
         "",
         arpId},
        {MakeMenuItemId(nodeId_, "root", "program"),
         "Program",
         false,
         MenuItemActionKind::kEnterSection,
         0.0f,
         "",
         programId},
        {MakeMenuItemId(nodeId_, "root", "utilities"),
         "Utilities",
         false,
         MenuItemActionKind::kEnterSection,
         0.0f,
         "",
         utilitiesId},
        {MakeMenuItemId(nodeId_, "root", "info"),
         "Info",
         false,
         MenuItemActionKind::kEnterSection,
         0.0f,
         "",
         infoId},
    };

    MenuSection& pages = addSection(pagesId, "Pages");
    pages.items = {
        {MakeMenuItemId(nodeId_, "pages", "back"),
         "Back",
         false,
         MenuItemActionKind::kBack},
        {MakeMenuItemId(nodeId_, "pages", "page"),
         "Page",
         true,
         MenuItemActionKind::kValue,
         static_cast<float>(PageIndex(sharedCore_.GetActivePage())) / 3.0f,
         FormatPageText(sharedCore_.GetActivePage()),
         ""},
    };

    MenuSection& macros = addSection(macrosId, "Macros");
    macros.items = {
        {MakeMenuItemId(nodeId_, "macros", "back"),
         "Back",
         false,
         MenuItemActionKind::kBack},
        {MakeMenuItemId(nodeId_, "macros", "blend"),
         "Blend",
         true,
         MenuItemActionKind::kValue,
         metaControllers_[0].normalizedValue,
         FormatPerformanceValue(metaControllers_[0].normalizedValue),
         ""},
        {MakeMenuItemId(nodeId_, "macros", "space"),
         "Space",
         true,
         MenuItemActionKind::kValue,
         metaControllers_[1].normalizedValue,
         FormatPerformanceValue(metaControllers_[1].normalizedValue),
         ""},
        {MakeMenuItemId(nodeId_, "macros", "motion"),
         "Motion",
         true,
         MenuItemActionKind::kValue,
         metaControllers_[2].normalizedValue,
         FormatPerformanceValue(metaControllers_[2].normalizedValue),
         ""},
        {MakeMenuItemId(nodeId_, "macros", "tone"),
         "Tone",
         true,
         MenuItemActionKind::kValue,
         metaControllers_[3].normalizedValue,
         FormatPerformanceValue(metaControllers_[3].normalizedValue),
         ""},
    };

    float arpEnabledValue = 0.0f;
    float arpRateValue = 0.0f;
    float arpPatternValue = 0.0f;
    float arpDepthValue = 0.0f;
    float arpTargetValue = 0.0f;
    sharedCore_.GetParameterValue("arp_enabled", &arpEnabledValue);
    sharedCore_.GetParameterValue("arp_rate", &arpRateValue);
    sharedCore_.GetParameterValue("arp_pattern", &arpPatternValue);
    sharedCore_.GetParameterValue("arp_depth", &arpDepthValue);
    sharedCore_.GetParameterValue("arp_target", &arpTargetValue);

    MenuSection& arp = addSection(arpId, "Arp");
    arp.items = {
        {MakeMenuItemId(nodeId_, "arp", "back"),
         "Back",
         false,
         MenuItemActionKind::kBack},
        {MakeMenuItemId(nodeId_, "arp", "enabled"),
         "Enabled",
         true,
         MenuItemActionKind::kValue,
         arpEnabledValue,
         OnOffText(arpEnabledValue),
         ""},
        {MakeMenuItemId(nodeId_, "arp", "rate"),
         "Rate",
         true,
         MenuItemActionKind::kValue,
         arpRateValue,
         FormatArpRateText(arpRateValue),
         ""},
        {MakeMenuItemId(nodeId_, "arp", "pattern"),
         "Pattern",
         true,
         MenuItemActionKind::kValue,
         arpPatternValue,
         FormatArpPatternText(arpPatternValue),
         ""},
        {MakeMenuItemId(nodeId_, "arp", "target"),
         "Target",
         true,
         MenuItemActionKind::kValue,
         arpTargetValue,
         FormatArpTargetText(arpTargetValue),
         ""},
        {MakeMenuItemId(nodeId_, "arp", "depth"),
         "Depth",
         true,
         MenuItemActionKind::kValue,
         arpDepthValue,
         FormatPerformanceValue(arpDepthValue),
         ""},
    };

    MenuSection& program = addSection(programId, "Program");
    program.items = {
        {MakeMenuItemId(nodeId_, "program", "back"),
         "Back",
         false,
         MenuItemActionKind::kBack},
        {MakeMenuItemId(nodeId_, "program", "program"),
         "Program",
         true,
         MenuItemActionKind::kValue,
         0.0f,
         sharedCore_.GetProgramLabel(),
         ""},
    };

    float bypassValue = 0.0f;
    sharedCore_.GetParameterValue("bypass", &bypassValue);
    float interpolationValue = 0.0f;
    sharedCore_.GetParameterValue("global_interpolation", &interpolationValue);

    MenuSection& utilities = addSection(utilitiesId, "Utilities");
    utilities.items = {
        {MakeMenuItemId(nodeId_, "utilities", "back"),
         "Back",
         false,
         MenuItemActionKind::kBack},
        {MakeMenuItemId(nodeId_, "utilities", "bypass"),
         "Bypass",
         true,
         MenuItemActionKind::kValue,
         bypassValue,
         OnOffText(bypassValue),
         ""},
        {MakeMenuItemId(nodeId_, "utilities", "clear_tails"),
         "Clear Tails",
         false,
         MenuItemActionKind::kMomentary,
         0.0f,
         "Press",
         ""},
        {MakeMenuItemId(nodeId_, "utilities", "randomize_seeds"),
         "Randomize Seeds",
         false,
         MenuItemActionKind::kMomentary,
         0.0f,
         "Press",
         ""},
        {MakeMenuItemId(nodeId_, "utilities", "interpolation"),
         "Interpolation",
         true,
         MenuItemActionKind::kValue,
         interpolationValue,
         OnOffText(interpolationValue),
         ""},
    };

    MenuSection& info = addSection(infoId, "Info");
    info.items = {
        {MakeMenuItemId(nodeId_, "info", "back"),
         "Back",
         false,
         MenuItemActionKind::kBack},
        {MakeMenuItemId(nodeId_, "info", "program"),
         "Program",
         false,
         MenuItemActionKind::kReadonly,
         0.0f,
         sharedCore_.GetProgramLabel(),
         ""},
        {MakeMenuItemId(nodeId_, "info", "page"),
         "Page",
         false,
         MenuItemActionKind::kReadonly,
         0.0f,
         FormatPageText(sharedCore_.GetActivePage()),
         ""},
        {MakeMenuItemId(nodeId_, "info", "arp"),
         "Arp",
         false,
         MenuItemActionKind::kReadonly,
         0.0f,
         OnOffText(arpEnabledValue) + " " + FormatArpTargetText(arpTargetValue),
         ""},
        {MakeMenuItemId(nodeId_, "info", "seeds"),
         "Seeds",
         false,
         MenuItemActionKind::kReadonly,
         0.0f,
         sharedCore_.GetSeedSummary(),
         ""},
    };

    for(auto& section : menu_.sections)
    {
        if(section.items.empty())
        {
            section.selectedIndex = 0;
            continue;
        }
        section.selectedIndex = ClampInt(menuSelections_[section.id],
                                         0,
                                         static_cast<int>(section.items.size()) - 1);
        menuSelections_[section.id] = section.selectedIndex;
    }

    if(menu_.sectionStack.empty())
    {
        menu_.sectionStack = {rootId};
    }
    if(menu_.currentSectionId.empty())
    {
        menu_.currentSectionId = rootId;
    }

    const auto currentIt = std::find_if(menu_.sections.begin(),
                                        menu_.sections.end(),
                                        [this](const MenuSection& section) {
                                            return section.id == menu_.currentSectionId;
                                        });
    if(currentIt == menu_.sections.end())
    {
        menu_.currentSectionId = rootId;
        menu_.currentSelection = menuSelections_[rootId];
    }
    else
    {
        menu_.currentSelection = currentIt->selectedIndex;
    }
}

void CloudSeedCore::BuildDisplay()
{
    display_.width  = 128;
    display_.height = 64;
    ++display_.revision;
    display_.texts.clear();
    display_.bars.clear();

    if(!menu_.isOpen)
    {
        display_.mode  = DisplayMode::kStatus;
        display_.title = "CloudSeed";
        const auto binding = sharedCore_.GetActivePageBinding();
        float arpEnabledValue = 0.0f;
        sharedCore_.GetParameterValue("arp_enabled", &arpEnabledValue);
        display_.texts.push_back({0, 0, "CloudSeed", false});
        display_.texts.push_back(
            {0,
             12,
             binding.pageLabel + " / " + sharedCore_.GetProgramLabel()
                 + (arpEnabledValue >= 0.5f ? " / Arp" : ""),
             false});
        for(std::size_t index = 0; index < binding.parameterIds.size(); ++index)
        {
            float value = 0.0f;
            sharedCore_.GetParameterValue(binding.parameterIds[index], &value);
            display_.texts.push_back(
                {0,
                 24 + static_cast<int>(index * 10),
                 binding.parameterLabels[index] + " " + FormatPerformanceValue(value),
                 false});
        }
        return;
    }

    display_.mode = DisplayMode::kMenu;

    const auto sectionIt = std::find_if(menu_.sections.begin(),
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
    const int maxStart  = std::max(0, itemCount - static_cast<int>(kVisibleMenuRows));
    const int startIndex
        = ClampInt(menu_.currentSelection - 1, 0, maxStart);
    const int endIndex
        = std::min(itemCount, startIndex + static_cast<int>(kVisibleMenuRows));

    for(int itemIndex = startIndex; itemIndex < endIndex; ++itemIndex)
    {
        const auto& item = sectionIt->items[static_cast<std::size_t>(itemIndex)];
        const bool  selected = itemIndex == menu_.currentSelection;

        std::string line = selected ? (menu_.isEditing ? "* " : "> ") : "  ";
        line += item.label;
        if(!item.valueText.empty())
        {
            line += ": " + item.valueText;
        }
        display_.texts.push_back(
            {0, 14 + ((itemIndex - startIndex) * 12), line, false});
    }
}

void CloudSeedCore::ResetMenuState()
{
    menu_ = {};
    menu_.currentSectionId = MakeMenuRootSectionId(nodeId_);
    menu_.sectionStack     = {menu_.currentSectionId};
    menuSelections_.clear();
    menuSelections_[MakeMenuRootSectionId(nodeId_)]      = 0;
    menuSelections_[MakeMenuSectionId(nodeId_, "pages")] = 0;
    menuSelections_[MakeMenuSectionId(nodeId_, "macros")] = 0;
    menuSelections_[MakeMenuSectionId(nodeId_, "arp")] = 0;
    menuSelections_[MakeMenuSectionId(nodeId_, "program")] = 0;
    menuSelections_[MakeMenuSectionId(nodeId_, "utilities")] = 0;
    menuSelections_[MakeMenuSectionId(nodeId_, "info")] = 0;
}

float CloudSeedCore::GetMenuStepSize(const std::string& itemId) const
{
    if(itemId.rfind(nodeId_ + "/menu/macros/", 0) == 0)
    {
        return 0.05f;
    }
    if(itemId == MakeMenuItemId(nodeId_, "arp", "depth"))
    {
        return 0.05f;
    }
    if(itemId == MakeMenuItemId(nodeId_, "pages", "page"))
    {
        return 1.0f / 3.0f;
    }
    return 1.0f;
}

void CloudSeedCore::MoveSelection(int delta)
{
    const auto sectionIt = std::find_if(menu_.sections.begin(),
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

const MenuItem* CloudSeedCore::GetSelectedMenuItem() const
{
    const auto sectionIt = std::find_if(menu_.sections.begin(),
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

MenuItem* CloudSeedCore::GetSelectedMenuItem()
{
    auto* self = const_cast<const CloudSeedCore*>(this);
    return const_cast<MenuItem*>(self->GetSelectedMenuItem());
}

std::string CloudSeedCore::StripParameterId(const std::string& parameterId) const
{
    const std::string prefix = nodeId_ + "/param/";
    if(parameterId.rfind(prefix, 0) != 0)
    {
        return {};
    }
    return parameterId.substr(prefix.size());
}

std::string CloudSeedCore::StripControlId(const std::string& controlId) const
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
