#include "DaisyHostPluginProcessor.h"

#include <algorithm>
#include <cmath>

#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>

#include "DaisyHostPluginEditor.h"
#include "daisyhost/ComputerKeyboardMidi.h"
#include "daisyhost/HostStartupPolicy.h"

namespace
{
daisyhost::MidiMessageEvent ToMidiEvent(const juce::MidiMessage& message)
{
    const auto* rawData = message.getRawData();
    const auto  rawSize = message.getRawDataSize();
    daisyhost::MidiMessageEvent event;
    if(rawSize > 0)
    {
        event.status = rawData[0];
    }
    if(rawSize > 1)
    {
        event.data1 = rawData[1];
    }
    if(rawSize > 2)
    {
        event.data2 = rawData[2];
    }
    return event;
}

std::vector<daisyhost::MidiMessageEvent>
ToMidiEvents(const juce::MidiBuffer& midiBuffer)
{
    std::vector<daisyhost::MidiMessageEvent> events;
    for(const auto metadata : midiBuffer)
    {
        events.push_back(ToMidiEvent(metadata.getMessage()));
    }
    return events;
}

float PeakForSamples(const float* data, int numSamples)
{
    float peak = 0.0f;
    for(int sample = 0; sample < numSamples; ++sample)
    {
        const float magnitude = std::abs(data[sample]);
        if(magnitude > peak)
        {
            peak = magnitude;
        }
    }
    return peak;
}

float PeakForChannel(const juce::AudioBuffer<float>& buffer, int channelIndex)
{
    if(channelIndex < 0 || channelIndex >= buffer.getNumChannels())
    {
        return 0.0f;
    }

    return PeakForSamples(buffer.getReadPointer(channelIndex), buffer.getNumSamples());
}
} // namespace

DaisyHostPatchAudioProcessor::DaisyHostPatchAudioProcessor()
: juce::AudioProcessor(BusesProperties().withInput(
      "Input", juce::AudioChannelSet::stereo(), true)
                           .withOutput(
                               "Output", juce::AudioChannelSet::stereo(), true)),
  boardProfile_(daisyhost::MakeDaisyPatchProfile("node0")),
  core_("node0"),
  dryWetId_(daisyhost::apps::MultiDelayCore::MakeDryWetControlId("node0")),
  encoderButtonId_(
      daisyhost::apps::MultiDelayCore::MakeEncoderButtonControlId("node0")),
  midiInputPortId_(
      daisyhost::apps::MultiDelayCore::MakeMidiInputPortId("node0", 1))
{
    for(std::size_t i = 0; i < knobIds_.size(); ++i)
    {
        knobIds_[i]
            = daisyhost::apps::MultiDelayCore::MakeKnobControlId("node0", i + 1);
        cvPortIds_[i]
            = daisyhost::apps::MultiDelayCore::MakeCvInputPortId("node0", i + 1);
        audioInputPortIds_[i] = daisyhost::apps::MultiDelayCore::MakeAudioInputPortId(
            "node0", i + 1);
        audioOutputPortIds_[i]
            = daisyhost::apps::MultiDelayCore::MakeAudioOutputPortId("node0", i + 1);
        knobValues_[i].store(0.0f);
        cvValues_[i].store(0.5f);
        audioInputPeaks_[i].store(0.0f);
        audioOutputPeaks_[i].store(0.0f);
    }

    for(std::size_t i = 0; i < gatePortIds_.size(); ++i)
    {
        gatePortIds_[i]
            = daisyhost::apps::MultiDelayCore::MakeGateInputPortId("node0", i + 1);
        gateValues_[i].store(false);
        previousGateValues_[i] = false;
    }

    dryWetValue_.store(0.5f);
    encoderPressed_.store(false);
    midiActivity_.store(0.0f);
    computerKeyboardEnabled_.store(true);
    computerKeyboardOctave_.store(4);
    testInputMode_.store(kHostInput);
    testInputLevel_.store(0.35f);
    impulseRequested_.store(false);
    latestDisplay_ = core_.GetDisplayModel();
}

