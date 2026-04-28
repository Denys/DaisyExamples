#include "daisyhost/apps/SubharmoniqCore.h"

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
constexpr int kSubharmoniqPageCount = 4;

float Clamp01(float value)
{
    return value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value);
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

const char* FieldKeyMenuItemSuffix(std::size_t zeroBasedIndex)
{
    static constexpr std::array<const char*, 16> kSuffixes = {{
        "a1", "a2", "a3", "a4", "a5", "a6", "a7", "a8",
        "b1", "b2", "b3", "b4", "b5", "b6", "b7", "b8"}};
    return zeroBasedIndex < kSuffixes.size() ? kSuffixes[zeroBasedIndex] : "";
}

const char* FieldKeyDetailLabel(std::size_t zeroBasedIndex)
{
    static constexpr std::array<const char*, 16> kLabels = {{
        "Seq1 Step1",
        "Seq1 Step2",
        "Seq1 Step3",
        "Seq1 Step4",
        "Rhythm 1",
        "Rhythm 2",
        "Rhythm 3",
        "Rhythm 4",
        "Seq2 Step1",
        "Seq2 Step2",
        "Seq2 Step3",
        "Seq2 Step4",
        "Quantize",
        "Seq Octave",
        "Play/Stop",
        "Reset",
    }};
    return zeroBasedIndex < kLabels.size() ? kLabels[zeroBasedIndex] : "";
}

DaisySubharmoniqRhythmTarget NextRhythmTarget(
    DaisySubharmoniqRhythmTarget target)
{
    switch(target)
    {
        case DaisySubharmoniqRhythmTarget::kOff:
            return DaisySubharmoniqRhythmTarget::kSeq1;
        case DaisySubharmoniqRhythmTarget::kSeq1:
            return DaisySubharmoniqRhythmTarget::kSeq2;
        case DaisySubharmoniqRhythmTarget::kSeq2:
            return DaisySubharmoniqRhythmTarget::kBoth;
        default:
            return DaisySubharmoniqRhythmTarget::kOff;
    }
}

std::string FormatPageLabel(DaisySubharmoniqPage page)
{
    switch(page)
    {
        case DaisySubharmoniqPage::kVco: return "VCO";
        case DaisySubharmoniqPage::kVcf: return "VCF";
        case DaisySubharmoniqPage::kVcaMix: return "VCA/Mix";
        default: return "Seq/Rhy";
    }
}

std::string FormatRhythmTarget(DaisySubharmoniqRhythmTarget target)
{
    switch(target)
    {
        case DaisySubharmoniqRhythmTarget::kSeq1: return "S1";
        case DaisySubharmoniqRhythmTarget::kSeq2: return "S2";
        case DaisySubharmoniqRhythmTarget::kBoth: return "Both";
        default: return "Off";
    }
}

std::string FormatQuantize(DaisySubharmoniqQuantizeMode mode)
{
    switch(mode)
    {
        case DaisySubharmoniqQuantizeMode::kOff: return "Off";
        case DaisySubharmoniqQuantizeMode::kEightEqual: return "8-ET";
        case DaisySubharmoniqQuantizeMode::kTwelveJust: return "12-JI";
        case DaisySubharmoniqQuantizeMode::kEightJust: return "8-JI";
        default: return "12-ET";
    }
}

#if DAISYHOST_ENABLE_FIELD_OLED_TRANSIENTS
std::string FormatQuantizeDetail(DaisySubharmoniqQuantizeMode mode)
{
    switch(mode)
    {
        case DaisySubharmoniqQuantizeMode::kOff: return "Off";
        case DaisySubharmoniqQuantizeMode::kEightEqual: return "8-note Equal";
        case DaisySubharmoniqQuantizeMode::kTwelveJust: return "12-note Just";
        case DaisySubharmoniqQuantizeMode::kEightJust: return "8-note Just";
        default: return "12-note Equal";
    }
}

std::string FormatRhythmTargetDetail(DaisySubharmoniqRhythmTarget target)
{
    switch(target)
    {
        case DaisySubharmoniqRhythmTarget::kSeq1: return "Seq1";
        case DaisySubharmoniqRhythmTarget::kSeq2: return "Seq2";
        case DaisySubharmoniqRhythmTarget::kBoth: return "Both";
        default: return "Off";
    }
}

std::string FormatFrequencyFromNormalized(float normalized)
{
    const float clamped = Clamp01(normalized);
    const float hz      = 20.0f * std::pow(1000.0f, clamped);
    char        buffer[32];
    if(hz >= 1000.0f)
    {
        std::snprintf(buffer, sizeof(buffer), "%.2f kHz", hz / 1000.0f);
    }
    else
    {
        std::snprintf(buffer, sizeof(buffer), "%.0f Hz", hz);
    }
    return std::string(buffer);
}

std::string FieldKeyName(std::size_t zeroBasedIndex)
{
    const char        row = zeroBasedIndex < 8 ? 'A' : 'B';
    const std::size_t number
        = zeroBasedIndex < 8 ? zeroBasedIndex + 1 : zeroBasedIndex - 7;
    return std::string(1, row) + std::to_string(number);
}
#endif

std::string FormatPercent(float normalized)
{
    char buffer[16];
    std::snprintf(buffer,
                  sizeof(buffer),
                  "%d%%",
                  static_cast<int>(std::round(Clamp01(normalized) * 100.0f)));
    return std::string(buffer);
}

bool ParameterValueDiffers(const DaisySubharmoniqCore& core,
                           const std::string&          parameterId,
                           float                       normalizedValue)
{
    float current = 0.0f;
    return core.GetParameterValue(parameterId, &current)
           && std::abs(current - normalizedValue) > 0.000001f;
}

