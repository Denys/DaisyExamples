#include "daisyhost/apps/MultiDelayCore.h"

#include <algorithm>
#include <cmath>

namespace daisyhost
{
namespace apps
{
namespace
{
std::string MakeIndexedId(const std::string& nodeId,
                          const std::string& section,
                          const std::string& stem,
                          std::size_t        oneBasedIndex)
{
    return nodeId + "/" + section + "/" + stem + std::to_string(oneBasedIndex);
}

float AbsMax(float currentPeak, float sample)
{
    const float magnitude = std::fabs(sample);
    return magnitude > currentPeak ? magnitude : currentPeak;
}
} // namespace

MultiDelayCore::MultiDelayCore(const std::string& nodeId)
: nodeId_(nodeId)
{
    controlsNormalized_.fill(0.0f);

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

    UpdateDisplay();
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

    UpdateMappedStateFromControls();
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
            controlsNormalized_[i] = value;
            UpdateMappedStateFromControls();
            return;
        }
    }

    if(controlId == MakeDryWetControlId(nodeId_))
    {
        dryWetPercent_ = static_cast<int>(std::round(value * 100.0f));
        UpdateDisplay();
        return;
    }

    if(controlId == MakeEncoderButtonControlId(nodeId_))
    {
        encoderPressed_ = value >= 0.5f;
    }
}

void MultiDelayCore::SetEncoderDelta(int delta)
{
    dryWetPercent_ += delta * 5;
    if(dryWetPercent_ < 0)
    {
        dryWetPercent_ = 0;
    }
    if(dryWetPercent_ > 100)
    {
        dryWetPercent_ = 100;
    }
    UpdateDisplay();
}

void MultiDelayCore::SetEncoderPress(bool pressed)
{
    encoderPressed_ = pressed;
}

void MultiDelayCore::SetPortInput(const std::string& portId, const PortValue& value)
{
    portInputs_[portId] = value;
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

void MultiDelayCore::UpdateMappedStateFromControls()
{
    for(std::size_t i = 0; i < kDelayCount; ++i)
    {
        delays_[i].targetDelay = MapLogControl(
            controlsNormalized_[i],
            static_cast<float>(sampleRate_ * 0.05),
            static_cast<float>(kMaxDelaySamples));
    }

    feedback_ = controlsNormalized_[3];
}

void MultiDelayCore::UpdateDisplay()
{
    ++display_.revision;
    display_.texts.clear();
    display_.bars.clear();

    display_.texts.push_back({0, 0, "Multi Delay", false});
    display_.texts.push_back({0, 24, "Dry/Wet:", false});
    display_.texts.push_back(
        {64, 24, std::to_string(dryWetPercent_) + "%", false});
    display_.bars.push_back(
        {0, 44, 96, 10, static_cast<float>(dryWetPercent_) / 100.0f});
}
} // namespace apps
} // namespace daisyhost