DaisyHostPatchAudioProcessor::~DaisyHostPatchAudioProcessor() {}

void DaisyHostPatchAudioProcessor::prepareToPlay(double sampleRate,
                                                 int    samplesPerBlock)
{
    if(!didApplyStartupTestInputPolicy_)
    {
        SetTestInputMode(daisyhost::ResolveStartupTestInputMode(
            wrapperType == juce::AudioProcessor::wrapperType_Standalone,
            hasRestoredTestInputMode_,
            GetTestInputMode()));
        didApplyStartupTestInputPolicy_ = true;
    }

    core_.Prepare(sampleRate, static_cast<std::size_t>(samplesPerBlock));
    midiNotePreview_.Prepare(sampleRate);
    RefreshMidiInputStatus();
    scratchOutput_.setSize(4, samplesPerBlock, false, false, true);
    zeroBuffer_.assign(static_cast<std::size_t>(samplesPerBlock), 0.0f);
    generatedInput_.assign(static_cast<std::size_t>(samplesPerBlock), 0.0f);
    testPhase_ = 0.0f;
    ApplyControlStateToCore();
    ApplyVirtualPortStateToCore(std::vector<daisyhost::MidiMessageEvent>());
    UpdateDisplaySnapshot();
}

void DaisyHostPatchAudioProcessor::releaseResources()
{
    AllVirtualKeyboardNotesOff();
    scratchOutput_.setSize(0, 0);
    zeroBuffer_.clear();
    generatedInput_.clear();
}

bool DaisyHostPatchAudioProcessor::isBusesLayoutSupported(
    const BusesLayout& layouts) const
{
    const auto inputSet  = layouts.getMainInputChannelSet();
    const auto outputSet = layouts.getMainOutputChannelSet();

    const bool validInput
        = inputSet == juce::AudioChannelSet::mono()
          || inputSet == juce::AudioChannelSet::stereo();
    const bool validOutput
        = outputSet == juce::AudioChannelSet::mono()
          || outputSet == juce::AudioChannelSet::stereo()
          || outputSet == juce::AudioChannelSet::discreteChannels(4);

    return validInput && validOutput;
}

void DaisyHostPatchAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                                juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    const int              numSamples = buffer.getNumSamples();

    virtualKeyboardState_.processNextMidiBuffer(
        midiMessages, 0, numSamples, true);

    midiActivity_.store(std::max(0.0f, midiActivity_.load() - 0.05f));

    for(const auto metadata : midiMessages)
    {
        const auto& message = metadata.getMessage();
        const auto  event   = ToMidiEvent(message);
        midiNotePreview_.HandleMidiEvent(event);
        midiEventTracker_.Record(event);

        if(message.isController())
        {
            const int   cc         = message.getControllerNumber();
            const float normalized = Clamp01(message.getControllerValue() / 127.0f);
            std::string mappedControl;

            {
                std::lock_guard<std::mutex> lock(midiLearnMutex_);
                if(!learningTargetId_.empty())
                {
                    midiLearnMap_.Assign(cc, learningTargetId_);
                    mappedControl = learningTargetId_;
                    learningTargetId_.clear();
                }
                else
                {
                    midiLearnMap_.TryLookup(cc, &mappedControl);
                }
            }

            if(mappedControl == dryWetId_)
            {
                SetDryWetValue(normalized);
            }
            else if(!mappedControl.empty())
            {
                for(std::size_t i = 0; i < knobIds_.size(); ++i)
                {
                    if(mappedControl == knobIds_[i])
                    {
                        SetKnobValue(i, normalized);
                        break;
                    }
                }
            }

            midiActivity_.store(1.0f);
        }
        else if(message.isNoteOn())
        {
            impulseRequested_.store(true);
            midiActivity_.store(1.0f);
        }
    }

    ApplyControlStateToCore();
    ApplyVirtualPortStateToCore(ToMidiEvents(midiMessages));

    if(scratchOutput_.getNumSamples() != numSamples)
    {
        scratchOutput_.setSize(4, numSamples, false, false, true);
    }
    scratchOutput_.clear();

    if(static_cast<int>(zeroBuffer_.size()) < numSamples)
    {
        zeroBuffer_.assign(static_cast<std::size_t>(numSamples), 0.0f);
    }
    if(static_cast<int>(generatedInput_.size()) < numSamples)
    {
        generatedInput_.assign(static_cast<std::size_t>(numSamples), 0.0f);
    }

    std::array<const float*, 4> inputPtrs = {
        {zeroBuffer_.data(), zeroBuffer_.data(), zeroBuffer_.data(), zeroBuffer_.data()}};

    if(testInputMode_.load() == kHostInput)
    {
        for(int channel = 0;
            channel < std::min(buffer.getNumChannels(), getTotalNumInputChannels())
            && channel < static_cast<int>(inputPtrs.size());
            ++channel)
        {
            inputPtrs[static_cast<std::size_t>(channel)] = buffer.getReadPointer(channel);
        }

        for(std::size_t i = 0; i < audioInputPeaks_.size(); ++i)
        {
            audioInputPeaks_[i].store(PeakForChannel(buffer, static_cast<int>(i)));
        }
    }
    else
    {
        GenerateTestInput(generatedInput_.data(), numSamples);
        inputPtrs[0] = generatedInput_.data();

        audioInputPeaks_[0].store(PeakForSamples(generatedInput_.data(), numSamples));
        for(std::size_t i = 1; i < audioInputPeaks_.size(); ++i)
        {
            audioInputPeaks_[i].store(0.0f);
        }
    }

    if(wrapperType == juce::AudioProcessor::wrapperType_Standalone)
    {
        if(testInputMode_.load() == kHostInput)
        {
            std::fill(generatedInput_.begin(),
                      generatedInput_.begin() + numSamples,
                      0.0f);
            if(getTotalNumInputChannels() > 0 && buffer.getNumChannels() > 0)
            {
                std::copy(buffer.getReadPointer(0),
                          buffer.getReadPointer(0) + numSamples,
                          generatedInput_.begin());
            }
        }

        midiNotePreview_.RenderAdd(
            generatedInput_.data(),
            static_cast<std::size_t>(numSamples),
            testInputLevel_.load());
        inputPtrs[0] = generatedInput_.data();
        audioInputPeaks_[0].store(PeakForSamples(generatedInput_.data(), numSamples));
    }

    std::array<float*, 4> outputPtrs = {
        {scratchOutput_.getWritePointer(0),
         scratchOutput_.getWritePointer(1),
         scratchOutput_.getWritePointer(2),
         scratchOutput_.getWritePointer(3)}};

    core_.Process({inputPtrs.data(), inputPtrs.size()},
                  {outputPtrs.data(), outputPtrs.size()},
                  static_cast<std::size_t>(numSamples));
    core_.TickUi((1000.0 * static_cast<double>(numSamples)) / getSampleRate());

    for(std::size_t i = 0; i < audioOutputPeaks_.size(); ++i)
    {
        audioOutputPeaks_[i].store(
            PeakForChannel(scratchOutput_, static_cast<int>(i)));
    }

    buffer.clear();
    if(getTotalNumOutputChannels() >= 4)
    {
        for(int channel = 0; channel < 4; ++channel)
        {
            buffer.copyFrom(channel, 0, scratchOutput_, channel, 0, numSamples);
        }
    }
    else if(getTotalNumOutputChannels() == 2)
    {
        buffer.copyFrom(0, 0, scratchOutput_, 3, 0, numSamples);
        buffer.copyFrom(1, 0, scratchOutput_, 3, 0, numSamples);
    }
    else if(getTotalNumOutputChannels() == 1)
    {
        buffer.copyFrom(0, 0, scratchOutput_, 3, 0, numSamples);
    }

    UpdateDisplaySnapshot();
}

juce::AudioProcessorEditor* DaisyHostPatchAudioProcessor::createEditor()
{
    return new DaisyHostPatchAudioProcessorEditor(*this);
}

bool DaisyHostPatchAudioProcessor::hasEditor() const
{
    return true;
}