bool EffectiveParameterValueDiffers(const DaisySubharmoniqCore& core,
                                    const std::string&          parameterId,
                                    float normalizedValue)
{
    float current = 0.0f;
    return core.GetEffectiveParameterValue(parameterId, &current)
           && std::abs(current - normalizedValue) > 0.000001f;
}
} // namespace

SubharmoniqCore::SubharmoniqCore(const std::string& nodeId) : nodeId_(nodeId)
{
    for(std::size_t i = 1; i <= 4; ++i)
    {
        portInputs_[MakeCvInputPortId(nodeId_, i)].type = VirtualPortType::kCv;
    }
    portInputs_[MakeGateInputPortId(nodeId_, 1)].type = VirtualPortType::kGate;
    portInputs_[MakeMidiInputPortId(nodeId_, 1)].type = VirtualPortType::kMidi;
    portOutputs_[MakeGateOutputPortId(nodeId_, 1)].type = VirtualPortType::kGate;
    portOutputs_[MakeAudioOutputPortId(nodeId_, 1)].type = VirtualPortType::kAudio;
    portOutputs_[MakeAudioOutputPortId(nodeId_, 2)].type = VirtualPortType::kAudio;

    RefreshSnapshots();
}

std::string SubharmoniqCore::GetAppId() const
{
    return "subharmoniq";
}

std::string SubharmoniqCore::GetAppDisplayName() const
{
    return "Subharmoniq";
}

HostedAppCapabilities SubharmoniqCore::GetCapabilities() const
{
    HostedAppCapabilities capabilities;
    capabilities.acceptsAudioInput  = false;
    capabilities.acceptsMidiInput   = true;
    capabilities.producesMidiOutput = false;
    return capabilities;
}

HostedAppPatchBindings SubharmoniqCore::GetPatchBindings() const
{
    HostedAppPatchBindings bindings;
    const auto pageBinding = sharedCore_.GetActivePageBinding();
    for(std::size_t i = 0; i < 4; ++i)
    {
        bindings.knobControlIds[i] = MakeControlId(nodeId_, pageBinding.parameterIds[i]);
        bindings.knobParameterIds[i] = MakeParameterId(nodeId_, pageBinding.parameterIds[i]);
        bindings.knobDetailLabels[i] = pageBinding.parameterLabels[i];
    }
    for(std::size_t i = 0; i < pageBinding.parameterIds.size(); ++i)
    {
        if(!pageBinding.parameterIds[i].empty())
        {
            bindings.fieldKnobControlIds[i]
                = MakeControlId(nodeId_, pageBinding.parameterIds[i]);
            bindings.fieldKnobParameterIds[i]
                = MakeParameterId(nodeId_, pageBinding.parameterIds[i]);
        }
        bindings.fieldKnobDetailLabels[i] = pageBinding.parameterLabels[i];
    }
    bindings.encoderControlId       = MakeEncoderControlId(nodeId_);
    bindings.encoderButtonControlId = MakeEncoderButtonControlId(nodeId_);
    for(std::size_t i = 0; i < 4; ++i)
    {
        bindings.cvInputPortIds[i] = MakeCvInputPortId(nodeId_, i + 1);
    }
    bindings.gateInputPortIds[0]   = MakeGateInputPortId(nodeId_, 1);
    bindings.gateOutputPortId      = MakeGateOutputPortId(nodeId_, 1);
    bindings.midiInputPortId       = MakeMidiInputPortId(nodeId_, 1);
    bindings.audioOutputPortIds[0] = MakeAudioOutputPortId(nodeId_, 1);
    bindings.audioOutputPortIds[1] = MakeAudioOutputPortId(nodeId_, 2);
    bindings.mainOutputChannels    = {0, 1};
    for(std::size_t i = 0; i < bindings.fieldKeyMenuItemIds.size(); ++i)
    {
        bindings.fieldKeyMenuItemIds[i]
            = MakeMenuItemId(nodeId_, "field_keys", FieldKeyMenuItemSuffix(i));
        bindings.fieldKeyDetailLabels[i] = FieldKeyDetailLabel(i);
    }
    return bindings;
}

void SubharmoniqCore::Prepare(double sampleRate, std::size_t maxBlockSize)
{
    sharedCore_.Prepare(sampleRate, maxBlockSize);
    RefreshSnapshots();
}

void SubharmoniqCore::Process(const AudioBufferView&,
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

    float* left = output.channelCount > 0 ? output.channels[0] : nullptr;
    float* right = output.channelCount > 1 ? output.channels[1] : nullptr;
    sharedCore_.Process(left, right, frameCount);

    PortValue leftOutput;
    leftOutput.type = VirtualPortType::kAudio;
    leftOutput.scalar = PeakForBuffer(left, frameCount);
    portOutputs_[MakeAudioOutputPortId(nodeId_, 1)] = leftOutput;

    PortValue rightOutput;
    rightOutput.type = VirtualPortType::kAudio;
    rightOutput.scalar = PeakForBuffer(right, frameCount);
    portOutputs_[MakeAudioOutputPortId(nodeId_, 2)] = rightOutput;

    PortValue gateOutput;
    gateOutput.type = VirtualPortType::kGate;
    gateOutput.gate = sharedCore_.GetGateOutputPulse() > 0.5f;
    gateOutput.scalar = sharedCore_.GetGateOutputPulse();
    portOutputs_[MakeGateOutputPortId(nodeId_, 1)] = gateOutput;
}

