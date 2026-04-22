#include "DaisyHostPluginProcessor.h"

#include <algorithm>
#include <cmath>

#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>

#include "DaisyHostPluginEditor.h"
#include "daisyhost/AppRegistry.h"
#include "daisyhost/ComputerKeyboardMidi.h"
#include "daisyhost/HostStartupPolicy.h"
#include "daisyhost/TestInputSignal.h"
#include "daisyhost/VersionInfo.h"

namespace
{
constexpr float kAutomationParameterEpsilon = 0.0001f;

daisyhost::CvInputSourceMode ClampCvMode(int mode)
{
    return mode <= 0 ? daisyhost::CvInputSourceMode::kManual
                     : daisyhost::CvInputSourceMode::kGenerator;
}

std::string MakeCvHostStateId(std::size_t index, const char* suffix)
{
    return "node0/host/cv_" + std::to_string(index + 1) + "_" + suffix;
}

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

const daisyhost::MenuItem* FindMenuItem(const daisyhost::MenuModel& menu,
                                        const std::string&          itemId)
{
    for(const auto& section : menu.sections)
    {
        for(const auto& item : section.items)
        {
            if(item.id == itemId)
            {
                return &item;
            }
        }
    }
    return nullptr;
}

std::optional<daisyhost::HubStartupRequest> LoadHubStartupRequestFromCommandLine()
{
    daisyhost::HubStartupRequest request;
    const auto arguments = juce::JUCEApplicationBase::getCommandLineParameterArray();
    for(int index = 0; index < arguments.size(); ++index)
    {
        const auto argument = arguments[index];
        if(argument == "--app" && index + 1 < arguments.size())
        {
            request.appId = arguments[++index].toStdString();
        }
        else if(argument == "--board" && index + 1 < arguments.size())
        {
            request.boardId = arguments[++index].toStdString();
        }
    }

    if(request.appId.empty() && request.boardId.empty())
    {
        return std::nullopt;
    }

    return request;
}

bool HasMeaningfulAutomationDelta(float left, float right)
{
    return std::abs(left - right) > kAutomationParameterEpsilon;
}
} // namespace

class DaisyHostAutomationParameter final : public juce::AudioParameterFloat
{
  public:
    DaisyHostAutomationParameter(std::size_t                        slotIndex,
                                 DaisyHostPatchAudioProcessor&      owner)
    : juce::AudioParameterFloat(
          juce::ParameterID(daisyhost::MakeHostAutomationSlotId(slotIndex), 1),
          daisyhost::MakeHostAutomationSlotName(slotIndex),
          juce::NormalisableRange<float>(0.0f, 1.0f),
          0.0f),
      slotIndex_(slotIndex),
      owner_(owner)
    {
    }

  protected:
    void valueChanged(float newValue) override
    {
        owner_.HandleAutomationParameterValueChanged(slotIndex_, newValue);
    }

  private:
    std::size_t                   slotIndex_;
    DaisyHostPatchAudioProcessor& owner_;
};

DaisyHostPatchAudioProcessor::DaisyHostPatchAudioProcessor()
: juce::AudioProcessor(BusesProperties().withInput(
      "Input", juce::AudioChannelSet::stereo(), true)
                           .withOutput(
                               "Output", juce::AudioChannelSet::stereo(), true)),
  boardProfile_(daisyhost::MakeDaisyPatchProfile("node0")),
  core_(daisyhost::CreateHostedAppCore(
      daisyhost::GetDefaultHostedAppId(), "node0", &activeAppId_))
{
    for(std::size_t slotIndex = 0; slotIndex < automationParameters_.size();
        ++slotIndex)
    {
        auto* parameter
            = new DaisyHostAutomationParameter(slotIndex, *this);
        automationParameters_[slotIndex] = parameter;
        pendingAutomationParameterValues_[slotIndex].store(0.0f);
        pendingAutomationParameterDirty_[slotIndex].store(false);
        addParameter(parameter);
    }

    pendingHubStartupRequest_ = LoadHubStartupRequestFromCommandLine();
    if(!pendingHubStartupRequest_.has_value())
    {
        pendingHubStartupRequest_
            = daisyhost::LoadAndConsumeHubStartupRequest(
                daisyhost::GetDefaultHubLaunchRequestFile());
    }
    if(pendingHubStartupRequest_.has_value()
       && !pendingHubStartupRequest_->appId.empty())
    {
        RecreateHostedApp(pendingHubStartupRequest_->appId);
    }
    activeBindings_ = core_->GetPatchBindings();
    for(std::size_t i = 0; i < topControlValues_.size(); ++i)
    {
        topControlValues_[i].store(0.0f);
        cvValues_[i].store(0.5f);
        cvVoltages_[i].store(2.5f);
        audioInputPeaks_[i].store(0.0f);
        audioOutputPeaks_[i].store(0.0f);
        cvGeneratorStates_[i].manualVolts    = 2.5f;
        cvGeneratorStates_[i].biasVolts      = 2.5f;
        cvGeneratorStates_[i].amplitudeVolts = 2.5f;
        cvGeneratorStates_[i].frequencyHz    = 1.0f;
    }
    for(std::size_t i = 0; i < gateValues_.size(); ++i)
    {
        gateValues_[i].store(false);
        previousGateValues_[i] = false;
    }
    encoderPressed_.store(false);
    midiActivity_.store(0.0f);
    computerKeyboardEnabled_.store(true);
    computerKeyboardOctave_.store(4);
    testInputMode_.store(kHostInput);
    testInputLevel_.store(3.5f);
    testInputFrequencyHz_.store(220.0f);
    impulseRequested_.store(false);
    pendingEncoderDelta_.store(0);
    pendingEncoderPressCount_.store(0);
    SyncHostStateFromCore();
    latestDisplay_ = core_->GetDisplayModel();
    latestMenu_    = core_->GetMenuModel();
    latestParameters_ = core_->GetParameters();
    automationSlotBindings_
        = daisyhost::BuildHostAutomationSlotBindings(latestParameters_);
    SyncAutomationParametersFromCore();
}