const juce::String DaisyHostPatchAudioProcessor::getName() const
{
    return "DaisyHost Patch";
}

bool DaisyHostPatchAudioProcessor::acceptsMidi() const
{
    return true;
}

bool DaisyHostPatchAudioProcessor::producesMidi() const
{
    return true;
}

bool DaisyHostPatchAudioProcessor::isMidiEffect() const
{
    return false;
}

double DaisyHostPatchAudioProcessor::getTailLengthSeconds() const
{
    return 1.0;
}

int DaisyHostPatchAudioProcessor::getNumPrograms()
{
    return 1;
}

int DaisyHostPatchAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DaisyHostPatchAudioProcessor::setCurrentProgram(int)
{
}

const juce::String DaisyHostPatchAudioProcessor::getProgramName(int)
{
    return {};
}

void DaisyHostPatchAudioProcessor::changeProgramName(int,
                                                     const juce::String&)
{
}

void DaisyHostPatchAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    daisyhost::HostSessionState state;
    for(std::size_t i = 0; i < knobIds_.size(); ++i)
    {
        state.controlValues[knobIds_[i]] = knobValues_[i].load();
        state.cvValues[cvPortIds_[i]]    = cvValues_[i].load();
    }

    for(std::size_t i = 0; i < gatePortIds_.size(); ++i)
    {
        state.gateValues[gatePortIds_[i]] = gateValues_[i].load();
    }

    state.controlValues[dryWetId_] = dryWetValue_.load();
    state.controlValues[computerKeyboardEnabledStateId_]
        = computerKeyboardEnabled_.load() ? 1.0f : 0.0f;
    state.controlValues[computerKeyboardOctaveStateId_]
        = static_cast<float>(computerKeyboardOctave_.load());
    state.controlValues[testInputModeStateId_]
        = static_cast<float>(testInputMode_.load());
    state.controlValues[testInputLevelStateId_] = testInputLevel_.load();

    {
        std::lock_guard<std::mutex> lock(midiLearnMutex_);
        state.midiLearn = midiLearnMap_;
    }

    const auto serialized = state.Serialize();
    destData.reset();
    destData.append(serialized.data(), serialized.size());
}

void DaisyHostPatchAudioProcessor::setStateInformation(const void* data,
                                                       int         sizeInBytes)
{
    const std::string text(static_cast<const char*>(data),
                           static_cast<std::size_t>(sizeInBytes));
    LoadSession(daisyhost::HostSessionState::Deserialize(text));
}

const daisyhost::BoardProfile& DaisyHostPatchAudioProcessor::GetBoardProfile() const
{
    return boardProfile_;
}

float DaisyHostPatchAudioProcessor::GetKnobValue(std::size_t index) const
{
    return index < knobValues_.size() ? knobValues_[index].load() : 0.0f;
}

float DaisyHostPatchAudioProcessor::GetDryWetValue() const
{
    return dryWetValue_.load();
}

int DaisyHostPatchAudioProcessor::GetDryWetPercent() const
{
    return static_cast<int>(std::round(dryWetValue_.load() * 100.0f));
}

bool DaisyHostPatchAudioProcessor::GetEncoderPressed() const
{
    return encoderPressed_.load();
}

void DaisyHostPatchAudioProcessor::SetKnobValue(std::size_t index,
                                                float       normalizedValue)
{
    if(index < knobValues_.size())
    {
        knobValues_[index].store(Clamp01(normalizedValue));
    }
}

void DaisyHostPatchAudioProcessor::SetDryWetValue(float normalizedValue)
{
    dryWetValue_.store(Clamp01(normalizedValue));
}

void DaisyHostPatchAudioProcessor::SetEncoderPressed(bool pressed)
{
    encoderPressed_.store(pressed);
}

float DaisyHostPatchAudioProcessor::GetCvValue(std::size_t index) const
{
    return index < cvValues_.size() ? cvValues_[index].load() : 0.0f;
}

void DaisyHostPatchAudioProcessor::SetCvValue(std::size_t index,
                                              float       normalizedValue)
{
    if(index < cvValues_.size())
    {
        cvValues_[index].store(Clamp01(normalizedValue));
    }
}