void SubharmoniqCore::SetControl(const std::string& controlId,
                                 float              normalizedValue)
{
    if(controlId == MakeEncoderButtonControlId(nodeId_))
    {
        SetEncoderPress(normalizedValue >= 0.5f);
        return;
    }

    const std::string suffix = StripControlId(controlId);
    const bool shouldRecordZoom
        = ParameterValueDiffers(sharedCore_, suffix, normalizedValue);
    if(!suffix.empty() && sharedCore_.SetParameterValue(suffix, normalizedValue))
    {
        if(shouldRecordZoom)
        {
            RecordParameterZoom(suffix, normalizedValue);
        }
        RefreshSnapshots();
    }
}

void SubharmoniqCore::SetEncoderDelta(int delta)
{
    MenuRotate(delta);
}

void SubharmoniqCore::SetEncoderPress(bool pressed)
{
    if(pressed && !encoderPressed_)
    {
        MenuPress();
    }
    encoderPressed_ = pressed;
}

void SubharmoniqCore::SetPortInput(const std::string& portId,
                                   const PortValue&   value)
{
    portInputs_[portId] = value;

    if(portId == MakeGateInputPortId(nodeId_, 1) && value.type == VirtualPortType::kGate)
    {
        sharedCore_.TriggerGate(value.gate);
    }
    else if(portId == MakeMidiInputPortId(nodeId_, 1) && value.type == VirtualPortType::kMidi)
    {
        for(const auto& event : value.midiEvents)
        {
            sharedCore_.HandleMidiEvent(event.status, event.data1, event.data2);
        }
    }
    else if(value.type == VirtualPortType::kCv)
    {
        if(portId == MakeCvInputPortId(nodeId_, 1))
        {
            const bool shouldRecordZoom
                = ParameterValueDiffers(sharedCore_, "root_cv", value.scalar);
            if(sharedCore_.SetParameterValue("root_cv", value.scalar))
            {
                if(shouldRecordZoom)
                {
                    RecordCvZoom(1, "root_cv", value.scalar);
                }
            }
        }
        else if(portId == MakeCvInputPortId(nodeId_, 2))
        {
            const bool shouldRecordZoom
                = ParameterValueDiffers(sharedCore_, "cutoff_cv", value.scalar);
            if(sharedCore_.SetParameterValue("cutoff_cv", value.scalar))
            {
                if(shouldRecordZoom)
                {
                    RecordCvZoom(2, "cutoff_cv", value.scalar);
                }
            }
        }
        else if(portId == MakeCvInputPortId(nodeId_, 3))
        {
            const bool shouldRecordZoom
                = ParameterValueDiffers(sharedCore_, "rhythm_cv", value.scalar);
            if(sharedCore_.SetParameterValue("rhythm_cv", value.scalar))
            {
                if(shouldRecordZoom)
                {
                    RecordCvZoom(3, "rhythm_cv", value.scalar);
                }
            }
        }
        else if(portId == MakeCvInputPortId(nodeId_, 4))
        {
            const bool shouldRecordZoom
                = ParameterValueDiffers(sharedCore_, "sub_cv", value.scalar);
            if(sharedCore_.SetParameterValue("sub_cv", value.scalar))
            {
                if(shouldRecordZoom)
                {
                    RecordCvZoom(4, "sub_cv", value.scalar);
                }
            }
        }
    }
    RefreshSnapshots();
}

PortValue SubharmoniqCore::GetPortOutput(const std::string& portId) const
{
    const auto it = portOutputs_.find(portId);
    return it != portOutputs_.end() ? it->second : PortValue{};
}

void SubharmoniqCore::TickUi(double deltaMs)
{
#if DAISYHOST_ENABLE_FIELD_OLED_TRANSIENTS
    oledTransient_.Tick(deltaMs);
#else
    (void)deltaMs;
#endif
    BuildDisplay();
}

bool SubharmoniqCore::SetParameterValue(const std::string& parameterId,
                                        float              normalizedValue)
{
    const std::string stripped = StripParameterId(parameterId);
    const bool shouldRecordZoom
        = ParameterValueDiffers(sharedCore_, stripped, normalizedValue);
    const bool changed = sharedCore_.SetParameterValue(stripped, normalizedValue);
    if(changed)
    {
        if(shouldRecordZoom)
        {
            RecordParameterZoom(stripped, normalizedValue);
        }
        RefreshSnapshots();
    }
    return changed;
}

bool SubharmoniqCore::SetEffectiveParameterValue(
    const std::string& parameterId,
    float              normalizedValue)
{
    const std::string stripped = StripParameterId(parameterId);
    const bool shouldRecordZoom
        = EffectiveParameterValueDiffers(sharedCore_, stripped, normalizedValue);
    const bool changed
        = sharedCore_.SetEffectiveParameterValue(stripped, normalizedValue);
    if(changed)
    {
        if(shouldRecordZoom)
        {
            RecordParameterZoom(stripped, normalizedValue);
        }
        RefreshSnapshots();
    }
    return changed;
}

ParameterValueLookup SubharmoniqCore::GetControlValue(
    const std::string& controlId) const
{
    return GetParameterValue(MakeParameterId(nodeId_, StripControlId(controlId)));
}

ParameterValueLookup SubharmoniqCore::GetParameterValue(
    const std::string& parameterId) const
{
    float value = 0.0f;
    if(sharedCore_.GetParameterValue(StripParameterId(parameterId), &value))
    {
        return {true, value};
    }
    return {};
}

ParameterValueLookup SubharmoniqCore::GetEffectiveParameterValue(
    const std::string& parameterId) const
{
    float value = 0.0f;
    if(sharedCore_.GetEffectiveParameterValue(StripParameterId(parameterId), &value))
    {
        return {true, value};
    }
    return {};
}