DaisyHostPatchAudioProcessor::~DaisyHostPatchAudioProcessor() {}

void DaisyHostPatchAudioProcessor::prepareToPlay(double sampleRate,
                                                 int    samplesPerBlock)
{
    currentSampleRate_ = sampleRate;
    currentBlockSize_  = static_cast<std::size_t>(std::max(1, samplesPerBlock));

    if(!didApplyStartupTestInputPolicy_)
    {
        SetTestInputMode(daisyhost::ResolveStartupTestInputMode(
            wrapperType == juce::AudioProcessor::wrapperType_Standalone,
            hasRestoredTestInputMode_,
            GetTestInputMode()));
        didApplyStartupTestInputPolicy_ = true;
    }

    ApplyHubStartupRequestIfNeeded();
    core_->Prepare(sampleRate, static_cast<std::size_t>(samplesPerBlock));
    ApplyCanonicalSessionStateToCore();
    midiNotePreview_.Prepare(sampleRate);
    RefreshMidiInputStatus();
    scratchOutput_.setSize(4, samplesPerBlock, false, false, true);
    zeroBuffer_.assign(static_cast<std::size_t>(samplesPerBlock), 0.0f);
    generatedInput_.assign(static_cast<std::size_t>(samplesPerBlock), 0.0f);
    testPhase_ = 0.0f;
    noiseState_ = 1u;
    ApplyControlStateToCore();
    ApplyPendingMenuInteractionsToCore();
    UpdateCvGeneratorOutputs(static_cast<double>(currentBlockSize_) / sampleRate);
    ApplyVirtualPortStateToCore(std::vector<daisyhost::MidiMessageEvent>());
    UpdateCoreSnapshots();
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
    isProcessingBlock_.store(true);

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

            if(!mappedControl.empty())
            {
                for(std::size_t i = 0; i < activeBindings_.knobControlIds.size(); ++i)
                {
                    if(mappedControl == activeBindings_.knobControlIds[i])
                    {
                        SetTopControlValue(i, normalized);
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

    ApplyPendingAutomationParametersToCore();
    ApplyControlStateToCore();
    ApplyPendingMenuInteractionsToCore();
    UpdateCvGeneratorOutputs(static_cast<double>(numSamples) / getSampleRate());
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
            Clamp01(testInputLevel_.load() / 10.0f));
        inputPtrs[0] = generatedInput_.data();
        audioInputPeaks_[0].store(PeakForSamples(generatedInput_.data(), numSamples));
    }

    std::array<float*, 4> outputPtrs = {
        {scratchOutput_.getWritePointer(0),
         scratchOutput_.getWritePointer(1),
         scratchOutput_.getWritePointer(2),
         scratchOutput_.getWritePointer(3)}};

    core_->Process({inputPtrs.data(), inputPtrs.size()},
                   {outputPtrs.data(), outputPtrs.size()},
                   static_cast<std::size_t>(numSamples));
    core_->TickUi((1000.0 * static_cast<double>(numSamples)) / getSampleRate());
    UpdateCoreSnapshots();
    SyncHostStateFromCore();

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
        const int leftChannel = std::clamp(activeBindings_.mainOutputChannels[0], 0, 3);
        const int rightChannel = std::clamp(activeBindings_.mainOutputChannels[1], 0, 3);
        buffer.copyFrom(0, 0, scratchOutput_, leftChannel, 0, numSamples);
        buffer.copyFrom(1, 0, scratchOutput_, rightChannel, 0, numSamples);
    }
    else if(getTotalNumOutputChannels() == 1)
    {
        const int monoChannel = std::clamp(activeBindings_.mainOutputChannels[0], 0, 3);
        buffer.copyFrom(0, 0, scratchOutput_, monoChannel, 0, numSamples);
    }

    isProcessingBlock_.store(false);
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
    state.appId = activeAppId_;
    for(std::size_t i = 0; i < activeBindings_.knobControlIds.size(); ++i)
    {
        if(!activeBindings_.knobControlIds[i].empty())
        {
            state.controlValues[activeBindings_.knobControlIds[i]]
                = topControlValues_[i].load();
        }
        if(!activeBindings_.cvInputPortIds[i].empty())
        {
            state.cvValues[activeBindings_.cvInputPortIds[i]] = cvValues_[i].load();
        }
        state.controlValues[MakeCvHostStateId(i, "mode")]
            = static_cast<float>(
                cvGeneratorStates_[i].mode == daisyhost::CvInputSourceMode::kGenerator);
        state.controlValues[MakeCvHostStateId(i, "waveform")]
            = static_cast<float>(static_cast<int>(cvGeneratorStates_[i].waveform));
        state.controlValues[MakeCvHostStateId(i, "frequency_hz")]
            = cvGeneratorStates_[i].frequencyHz;
        state.controlValues[MakeCvHostStateId(i, "amplitude_volts")]
            = cvGeneratorStates_[i].amplitudeVolts;
        state.controlValues[MakeCvHostStateId(i, "bias_volts")]
            = cvGeneratorStates_[i].biasVolts;
        state.controlValues[MakeCvHostStateId(i, "manual_volts")]
            = cvGeneratorStates_[i].manualVolts;
    }

    for(std::size_t i = 0; i < activeBindings_.gateInputPortIds.size(); ++i)
    {
        if(!activeBindings_.gateInputPortIds[i].empty())
        {
            state.gateValues[activeBindings_.gateInputPortIds[i]]
                = gateValues_[i].load();
        }
    }

    state.controlValues[computerKeyboardEnabledStateId_]
        = computerKeyboardEnabled_.load() ? 1.0f : 0.0f;
    state.controlValues[computerKeyboardOctaveStateId_]
        = static_cast<float>(computerKeyboardOctave_.load());
    state.controlValues[testInputModeStateId_]
        = static_cast<float>(testInputMode_.load());
    state.controlValues[testInputLevelStateId_] = testInputLevel_.load();
    state.controlValues[testInputFrequencyStateId_] = testInputFrequencyHz_.load();
    state.parameterValues = core_->CaptureStatefulParameterValues();
    state.randomSeed      = appRandomSeed_;

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
    auto state = daisyhost::HostSessionState::Deserialize(text);
    if(pendingHubStartupRequest_.has_value()
       && !pendingHubStartupRequest_->appId.empty())
    {
        state.appId = pendingHubStartupRequest_->appId;
    }
    LoadSession(state);
    ApplyHubStartupRequestIfNeeded();
}