bool DaisyHostPatchAudioProcessor::GetGateValue(std::size_t index) const
{
    return index < gateValues_.size() ? gateValues_[index].load() : false;
}

void DaisyHostPatchAudioProcessor::SetGateValue(std::size_t index, bool enabled)
{
    if(index < gateValues_.size())
    {
        gateValues_[index].store(enabled);
    }
}

float DaisyHostPatchAudioProcessor::GetAudioInputPeak(std::size_t index) const
{
    return index < audioInputPeaks_.size() ? audioInputPeaks_[index].load() : 0.0f;
}

float DaisyHostPatchAudioProcessor::GetAudioOutputPeak(std::size_t index) const
{
    return index < audioOutputPeaks_.size()
               ? audioOutputPeaks_[index].load()
               : 0.0f;
}

float DaisyHostPatchAudioProcessor::GetMidiActivity() const
{
    return midiActivity_.load();
}

juce::String DaisyHostPatchAudioProcessor::GetMidiInputStatusText() const
{
    const auto snapshot = midiEventTracker_.GetSnapshot();
    if(wrapperType == juce::AudioProcessor::wrapperType_Standalone)
    {
        return "MIDI In: " + juce::String(snapshot.enabledInputs) + "/"
               + juce::String(snapshot.availableInputs)
               + " enabled in Settings";
    }

    return "MIDI In: provided by host";
}

juce::StringArray DaisyHostPatchAudioProcessor::GetRecentMidiEventLines() const
{
    juce::StringArray lines;
    const auto        snapshot = midiEventTracker_.GetSnapshot();
    for(const auto& message : snapshot.recentMessages)
    {
        lines.add(message);
    }
    if(lines.isEmpty())
    {
        lines.add("No MIDI events received yet");
    }
    return lines;
}

daisyhost::DisplayModel DaisyHostPatchAudioProcessor::GetDisplayModelSnapshot() const
{
    std::lock_guard<std::mutex> lock(displayMutex_);
    return latestDisplay_;
}

juce::MidiKeyboardState& DaisyHostPatchAudioProcessor::GetVirtualKeyboardState()
{
    return virtualKeyboardState_;
}

bool DaisyHostPatchAudioProcessor::GetComputerKeyboardEnabled() const
{
    return computerKeyboardEnabled_.load();
}

void DaisyHostPatchAudioProcessor::SetComputerKeyboardEnabled(bool enabled)
{
    computerKeyboardEnabled_.store(enabled);
    if(!enabled)
    {
        AllVirtualKeyboardNotesOff();
    }
}

int DaisyHostPatchAudioProcessor::GetComputerKeyboardOctave() const
{
    return computerKeyboardOctave_.load();
}

void DaisyHostPatchAudioProcessor::SetComputerKeyboardOctave(int octave)
{
    computerKeyboardOctave_.store(
        daisyhost::ComputerKeyboardMidi::ClampOctave(octave));
}

void DaisyHostPatchAudioProcessor::SetVirtualKeyboardNote(int midiNote,
                                                          bool isDown,
                                                          float velocity)
{
    if(midiNote < 0 || midiNote > 127)
    {
        return;
    }

    if(isDown)
    {
        virtualKeyboardState_.noteOn(1, midiNote, velocity);
    }
    else
    {
        virtualKeyboardState_.noteOff(1, midiNote, velocity);
    }
}

void DaisyHostPatchAudioProcessor::AllVirtualKeyboardNotesOff()
{
    for(int channel = 1; channel <= 16; ++channel)
    {
        virtualKeyboardState_.allNotesOff(channel);
    }
}

int DaisyHostPatchAudioProcessor::GetTestInputMode() const
{
    return testInputMode_.load();
}

void DaisyHostPatchAudioProcessor::SetTestInputMode(int mode)
{
    if(mode < 0)
    {
        mode = 0;
    }
    if(mode >= kNumTestInputModes)
    {
        mode = kNumTestInputModes - 1;
    }
    testInputMode_.store(mode);
}