std::array<float, 16> SubharmoniqCore::GetFieldKeyLedValues() const
{
    std::array<float, 16> leds{};

    if(sharedCore_.IsPlaying())
    {
        const int seq1Step = sharedCore_.GetSequencerStepIndex(0);
        const int seq2Step = sharedCore_.GetSequencerStepIndex(1);
        if(seq1Step >= 0 && seq1Step < 4)
        {
            leds[static_cast<std::size_t>(seq1Step)] = 1.0f;
        }
        if(seq2Step >= 0 && seq2Step < 4)
        {
            leds[8u + static_cast<std::size_t>(seq2Step)] = 1.0f;
        }
    }

    for(std::size_t rhythm = 0; rhythm < 4; ++rhythm)
    {
        switch(sharedCore_.GetRhythmTarget(rhythm))
        {
            case DaisySubharmoniqRhythmTarget::kOff: leds[4u + rhythm] = 0.0f; break;
            case DaisySubharmoniqRhythmTarget::kSeq1:
            case DaisySubharmoniqRhythmTarget::kSeq2: leds[4u + rhythm] = 0.5f; break;
            case DaisySubharmoniqRhythmTarget::kBoth: leds[4u + rhythm] = 1.0f; break;
        }
    }

    switch(sharedCore_.GetQuantizeMode())
    {
        case DaisySubharmoniqQuantizeMode::kOff: leds[12] = 0.0f; break;
        case DaisySubharmoniqQuantizeMode::kTwelveEqual:
        case DaisySubharmoniqQuantizeMode::kEightEqual: leds[12] = 0.5f; break;
        case DaisySubharmoniqQuantizeMode::kTwelveJust:
        case DaisySubharmoniqQuantizeMode::kEightJust: leds[12] = 1.0f; break;
    }

    const int octaveRange = sharedCore_.GetSeqOctaveRange();
    leds[13] = octaveRange <= 1 ? 0.0f : (octaveRange == 2 ? 0.5f : 1.0f);
    leds[14] = sharedCore_.IsPlaying() ? 1.0f : 0.0f;
    leds[15] = 0.0f;
    return leds;
}

void SubharmoniqCore::ResetToDefaultState(std::uint32_t seed)
{
    sharedCore_.ResetToDefaultState(seed);
    RefreshSnapshots();
}

std::unordered_map<std::string, float>
SubharmoniqCore::CaptureStatefulParameterValues() const
{
    std::unordered_map<std::string, float> values;
    for(const auto& entry : sharedCore_.CaptureStatefulParameterValues())
    {
        values[MakeParameterId(nodeId_, entry.first)] = entry.second;
    }
    return values;
}

void SubharmoniqCore::RestoreStatefulParameterValues(
    const std::unordered_map<std::string, float>& values)
{
    std::unordered_map<std::string, float> stripped;
    for(const auto& entry : values)
    {
        stripped[StripParameterId(entry.first)] = entry.second;
    }
    sharedCore_.RestoreStatefulParameterValues(stripped);
    RefreshSnapshots();
}

const std::vector<ParameterDescriptor>& SubharmoniqCore::GetParameters() const
{
    return parameters_;
}

const MenuModel& SubharmoniqCore::GetMenuModel() const
{
    return menu_;
}

void SubharmoniqCore::MenuRotate(int delta)
{
    const auto next = static_cast<int>(sharedCore_.GetActivePage()) + delta;
    const int wrapped = (next + kSubharmoniqPageCount) % kSubharmoniqPageCount;
    sharedCore_.SetActivePage(static_cast<DaisySubharmoniqPage>(wrapped));
    RecordPageZoom();
    RefreshSnapshots();
}

void SubharmoniqCore::MenuPress()
{
    menu_.isOpen = !menu_.isOpen;
    BuildDisplay();
}

void SubharmoniqCore::SetMenuItemValue(const std::string& itemId,
                                       float              normalizedValue)
{
    if(itemId == MakeMenuItemId(nodeId_, "pages", "page"))
    {
        const int rawPage = static_cast<int>(
            std::round(Clamp01(normalizedValue) * static_cast<float>(kSubharmoniqPageCount - 1)));
        const int page = rawPage < 0 ? 0 : (rawPage >= kSubharmoniqPageCount
                                                ? kSubharmoniqPageCount - 1
                                                : rawPage);
        sharedCore_.SetActivePage(static_cast<DaisySubharmoniqPage>(page));
        RecordPageZoom();
    }
    else if(itemId == MakeMenuItemId(nodeId_, "transport", "play"))
    {
        sharedCore_.TriggerMomentaryAction("play_toggle");
    }
    else if(itemId == MakeMenuItemId(nodeId_, "transport", "reset"))
    {
        sharedCore_.TriggerMomentaryAction("reset");
    }
    else if(itemId == MakeMenuItemId(nodeId_, "transport", "panic"))
    {
        sharedCore_.TriggerMomentaryAction("panic");
    }
    else
    {
        bool handled = false;
        for(std::size_t i = 0; i < 16; ++i)
        {
            if(itemId
               == MakeMenuItemId(nodeId_, "field_keys", FieldKeyMenuItemSuffix(i)))
            {
                handled = TriggerFieldKeyAction(i);
                if(handled)
                {
                    RecordFieldKeyZoom(i);
                }
                break;
            }
        }
        if(!handled)
        {
            const std::string marker = "/param/";
            const auto markerPos = itemId.find(marker);
            if(markerPos != std::string::npos)
            {
                const std::string parameterId = itemId.substr(markerPos + marker.size());
                const bool shouldRecordZoom
                    = ParameterValueDiffers(sharedCore_, parameterId, normalizedValue);
                if(sharedCore_.SetParameterValue(parameterId, normalizedValue)
                   && shouldRecordZoom)
                {
                    RecordParameterZoom(parameterId, normalizedValue);
                }
            }
        }
    }
    RefreshSnapshots();
}