const daisyhost::BoardProfile& DaisyHostPatchAudioProcessor::GetBoardProfile() const
{
    return boardProfile_;
}

const daisyhost::HostedAppPatchBindings&
DaisyHostPatchAudioProcessor::GetActivePatchBindings() const
{
    return activeBindings_;
}

juce::String DaisyHostPatchAudioProcessor::GetActiveAppId() const
{
    return activeAppId_;
}

juce::String DaisyHostPatchAudioProcessor::GetActiveAppDisplayName() const
{
    return core_ != nullptr ? juce::String(core_->GetAppDisplayName())
                            : juce::String();
}

float DaisyHostPatchAudioProcessor::GetTopControlValue(std::size_t index) const
{
    return index < topControlValues_.size() ? topControlValues_[index].load() : 0.0f;
}

juce::String DaisyHostPatchAudioProcessor::GetTopControlLabel(std::size_t index) const
{
    return "CTRL " + juce::String(static_cast<int>(index + 1));
}

juce::String DaisyHostPatchAudioProcessor::GetTopControlDetailLabel(std::size_t index) const
{
    return index < activeBindings_.knobDetailLabels.size()
               ? juce::String(activeBindings_.knobDetailLabels[index])
               : juce::String();
}

std::string DaisyHostPatchAudioProcessor::GetTopControlId(std::size_t index) const
{
    return index < activeBindings_.knobControlIds.size()
               ? activeBindings_.knobControlIds[index]
               : std::string();
}

bool DaisyHostPatchAudioProcessor::GetEncoderPressed() const
{
    return encoderPressed_.load();
}

void DaisyHostPatchAudioProcessor::SetTopControlValue(std::size_t index,
                                                      float       normalizedValue)
{
    if(index < topControlValues_.size())
    {
        topControlValues_[index].store(Clamp01(normalizedValue));
        RefreshCoreStateFromIdleHostChange();
    }
}

void DaisyHostPatchAudioProcessor::SetEncoderPressed(bool pressed)
{
    const bool previous = encoderPressed_.exchange(pressed);
    if(pressed && !previous)
    {
        pendingEncoderPressCount_.fetch_add(1);
        RefreshCoreStateFromIdleHostChange();
    }
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
        cvVoltages_[index].store(daisyhost::NormalizedToCvVolts(normalizedValue));
        cvGeneratorStates_[index].manualVolts
            = daisyhost::NormalizedToCvVolts(normalizedValue);
        RefreshCoreStateFromIdleHostChange();
    }
}

float DaisyHostPatchAudioProcessor::GetCvVoltage(std::size_t index) const
{
    return index < cvVoltages_.size() ? cvVoltages_[index].load() : 0.0f;
}