float DaisyHostPatchAudioProcessor::GetTestInputLevel() const
{
    return testInputLevel_.load();
}

void DaisyHostPatchAudioProcessor::SetTestInputLevel(float level)
{
    testInputLevel_.store(Clamp01(level));
}

void DaisyHostPatchAudioProcessor::TriggerImpulse()
{
    impulseRequested_.store(true);
}

juce::String DaisyHostPatchAudioProcessor::GetTestInputModeName(int mode) const
{
    switch(mode)
    {
        case kHostInput: return "Host In";
        case kSineInput: return "Sine";
        case kNoiseInput: return "Noise";
        case kImpulseInput: return "Impulse";
        default: return "Unknown";
    }
}

void DaisyHostPatchAudioProcessor::BeginMidiLearn(const std::string& controlId)
{
    std::lock_guard<std::mutex> lock(midiLearnMutex_);
    learningTargetId_ = controlId;
}

void DaisyHostPatchAudioProcessor::CancelMidiLearn()
{
    std::lock_guard<std::mutex> lock(midiLearnMutex_);
    learningTargetId_.clear();
}

bool DaisyHostPatchAudioProcessor::IsLearning(const std::string& controlId) const
{
    std::lock_guard<std::mutex> lock(midiLearnMutex_);
    return learningTargetId_ == controlId;
}

std::string DaisyHostPatchAudioProcessor::GetMidiBindingLabel(
    const std::string& controlId) const
{
    std::lock_guard<std::mutex> lock(midiLearnMutex_);
    const auto bindings = midiLearnMap_.Bindings();
    for(const auto& binding : bindings)
    {
        if(binding.controlId == controlId)
        {
            return "CC " + std::to_string(binding.cc);
        }
    }
    return "Learn CC";
}

float DaisyHostPatchAudioProcessor::Clamp01(float value) const
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

void DaisyHostPatchAudioProcessor::ApplyControlStateToCore()
{
    for(std::size_t i = 0; i < knobIds_.size(); ++i)
    {
        const float effectiveValue
            = Clamp01(knobValues_[i].load() + (cvValues_[i].load() - 0.5f));
        core_.SetControl(knobIds_[i], effectiveValue);
    }
    core_.SetControl(dryWetId_, dryWetValue_.load());
    core_.SetControl(encoderButtonId_, encoderPressed_.load() ? 1.0f : 0.0f);
}

void DaisyHostPatchAudioProcessor::ApplyVirtualPortStateToCore(
    const std::vector<daisyhost::MidiMessageEvent>& midiEvents)
{
    for(std::size_t i = 0; i < cvPortIds_.size(); ++i)
    {
        daisyhost::PortValue value;
        value.type   = daisyhost::VirtualPortType::kCv;
        value.scalar = cvValues_[i].load();
        core_.SetPortInput(cvPortIds_[i], value);
    }

    for(std::size_t i = 0; i < gatePortIds_.size(); ++i)
    {
        const bool gateState = gateValues_[i].load();

        daisyhost::PortValue value;
        value.type = daisyhost::VirtualPortType::kGate;
        value.gate = gateState;
        core_.SetPortInput(gatePortIds_[i], value);

        if(i == 0 && gateState && !previousGateValues_[i])
        {
            impulseRequested_.store(true);
        }
        previousGateValues_[i] = gateState;
    }

    daisyhost::PortValue midiValue;
    midiValue.type       = daisyhost::VirtualPortType::kMidi;
    midiValue.midiEvents = midiEvents;
    core_.SetPortInput(midiInputPortId_, midiValue);

    if(!midiEvents.empty())
    {
        midiActivity_.store(1.0f);
    }
}

void DaisyHostPatchAudioProcessor::UpdateDisplaySnapshot()
{
    std::lock_guard<std::mutex> lock(displayMutex_);
    latestDisplay_ = core_.GetDisplayModel();
}