bool SubharmoniqCore::TriggerFieldKeyAction(std::size_t zeroBasedIndex)
{
    if(zeroBasedIndex < 4)
    {
        sharedCore_.SetActivePage(DaisySubharmoniqPage::kSeqRhythm);
        return true;
    }
    if(zeroBasedIndex >= 8 && zeroBasedIndex < 12)
    {
        sharedCore_.SetActivePage(DaisySubharmoniqPage::kSeqRhythm);
        return true;
    }
    if(zeroBasedIndex >= 4 && zeroBasedIndex < 8)
    {
        const std::size_t rhythm = zeroBasedIndex - 4;
        sharedCore_.SetRhythmTarget(
            rhythm,
            NextRhythmTarget(sharedCore_.GetRhythmTarget(rhythm)));
        sharedCore_.SetActivePage(DaisySubharmoniqPage::kSeqRhythm);
        return true;
    }
    if(zeroBasedIndex == 12)
    {
        const int next = (static_cast<int>(sharedCore_.GetQuantizeMode()) + 1) % 5;
        sharedCore_.SetQuantizeMode(static_cast<DaisySubharmoniqQuantizeMode>(next));
        return true;
    }
    if(zeroBasedIndex == 13)
    {
        const int current = sharedCore_.GetSeqOctaveRange();
        sharedCore_.SetSeqOctaveRange(current == 1 ? 2 : (current == 2 ? 5 : 1));
        return true;
    }
    if(zeroBasedIndex == 14)
    {
        sharedCore_.TriggerMomentaryAction("play_toggle");
        return true;
    }
    if(zeroBasedIndex == 15)
    {
        sharedCore_.TriggerMomentaryAction("reset");
        return true;
    }
    return false;
}

const DisplayModel& SubharmoniqCore::GetDisplayModel() const
{
    return display_;
}

DaisySubharmoniqPageBinding SubharmoniqCore::GetActivePageBinding() const
{
    return sharedCore_.GetActivePageBinding();
}

bool SubharmoniqCore::SetActivePage(DaisySubharmoniqPage page)
{
    const bool changed = sharedCore_.SetActivePage(page);
    if(changed)
    {
        RecordPageZoom();
        RefreshSnapshots();
    }
    return changed;
}

bool SubharmoniqCore::TriggerMomentaryAction(const std::string& actionId)
{
    const bool handled = sharedCore_.TriggerMomentaryAction(actionId);
    if(handled)
    {
        RefreshSnapshots();
    }
    return handled;
}

void SubharmoniqCore::SetSequencerStepValue(std::size_t sequencer,
                                            std::size_t step,
                                            float       normalizedValue)
{
    sharedCore_.SetSequencerStepValue(sequencer, step, normalizedValue);
    RefreshSnapshots();
}

void SubharmoniqCore::SetRhythmDivisor(std::size_t rhythm, int divisor)
{
    sharedCore_.SetRhythmDivisor(rhythm, divisor);
    RefreshSnapshots();
}

int SubharmoniqCore::GetRhythmDivisor(std::size_t rhythm) const
{
    return sharedCore_.GetRhythmDivisor(rhythm);
}

void SubharmoniqCore::SetRhythmTarget(
    std::size_t rhythm,
    DaisySubharmoniqRhythmTarget target)
{
    sharedCore_.SetRhythmTarget(rhythm, target);
    RefreshSnapshots();
}

DaisySubharmoniqRhythmTarget SubharmoniqCore::GetRhythmTarget(
    std::size_t rhythm) const
{
    return sharedCore_.GetRhythmTarget(rhythm);
}

void SubharmoniqCore::HandleMidiEvent(std::uint8_t status,
                                      std::uint8_t data1,
                                      std::uint8_t data2)
{
    sharedCore_.HandleMidiEvent(status, data1, data2);
}

void SubharmoniqCore::TriggerGate(bool high)
{
    sharedCore_.TriggerGate(high);
}

void SubharmoniqCore::ProcessAudio(float* outputLeft,
                                   float* outputRight,
                                   std::size_t frameCount)
{
    sharedCore_.Process(outputLeft, outputRight, frameCount);
}

bool SubharmoniqCore::SetParameterValue(const char* parameterId,
                                        float       normalizedValue)
{
    if(parameterId == nullptr)
    {
        return false;
    }

    const bool shouldRecordZoom
        = ParameterValueDiffers(sharedCore_, parameterId, normalizedValue);
    const bool changed = sharedCore_.SetParameterValue(parameterId, normalizedValue);
    if(changed)
    {
        if(shouldRecordZoom)
        {
            RecordParameterZoom(parameterId, normalizedValue);
        }
        RefreshSnapshots();
    }
    return changed;
}

void SubharmoniqCore::SetCvInput(std::size_t oneBasedIndex,
                                 float       normalizedValue)
{
    const char* parameterId = nullptr;
    switch(oneBasedIndex)
    {
        case 1: parameterId = "root_cv"; break;
        case 2: parameterId = "cutoff_cv"; break;
        case 3: parameterId = "rhythm_cv"; break;
        case 4: parameterId = "sub_cv"; break;
        default: return;
    }

    const bool shouldRecordZoom
        = ParameterValueDiffers(sharedCore_, parameterId, normalizedValue);
    if(sharedCore_.SetParameterValue(parameterId, normalizedValue))
    {
        if(shouldRecordZoom)
        {
            RecordCvZoom(oneBasedIndex, parameterId, normalizedValue);
        }
        RefreshSnapshots();
    }
}

float SubharmoniqCore::GetGateOutputPulse() const
{
    return sharedCore_.GetGateOutputPulse();
}