void DaisyHostPatchAudioProcessor::SetCvManualVoltage(std::size_t index,
                                                      float       volts)
{
    if(index < cvGeneratorStates_.size())
    {
        const float clamped = daisyhost::ClampCvVoltage(volts);
        cvGeneratorStates_[index].manualVolts = clamped;
        if(cvGeneratorStates_[index].mode == daisyhost::CvInputSourceMode::kManual)
        {
            cvVoltages_[index].store(clamped);
            cvValues_[index].store(daisyhost::CvVoltsToNormalized(clamped));
        }
        RefreshCoreStateFromIdleHostChange();
    }
}

int DaisyHostPatchAudioProcessor::GetCvSourceMode(std::size_t index) const
{
    return index < cvGeneratorStates_.size()
               ? static_cast<int>(
                     cvGeneratorStates_[index].mode
                     == daisyhost::CvInputSourceMode::kGenerator)
               : 0;
}

void DaisyHostPatchAudioProcessor::SetCvSourceMode(std::size_t index, int mode)
{
    if(index < cvGeneratorStates_.size())
    {
        cvGeneratorStates_[index].mode = ClampCvMode(mode);
        if(cvGeneratorStates_[index].mode == daisyhost::CvInputSourceMode::kManual)
        {
            SetCvManualVoltage(index, cvGeneratorStates_[index].manualVolts);
        }
        else
        {
            RefreshCoreStateFromIdleHostChange();
        }
    }
}

int DaisyHostPatchAudioProcessor::GetCvWaveform(std::size_t index) const
{
    return index < cvGeneratorStates_.size()
               ? static_cast<int>(cvGeneratorStates_[index].waveform)
               : 0;
}

void DaisyHostPatchAudioProcessor::SetCvWaveform(std::size_t index, int waveform)
{
    if(index < cvGeneratorStates_.size())
    {
        cvGeneratorStates_[index].waveform
            = static_cast<daisyhost::BasicWaveform>(
                daisyhost::ClampBasicWaveform(waveform));
        RefreshCoreStateFromIdleHostChange();
    }
}

float DaisyHostPatchAudioProcessor::GetCvFrequencyHz(std::size_t index) const
{
    return index < cvGeneratorStates_.size() ? cvGeneratorStates_[index].frequencyHz
                                             : 1.0f;
}

void DaisyHostPatchAudioProcessor::SetCvFrequencyHz(std::size_t index,
                                                    float       frequencyHz)
{
    if(index < cvGeneratorStates_.size())
    {
        cvGeneratorStates_[index].frequencyHz = std::clamp(frequencyHz, 0.01f, 20.0f);
        RefreshCoreStateFromIdleHostChange();
    }
}

float DaisyHostPatchAudioProcessor::GetCvAmplitudeVolts(std::size_t index) const
{
    return index < cvGeneratorStates_.size()
               ? cvGeneratorStates_[index].amplitudeVolts
               : 0.0f;
}

void DaisyHostPatchAudioProcessor::SetCvAmplitudeVolts(std::size_t index,
                                                       float       volts)
{
    if(index < cvGeneratorStates_.size())
    {
        cvGeneratorStates_[index].amplitudeVolts
            = daisyhost::ClampCvAmplitudeVolts(volts);
        RefreshCoreStateFromIdleHostChange();
    }
}

float DaisyHostPatchAudioProcessor::GetCvBiasVolts(std::size_t index) const
{
    return index < cvGeneratorStates_.size() ? cvGeneratorStates_[index].biasVolts
                                             : 0.0f;
}

