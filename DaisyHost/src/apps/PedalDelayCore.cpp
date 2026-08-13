#include "daisyhost/apps/PedalDelayCore.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace daisyhost
{
namespace apps
{
    namespace
    {
        float Clamp01(float value)
        {
            return std::clamp(std::isfinite(value) ? value : 0.0f, 0.0f, 1.0f);
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

        ParameterRole RoleForSlot(PedalSlot slot)
        {
            switch(slot)
            {
                case PedalSlot::kTime: return ParameterRole::kPrimaryDelay;
                case PedalSlot::kFeedback: return ParameterRole::kFeedback;
                case PedalSlot::kMix: return ParameterRole::kMix;
                default: return ParameterRole::kGeneric;
            }
        }

        std::string FormatValue(float value, const char* unit)
        {
            char buffer[48];
            const float magnitude = std::abs(value);
            const int   precision = magnitude >= 100.0f ? 0
                                    : magnitude >= 10.0f ? 1
                                                         : 2;
            std::snprintf(buffer,
                          sizeof(buffer),
                          "%.*f %s",
                          precision,
                          static_cast<double>(value),
                          unit);
            return buffer;
        }

        std::string JoinTargets(const PedalSlotDescriptor& descriptor)
        {
            std::string joined;
            for(const char* target : descriptor.dspTargets)
            {
                if(target == nullptr || target[0] == '\0')
                {
                    continue;
                }
                if(!joined.empty())
                {
                    joined += ", ";
                }
                joined += target;
            }
            return joined;
        }

        constexpr std::size_t kModeKeyBase   = 3; // A4..A8
        constexpr std::size_t kFreezeKeyBase = 8; // B1..B5
    } // namespace

    PedalDelayCore::PedalDelayCore(const std::string& nodeId)
    : nodeId_(nodeId),
      gateInputPortId_(MakeGateInputPortId(nodeId)),
      audioOutputPortId1_(MakeAudioOutputPortId(nodeId, 1)),
      audioOutputPortId2_(MakeAudioOutputPortId(nodeId, 2))
    {
        portInputs_[MakeAudioInputPortId(nodeId_, 1)].type
            = VirtualPortType::kAudio;
        portInputs_[MakeAudioInputPortId(nodeId_, 2)].type
            = VirtualPortType::kAudio;
        portInputs_[MakeGateInputPortId(nodeId_)].type = VirtualPortType::kGate;
        portInputs_[MakeMidiInputPortId(nodeId_)].type = VirtualPortType::kMidi;
        for(std::size_t i = 1; i <= 4; ++i)
        {
            portInputs_[MakeCvInputPortId(nodeId_, i)].type
                = VirtualPortType::kCv;
        }
        portOutputs_[MakeAudioOutputPortId(nodeId_, 1)].type
            = VirtualPortType::kAudio;
        portOutputs_[MakeAudioOutputPortId(nodeId_, 2)].type
            = VirtualPortType::kAudio;

        menu_.currentSectionId = MakeMenuSectionId(nodeId_, "root");
        ResetToDefaultState(0);
    }

    std::string PedalDelayCore::GetAppId() const
    {
        return "pedal_multidelay";
    }

    std::string PedalDelayCore::GetAppDisplayName() const
    {
        return "Pedal Multi-Delay";
    }

    HostedAppCapabilities PedalDelayCore::GetCapabilities() const
    {
        HostedAppCapabilities capabilities;
        capabilities.acceptsAudioInput  = true;
        capabilities.acceptsMidiInput   = false;
        capabilities.producesMidiOutput = false;
        return capabilities;
    }

    HostedAppPatchBindings PedalDelayCore::GetPatchBindings() const
    {
        HostedAppPatchBindings bindings;
        const auto slotIds = GetPerformanceSlotParameterIds();

        // Patch exposes four knobs; the fifth slot lives on the encoder menu.
        for(std::size_t slot = 0; slot < 4; ++slot)
        {
            const auto& descriptor = GetPedalSlotDescriptor(
                engine_.GetMode(), static_cast<PedalSlot>(slot));
            bindings.knobControlIds[slot]
                = MakeControlId(nodeId_,
                                SlotSuffix(static_cast<PedalSlot>(slot)));
            bindings.knobParameterIds[slot] = slotIds[slot];
            bindings.knobDetailLabels[slot]
                = std::string(descriptor.musicianLabel) + " - "
                  + descriptor.engineeringLabel;
        }

        // Field exposes eight knobs; all five slots fit natively.
        for(std::size_t slot = 0; slot < kPedalSlotCount; ++slot)
        {
            const auto& descriptor = GetPedalSlotDescriptor(
                engine_.GetMode(), static_cast<PedalSlot>(slot));
            bindings.fieldKnobControlIds[slot]
                = MakeControlId(nodeId_,
                                SlotSuffix(static_cast<PedalSlot>(slot)));
            bindings.fieldKnobParameterIds[slot] = slotIds[slot];
            bindings.fieldKnobDetailLabels[slot]
                = std::string(descriptor.musicianLabel) + " - "
                  + descriptor.engineeringLabel;
        }

        static const std::array<const char*, 16> kKeyItems = {{
            "bypass",
            "tap",
            "trails",
            "digi",
            "tape",
            "mod",
            "rev",
            "freeze",
            "capture",
            "hold",
            "accumulate",
            "replace",
            "clear",
            "",
            "",
            "",
        }};
        static const std::array<const char*, 16> kKeyLabels = {{
            "A1 Bypass",
            "A2 Tap/Hold",
            "A3 Trails",
            "A4 DIGI",
            "A5 TAPE",
            "A6 MOD",
            "A7 REV",
            "A8 FREEZE",
            "B1 Freeze capture",
            "B2 Freeze hold",
            "B3 Freeze accumulate",
            "B4 Freeze replace",
            "B5 Freeze clear",
            "",
            "",
            "",
        }};
        for(std::size_t i = 0; i < kKeyItems.size(); ++i)
        {
            if(kKeyItems[i][0] == '\0')
            {
                continue;
            }
            bindings.fieldKeyMenuItemIds[i]
                = MakeMenuItemId(nodeId_, "switches", kKeyItems[i]);
            bindings.fieldKeyDetailLabels[i] = kKeyLabels[i];
        }

        bindings.encoderControlId = MakeControlId(nodeId_, "encoder");
        bindings.encoderButtonControlId
            = MakeControlId(nodeId_, "encoder_button");
        for(std::size_t i = 0; i < 4; ++i)
        {
            bindings.cvInputPortIds[i] = MakeCvInputPortId(nodeId_, i + 1);
        }
        bindings.gateInputPortIds[0]   = MakeGateInputPortId(nodeId_);
        bindings.audioInputPortIds[0]  = MakeAudioInputPortId(nodeId_, 1);
        bindings.audioInputPortIds[1]  = MakeAudioInputPortId(nodeId_, 2);
        bindings.audioOutputPortIds[0] = MakeAudioOutputPortId(nodeId_, 1);
        bindings.audioOutputPortIds[1] = MakeAudioOutputPortId(nodeId_, 2);
        bindings.midiInputPortId       = MakeMidiInputPortId(nodeId_);
        bindings.mainOutputChannels    = {0, 1};
        return bindings;
    }

    void PedalDelayCore::Prepare(double sampleRate, std::size_t maxBlockSize)
    {
        const std::size_t historySamples
            = PedalDelayEngine::HistorySamplesForRate(sampleRate);
        const std::size_t freezeSamples
            = PedalDelayEngine::FreezeSamplesForRate(sampleRate);
        historyStorage_.assign(historySamples * kPedalChannelCount, 0.0f);
        freezeStorage_.assign(freezeSamples * kPedalChannelCount, 0.0f);
        engine_.AttachStorage(historyStorage_.data(),
                              historySamples,
                              freezeStorage_.data(),
                              freezeSamples);
        engine_.Prepare(sampleRate, maxBlockSize);
        scratchLeft_.assign(std::max<std::size_t>(maxBlockSize, 1), 0.0f);
        scratchRight_.assign(std::max<std::size_t>(maxBlockSize, 1), 0.0f);
        RefreshSnapshots();
    }

    void PedalDelayCore::Process(const AudioBufferView&      input,
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

        ConsumeGateInput();

        if(scratchLeft_.size() < frameCount)
        {
            scratchLeft_.assign(frameCount, 0.0f);
            scratchRight_.assign(frameCount, 0.0f);
        }

        const float* inputLeft
            = (input.channelCount > 0 && input.channels[0] != nullptr)
                  ? input.channels[0]
                  : nullptr;
        // The pedal is a mono instrument input driving a dual-mono wet path,
        // matching the Patch convention in MultiDelayCore. True stereo /
        // ping-pong routing is still an open product decision.
        const float* inputRight = inputLeft;

        engine_.Process(inputLeft,
                        inputRight,
                        scratchLeft_.data(),
                        scratchRight_.data(),
                        frameCount);

        if(output.channelCount > 0 && output.channels[0] != nullptr)
        {
            std::copy(scratchLeft_.begin(),
                      scratchLeft_.begin()
                          + static_cast<std::ptrdiff_t>(frameCount),
                      output.channels[0]);
        }
        if(output.channelCount > 1 && output.channels[1] != nullptr)
        {
            std::copy(scratchRight_.begin(),
                      scratchRight_.begin()
                          + static_cast<std::ptrdiff_t>(frameCount),
                      output.channels[1]);
        }

        // Port ids are cached so the audio path never constructs a string.
        auto& leftOutput  = portOutputs_[audioOutputPortId1_];
        leftOutput.type   = VirtualPortType::kAudio;
        leftOutput.scalar = PeakForBuffer(scratchLeft_.data(), frameCount);

        auto& rightOutput  = portOutputs_[audioOutputPortId2_];
        rightOutput.type   = VirtualPortType::kAudio;
        rightOutput.scalar = PeakForBuffer(scratchRight_.data(), frameCount);
    }

    void PedalDelayCore::SetControl(const std::string& controlId,
                                    float              normalizedValue)
    {
        const auto suffix
            = StripPrefix(controlId, nodeId_ + "/control/");
        if(suffix.empty())
        {
            return;
        }
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
        SetParameterValue(MakeParameterId(nodeId_, suffix), normalizedValue);
    }

    void PedalDelayCore::SetEncoderDelta(int delta)
    {
        MenuRotate(delta);
    }

    void PedalDelayCore::SetEncoderPress(bool pressed)
    {
        if(pressed && !encoderPressed_)
        {
            MenuPress();
        }
        encoderPressed_ = pressed;
    }

    void PedalDelayCore::SetPortInput(const std::string& portId,
                                      const PortValue&   value)
    {
        portInputs_[portId] = value;
    }

    PortValue PedalDelayCore::GetPortOutput(const std::string& portId) const
    {
        const auto it = portOutputs_.find(portId);
        return it != portOutputs_.end() ? it->second : PortValue{};
    }

    void PedalDelayCore::TickUi(double deltaMs)
    {
        uiClockMs_ += std::isfinite(deltaMs) ? deltaMs : 0.0;
        BuildDisplay();
    }

    bool PedalDelayCore::SetParameterValue(const std::string& parameterId,
                                           float              normalizedValue)
    {
        const auto suffix = StripPrefix(parameterId, nodeId_ + "/param/");
        if(suffix.empty() || !std::isfinite(normalizedValue))
        {
            return false;
        }

        if(suffix == "mode")
        {
            const std::size_t index = static_cast<std::size_t>(
                std::lround(Clamp01(normalizedValue)
                            * static_cast<float>(kPedalDelayModeCount - 1)));
            SelectMode(static_cast<PedalDelayMode>(index));
            return true;
        }
        if(suffix == "bypass")
        {
            engine_.SetBypass(normalizedValue >= 0.5f);
            RefreshSnapshots();
            return true;
        }
        if(suffix == "trails")
        {
            engine_.SetTrails(normalizedValue >= 0.5f);
            RefreshSnapshots();
            return true;
        }
        if(suffix == "tap")
        {
            if(normalizedValue >= 0.5f)
            {
                if(engine_.GetMode() == PedalDelayMode::kFreeze)
                {
                    engine_.FreezeToggleHold();
                }
                else
                {
                    engine_.Tap(uiClockMs_);
                }
                RefreshSnapshots();
            }
            return true;
        }
        if(suffix == "freeze_state")
        {
            const std::size_t stateCount
                = static_cast<std::size_t>(PedalFreezeState::kCount);
            const std::size_t index = static_cast<std::size_t>(
                std::lround(Clamp01(normalizedValue)
                            * static_cast<float>(stateCount - 1)));
            engine_.SetFreezeState(static_cast<PedalFreezeState>(index));
            RefreshSnapshots();
            return true;
        }

        for(std::size_t slot = 0; slot < kPedalSlotCount; ++slot)
        {
            if(suffix == SlotSuffix(static_cast<PedalSlot>(slot)))
            {
                ApplySlot(static_cast<PedalSlot>(slot), normalizedValue, true);
                return true;
            }
        }
        return false;
    }

    bool PedalDelayCore::SetEffectiveParameterValue(
        const std::string& parameterId,
        float              normalizedValue)
    {
        const auto suffix = StripPrefix(parameterId, nodeId_ + "/param/");
        if(suffix.empty() || !std::isfinite(normalizedValue))
        {
            return false;
        }
        for(std::size_t slot = 0; slot < kPedalSlotCount; ++slot)
        {
            if(suffix == SlotSuffix(static_cast<PedalSlot>(slot)))
            {
                ApplySlot(static_cast<PedalSlot>(slot), normalizedValue, false);
                return true;
            }
        }
        return SetParameterValue(parameterId, normalizedValue);
    }

    void PedalDelayCore::ClearEffectiveParameterOverrides()
    {
        const auto modeIndex = static_cast<std::size_t>(engine_.GetMode());
        for(std::size_t slot = 0; slot < kPedalSlotCount; ++slot)
        {
            ApplySlot(static_cast<PedalSlot>(slot),
                      baseNormalized_[modeIndex][slot],
                      false);
        }
    }

    ParameterValueLookup
    PedalDelayCore::GetControlValue(const std::string& controlId) const
    {
        const auto suffix = StripPrefix(controlId, nodeId_ + "/control/");
        if(suffix.empty())
        {
            return {};
        }
        return GetParameterValue(MakeParameterId(nodeId_, suffix));
    }

    ParameterValueLookup
    PedalDelayCore::GetParameterValue(const std::string& parameterId) const
    {
        const auto* parameter = FindParameter(parameterId);
        if(parameter == nullptr)
        {
            return {};
        }
        return {true, parameter->normalizedValue};
    }

    ParameterValueLookup PedalDelayCore::GetEffectiveParameterValue(
        const std::string& parameterId) const
    {
        const auto* parameter = FindParameter(parameterId);
        if(parameter == nullptr)
        {
            return {};
        }
        return {true, parameter->effectiveNormalizedValue};
    }

    std::array<float, 16> PedalDelayCore::GetFieldKeyLedValues() const
    {
        std::array<float, 16> values{};
        values[0] = engine_.GetBypass() ? 0.85f : 0.03f;
        values[1] = engine_.GetFreezeState() == PedalFreezeState::kHold ? 0.85f
                                                                        : 0.03f;
        values[2] = engine_.GetTrails() ? 0.55f : 0.03f;
        for(std::size_t mode = 0; mode < kPedalDelayModeCount; ++mode)
        {
            values[kModeKeyBase + mode]
                = static_cast<std::size_t>(engine_.GetMode()) == mode ? 0.85f
                                                                      : 0.03f;
        }
        const std::size_t stateCount
            = static_cast<std::size_t>(PedalFreezeState::kCount);
        for(std::size_t state = 1; state < stateCount; ++state)
        {
            values[kFreezeKeyBase + state - 1]
                = static_cast<std::size_t>(engine_.GetFreezeState()) == state
                      ? 0.85f
                      : 0.03f;
        }
        return values;
    }

    void PedalDelayCore::ResetToDefaultState(std::uint32_t)
    {
        engine_.ResetParameters();
        engine_.Reset();
        engine_.SetMode(PedalDelayMode::kDigi);
        for(std::size_t mode = 0; mode < kPedalDelayModeCount; ++mode)
        {
            for(std::size_t slot = 0; slot < kPedalSlotCount; ++slot)
            {
                const auto& descriptor
                    = GetPedalSlotDescriptor(static_cast<PedalDelayMode>(mode),
                                             static_cast<PedalSlot>(slot));
                baseNormalized_[mode][slot] = engine_.NativeToNormalized(
                    static_cast<PedalDelayMode>(mode),
                    static_cast<PedalSlot>(slot),
                    descriptor.defaultValue);
            }
        }
        lastTouchedSlot_       = PedalSlot::kTime;
        menu_                  = {};
        menu_.currentSectionId = MakeMenuSectionId(nodeId_, "root");
        RefreshSnapshots();
    }

    std::unordered_map<std::string, float>
    PedalDelayCore::CaptureStatefulParameterValues() const
    {
        std::unordered_map<std::string, float> values;
        values.emplace(MakeParameterId(nodeId_, "mode"),
                       static_cast<float>(engine_.GetMode())
                           / static_cast<float>(kPedalDelayModeCount - 1));
        values.emplace(MakeParameterId(nodeId_, "bypass"),
                       engine_.GetBypass() ? 1.0f : 0.0f);
        values.emplace(MakeParameterId(nodeId_, "trails"),
                       engine_.GetTrails() ? 1.0f : 0.0f);
        for(std::size_t mode = 0; mode < kPedalDelayModeCount; ++mode)
        {
            for(std::size_t slot = 0; slot < kPedalSlotCount; ++slot)
            {
                values.emplace(
                    MakeParameterId(
                        nodeId_,
                        std::string(
                            ModeSuffix(static_cast<PedalDelayMode>(mode)))
                            + "."
                            + SlotSuffix(static_cast<PedalSlot>(slot))),
                    baseNormalized_[mode][slot]);
            }
        }
        return values;
    }

    void PedalDelayCore::RestoreStatefulParameterValues(
        const std::unordered_map<std::string, float>& values)
    {
        for(std::size_t mode = 0; mode < kPedalDelayModeCount; ++mode)
        {
            for(std::size_t slot = 0; slot < kPedalSlotCount; ++slot)
            {
                const auto key = MakeParameterId(
                    nodeId_,
                    std::string(ModeSuffix(static_cast<PedalDelayMode>(mode)))
                        + "." + SlotSuffix(static_cast<PedalSlot>(slot)));
                const auto it = values.find(key);
                if(it != values.end())
                {
                    baseNormalized_[mode][slot] = Clamp01(it->second);
                }
            }
        }

        const auto bypassIt = values.find(MakeParameterId(nodeId_, "bypass"));
        if(bypassIt != values.end())
        {
            engine_.SetBypass(bypassIt->second >= 0.5f);
        }
        const auto trailsIt = values.find(MakeParameterId(nodeId_, "trails"));
        if(trailsIt != values.end())
        {
            engine_.SetTrails(trailsIt->second >= 0.5f);
        }

        auto              requested = engine_.GetMode();
        const auto        modeIt    = values.find(MakeParameterId(nodeId_, "mode"));
        if(modeIt != values.end())
        {
            const std::size_t index = static_cast<std::size_t>(
                std::lround(Clamp01(modeIt->second)
                            * static_cast<float>(kPedalDelayModeCount - 1)));
            requested = static_cast<PedalDelayMode>(index);
        }
        SelectMode(requested);
    }

    const std::vector<ParameterDescriptor>& PedalDelayCore::GetParameters() const
    {
        return parameters_;
    }

    const MenuModel& PedalDelayCore::GetMenuModel() const
    {
        return menu_;
    }

    void PedalDelayCore::MenuRotate(int delta)
    {
        if(!menu_.isOpen)
        {
            menu_.isOpen = true;
            BuildDisplay();
            return;
        }

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

        if(menu_.isEditing)
        {
            auto& item = sectionIt->items[sectionIt->selectedIndex];
            if(item.actionKind == MenuItemActionKind::kValue)
            {
                SetMenuItemValue(item.id,
                                 Clamp01(item.normalizedValue
                                         + static_cast<float>(delta) * 0.03f));
            }
            return;
        }

        MoveSelection(delta);
    }

    void PedalDelayCore::MenuPress()
    {
        if(!menu_.isOpen)
        {
            menu_.isOpen = true;
            BuildDisplay();
            return;
        }
        PressSelectedItem();
    }

    void PedalDelayCore::SetMenuItemValue(const std::string& itemId,
                                          float              normalizedValue)
    {
        const auto performance = StripMenuItemPrefix(itemId, "performance");
        if(!performance.empty())
        {
            SetParameterValue(MakeParameterId(nodeId_, performance),
                              normalizedValue);
            return;
        }

        const auto modeItem = StripMenuItemPrefix(itemId, "mode");
        if(!modeItem.empty() && normalizedValue >= 0.5f)
        {
            for(std::size_t mode = 0; mode < kPedalDelayModeCount; ++mode)
            {
                if(modeItem == ModeSuffix(static_cast<PedalDelayMode>(mode)))
                {
                    SelectMode(static_cast<PedalDelayMode>(mode));
                    return;
                }
            }
            return;
        }

        const auto switchItem = StripMenuItemPrefix(itemId, "switches");
        if(switchItem.empty() || normalizedValue < 0.5f)
        {
            return;
        }

        if(switchItem == "bypass")
        {
            engine_.SetBypass(!engine_.GetBypass());
        }
        else if(switchItem == "trails")
        {
            engine_.SetTrails(!engine_.GetTrails());
        }
        else if(switchItem == "tap")
        {
            SetParameterValue(MakeParameterId(nodeId_, "tap"), 1.0f);
            return;
        }
        else if(switchItem == "capture")
        {
            engine_.SetFreezeState(PedalFreezeState::kCapture);
        }
        else if(switchItem == "hold")
        {
            engine_.SetFreezeState(PedalFreezeState::kHold);
        }
        else if(switchItem == "accumulate")
        {
            engine_.SetFreezeState(PedalFreezeState::kAccumulate);
        }
        else if(switchItem == "replace")
        {
            engine_.SetFreezeState(PedalFreezeState::kReplace);
        }
        else if(switchItem == "clear")
        {
            engine_.FreezeClear();
        }
        else
        {
            for(std::size_t mode = 0; mode < kPedalDelayModeCount; ++mode)
            {
                if(switchItem == ModeSuffix(static_cast<PedalDelayMode>(mode)))
                {
                    SelectMode(static_cast<PedalDelayMode>(mode));
                    return;
                }
            }
            return;
        }
        RefreshSnapshots();
    }

    const DisplayModel& PedalDelayCore::GetDisplayModel() const
    {
        return display_;
    }

    std::array<std::string, kPedalSlotCount>
    PedalDelayCore::GetPerformanceSlotParameterIds() const
    {
        std::array<std::string, kPedalSlotCount> ids;
        for(std::size_t slot = 0; slot < kPedalSlotCount; ++slot)
        {
            ids[slot] = MakeParameterId(
                nodeId_, SlotSuffix(static_cast<PedalSlot>(slot)));
        }
        return ids;
    }

    std::string PedalDelayCore::FormatSlotValue(PedalSlot slot) const
    {
        const auto& descriptor
            = GetPedalSlotDescriptor(engine_.GetMode(), slot);
        return FormatValue(engine_.GetSlotNative(slot), descriptor.unit);
    }

    std::string PedalDelayCore::MakeParameterId(const std::string& nodeId,
                                                const std::string& suffix)
    {
        return nodeId + "/param/" + suffix;
    }

    std::string PedalDelayCore::MakeControlId(const std::string& nodeId,
                                              const std::string& suffix)
    {
        return nodeId + "/control/" + suffix;
    }

    std::string PedalDelayCore::MakeMenuSectionId(const std::string& nodeId,
                                                  const std::string& suffix)
    {
        return nodeId + "/menu/" + suffix;
    }

    std::string PedalDelayCore::MakeMenuItemId(const std::string& nodeId,
                                               const std::string& section,
                                               const std::string& item)
    {
        return nodeId + "/menu/" + section + "/" + item;
    }

    std::string PedalDelayCore::MakeAudioInputPortId(const std::string& nodeId,
                                                     std::size_t oneBasedIndex)
    {
        return nodeId + "/port/audio_in_" + std::to_string(oneBasedIndex);
    }

    std::string PedalDelayCore::MakeAudioOutputPortId(const std::string& nodeId,
                                                      std::size_t oneBasedIndex)
    {
        return nodeId + "/port/audio_out_" + std::to_string(oneBasedIndex);
    }

    std::string PedalDelayCore::MakeCvInputPortId(const std::string& nodeId,
                                                  std::size_t oneBasedIndex)
    {
        return nodeId + "/port/cv_in_" + std::to_string(oneBasedIndex);
    }

    std::string PedalDelayCore::MakeGateInputPortId(const std::string& nodeId)
    {
        return nodeId + "/port/gate_in_1";
    }

    std::string PedalDelayCore::MakeMidiInputPortId(const std::string& nodeId)
    {
        return nodeId + "/port/midi_in";
    }

    const char* PedalDelayCore::SlotSuffix(PedalSlot slot)
    {
        switch(slot)
        {
            case PedalSlot::kTime: return "time";
            case PedalSlot::kFeedback: return "feedback";
            case PedalSlot::kMix: return "mix";
            case PedalSlot::kColor: return "color";
            case PedalSlot::kMotion: return "motion";
            default: return "time";
        }
    }

    const char* PedalDelayCore::ModeSuffix(PedalDelayMode mode)
    {
        switch(mode)
        {
            case PedalDelayMode::kDigi: return "digi";
            case PedalDelayMode::kTape: return "tape";
            case PedalDelayMode::kMod: return "mod";
            case PedalDelayMode::kRev: return "rev";
            case PedalDelayMode::kFreeze: return "freeze";
            default: return "digi";
        }
    }

    void PedalDelayCore::ApplySlot(PedalSlot slot,
                                   float     normalizedValue,
                                   bool      updateBase)
    {
        const float clamped = Clamp01(normalizedValue);
        if(updateBase)
        {
            baseNormalized_[static_cast<std::size_t>(engine_.GetMode())]
                           [static_cast<std::size_t>(slot)]
                = clamped;
        }
        engine_.SetSlotNormalized(slot, clamped);
        lastTouchedSlot_ = slot;
        RefreshSnapshots();
    }

    void PedalDelayCore::SelectMode(PedalDelayMode mode)
    {
        if(static_cast<std::size_t>(mode) >= kPedalDelayModeCount)
        {
            return;
        }
        engine_.SetMode(mode);
        // Reapply the stored base values for the newly selected mode so no
        // effective-value override leaks across the mode change.
        const auto modeIndex = static_cast<std::size_t>(mode);
        for(std::size_t slot = 0; slot < kPedalSlotCount; ++slot)
        {
            engine_.SetSlotNormalized(static_cast<PedalSlot>(slot),
                                      baseNormalized_[modeIndex][slot]);
        }
        lastTouchedSlot_ = PedalSlot::kTime;
        RefreshSnapshots();
    }

    void PedalDelayCore::RefreshSnapshots()
    {
        RefreshParameters();
        BuildMenuModel();
        BuildDisplay();
    }

    void PedalDelayCore::RefreshParameters()
    {
        parameters_.clear();

        ParameterDescriptor modeParameter;
        modeParameter.id              = MakeParameterId(nodeId_, "mode");
        modeParameter.label           = "MODE";
        modeParameter.engineeringLabel = "Active delay algorithm selection";
        modeParameter.curve           = "linear";
        modeParameter.dspTargets      = {"algorithm_registry.active_mode"};
        modeParameter.normalizedValue
            = static_cast<float>(engine_.GetMode())
              / static_cast<float>(kPedalDelayModeCount - 1);
        modeParameter.defaultNormalizedValue   = 0.0f;
        modeParameter.effectiveNormalizedValue = modeParameter.normalizedValue;
        modeParameter.unitLabel                = "state";
        modeParameter.stepCount  = static_cast<int>(kPedalDelayModeCount);
        modeParameter.role       = ParameterRole::kGeneric;
        modeParameter.automatable = false;
        modeParameter.stateful    = true;
        modeParameter.menuEditable = true;
        modeParameter.nativeMinimum
            = 0.0f;
        modeParameter.nativeMaximum
            = static_cast<float>(kPedalDelayModeCount - 1);
        modeParameter.nativePrecision = 0;
        parameters_.push_back(modeParameter);

        const auto modeIndex = static_cast<std::size_t>(engine_.GetMode());
        for(std::size_t slot = 0; slot < kPedalSlotCount; ++slot)
        {
            const auto  pedalSlot  = static_cast<PedalSlot>(slot);
            const auto& source     = GetPedalSlotDescriptor(engine_.GetMode(),
                                                        pedalSlot);
            ParameterDescriptor descriptor;
            descriptor.id     = MakeParameterId(nodeId_, SlotSuffix(pedalSlot));
            descriptor.label  = source.musicianLabel;
            descriptor.slotId = source.slotId;
            descriptor.engineeringLabel = source.engineeringLabel;
            descriptor.curve            = source.curve;
            for(const char* target : source.dspTargets)
            {
                if(target != nullptr && target[0] != '\0')
                {
                    descriptor.dspTargets.emplace_back(target);
                }
            }
            descriptor.smoothingMs = source.smoothingMs;
            descriptor.isMacro     = source.isMacro;
            descriptor.provisional = source.provisional;
            descriptor.normalizedValue = baseNormalized_[modeIndex][slot];
            descriptor.defaultNormalizedValue
                = engine_.NativeToNormalized(engine_.GetMode(),
                                             pedalSlot,
                                             source.defaultValue);
            descriptor.effectiveNormalizedValue
                = engine_.GetSlotNormalized(pedalSlot);
            descriptor.unitLabel      = source.unit;
            descriptor.stepCount      = 0;
            descriptor.role           = RoleForSlot(pedalSlot);
            descriptor.importanceRank = static_cast<int>(slot) + 1;
            descriptor.automatable    = true;
            descriptor.stateful       = true;
            descriptor.menuEditable   = true;
            descriptor.nativeMinimum  = source.minimum;
            descriptor.nativeMaximum  = source.maximum;
            descriptor.nativeDefault  = source.defaultValue;
            descriptor.nativePrecision = 2;
            parameters_.push_back(descriptor);
        }

        const auto addSwitch = [this](const char* suffix,
                                      const char* label,
                                      const char* engineering,
                                      const char* target,
                                      float       value) {
            ParameterDescriptor descriptor;
            descriptor.id                = MakeParameterId(nodeId_, suffix);
            descriptor.label             = label;
            descriptor.engineeringLabel  = engineering;
            descriptor.curve             = "linear";
            descriptor.dspTargets        = {target};
            descriptor.normalizedValue   = value;
            descriptor.effectiveNormalizedValue = value;
            descriptor.unitLabel         = "state";
            descriptor.stepCount         = 2;
            descriptor.automatable       = false;
            descriptor.stateful          = true;
            descriptor.menuEditable      = true;
            descriptor.nativeMaximum     = 1.0f;
            descriptor.nativePrecision   = 0;
            parameters_.push_back(descriptor);
        };
        addSwitch("bypass",
                  "BYPASS",
                  "Wet-path input send mute",
                  "output.input_send_gain",
                  engine_.GetBypass() ? 1.0f : 0.0f);
        addSwitch("trails",
                  "TRAILS",
                  "Bypass spillover policy",
                  "output.bypass_trails_enabled",
                  engine_.GetTrails() ? 1.0f : 0.0f);
        addSwitch("tap",
                  "TAP/HOLD",
                  "Tap-tempo interval capture / freeze hold toggle",
                  "transport.tap_tempo_ms",
                  0.0f);

        ParameterDescriptor freezeState;
        freezeState.id    = MakeParameterId(nodeId_, "freeze_state");
        freezeState.label = "FREEZE STATE";
        freezeState.engineeringLabel
            = "Freeze loop state machine (idle/capture/hold/accumulate/replace)";
        freezeState.curve      = "linear";
        freezeState.dspTargets = {"freeze_loop.state"};
        freezeState.normalizedValue
            = static_cast<float>(engine_.GetFreezeState())
              / static_cast<float>(
                  static_cast<std::size_t>(PedalFreezeState::kCount) - 1);
        freezeState.effectiveNormalizedValue = freezeState.normalizedValue;
        freezeState.unitLabel                = "state";
        freezeState.stepCount                = static_cast<int>(
            static_cast<std::size_t>(PedalFreezeState::kCount));
        freezeState.automatable    = false;
        freezeState.stateful       = false;
        freezeState.menuEditable   = true;
        freezeState.nativeMaximum  = static_cast<float>(
            static_cast<std::size_t>(PedalFreezeState::kCount) - 1);
        freezeState.nativePrecision = 0;
        parameters_.push_back(freezeState);
    }

    void PedalDelayCore::BuildMenuModel()
    {
        const bool wasOpen    = menu_.isOpen;
        const bool wasEditing = menu_.isEditing;
        const auto stack      = menu_.sectionStack;
        const auto sectionId  = menu_.currentSectionId.empty()
                                    ? MakeMenuSectionId(nodeId_, "root")
                                    : menu_.currentSectionId;
        std::unordered_map<std::string, int> selections;
        for(const auto& section : menu_.sections)
        {
            selections[section.id] = section.selectedIndex;
        }

        menu_                  = {};
        menu_.isOpen           = wasOpen;
        menu_.isEditing        = wasEditing;
        menu_.sectionStack     = stack;
        menu_.currentSectionId = sectionId;

        MenuSection root;
        root.id    = MakeMenuSectionId(nodeId_, "root");
        root.title = GetAppDisplayName();
        root.items = {
            {MakeMenuItemId(nodeId_, "root", "mode"),
             "Mode",
             false,
             MenuItemActionKind::kEnterSection,
             0.0f,
             PedalDelayModeName(engine_.GetMode()),
             MakeMenuSectionId(nodeId_, "mode")},
            {MakeMenuItemId(nodeId_, "root", "performance"),
             "Performance",
             false,
             MenuItemActionKind::kEnterSection,
             0.0f,
             "5 slots",
             MakeMenuSectionId(nodeId_, "performance")},
            {MakeMenuItemId(nodeId_, "root", "engineering"),
             "Engineering",
             false,
             MenuItemActionKind::kEnterSection,
             0.0f,
             "targets",
             MakeMenuSectionId(nodeId_, "engineering")},
            {MakeMenuItemId(nodeId_, "root", "switches"),
             "Switches",
             false,
             MenuItemActionKind::kEnterSection,
             0.0f,
             PedalFreezeStateName(engine_.GetFreezeState()),
             MakeMenuSectionId(nodeId_, "switches")},
        };
        menu_.sections.push_back(root);

        MenuSection modes;
        modes.id    = MakeMenuSectionId(nodeId_, "mode");
        modes.title = "Mode";
        for(std::size_t mode = 0; mode < kPedalDelayModeCount; ++mode)
        {
            const auto pedalMode = static_cast<PedalDelayMode>(mode);
            modes.items.push_back(
                {MakeMenuItemId(nodeId_, "mode", ModeSuffix(pedalMode)),
                 PedalDelayModeName(pedalMode),
                 false,
                 MenuItemActionKind::kMomentary,
                 engine_.GetMode() == pedalMode ? 1.0f : 0.0f,
                 engine_.GetMode() == pedalMode ? "selected" : "",
                 ""});
        }
        modes.items.push_back({MakeMenuItemId(nodeId_, "mode", "back"),
                               "Back",
                               false,
                               MenuItemActionKind::kBack});
        menu_.sections.push_back(modes);

        MenuSection performance;
        performance.id    = MakeMenuSectionId(nodeId_, "performance");
        performance.title = std::string(PedalDelayModeName(engine_.GetMode()))
                            + " performance";
        for(std::size_t slot = 0; slot < kPedalSlotCount; ++slot)
        {
            const auto  pedalSlot = static_cast<PedalSlot>(slot);
            const auto& source
                = GetPedalSlotDescriptor(engine_.GetMode(), pedalSlot);
            performance.items.push_back(
                {MakeMenuItemId(nodeId_, "performance", SlotSuffix(pedalSlot)),
                 std::string(source.slotId) + " - " + source.musicianLabel,
                 true,
                 MenuItemActionKind::kValue,
                 engine_.GetSlotNormalized(pedalSlot),
                 FormatSlotValue(pedalSlot),
                 ""});
        }
        performance.items.push_back(
            {MakeMenuItemId(nodeId_, "performance", "back"),
             "Back",
             false,
             MenuItemActionKind::kBack});
        menu_.sections.push_back(performance);

        MenuSection engineering;
        engineering.id    = MakeMenuSectionId(nodeId_, "engineering");
        engineering.title = "Engineering targets";
        for(std::size_t slot = 0; slot < kPedalSlotCount; ++slot)
        {
            const auto  pedalSlot = static_cast<PedalSlot>(slot);
            const auto& source
                = GetPedalSlotDescriptor(engine_.GetMode(), pedalSlot);
            engineering.items.push_back(
                {MakeMenuItemId(nodeId_, "engineering", SlotSuffix(pedalSlot)),
                 source.engineeringLabel,
                 false,
                 MenuItemActionKind::kReadonly,
                 0.0f,
                 JoinTargets(source),
                 ""});
        }
        engineering.items.push_back(
            {MakeMenuItemId(nodeId_, "engineering", "back"),
             "Back",
             false,
             MenuItemActionKind::kBack});
        menu_.sections.push_back(engineering);

        MenuSection switches;
        switches.id    = MakeMenuSectionId(nodeId_, "switches");
        switches.title = "Switches";
        const auto addSwitchItem = [this, &switches](const char* id,
                                                     const char* label,
                                                     const std::string& value) {
            switches.items.push_back({MakeMenuItemId(nodeId_, "switches", id),
                                      label,
                                      false,
                                      MenuItemActionKind::kMomentary,
                                      0.0f,
                                      value,
                                      ""});
        };
        addSwitchItem("bypass", "Bypass", engine_.GetBypass() ? "on" : "off");
        addSwitchItem("trails", "Trails", engine_.GetTrails() ? "on" : "off");
        addSwitchItem("tap", "Tap / Hold", FormatSlotValue(PedalSlot::kTime));
        addSwitchItem("capture", "Freeze capture", "");
        addSwitchItem("hold", "Freeze hold", "");
        addSwitchItem("accumulate", "Freeze accumulate", "");
        addSwitchItem("replace", "Freeze replace", "");
        addSwitchItem("clear", "Freeze clear", "");
        switches.items.push_back({MakeMenuItemId(nodeId_, "switches", "back"),
                                  "Back",
                                  false,
                                  MenuItemActionKind::kBack});
        menu_.sections.push_back(switches);

        for(auto& section : menu_.sections)
        {
            const auto it = selections.find(section.id);
            if(it != selections.end() && !section.items.empty())
            {
                section.selectedIndex
                    = std::clamp(it->second,
                                 0,
                                 static_cast<int>(section.items.size()) - 1);
            }
            if(section.id == menu_.currentSectionId)
            {
                menu_.currentSelection = section.selectedIndex;
            }
        }
    }

    void PedalDelayCore::BuildDisplay()
    {
        const auto& source
            = GetPedalSlotDescriptor(engine_.GetMode(), lastTouchedSlot_);

        display_       = {};
        display_.title = GetAppDisplayName();
        display_.mode  = menu_.isOpen ? DisplayMode::kMenu : DisplayMode::kStatus;
        display_.texts.push_back(
            {0,
             0,
             std::string(PedalDelayModeName(engine_.GetMode())) + "  "
                 + (engine_.GetBypass() ? "BYP" : "ON") + "  "
                 + PedalFreezeStateName(engine_.GetFreezeState()),
             false});
        // Contract 10 layout: slot/musician, engineering, value+unit, range,
        // DSP target(s).
        display_.texts.push_back(
            {0,
             12,
             std::string(source.slotId) + " - " + source.musicianLabel,
             false});
        display_.texts.push_back({0, 22, source.engineeringLabel, false});
        display_.texts.push_back(
            {0,
             32,
             FormatSlotValue(lastTouchedSlot_) + "  ["
                 + FormatValue(source.minimum, "") + ".."
                 + FormatValue(source.maximum, source.unit) + "]",
             false});
        display_.texts.push_back({0, 42, "Target: " + JoinTargets(source), false});
        display_.bars.push_back(
            {0, 56, 128, 6, engine_.GetSlotNormalized(lastTouchedSlot_)});
        display_.revision += 1;
    }

    void PedalDelayCore::MoveSelection(int delta)
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
        int       next      = (sectionIt->selectedIndex + delta) % itemCount;
        if(next < 0)
        {
            next += itemCount;
        }
        sectionIt->selectedIndex = next;
        menu_.currentSelection   = next;
        BuildDisplay();
    }

    void PedalDelayCore::PressSelectedItem()
    {
        auto sectionIt
            = std::find_if(menu_.sections.begin(),
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
            }
            menu_.currentSelection = 0;
            menu_.isEditing        = false;
        }
        else if(item.actionKind == MenuItemActionKind::kValue && item.editable)
        {
            menu_.isEditing = !menu_.isEditing;
        }
        else if(item.actionKind == MenuItemActionKind::kMomentary)
        {
            const auto id = item.id;
            SetMenuItemValue(id, 1.0f);
        }
        else
        {
            menu_.isOpen = false;
        }
        BuildMenuModel();
        BuildDisplay();
    }

    void PedalDelayCore::ConsumeGateInput()
    {
        const auto it = portInputs_.find(gateInputPortId_);
        if(it == portInputs_.end())
        {
            return;
        }
        const bool high = it->second.gate;
        if(high && !gateHigh_)
        {
            // Drive the engine directly: the string-keyed parameter path and
            // the descriptor/menu rebuild must not run on the audio thread.
            // The UI catches up on the next TickUi.
            if(engine_.GetMode() == PedalDelayMode::kFreeze)
            {
                engine_.FreezeToggleHold();
            }
            else
            {
                engine_.Tap(uiClockMs_);
            }
        }
        gateHigh_ = high;
    }

    std::string PedalDelayCore::StripPrefix(const std::string& value,
                                            const std::string& prefix) const
    {
        if(value.rfind(prefix, 0) != 0)
        {
            return {};
        }
        return value.substr(prefix.size());
    }

    std::string
    PedalDelayCore::StripMenuItemPrefix(const std::string& itemId,
                                        const char*        section) const
    {
        return StripPrefix(itemId,
                           nodeId_ + "/menu/" + section + "/");
    }

    const ParameterDescriptor*
    PedalDelayCore::FindParameter(const std::string& parameterId) const
    {
        const auto it
            = std::find_if(parameters_.begin(),
                           parameters_.end(),
                           [&parameterId](const ParameterDescriptor& parameter) {
                               return parameter.id == parameterId;
                           });
        return it != parameters_.end() ? &(*it) : nullptr;
    }
} // namespace apps
} // namespace daisyhost