DaisySubharmoniqPage SubharmoniqCore::GetActivePage() const
{
    return sharedCore_.GetActivePage();
}

int SubharmoniqCore::GetSequencerStepIndex(std::size_t sequencer) const
{
    return sharedCore_.GetSequencerStepIndex(sequencer);
}

void SubharmoniqCore::SetQuantizeMode(DaisySubharmoniqQuantizeMode mode)
{
    sharedCore_.SetQuantizeMode(mode);
    RefreshSnapshots();
}

DaisySubharmoniqQuantizeMode SubharmoniqCore::GetQuantizeMode() const
{
    return sharedCore_.GetQuantizeMode();
}

void SubharmoniqCore::SetSeqOctaveRange(int octaveRange)
{
    sharedCore_.SetSeqOctaveRange(octaveRange);
    RefreshSnapshots();
}

int SubharmoniqCore::GetSeqOctaveRange() const
{
    return sharedCore_.GetSeqOctaveRange();
}

float SubharmoniqCore::GetSequencerCv(std::size_t sequencer) const
{
    return sharedCore_.GetSequencerCv(sequencer);
}

std::uint32_t SubharmoniqCore::GetTriggerCount() const
{
    return sharedCore_.GetTriggerCount();
}

int SubharmoniqCore::GetCurrentMidiNote() const
{
    return sharedCore_.GetCurrentMidiNote();
}

bool SubharmoniqCore::IsPlaying() const
{
    return sharedCore_.IsPlaying();
}

void SubharmoniqCore::RefreshSnapshots()
{
    RefreshParameters();
    BuildMenuModel();
    BuildDisplay();
}

void SubharmoniqCore::RefreshParameters()
{
    parameters_.clear();
    for(const auto& parameter : sharedCore_.GetParameters())
    {
        parameters_.push_back(ParameterDescriptor{
            MakeParameterId(nodeId_, parameter.id),
            parameter.label,
            parameter.normalizedValue,
            parameter.defaultNormalizedValue,
            parameter.effectiveNormalizedValue,
            "",
            parameter.stepCount,
            ParameterRole::kGeneric,
            parameter.importanceRank,
            parameter.automatable,
            parameter.stateful,
            true});
    }
}

void SubharmoniqCore::BuildMenuModel()
{
    menu_.sections.clear();
    menu_.currentSectionId = MakeMenuSectionId(nodeId_, "pages");
    menu_.currentSelection = 0;

    auto makeReadonly = [](const std::string& id,
                           const std::string& label,
                           const std::string& value) {
        MenuItem item;
        item.id = id;
        item.label = label;
        item.actionKind = MenuItemActionKind::kReadonly;
        item.valueText = value;
        return item;
    };

    MenuSection pages;
    pages.id = MakeMenuSectionId(nodeId_, "pages");
    pages.title = "Pages";
    MenuItem pageItem;
    pageItem.id = MakeMenuItemId(nodeId_, "pages", "page");
    pageItem.label = "Page";
    pageItem.editable = true;
    pageItem.actionKind = MenuItemActionKind::kValue;
    pageItem.normalizedValue
        = static_cast<float>(static_cast<int>(sharedCore_.GetActivePage()))
          / static_cast<float>(kSubharmoniqPageCount - 1);
    pageItem.valueText = FormatPageLabel(sharedCore_.GetActivePage());
    pages.items.push_back(pageItem);
    menu_.sections.push_back(pages);

    MenuSection transport;
    transport.id = MakeMenuSectionId(nodeId_, "transport");
    transport.title = "Transport";
    transport.items.push_back(makeReadonly(MakeMenuItemId(nodeId_, "transport", "state"),
                                           "State",
                                           sharedCore_.IsPlaying() ? "Playing" : "Stopped"));
    transport.items.push_back(makeReadonly(MakeMenuItemId(nodeId_, "transport", "play"),
                                           "Play/Stop",
                                           "Momentary"));
    transport.items.push_back(makeReadonly(MakeMenuItemId(nodeId_, "transport", "reset"),
                                           "Reset",
                                           "Momentary"));
    transport.items.push_back(makeReadonly(MakeMenuItemId(nodeId_, "transport", "panic"),
                                           "Panic",
                                           "Momentary"));
    menu_.sections.push_back(transport);

    MenuSection fieldKeys;
    fieldKeys.id = MakeMenuSectionId(nodeId_, "field_keys");
    fieldKeys.title = "Field Keys";
    for(std::size_t i = 0; i < 16; ++i)
    {
        MenuItem item;
        item.id = MakeMenuItemId(nodeId_, "field_keys", FieldKeyMenuItemSuffix(i));
        item.label = FieldKeyDetailLabel(i);
        item.actionKind = MenuItemActionKind::kMomentary;
        item.valueText = "Momentary";
        fieldKeys.items.push_back(item);
    }
    menu_.sections.push_back(fieldKeys);

    const std::array<std::pair<const char*, const char*>, 5> pageSections = {{
        {"seq_rhythm", "Seq/Rhy"},
        {"voice", "Voice"},
        {"filter", "Filter"},
        {"mix", "Mix"},
        {"about", "About"},
    }};

    for(const auto& sectionInfo : pageSections)
    {
        MenuSection section;
        section.id = MakeMenuSectionId(nodeId_, sectionInfo.first);
        section.title = sectionInfo.second;
        if(std::string(sectionInfo.first) == "about")
        {
            section.items.push_back(makeReadonly(MakeMenuItemId(nodeId_, "about", "version"),
                                                 "Round",
                                                 "1 portable core"));
            section.items.push_back(makeReadonly(MakeMenuItemId(nodeId_, "about", "filter_reminder"),
                                                 "Filter",
                                                 "Round 2: SVF LPF/BPF + ladder LPF"));
        }
        else
        {
            for(const auto& parameter : sharedCore_.GetParameters())
            {
                const bool includeParameter
                    = (std::string(sectionInfo.first) == "seq_rhythm"
                       && (parameter.groupLabel == "Seq"
                           || parameter.groupLabel == "Rhythm"))
                      || (std::string(sectionInfo.first) != "seq_rhythm"
                          && parameter.groupLabel == sectionInfo.second);
                if(includeParameter)
                {
                    MenuItem item;
                    item.id = MakeMenuItemId(nodeId_, sectionInfo.first, "param/")
                              + parameter.id;
                    item.label = parameter.label;
                    item.editable = true;
                    item.actionKind = MenuItemActionKind::kValue;
                    item.normalizedValue = parameter.normalizedValue;
                    item.valueText = FormatPercent(parameter.normalizedValue);
                    section.items.push_back(item);
                }
            }
        }
        menu_.sections.push_back(section);
    }
}