void DaisyHostPatchAudioProcessor::LoadSession(
    const daisyhost::HostSessionState& state)
{
    for(std::size_t i = 0; i < knobIds_.size(); ++i)
    {
        const auto controlIt = state.controlValues.find(knobIds_[i]);
        if(controlIt != state.controlValues.end())
        {
            knobValues_[i].store(Clamp01(controlIt->second));
        }

        const auto cvIt = state.cvValues.find(cvPortIds_[i]);
        if(cvIt != state.cvValues.end())
        {
            cvValues_[i].store(Clamp01(cvIt->second));
        }
    }

    for(std::size_t i = 0; i < gatePortIds_.size(); ++i)
    {
        const auto gateIt = state.gateValues.find(gatePortIds_[i]);
        if(gateIt != state.gateValues.end())
        {
            gateValues_[i].store(gateIt->second);
        }
    }

    const auto dryWetIt = state.controlValues.find(dryWetId_);
    if(dryWetIt != state.controlValues.end())
    {
        dryWetValue_.store(Clamp01(dryWetIt->second));
    }

    const auto keyboardEnabledIt
        = state.controlValues.find(computerKeyboardEnabledStateId_);
    if(keyboardEnabledIt != state.controlValues.end())
    {
        SetComputerKeyboardEnabled(keyboardEnabledIt->second >= 0.5f);
    }

    const auto keyboardOctaveIt
        = state.controlValues.find(computerKeyboardOctaveStateId_);
    if(keyboardOctaveIt != state.controlValues.end())
    {
        SetComputerKeyboardOctave(
            static_cast<int>(std::round(keyboardOctaveIt->second)));
    }

    const auto testModeIt = state.controlValues.find(testInputModeStateId_);
    if(testModeIt != state.controlValues.end())
    {
        hasRestoredTestInputMode_ = true;
        SetTestInputMode(static_cast<int>(std::round(testModeIt->second)));
    }

    const auto testLevelIt = state.controlValues.find(testInputLevelStateId_);
    if(testLevelIt != state.controlValues.end())
    {
        SetTestInputLevel(testLevelIt->second);
    }

    {
        std::lock_guard<std::mutex> lock(midiLearnMutex_);
        midiLearnMap_ = state.midiLearn;
        learningTargetId_.clear();
    }
}

void DaisyHostPatchAudioProcessor::GenerateTestInput(float* destination,
                                                     int    numSamples)
{
    std::fill(destination, destination + numSamples, 0.0f);

    const float level = testInputLevel_.load();
    switch(testInputMode_.load())
    {
        case kHostInput: break;

        case kSineInput:
        {
            const float phaseIncrement
                = static_cast<float>((juce::MathConstants<double>::twoPi * 220.0)
                                     / getSampleRate());
            for(int sample = 0; sample < numSamples; ++sample)
            {
                destination[sample] = std::sin(testPhase_) * level;
                testPhase_ += phaseIncrement;
                if(testPhase_ > juce::MathConstants<float>::twoPi)
                {
                    testPhase_ -= juce::MathConstants<float>::twoPi;
                }
            }
            break;
        }

        case kNoiseInput:
            for(int sample = 0; sample < numSamples; ++sample)
            {
                destination[sample]
                    = (noise_.nextFloat() * 2.0f - 1.0f) * level;
            }
            break;

        case kImpulseInput:
            if(impulseRequested_.exchange(false))
            {
                destination[0] = level;
            }
            break;

        default: break;
    }
}

void DaisyHostPatchAudioProcessor::RefreshMidiInputStatus()
{
    if(wrapperType != juce::AudioProcessor::wrapperType_Standalone)
    {
        return;
    }

    int availableInputs = 0;
    int enabledInputs   = 0;
    if(auto* holder = juce::StandalonePluginHolder::getInstance())
    {
        const auto devices = juce::MidiInput::getAvailableDevices();
        availableInputs    = devices.size();
        for(const auto& device : devices)
        {
            if(holder->deviceManager.isMidiInputDeviceEnabled(device.identifier))
            {
                ++enabledInputs;
            }
        }
    }

    midiEventTracker_.SetInputDeviceCounts(availableInputs, enabledInputs);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DaisyHostPatchAudioProcessor();
}