void DaisyHostPatchAudioProcessor::SetCvBiasVolts(std::size_t index, float volts)
{
    if(index < cvGeneratorStates_.size())
    {
        cvGeneratorStates_[index].biasVolts = daisyhost::ClampCvVoltage(volts);
        RefreshCoreStateFromIdleHostChange();
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
        RefreshCoreStateFromIdleHostChange();
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

daisyhost::MenuModel DaisyHostPatchAudioProcessor::GetMenuModelSnapshot() const
{
    std::lock_guard<std::mutex> lock(menuSnapshotMutex_);
    return latestMenu_;
}

std::vector<daisyhost::ParameterDescriptor>
DaisyHostPatchAudioProcessor::GetParameterSnapshot() const
{
    std::lock_guard<std::mutex> lock(menuSnapshotMutex_);
    return latestParameters_;
}

daisyhost::EffectiveHostStateSnapshot
DaisyHostPatchAudioProcessor::GetEffectiveHostStateSnapshot() const
{
    std::vector<daisyhost::ParameterDescriptor> parameters;
    daisyhost::HostAutomationSlotBindings       automationSlots;
    {
        std::lock_guard<std::mutex> lock(menuSnapshotMutex_);
        parameters      = latestParameters_;
        automationSlots = automationSlotBindings_;
    }

    std::array<daisyhost::HostCvInputState, 4> cvInputs{};
    for(std::size_t index = 0; index < cvInputs.size(); ++index)
    {
        cvInputs[index].normalizedValue = cvValues_[index].load();
        cvInputs[index].volts           = cvVoltages_[index].load();
        cvInputs[index].sourceMode
            = static_cast<int>(cvGeneratorStates_[index].mode
                               == daisyhost::CvInputSourceMode::kGenerator);
        cvInputs[index].waveform = static_cast<int>(cvGeneratorStates_[index].waveform);
        cvInputs[index].frequencyHz    = cvGeneratorStates_[index].frequencyHz;
        cvInputs[index].amplitudeVolts = cvGeneratorStates_[index].amplitudeVolts;
        cvInputs[index].biasVolts      = cvGeneratorStates_[index].biasVolts;
        cvInputs[index].manualVolts    = cvGeneratorStates_[index].manualVolts;
    }

    std::array<daisyhost::HostGateInputState, 2> gateInputs{};
    for(std::size_t index = 0; index < gateInputs.size(); ++index)
    {
        gateInputs[index].value = gateValues_[index].load();
    }

    daisyhost::HostAudioInputState audioInput;
    audioInput.mode        = testInputMode_.load();
    audioInput.level       = testInputLevel_.load();
    audioInput.frequencyHz = testInputFrequencyHz_.load();

    return daisyhost::BuildEffectiveHostStateSnapshot(
        activeAppId_,
        core_ != nullptr ? core_->GetAppDisplayName() : std::string(),
        activeBindings_,
        parameters,
        automationSlots,
        cvInputs,
        gateInputs,
        audioInput);
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
    {
        std::lock_guard<std::mutex> lock(pendingMenuValueMutex_);
        pendingMenuValues_["node0/menu/midi/computer_keys"] = enabled ? 1.0f : 0.0f;
    }
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
    const int clamped = daisyhost::ComputerKeyboardMidi::ClampOctave(octave);
    computerKeyboardOctave_.store(clamped);
    {
        std::lock_guard<std::mutex> lock(pendingMenuValueMutex_);
        pendingMenuValues_["node0/menu/midi/octave"]
            = static_cast<float>(std::clamp(clamped - 1, 0, 4)) / 4.0f;
    }
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
    const int clamped = daisyhost::ClampTestInputSignalMode(mode);
    testInputMode_.store(clamped);
    {
        std::lock_guard<std::mutex> lock(pendingMenuValueMutex_);
        pendingMenuValues_["node0/menu/input/source"]
            = daisyhost::TestInputSignalModeToNormalized(clamped);
    }
    RefreshCoreStateFromIdleHostChange();
}

float DaisyHostPatchAudioProcessor::GetTestInputLevel() const
{
    return testInputLevel_.load();
}

void DaisyHostPatchAudioProcessor::SetTestInputLevel(float level)
{
    const float clamped = std::clamp(level, 0.0f, 10.0f);
    testInputLevel_.store(clamped);
    {
        std::lock_guard<std::mutex> lock(pendingMenuValueMutex_);
        pendingMenuValues_["node0/menu/input/level"] = Clamp01(clamped / 10.0f);
    }
    RefreshCoreStateFromIdleHostChange();
}

float DaisyHostPatchAudioProcessor::GetTestInputFrequencyHz() const
{
    return testInputFrequencyHz_.load();
}

void DaisyHostPatchAudioProcessor::SetTestInputFrequencyHz(float frequencyHz)
{
    testInputFrequencyHz_.store(std::clamp(frequencyHz, 20.0f, 5000.0f));
    RefreshCoreStateFromIdleHostChange();
}

void DaisyHostPatchAudioProcessor::TriggerImpulse()
{
    impulseRequested_.store(true);
    {
        std::lock_guard<std::mutex> lock(pendingMenuValueMutex_);
        pendingMenuValues_["node0/menu/input/fire_impulse"] = 1.0f;
    }
    RefreshCoreStateFromIdleHostChange();
}

juce::String DaisyHostPatchAudioProcessor::GetTestInputModeName(int mode) const
{
    return daisyhost::GetTestInputSignalModeName(mode);
}

juce::String DaisyHostPatchAudioProcessor::GetVersionText() const
{
    return daisyhost::GetVersionInfo().version;
}

juce::String DaisyHostPatchAudioProcessor::GetBuildIdentityText() const
{
    return daisyhost::GetVersionInfo().buildIdentity;
}

juce::StringArray DaisyHostPatchAudioProcessor::GetReleaseHighlightLines() const
{
    juce::StringArray lines;
    for(const auto& line : daisyhost::GetVersionInfo().releaseHighlights)
    {
        lines.add(line);
    }
    return lines;
}

bool DaisyHostPatchAudioProcessor::IsStandaloneWrapper() const
{
    return wrapperType == juce::AudioProcessor::wrapperType_Standalone;
}

void DaisyHostPatchAudioProcessor::RotateEncoder(int delta)
{
    if(delta != 0)
    {
        pendingEncoderDelta_.fetch_add(delta);
        RefreshCoreStateFromIdleHostChange();
    }
}

void DaisyHostPatchAudioProcessor::PulseEncoderPress()
{
    pendingEncoderPressCount_.fetch_add(1);
    encoderPressed_.store(true);
    RefreshCoreStateFromIdleHostChange();
}

void DaisyHostPatchAudioProcessor::SetMenuItemValue(const std::string& itemId,
                                                    float normalizedValue)
{
    {
        std::lock_guard<std::mutex> lock(pendingMenuValueMutex_);
        pendingMenuValues_[itemId] = Clamp01(normalizedValue);
    }
    RefreshCoreStateFromIdleHostChange();
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
    for(std::size_t i = 0; i < activeBindings_.knobControlIds.size(); ++i)
    {
        if(!activeBindings_.knobControlIds[i].empty())
        {
            core_->SetControl(activeBindings_.knobControlIds[i],
                              Clamp01(topControlValues_[i].load()));
        }
    }
}

void DaisyHostPatchAudioProcessor::ApplyPendingAutomationParametersToCore()
{
    daisyhost::HostAutomationSlotBindings automationSlots;
    {
        std::lock_guard<std::mutex> lock(menuSnapshotMutex_);
        automationSlots = automationSlotBindings_;
    }

    for(std::size_t slotIndex = 0; slotIndex < automationParameters_.size();
        ++slotIndex)
    {
        if(!pendingAutomationParameterDirty_[slotIndex].exchange(false))
        {
            continue;
        }

        const auto& binding = automationSlots[slotIndex];
        if(!binding.available || binding.parameterId.empty())
        {
            continue;
        }

        const float normalizedValue
            = Clamp01(pendingAutomationParameterValues_[slotIndex].load());
        core_->SetParameterValue(binding.parameterId, normalizedValue);
    }
}

void DaisyHostPatchAudioProcessor::ApplyPendingMenuInteractionsToCore()
{
    const int encoderDelta = pendingEncoderDelta_.exchange(0);
    if(encoderDelta != 0)
    {
        core_->MenuRotate(encoderDelta);
    }

    int pressCount = pendingEncoderPressCount_.exchange(0);
    while(pressCount-- > 0)
    {
        core_->MenuPress();
    }
    if(encoderPressed_.load())
    {
        encoderPressed_.store(false);
    }

    std::unordered_map<std::string, float> pendingValues;
    {
        std::lock_guard<std::mutex> lock(pendingMenuValueMutex_);
        pendingValues.swap(pendingMenuValues_);
    }

    for(const auto& entry : pendingValues)
    {
        core_->SetMenuItemValue(entry.first, entry.second);
    }
}

void DaisyHostPatchAudioProcessor::ApplyVirtualPortStateToCore(
    const std::vector<daisyhost::MidiMessageEvent>& midiEvents)
{
    for(std::size_t i = 0; i < activeBindings_.cvInputPortIds.size(); ++i)
    {
        if(activeBindings_.cvInputPortIds[i].empty())
        {
            continue;
        }
        daisyhost::PortValue value;
        value.type   = daisyhost::VirtualPortType::kCv;
        value.scalar = cvValues_[i].load();
        core_->SetPortInput(activeBindings_.cvInputPortIds[i], value);
    }

    for(std::size_t i = 0; i < activeBindings_.gateInputPortIds.size(); ++i)
    {
        if(activeBindings_.gateInputPortIds[i].empty())
        {
            continue;
        }
        const bool gateState = gateValues_[i].load();

        daisyhost::PortValue value;
        value.type = daisyhost::VirtualPortType::kGate;
        value.gate = gateState;
        core_->SetPortInput(activeBindings_.gateInputPortIds[i], value);

        if(i == 0 && gateState && !previousGateValues_[i])
        {
            impulseRequested_.store(true);
        }
        previousGateValues_[i] = gateState;
    }

    daisyhost::PortValue midiValue;
    midiValue.type       = daisyhost::VirtualPortType::kMidi;
    midiValue.midiEvents = midiEvents;
    if(!activeBindings_.midiInputPortId.empty())
    {
        core_->SetPortInput(activeBindings_.midiInputPortId, midiValue);
    }

    if(!midiEvents.empty())
    {
        midiActivity_.store(1.0f);
    }
}

void DaisyHostPatchAudioProcessor::UpdateCvGeneratorOutputs(
    double blockDurationSeconds)
{
    for(std::size_t i = 0; i < cvGeneratorStates_.size(); ++i)
    {
        const float volts
            = daisyhost::StepCvInputGenerator(&cvGeneratorStates_[i],
                                              blockDurationSeconds);
        cvVoltages_[i].store(volts);
        cvValues_[i].store(daisyhost::CvVoltsToNormalized(volts));
    }
}

void DaisyHostPatchAudioProcessor::UpdateDisplaySnapshot()
{
    std::lock_guard<std::mutex> lock(displayMutex_);
    latestDisplay_ = core_->GetDisplayModel();
}

void DaisyHostPatchAudioProcessor::ApplyCanonicalSessionStateToCore()
{
    core_->ResetToDefaultState(appRandomSeed_);
    if(!restoredParameterValues_.empty())
    {
        core_->RestoreStatefulParameterValues(restoredParameterValues_);
    }
    UpdateCoreSnapshots();
    SyncHostStateFromCore();
    SyncAutomationParametersFromCore();
}

void DaisyHostPatchAudioProcessor::ApplyHubStartupRequestIfNeeded()
{
    if(hubStartupRequestApplied_ || !pendingHubStartupRequest_.has_value())
    {
        return;
    }

    if(!pendingHubStartupRequest_->appId.empty())
    {
        if(pendingHubStartupRequest_->appId != activeAppId_)
        {
            RecreateHostedApp(pendingHubStartupRequest_->appId);
        }
    }

    pendingHubStartupRequest_.reset();
    hubStartupRequestApplied_ = true;
}

void DaisyHostPatchAudioProcessor::SyncHostStateFromCore()
{
    for(std::size_t i = 0; i < activeBindings_.knobControlIds.size(); ++i)
    {
        if(activeBindings_.knobControlIds[i].empty())
        {
            continue;
        }
        const auto value = core_->GetControlValue(activeBindings_.knobControlIds[i]);
        if(value.hasValue)
        {
            topControlValues_[i].store(Clamp01(value.value));
        }
    }
}

void DaisyHostPatchAudioProcessor::UpdateCoreSnapshots()
{
    activeBindings_ = core_->GetPatchBindings();
    UpdateDisplaySnapshot();

    {
        std::lock_guard<std::mutex> lock(menuSnapshotMutex_);
        latestMenu_            = core_->GetMenuModel();
        latestParameters_      = core_->GetParameters();
        automationSlotBindings_
            = daisyhost::BuildHostAutomationSlotBindings(latestParameters_);
    }
}

void DaisyHostPatchAudioProcessor::LoadSession(
    const daisyhost::HostSessionState& state)
{
    const std::string requestedAppId = state.appId.empty()
                                           ? daisyhost::GetDefaultHostedAppId()
                                           : state.appId;
    RecreateHostedApp(requestedAppId);
    appRandomSeed_          = state.randomSeed;
    restoredParameterValues_ = state.parameterValues;
    ApplyCanonicalSessionStateToCore();

    const bool hasCanonicalParameterState = !state.parameterValues.empty();

    for(std::size_t i = 0; i < activeBindings_.knobControlIds.size(); ++i)
    {
        if(!hasCanonicalParameterState)
        {
            const auto controlIt
                = state.controlValues.find(activeBindings_.knobControlIds[i]);
            if(!activeBindings_.knobControlIds[i].empty()
               && controlIt != state.controlValues.end())
            {
                topControlValues_[i].store(Clamp01(controlIt->second));
            }
        }

        const auto cvIt = state.cvValues.find(activeBindings_.cvInputPortIds[i]);
        if(!activeBindings_.cvInputPortIds[i].empty()
           && cvIt != state.cvValues.end())
        {
            cvValues_[i].store(Clamp01(cvIt->second));
            cvVoltages_[i].store(daisyhost::NormalizedToCvVolts(cvIt->second));
            cvGeneratorStates_[i].manualVolts
                = daisyhost::NormalizedToCvVolts(cvIt->second);
        }

        if(const auto it = state.controlValues.find(MakeCvHostStateId(i, "mode"));
           it != state.controlValues.end())
        {
            cvGeneratorStates_[i].mode
                = ClampCvMode(static_cast<int>(std::round(it->second)));
        }
        if(const auto it
           = state.controlValues.find(MakeCvHostStateId(i, "waveform"));
           it != state.controlValues.end())
        {
            cvGeneratorStates_[i].waveform
                = static_cast<daisyhost::BasicWaveform>(
                    daisyhost::ClampBasicWaveform(
                        static_cast<int>(std::round(it->second))));
        }
        if(const auto it
           = state.controlValues.find(MakeCvHostStateId(i, "frequency_hz"));
           it != state.controlValues.end())
        {
            cvGeneratorStates_[i].frequencyHz
                = std::clamp(it->second, 0.01f, 20.0f);
        }
        if(const auto it
           = state.controlValues.find(MakeCvHostStateId(i, "amplitude_volts"));
           it != state.controlValues.end())
        {
            cvGeneratorStates_[i].amplitudeVolts
                = daisyhost::ClampCvAmplitudeVolts(it->second);
        }
        if(const auto it
           = state.controlValues.find(MakeCvHostStateId(i, "bias_volts"));
           it != state.controlValues.end())
        {
            cvGeneratorStates_[i].biasVolts
                = daisyhost::ClampCvVoltage(it->second);
        }
        if(const auto it
           = state.controlValues.find(MakeCvHostStateId(i, "manual_volts"));
           it != state.controlValues.end())
        {
            cvGeneratorStates_[i].manualVolts
                = daisyhost::ClampCvVoltage(it->second);
        }
    }

    for(std::size_t i = 0; i < activeBindings_.gateInputPortIds.size(); ++i)
    {
        const auto gateIt
            = state.gateValues.find(activeBindings_.gateInputPortIds[i]);
        if(!activeBindings_.gateInputPortIds[i].empty()
           && gateIt != state.gateValues.end())
        {
            gateValues_[i].store(gateIt->second);
        }
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
        SetTestInputLevel(testLevelIt->second <= 1.0f ? testLevelIt->second * 10.0f
                                                      : testLevelIt->second);
    }

    const auto testFrequencyIt
        = state.controlValues.find(testInputFrequencyStateId_);
    if(testFrequencyIt != state.controlValues.end())
    {
        SetTestInputFrequencyHz(testFrequencyIt->second);
    }

    {
        std::lock_guard<std::mutex> lock(midiLearnMutex_);
        midiLearnMap_ = state.midiLearn;
        learningTargetId_.clear();
    }

    {
        std::lock_guard<std::mutex> lock(pendingMenuValueMutex_);
        pendingMenuValues_.clear();
    }

    UpdateCvGeneratorOutputs(0.0);

    if(!hasCanonicalParameterState)
    {
        ApplyControlStateToCore();
        UpdateCoreSnapshots();
    }

    SyncAutomationParametersFromCore();
}

bool DaisyHostPatchAudioProcessor::RecreateHostedApp(
    const std::string& requestedAppId)
{
    std::string resolvedAppId;
    auto newCore = daisyhost::CreateHostedAppCore(
        requestedAppId, boardProfile_.nodeId, &resolvedAppId);
    if(newCore == nullptr)
    {
        return false;
    }

    core_         = std::move(newCore);
    activeAppId_  = resolvedAppId;
    activeBindings_ = core_->GetPatchBindings();

    if(currentSampleRate_ > 1.0 && currentBlockSize_ > 0)
    {
        core_->Prepare(currentSampleRate_, currentBlockSize_);
    }

    ClearPendingAutomationParameterState();

    return true;
}

bool DaisyHostPatchAudioProcessor::SetActiveAppId(const std::string& requestedAppId)
{
    if(!RecreateHostedApp(requestedAppId))
    {
        return false;
    }

    restoredParameterValues_.clear();
    appRandomSeed_ = 0;
    ApplyCanonicalSessionStateToCore();
    return true;
}

void DaisyHostPatchAudioProcessor::GenerateTestInput(float* destination,
                                                     int    numSamples)
{
    bool impulseState = impulseRequested_.exchange(false);
    daisyhost::GenerateSyntheticTestInput(testInputMode_.load(),
                                          Clamp01(testInputLevel_.load() / 10.0f),
                                          testInputFrequencyHz_.load(),
                                          getSampleRate(),
                                          &testPhase_,
                                          &noiseState_,
                                          &impulseState,
                                          destination,
                                          numSamples);
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

void DaisyHostPatchAudioProcessor::ClearPendingAutomationParameterState()
{
    for(std::size_t slotIndex = 0; slotIndex < pendingAutomationParameterDirty_.size();
        ++slotIndex)
    {
        pendingAutomationParameterValues_[slotIndex].store(0.0f);
        pendingAutomationParameterDirty_[slotIndex].store(false);
    }
}

void DaisyHostPatchAudioProcessor::HandleAutomationParameterValueChanged(
    std::size_t slotIndex,
    float       normalizedValue)
{
    if(slotIndex >= pendingAutomationParameterValues_.size()
       || isSyncingAutomationParameters_.load())
    {
        return;
    }

    pendingAutomationParameterValues_[slotIndex].store(Clamp01(normalizedValue));
    pendingAutomationParameterDirty_[slotIndex].store(true);

    if(!isProcessingBlock_.load())
    {
        RefreshCoreStateFromIdleHostChange();
    }
}

void DaisyHostPatchAudioProcessor::SyncAutomationParametersFromCore()
{
    std::vector<daisyhost::ParameterDescriptor> parameters;
    daisyhost::HostAutomationSlotBindings       automationSlots;
    {
        std::lock_guard<std::mutex> lock(menuSnapshotMutex_);
        parameters      = latestParameters_;
        automationSlots = automationSlotBindings_;
    }

    isSyncingAutomationParameters_.store(true);
    for(std::size_t slotIndex = 0; slotIndex < automationParameters_.size();
        ++slotIndex)
    {
        auto* parameter = automationParameters_[slotIndex];
        if(parameter == nullptr)
        {
            continue;
        }

        float targetValue = 0.0f;
        if(const auto& binding = automationSlots[slotIndex];
           binding.available && !binding.parameterId.empty())
        {
            const auto parameterIt = std::find_if(
                parameters.begin(),
                parameters.end(),
                [&binding](const daisyhost::ParameterDescriptor& descriptor) {
                    return descriptor.id == binding.parameterId;
                });
            if(parameterIt != parameters.end())
            {
                targetValue = Clamp01(parameterIt->normalizedValue);
            }
        }

        if(HasMeaningfulAutomationDelta(parameter->get(), targetValue))
        {
            *parameter = targetValue;
        }
    }
    isSyncingAutomationParameters_.store(false);
    ClearPendingAutomationParameterState();
}

void DaisyHostPatchAudioProcessor::RefreshCoreStateFromIdleHostChange()
{
    if(isProcessingBlock_.load() || core_ == nullptr || currentSampleRate_ <= 1.0
       || currentBlockSize_ == 0)
    {
        return;
    }

    ApplyPendingAutomationParametersToCore();
    ApplyControlStateToCore();
    ApplyPendingMenuInteractionsToCore();
    UpdateCvGeneratorOutputs(0.0);
    ApplyVirtualPortStateToCore(std::vector<daisyhost::MidiMessageEvent>{});
    UpdateCoreSnapshots();
    SyncHostStateFromCore();
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DaisyHostPatchAudioProcessor();
}