void SubharmoniqCore::RecordParameterZoom(const std::string& parameterId,
                                          float              normalizedValue)
{
#if DAISYHOST_ENABLE_FIELD_OLED_TRANSIENTS
    const std::string knobLabel = FieldKnobLabelForParameter(parameterId);
    const std::string title     = knobLabel.empty() ? parameterId : knobLabel;
    oledTransient_.Show({title,
                         FormatParameterValue(parameterId, normalizedValue),
                         {"returns after 2.0s"},
                         false,
                         true,
                         normalizedValue});
#else
    (void)parameterId;
    (void)normalizedValue;
#endif
}

void SubharmoniqCore::RecordCvZoom(std::size_t       oneBasedIndex,
                                   const std::string& parameterId,
                                   float              normalizedValue)
{
#if DAISYHOST_ENABLE_FIELD_OLED_TRANSIENTS
    static constexpr std::array<const char*, 4> kLabels = {{
        "Root CV",
        "Cutoff CV",
        "Rhythm CV",
        "Sub CV",
    }};
    const std::string label
        = oneBasedIndex >= 1 && oneBasedIndex <= kLabels.size()
              ? kLabels[oneBasedIndex - 1]
              : parameterId;
    oledTransient_.Show({"CV" + std::to_string(oneBasedIndex) + " " + label,
                         FormatParameterValue(parameterId, normalizedValue),
                         {"returns after 2.0s"},
                         false,
                         true,
                         normalizedValue});
#else
    (void)oneBasedIndex;
    (void)parameterId;
    (void)normalizedValue;
#endif
}

void SubharmoniqCore::RecordPageZoom()
{
#if DAISYHOST_ENABLE_FIELD_OLED_TRANSIENTS
    const auto binding = sharedCore_.GetActivePageBinding();
    oledTransient_.Show({"Page",
                         binding.pageLabel,
                         {"SW1<- SW2->"},
                         false,
                         false,
                         0.0f});
#endif
}

void SubharmoniqCore::RecordFieldKeyZoom(std::size_t zeroBasedIndex)
{
#if DAISYHOST_ENABLE_FIELD_OLED_TRANSIENTS
    const std::string keyName = FieldKeyName(zeroBasedIndex);
    if(zeroBasedIndex == 12)
    {
        oledTransient_.Show(
            {keyName + " Quantize",
             FormatQuantizeDetail(sharedCore_.GetQuantizeMode()),
             {"Quant " + FormatQuantize(sharedCore_.GetQuantizeMode())},
             false,
             false,
             0.0f});
        return;
    }
    if(zeroBasedIndex == 13)
    {
        oledTransient_.Show({keyName + " Seq Oct",
                             std::to_string(sharedCore_.GetSeqOctaveRange())
                                 + " octave range",
                             {},
                             false,
                             false,
                             0.0f});
        return;
    }
    if(zeroBasedIndex == 14)
    {
        oledTransient_.Show(
            {keyName + (sharedCore_.IsPlaying() ? " Play" : " Stop"),
             sharedCore_.IsPlaying() ? "Running" : "Stopped",
             {"Step S1:"
                  + std::to_string(sharedCore_.GetSequencerStepIndex(0) + 1)
                  + " S2:"
                  + std::to_string(sharedCore_.GetSequencerStepIndex(1) + 1),
              "Quant: " + FormatQuantizeDetail(sharedCore_.GetQuantizeMode())},
             false,
             false,
             0.0f});
        return;
    }
    if(zeroBasedIndex == 15)
    {
        oledTransient_.Show({keyName + " Reset",
                             "Reset",
                             {"Steps S1:1 S2:1"},
                             false,
                             false,
                             0.0f});
        return;
    }
    if(zeroBasedIndex >= 4 && zeroBasedIndex < 8)
    {
        const std::size_t rhythm = zeroBasedIndex - 4;
        const auto        target = sharedCore_.GetRhythmTarget(rhythm);
        const std::string value  = FormatRhythmTargetDetail(target);
        const std::string detail = target == DaisySubharmoniqRhythmTarget::kBoth
                                       ? "R" + std::to_string(rhythm + 1)
                                             + " advances both"
                                       : "R" + std::to_string(rhythm + 1)
                                             + " routes to " + value;
        oledTransient_.Show({keyName + " Rhythm " + std::to_string(rhythm + 1),
                             value,
                             {detail},
                             false,
                             false,
                             0.0f});
        return;
    }

    oledTransient_.Show({keyName + " " + FieldKeyDetailLabel(zeroBasedIndex),
                         "Selected",
                         {"Seq/Rhy"},
                         false,
                         false,
                         0.0f});
#else
    (void)zeroBasedIndex;
#endif
}

std::string SubharmoniqCore::FormatParameterValue(
    const std::string& parameterId,
    float              normalizedValue) const
{
#if DAISYHOST_ENABLE_FIELD_OLED_TRANSIENTS
    if(parameterId == "cutoff" || parameterId == "cutoff_cv")
    {
        return FormatFrequencyFromNormalized(normalizedValue);
    }
    if(parameterId == "quantize_mode")
    {
        return FormatQuantizeDetail(sharedCore_.GetQuantizeMode());
    }
    if(parameterId == "seq_oct_range")
    {
        return std::to_string(sharedCore_.GetSeqOctaveRange()) + " octave range";
    }
#else
    (void)parameterId;
#endif
    return FormatPercent(normalizedValue);
}

std::string SubharmoniqCore::FieldKnobLabelForParameter(
    const std::string& parameterId) const
{
    const auto binding = sharedCore_.GetActivePageBinding();
    for(std::size_t i = 0; i < binding.parameterIds.size(); ++i)
    {
        if(binding.parameterIds[i] == parameterId)
        {
            return "K" + std::to_string(i + 1) + " " + binding.parameterLabels[i];
        }
    }
    return "";
}

void SubharmoniqCore::BuildDisplay()
{
    display_.texts.clear();
    display_.bars.clear();
    display_.mode = menu_.isOpen ? DisplayMode::kMenu : DisplayMode::kStatus;
    display_.title = "Subharmoniq";
    ++display_.revision;

#if DAISYHOST_ENABLE_FIELD_OLED_TRANSIENTS
    if(oledTransient_.IsVisible())
    {
        oledTransient_.ApplyToDisplay(display_);
        return;
    }
#endif

    const auto binding = sharedCore_.GetActivePageBinding();
    display_.texts.push_back({0, 0, "Subharmoniq " + binding.pageLabel, true});
    display_.texts.push_back({0,
                              10,
                              sharedCore_.IsPlaying() ? "Play" : "Stop",
                              false});
    display_.texts.push_back({42,
                              10,
                              "Quant " + FormatQuantize(sharedCore_.GetQuantizeMode()),
                              false});
    display_.texts.push_back({0,
                              20,
                              "S1 " + std::to_string(sharedCore_.GetSequencerStepIndex(0) + 1)
                                  + " S2 "
                                  + std::to_string(sharedCore_.GetSequencerStepIndex(1) + 1),
                              false});
    display_.texts.push_back({0,
                              30,
                              "R1/"
                                  + FormatRhythmTarget(sharedCore_.GetRhythmTarget(0))
                                  + " R2/"
                                  + FormatRhythmTarget(sharedCore_.GetRhythmTarget(1)),
                              false});
    display_.texts.push_back({0,
                              40,
                              "R3/"
                                  + FormatRhythmTarget(sharedCore_.GetRhythmTarget(2))
                                  + " R4/"
                                  + FormatRhythmTarget(sharedCore_.GetRhythmTarget(3)),
                              false});
    if(sharedCore_.GetActivePage() == DaisySubharmoniqPage::kVcf)
    {
        display_.texts.push_back({0, 52, "R2 SVF/BPF Ladder", false});
    }
    else
    {
        display_.texts.push_back({0, 52, "Assign A5-A8  SW1<- SW2->", false});
    }
}

std::string SubharmoniqCore::StripParameterId(
    const std::string& parameterId) const
{
    const std::string prefix = nodeId_ + "/param/";
    if(parameterId.rfind(prefix, 0) == 0)
    {
        return parameterId.substr(prefix.size());
    }
    return parameterId;
}

std::string SubharmoniqCore::StripControlId(const std::string& controlId) const
{
    const std::string prefix = nodeId_ + "/control/";
    if(controlId.rfind(prefix, 0) == 0)
    {
        return controlId.substr(prefix.size());
    }
    return controlId;
}

std::string SubharmoniqCore::MakeParameterId(const std::string& nodeId,
                                             const std::string& suffix)
{
    return nodeId + "/param/" + suffix;
}

std::string SubharmoniqCore::MakeControlId(const std::string& nodeId,
                                           const std::string& suffix)
{
    return nodeId + "/control/" + suffix;
}

std::string SubharmoniqCore::MakeEncoderControlId(const std::string& nodeId)
{
    return nodeId + "/control/encoder";
}

std::string SubharmoniqCore::MakeEncoderButtonControlId(const std::string& nodeId)
{
    return nodeId + "/control/encoder_button";
}

std::string SubharmoniqCore::MakeCvInputPortId(const std::string& nodeId,
                                               std::size_t       oneBasedIndex)
{
    return nodeId + "/port/cv_in_" + std::to_string(oneBasedIndex);
}

std::string SubharmoniqCore::MakeGateInputPortId(const std::string& nodeId,
                                                 std::size_t       oneBasedIndex)
{
    return nodeId + "/port/gate_in_" + std::to_string(oneBasedIndex);
}

std::string SubharmoniqCore::MakeGateOutputPortId(const std::string& nodeId,
                                                  std::size_t       oneBasedIndex)
{
    return nodeId + "/port/gate_out_" + std::to_string(oneBasedIndex);
}

std::string SubharmoniqCore::MakeMidiInputPortId(const std::string& nodeId,
                                                 std::size_t       oneBasedIndex)
{
    return nodeId + "/port/midi_in_" + std::to_string(oneBasedIndex);
}

std::string SubharmoniqCore::MakeAudioOutputPortId(const std::string& nodeId,
                                                   std::size_t       oneBasedIndex)
{
    return nodeId + "/port/audio_out_" + std::to_string(oneBasedIndex);
}
} // namespace apps
} // namespace daisyhost
