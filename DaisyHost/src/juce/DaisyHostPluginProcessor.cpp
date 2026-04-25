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

const daisyhost::BoardSurfaceBinding* FindFieldKnobBinding(
    const daisyhost::DaisyFieldControlMapping& mapping,
    std::size_t                                index)
{
    return index < mapping.knobs.size() ? &mapping.knobs[index] : nullptr;
}

const daisyhost::BoardSurfaceBinding* FindFieldSwitchBinding(
    const daisyhost::DaisyFieldControlMapping& mapping,
    std::size_t                                index)
{
    return index < mapping.switches.size() ? &mapping.switches[index] : nullptr;
}

bool ApplySurfaceBinding(daisyhost::HostedAppCore&              core,
                         const daisyhost::BoardSurfaceBinding& binding,
                         float                                normalizedValue)
{
    switch(binding.targetKind)
    {
        case daisyhost::BoardSurfaceTargetKind::kControl:
            core.SetControl(binding.targetId, normalizedValue);
            return true;
        case daisyhost::BoardSurfaceTargetKind::kParameter:
            return core.SetParameterValue(binding.targetId, normalizedValue);
        case daisyhost::BoardSurfaceTargetKind::kMenuItem:
            core.SetMenuItemValue(binding.targetId, normalizedValue);
            return true;
        case daisyhost::BoardSurfaceTargetKind::kUnavailable:
        case daisyhost::BoardSurfaceTargetKind::kCvInput:
        case daisyhost::BoardSurfaceTargetKind::kGateInput:
        case daisyhost::BoardSurfaceTargetKind::kMidiNote:
        case daisyhost::BoardSurfaceTargetKind::kLed: return false;
    }
    return false;
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
  boardProfile_(daisyhost::CreateBoardProfile("daisy_patch", "node0"))
{
    auto initializeRackNode = [this](RackNodeState& node, const std::string& nodeId) {
        node.nodeId = nodeId;
        for(std::size_t i = 0; i < node.topControlValues.size(); ++i)
        {
            node.topControlValues[i] = 0.0f;
            node.cvValues[i]         = 0.5f;
            node.cvVoltages[i]       = 2.5f;
            node.audioInputPeaks[i]  = 0.0f;
            node.audioOutputPeaks[i] = 0.0f;
            node.cvGeneratorStates[i].manualVolts    = 2.5f;
            node.cvGeneratorStates[i].biasVolts      = 2.5f;
            node.cvGeneratorStates[i].amplitudeVolts = 2.5f;
            node.cvGeneratorStates[i].frequencyHz    = 1.0f;
        }
        for(auto& value : node.fieldKnobValues)
        {
            value = 0.0f;
        }
        for(auto& pressed : node.fieldKeyPressed)
        {
            pressed = false;
        }
        for(auto& pressed : node.fieldSwitchPressed)
        {
            pressed = false;
        }
        for(std::size_t i = 0; i < node.gateValues.size(); ++i)
        {
            node.gateValues[i]         = false;
            node.previousGateValues[i] = false;
        }
    };
    initializeRackNode(rackNodes_[0], "node0");
    initializeRackNode(rackNodes_[1], "node1");

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

    RecreateRackNode(0, daisyhost::GetDefaultHostedAppId());
    RecreateRackNode(1, daisyhost::GetDefaultHostedAppId());
    selectedRackNodeIndex_ = 0;

    pendingHubStartupRequest_ = LoadHubStartupRequestFromCommandLine();
    if(!pendingHubStartupRequest_.has_value())
    {
        pendingHubStartupRequest_
            = daisyhost::LoadAndConsumeHubStartupRequest(
                daisyhost::GetDefaultHubLaunchRequestFile());
    }
    if(pendingHubStartupRequest_.has_value())
    {
        if(!pendingHubStartupRequest_->boardId.empty())
        {
            TryApplyBoardId(pendingHubStartupRequest_->boardId);
        }
        if(!pendingHubStartupRequest_->appId.empty())
        {
            RecreateRackNode(selectedRackNodeIndex_, pendingHubStartupRequest_->appId);
        }
    }

    encoderPressed_.store(false);
    midiActivity_.store(0.0f);
    impulseRequested_.store(false);
    pendingEncoderDelta_.store(0);
    pendingEncoderPressCount_.store(0);
    SyncSelectedNodeStateFromRack();
}

DaisyHostPatchAudioProcessor::~DaisyHostPatchAudioProcessor() {}

DaisyHostPatchAudioProcessor::RackNodeState&
DaisyHostPatchAudioProcessor::GetSelectedRackNode()
{
    return rackNodes_[selectedRackNodeIndex_];
}

const DaisyHostPatchAudioProcessor::RackNodeState&
DaisyHostPatchAudioProcessor::GetSelectedRackNode() const
{
    return rackNodes_[selectedRackNodeIndex_];
}

DaisyHostPatchAudioProcessor::RackNodeState*
DaisyHostPatchAudioProcessor::GetRackNodeById(const std::string& nodeId)
{
    for(auto& node : rackNodes_)
    {
        if(node.nodeId == nodeId)
        {
            return &node;
        }
    }
    return nullptr;
}

const DaisyHostPatchAudioProcessor::RackNodeState*
DaisyHostPatchAudioProcessor::GetRackNodeById(const std::string& nodeId) const
{
    for(const auto& node : rackNodes_)
    {
        if(node.nodeId == nodeId)
        {
            return &node;
        }
    }
    return nullptr;
}

std::string DaisyHostPatchAudioProcessor::MakeCvHostStateId(const std::string& nodeId,
                                                            std::size_t index,
                                                            const char* suffix) const
{
    return nodeId + "/host/cv_" + std::to_string(index + 1) + "_" + suffix;
}

std::string DaisyHostPatchAudioProcessor::MakeTestInputModeStateId(
    const std::string& nodeId) const
{
    return nodeId + "/host/test_input_mode";
}

std::string DaisyHostPatchAudioProcessor::MakeTestInputLevelStateId(
    const std::string& nodeId) const
{
    return nodeId + "/host/test_input_level";
}

std::string DaisyHostPatchAudioProcessor::MakeTestInputFrequencyStateId(
    const std::string& nodeId) const
{
    return nodeId + "/host/test_input_frequency_hz";
}

std::string DaisyHostPatchAudioProcessor::MakeComputerKeyboardEnabledStateId(
    const std::string& nodeId) const
{
    return nodeId + "/host/computer_keyboard_enabled";
}

std::string DaisyHostPatchAudioProcessor::MakeComputerKeyboardOctaveStateId(
    const std::string& nodeId) const
{
    return nodeId + "/host/computer_keyboard_octave";
}

daisyhost::DaisyFieldControlMapping
DaisyHostPatchAudioProcessor::BuildActiveFieldControlMapping() const
{
    std::lock_guard<std::recursive_mutex> coreLock(coreStateMutex_);
    const auto& selectedNode = GetSelectedRackNode();
    if(core_ != nullptr)
    {
        return daisyhost::BuildDaisyFieldControlMapping(
            activeBindings_,
            core_->GetParameters(),
            core_->GetMenuModel(),
            computerKeyboardOctave_.load(),
            selectedNode.nodeId);
    }

    return daisyhost::BuildDaisyFieldControlMapping(activeBindings_,
                                                    latestParameters_,
                                                    latestMenu_,
                                                    computerKeyboardOctave_.load(),
                                                    selectedNode.nodeId);
}

bool DaisyHostPatchAudioProcessor::TryApplyBoardId(const std::string& requestedBoardId)
{
    const std::string boardId
        = requestedBoardId.empty() ? std::string("daisy_patch") : requestedBoardId;
    if(boardId_ == "daisy_field" && boardId != "daisy_field")
    {
        ReleaseFieldKeys();
        ReleaseFieldSwitches();
    }
    if(const auto profile
       = daisyhost::TryCreateBoardProfile(boardId, GetSelectedRackNode().nodeId))
    {
        boardId_      = boardId;
        boardProfile_ = *profile;
        return true;
    }

    if(const auto fallback
       = daisyhost::TryCreateBoardProfile("daisy_patch", GetSelectedRackNode().nodeId))
    {
        boardId_      = "daisy_patch";
        boardProfile_ = *fallback;
    }
    return false;
}

void DaisyHostPatchAudioProcessor::UpdateSelectedBoardProfile()
{
    TryApplyBoardId(boardId_);
}

void DaisyHostPatchAudioProcessor::UpdateRackNodeSnapshots(RackNodeState& node)
{
    if(node.core == nullptr)
    {
        return;
    }

    node.bindings           = node.core->GetPatchBindings();
    node.latestDisplay      = node.core->GetDisplayModel();
    node.latestMenu         = node.core->GetMenuModel();
    node.latestParameters   = node.core->GetParameters();
    node.latestMetaControllers = node.core->GetMetaControllers();
    node.automationSlotBindings
        = daisyhost::BuildHostAutomationSlotBindings(node.latestParameters);
}

void DaisyHostPatchAudioProcessor::FlushSelectedNodeStateToRack()
{
    auto& node = GetSelectedRackNode();
    node.appId                  = activeAppId_;
    node.bindings               = activeBindings_;
    node.randomSeed             = appRandomSeed_;
    node.restoredParameterValues = restoredParameterValues_;
    node.latestDisplay          = latestDisplay_;
    node.latestMenu             = latestMenu_;
    node.latestParameters       = latestParameters_;
    node.latestMetaControllers  = latestMetaControllers_;
    node.automationSlotBindings = automationSlotBindings_;
    for(std::size_t i = 0; i < node.topControlValues.size(); ++i)
    {
        node.topControlValues[i] = topControlValues_[i].load();
        node.cvValues[i]         = cvValues_[i].load();
        node.cvVoltages[i]       = cvVoltages_[i].load();
        node.audioInputPeaks[i]  = audioInputPeaks_[i].load();
        node.audioOutputPeaks[i] = audioOutputPeaks_[i].load();
        node.cvGeneratorStates[i] = cvGeneratorStates_[i];
    }
    for(std::size_t i = 0; i < node.fieldKnobValues.size(); ++i)
    {
        node.fieldKnobValues[i] = fieldKnobValues_[i].load();
    }
    for(std::size_t i = 0; i < node.fieldKeyPressed.size(); ++i)
    {
        node.fieldKeyPressed[i] = fieldKeyPressed_[i].load();
    }
    for(std::size_t i = 0; i < node.fieldSwitchPressed.size(); ++i)
    {
        node.fieldSwitchPressed[i] = fieldSwitchPressed_[i].load();
    }
    for(std::size_t i = 0; i < node.gateValues.size(); ++i)
    {
        node.gateValues[i]         = gateValues_[i].load();
        node.previousGateValues[i] = previousGateValues_[i];
    }
    node.computerKeyboardEnabled = computerKeyboardEnabled_.load();
    node.computerKeyboardOctave  = computerKeyboardOctave_.load();
    node.testInputMode           = testInputMode_.load();
    node.testInputLevel          = testInputLevel_.load();
    node.testInputFrequencyHz    = testInputFrequencyHz_.load();
    node.impulseRequested        = impulseRequested_.load();
    {
        std::lock_guard<std::mutex> lock(pendingMenuValueMutex_);
        node.pendingMenuValues = pendingMenuValues_;
    }
}

void DaisyHostPatchAudioProcessor::SyncSelectedNodeStateFromRack()
{
    auto& node = GetSelectedRackNode();
    core_             = node.core.get();
    activeAppId_      = node.appId;
    activeBindings_   = node.bindings;
    appRandomSeed_    = node.randomSeed;
    restoredParameterValues_ = node.restoredParameterValues;
    latestDisplay_    = node.latestDisplay;
    latestMenu_       = node.latestMenu;
    latestParameters_ = node.latestParameters;
    latestMetaControllers_ = node.latestMetaControllers;
    automationSlotBindings_ = node.automationSlotBindings;
    for(std::size_t i = 0; i < node.topControlValues.size(); ++i)
    {
        topControlValues_[i].store(node.topControlValues[i]);
        cvValues_[i].store(node.cvValues[i]);
        cvVoltages_[i].store(node.cvVoltages[i]);
        audioInputPeaks_[i].store(node.audioInputPeaks[i]);
        audioOutputPeaks_[i].store(node.audioOutputPeaks[i]);
        cvGeneratorStates_[i] = node.cvGeneratorStates[i];
    }
    for(std::size_t i = 0; i < node.fieldKnobValues.size(); ++i)
    {
        fieldKnobValues_[i].store(node.fieldKnobValues[i]);
    }
    for(std::size_t i = 0; i < node.fieldKeyPressed.size(); ++i)
    {
        fieldKeyPressed_[i].store(node.fieldKeyPressed[i]);
    }
    for(std::size_t i = 0; i < node.fieldSwitchPressed.size(); ++i)
    {
        fieldSwitchPressed_[i].store(node.fieldSwitchPressed[i]);
    }
    for(std::size_t i = 0; i < node.gateValues.size(); ++i)
    {
        gateValues_[i].store(node.gateValues[i]);
        previousGateValues_[i] = node.previousGateValues[i];
    }
    computerKeyboardEnabled_.store(node.computerKeyboardEnabled);
    computerKeyboardOctave_.store(node.computerKeyboardOctave);
    testInputMode_.store(node.testInputMode);
    testInputLevel_.store(node.testInputLevel);
    testInputFrequencyHz_.store(node.testInputFrequencyHz);
    impulseRequested_.store(node.impulseRequested);
    {
        std::lock_guard<std::mutex> lock(pendingMenuValueMutex_);
        pendingMenuValues_ = node.pendingMenuValues;
    }
    UpdateSelectedBoardProfile();
}

void DaisyHostPatchAudioProcessor::StepRackNodeCvGenerators(
    RackNodeState& node,
    double         blockDurationSeconds)
{
    for(std::size_t i = 0; i < node.cvGeneratorStates.size(); ++i)
    {
        const float volts = daisyhost::StepCvInputGenerator(&node.cvGeneratorStates[i],
                                                            blockDurationSeconds);
        node.cvVoltages[i] = volts;
        node.cvValues[i]   = daisyhost::CvVoltsToNormalized(volts);
    }
}

void DaisyHostPatchAudioProcessor::ApplyRackNodeVirtualPortStateToCore(
    RackNodeState&                               node,
    const std::vector<daisyhost::MidiMessageEvent>& midiEvents)
{
    if(node.core == nullptr)
    {
        return;
    }

    for(std::size_t i = 0; i < node.bindings.cvInputPortIds.size(); ++i)
    {
        if(node.bindings.cvInputPortIds[i].empty())
        {
            continue;
        }

        daisyhost::PortValue value;
        value.type   = daisyhost::VirtualPortType::kCv;
        value.scalar = node.cvValues[i];
        node.core->SetPortInput(node.bindings.cvInputPortIds[i], value);
    }

    for(std::size_t i = 0; i < node.bindings.gateInputPortIds.size(); ++i)
    {
        if(node.bindings.gateInputPortIds[i].empty())
        {
            continue;
        }

        daisyhost::PortValue value;
        value.type = daisyhost::VirtualPortType::kGate;
        value.gate = node.gateValues[i];
        node.core->SetPortInput(node.bindings.gateInputPortIds[i], value);
        if(i == 0 && node.gateValues[i] && !node.previousGateValues[i])
        {
            node.impulseRequested = true;
        }
        node.previousGateValues[i] = node.gateValues[i];
    }

    if(!node.bindings.midiInputPortId.empty())
    {
        daisyhost::PortValue midiValue;
        midiValue.type       = daisyhost::VirtualPortType::kMidi;
        midiValue.midiEvents = midiEvents;
        node.core->SetPortInput(node.bindings.midiInputPortId, midiValue);
    }
}

void DaisyHostPatchAudioProcessor::prepareToPlay(double sampleRate,
                                                 int    samplesPerBlock)
{
    std::lock_guard<std::recursive_mutex> coreLock(coreStateMutex_);
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
    FlushSelectedNodeStateToRack();
    for(auto& node : rackNodes_)
    {
        if(node.core == nullptr)
        {
            continue;
        }
        node.core->Prepare(sampleRate, static_cast<std::size_t>(samplesPerBlock));
        node.core->ResetToDefaultState(node.randomSeed);
        if(!node.restoredParameterValues.empty())
        {
            node.core->RestoreStatefulParameterValues(node.restoredParameterValues);
        }
        node.testPhase  = 0.0f;
        node.noiseState = node.randomSeed == 0 ? 1u : node.randomSeed;
        UpdateRackNodeSnapshots(node);
    }
    SyncSelectedNodeStateFromRack();
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
    FlushSelectedNodeStateToRack();
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
    std::lock_guard<std::recursive_mutex> coreLock(coreStateMutex_);

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
    UpdateCoreSnapshots();
    SyncHostStateFromCore();
    FlushSelectedNodeStateToRack();

    const double blockDurationSeconds = static_cast<double>(numSamples) / getSampleRate();
    const auto topologyConfig = daisyhost::BuildLiveRackTopologyConfig(rackTopologyPreset_);
    auto findNodeIndex = [this](const std::string& nodeId) -> std::size_t {
        for(std::size_t index = 0; index < rackNodes_.size(); ++index)
        {
            if(rackNodes_[index].nodeId == nodeId)
            {
                return index;
            }
        }
        return rackNodes_.size();
    };
    const std::size_t entryNodeIndex  = findNodeIndex(topologyConfig.entryNodeId);
    const std::size_t outputNodeIndex = findNodeIndex(topologyConfig.outputNodeId);

    for(std::size_t index = 0; index < rackNodes_.size(); ++index)
    {
        if(index == selectedRackNodeIndex_)
        {
            continue;
        }
        StepRackNodeCvGenerators(rackNodes_[index], blockDurationSeconds);
        ApplyRackNodeVirtualPortStateToCore(rackNodes_[index], {});
    }

    if(static_cast<int>(zeroBuffer_.size()) < numSamples)
    {
        zeroBuffer_.assign(static_cast<std::size_t>(numSamples), 0.0f);
    }
    if(static_cast<int>(generatedInput_.size()) < numSamples)
    {
        generatedInput_.assign(static_cast<std::size_t>(numSamples), 0.0f);
    }

    std::array<juce::AudioBuffer<float>, 2> rackOutputs;
    for(auto& outputBuffer : rackOutputs)
    {
        outputBuffer.setSize(4, numSamples, false, false, true);
        outputBuffer.clear();
    }

    std::array<std::vector<float>, 2> syntheticInputs;
    for(auto& input : syntheticInputs)
    {
        input.assign(static_cast<std::size_t>(numSamples), 0.0f);
    }

    auto processNode = [&](std::size_t nodeIndex,
                           const std::vector<daisyhost::MidiMessageEvent>& midiEvents) {
        auto& node = rackNodes_[nodeIndex];
        if(node.core == nullptr)
        {
            return;
        }

        ApplyRackNodeVirtualPortStateToCore(node, midiEvents);

        std::array<const float*, 4> inputPtrs = {
            {zeroBuffer_.data(), zeroBuffer_.data(), zeroBuffer_.data(), zeroBuffer_.data()}};

        if(nodeIndex == entryNodeIndex)
        {
            if(node.testInputMode == kHostInput)
            {
                for(int channel = 0;
                    channel < std::min(buffer.getNumChannels(), getTotalNumInputChannels())
                    && channel < 4;
                    ++channel)
                {
                    inputPtrs[static_cast<std::size_t>(channel)] = buffer.getReadPointer(channel);
                }

                if(wrapperType == juce::AudioProcessor::wrapperType_Standalone
                   && nodeIndex == selectedRackNodeIndex_)
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
                    midiNotePreview_.RenderAdd(
                        generatedInput_.data(),
                        static_cast<std::size_t>(numSamples),
                        Clamp01(node.testInputLevel / 10.0f));
                    inputPtrs[0] = generatedInput_.data();
                }
            }
            else
            {
                bool impulseState = node.impulseRequested;
                daisyhost::GenerateSyntheticTestInput(
                    node.testInputMode,
                    Clamp01(node.testInputLevel / 10.0f),
                    node.testInputFrequencyHz,
                    getSampleRate(),
                    &node.testPhase,
                    &node.noiseState,
                    &impulseState,
                    syntheticInputs[nodeIndex].data(),
                    numSamples);
                node.impulseRequested = impulseState;
                inputPtrs[0]          = syntheticInputs[nodeIndex].data();
            }
        }
        else
        {
            for(const auto& route : topologyConfig.routes)
            {
                const bool targetsNode = route.destPortId.rfind(node.nodeId + "/", 0) == 0;
                if(!targetsNode)
                {
                    continue;
                }

                const std::size_t sourceNodeIndex
                    = route.sourcePortId.rfind("node1/", 0) == 0 ? 1u : 0u;
                const std::size_t sourceChannel
                    = route.sourcePortId.find("audio_out_2") != std::string::npos ? 1u : 0u;
                const std::size_t destChannel
                    = route.destPortId.find("audio_in_2") != std::string::npos ? 1u : 0u;
                inputPtrs[destChannel]
                    = rackOutputs[sourceNodeIndex].getReadPointer(static_cast<int>(sourceChannel));
            }
        }

        for(std::size_t channel = 0; channel < node.audioInputPeaks.size(); ++channel)
        {
            node.audioInputPeaks[channel]
                = (channel < inputPtrs.size() && inputPtrs[channel] != nullptr)
                      ? PeakForSamples(inputPtrs[channel], numSamples)
                      : 0.0f;
        }

        std::array<float*, 4> outputPtrs = {
            {rackOutputs[nodeIndex].getWritePointer(0),
             rackOutputs[nodeIndex].getWritePointer(1),
             rackOutputs[nodeIndex].getWritePointer(2),
             rackOutputs[nodeIndex].getWritePointer(3)}};
        node.core->Process({inputPtrs.data(), inputPtrs.size()},
                           {outputPtrs.data(), outputPtrs.size()},
                           static_cast<std::size_t>(numSamples));
        node.core->TickUi((1000.0 * static_cast<double>(numSamples)) / getSampleRate());
        for(std::size_t channel = 0; channel < node.audioOutputPeaks.size(); ++channel)
        {
            node.audioOutputPeaks[channel]
                = PeakForChannel(rackOutputs[nodeIndex], static_cast<int>(channel));
        }
        UpdateRackNodeSnapshots(node);
    };

    std::vector<daisyhost::MidiMessageEvent> selectedMidiEvents = ToMidiEvents(midiMessages);
    switch(rackTopologyPreset_)
    {
        case daisyhost::LiveRackTopologyPreset::kNode0Only:
            processNode(0, selectedRackNodeIndex_ == 0 ? selectedMidiEvents
                                                        : std::vector<daisyhost::MidiMessageEvent>{});
            break;
        case daisyhost::LiveRackTopologyPreset::kNode1Only:
            processNode(1, selectedRackNodeIndex_ == 1 ? selectedMidiEvents
                                                        : std::vector<daisyhost::MidiMessageEvent>{});
            break;
        case daisyhost::LiveRackTopologyPreset::kNode0ToNode1:
            processNode(0, selectedRackNodeIndex_ == 0 ? selectedMidiEvents
                                                        : std::vector<daisyhost::MidiMessageEvent>{});
            processNode(1, selectedRackNodeIndex_ == 1 ? selectedMidiEvents
                                                        : std::vector<daisyhost::MidiMessageEvent>{});
            break;
        case daisyhost::LiveRackTopologyPreset::kNode1ToNode0:
            processNode(1, selectedRackNodeIndex_ == 1 ? selectedMidiEvents
                                                        : std::vector<daisyhost::MidiMessageEvent>{});
            processNode(0, selectedRackNodeIndex_ == 0 ? selectedMidiEvents
                                                        : std::vector<daisyhost::MidiMessageEvent>{});
            break;
    }

    SyncSelectedNodeStateFromRack();
    SyncHostStateFromCore();
    UpdateCoreSnapshots();

    buffer.clear();
    const auto& outputBindings = rackNodes_[outputNodeIndex].bindings;
    if(getTotalNumOutputChannels() >= 4)
    {
        for(int channel = 0; channel < 4; ++channel)
        {
            buffer.copyFrom(channel, 0, rackOutputs[outputNodeIndex], channel, 0, numSamples);
        }
    }
    else if(getTotalNumOutputChannels() == 2)
    {
        const int leftChannel  = std::clamp(outputBindings.mainOutputChannels[0], 0, 3);
        const int rightChannel = std::clamp(outputBindings.mainOutputChannels[1], 0, 3);
        buffer.copyFrom(0, 0, rackOutputs[outputNodeIndex], leftChannel, 0, numSamples);
        buffer.copyFrom(1, 0, rackOutputs[outputNodeIndex], rightChannel, 0, numSamples);
    }
    else if(getTotalNumOutputChannels() == 1)
    {
        const int monoChannel = std::clamp(outputBindings.mainOutputChannels[0], 0, 3);
        buffer.copyFrom(0, 0, rackOutputs[outputNodeIndex], monoChannel, 0, numSamples);
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
    FlushSelectedNodeStateToRack();

    daisyhost::HostSessionState state;
    state.boardId        = boardId_;
    state.selectedNodeId = rackNodes_[selectedRackNodeIndex_].nodeId;
    const auto topologyConfig
        = daisyhost::BuildLiveRackTopologyConfig(rackTopologyPreset_);
    state.entryNodeId  = topologyConfig.entryNodeId;
    state.outputNodeId = topologyConfig.outputNodeId;
    state.appId        = rackNodes_[0].appId;
    state.randomSeed   = rackNodes_[0].randomSeed;
    for(const auto& node : rackNodes_)
    {
        state.nodes.push_back({node.nodeId, node.appId, node.randomSeed});
        for(std::size_t i = 0; i < node.bindings.knobControlIds.size(); ++i)
        {
            if(!node.bindings.knobControlIds[i].empty())
            {
                state.controlValues[node.bindings.knobControlIds[i]]
                    = node.topControlValues[i];
            }
            if(boardId_ == "daisy_field")
            {
                state.controlValues[daisyhost::MakeDaisyFieldKnobControlId(
                    node.nodeId, i)]
                    = node.fieldKnobValues[i];
            }
            if(!node.bindings.cvInputPortIds[i].empty())
            {
                state.cvValues[node.bindings.cvInputPortIds[i]] = node.cvValues[i];
            }
            state.controlValues[MakeCvHostStateId(node.nodeId, i, "mode")]
                = static_cast<float>(
                    node.cvGeneratorStates[i].mode
                    == daisyhost::CvInputSourceMode::kGenerator);
            state.controlValues[MakeCvHostStateId(node.nodeId, i, "waveform")]
                = static_cast<float>(
                    static_cast<int>(node.cvGeneratorStates[i].waveform));
            state.controlValues[MakeCvHostStateId(node.nodeId, i, "frequency_hz")]
                = node.cvGeneratorStates[i].frequencyHz;
            state.controlValues[MakeCvHostStateId(node.nodeId, i, "amplitude_volts")]
                = node.cvGeneratorStates[i].amplitudeVolts;
            state.controlValues[MakeCvHostStateId(node.nodeId, i, "bias_volts")]
                = node.cvGeneratorStates[i].biasVolts;
            state.controlValues[MakeCvHostStateId(node.nodeId, i, "manual_volts")]
                = node.cvGeneratorStates[i].manualVolts;
        }
        if(boardId_ == "daisy_field")
        {
            for(std::size_t i = node.bindings.knobControlIds.size();
                i < node.fieldKnobValues.size();
                ++i)
            {
                state.controlValues[daisyhost::MakeDaisyFieldKnobControlId(
                    node.nodeId, i)]
                    = node.fieldKnobValues[i];
            }
        }
        for(std::size_t i = 0; i < node.bindings.gateInputPortIds.size(); ++i)
        {
            if(!node.bindings.gateInputPortIds[i].empty())
            {
                state.gateValues[node.bindings.gateInputPortIds[i]]
                    = node.gateValues[i];
            }
        }
        state.controlValues[MakeComputerKeyboardEnabledStateId(node.nodeId)]
            = node.computerKeyboardEnabled ? 1.0f : 0.0f;
        state.controlValues[MakeComputerKeyboardOctaveStateId(node.nodeId)]
            = static_cast<float>(node.computerKeyboardOctave);
        state.controlValues[MakeTestInputModeStateId(node.nodeId)]
            = static_cast<float>(node.testInputMode);
        state.controlValues[MakeTestInputLevelStateId(node.nodeId)]
            = node.testInputLevel;
        state.controlValues[MakeTestInputFrequencyStateId(node.nodeId)]
            = node.testInputFrequencyHz;

        if(node.core != nullptr)
        {
            const auto parameters = node.core->CaptureStatefulParameterValues();
            state.parameterValues.insert(parameters.begin(), parameters.end());
        }
    }

    for(const auto& route : topologyConfig.routes)
    {
        state.routes.push_back({route.sourcePortId, route.destPortId});
    }

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
       && !pendingHubStartupRequest_->boardId.empty())
    {
        state.boardId = pendingHubStartupRequest_->boardId;
    }
    if(pendingHubStartupRequest_.has_value()
       && !pendingHubStartupRequest_->appId.empty())
    {
        state.appId = pendingHubStartupRequest_->appId;
        for(auto& node : state.nodes)
        {
            if(node.nodeId == "node0")
            {
                node.appId = pendingHubStartupRequest_->appId;
                break;
            }
        }
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
    std::lock_guard<std::recursive_mutex> coreLock(coreStateMutex_);
    return core_ != nullptr ? juce::String(core_->GetAppDisplayName())
                            : juce::String();
}

std::size_t DaisyHostPatchAudioProcessor::GetRackNodeCount() const
{
    return rackNodes_.size();
}

juce::String DaisyHostPatchAudioProcessor::GetSelectedRackNodeId() const
{
    std::lock_guard<std::recursive_mutex> coreLock(coreStateMutex_);
    return rackNodes_[selectedRackNodeIndex_].nodeId;
}

bool DaisyHostPatchAudioProcessor::SetSelectedRackNodeId(const std::string& nodeId)
{
    std::lock_guard<std::recursive_mutex> coreLock(coreStateMutex_);
    for(std::size_t index = 0; index < rackNodes_.size(); ++index)
    {
        if(rackNodes_[index].nodeId != nodeId)
        {
            continue;
        }

        ReleaseFieldKeys();
        ReleaseFieldSwitches();
        FlushSelectedNodeStateToRack();
        selectedRackNodeIndex_ = index;
        SyncSelectedNodeStateFromRack();
        SyncAutomationParametersFromCore();
        return true;
    }

    return false;
}

juce::String DaisyHostPatchAudioProcessor::GetRackNodeId(std::size_t index) const
{
    std::lock_guard<std::recursive_mutex> coreLock(coreStateMutex_);
    return index < rackNodes_.size() ? juce::String(rackNodes_[index].nodeId) : juce::String();
}

juce::String DaisyHostPatchAudioProcessor::GetRackNodeAppId(std::size_t index) const
{
    std::lock_guard<std::recursive_mutex> coreLock(coreStateMutex_);
    return index < rackNodes_.size() ? juce::String(rackNodes_[index].appId) : juce::String();
}

juce::String DaisyHostPatchAudioProcessor::GetRackNodeAppDisplayName(
    std::size_t index) const
{
    std::lock_guard<std::recursive_mutex> coreLock(coreStateMutex_);
    if(index >= rackNodes_.size() || rackNodes_[index].core == nullptr)
    {
        return {};
    }
    return rackNodes_[index].core->GetAppDisplayName();
}

bool DaisyHostPatchAudioProcessor::SetRackNodeAppId(std::size_t index,
                                                    const std::string& requestedAppId)
{
    std::lock_guard<std::recursive_mutex> coreLock(coreStateMutex_);
    if(index >= rackNodes_.size())
    {
        return false;
    }

    if(index == selectedRackNodeIndex_)
    {
        FlushSelectedNodeStateToRack();
    }

    if(!RecreateRackNode(index, requestedAppId))
    {
        return false;
    }

    auto& node                 = rackNodes_[index];
    node.randomSeed            = 0u;
    node.restoredParameterValues.clear();
    node.pendingMenuValues.clear();
    node.impulseRequested = false;
    node.testPhase        = 0.0f;
    node.noiseState       = 1u;
    if(node.core != nullptr)
    {
        node.core->ResetToDefaultState(node.randomSeed);
        UpdateRackNodeSnapshots(node);
    }

    if(index == selectedRackNodeIndex_)
    {
        SyncSelectedNodeStateFromRack();
        ApplyCanonicalSessionStateToCore();
    }
    return true;
}

int DaisyHostPatchAudioProcessor::GetRackTopologyPreset() const
{
    switch(rackTopologyPreset_)
    {
        case daisyhost::LiveRackTopologyPreset::kNode0Only: return 0;
        case daisyhost::LiveRackTopologyPreset::kNode1Only: return 1;
        case daisyhost::LiveRackTopologyPreset::kNode0ToNode1: return 2;
        case daisyhost::LiveRackTopologyPreset::kNode1ToNode0: return 3;
    }

    return 0;
}

void DaisyHostPatchAudioProcessor::SetRackTopologyPreset(int preset)
{
    switch(preset)
    {
        case 1: rackTopologyPreset_ = daisyhost::LiveRackTopologyPreset::kNode1Only; break;
        case 2: rackTopologyPreset_ = daisyhost::LiveRackTopologyPreset::kNode0ToNode1; break;
        case 3: rackTopologyPreset_ = daisyhost::LiveRackTopologyPreset::kNode1ToNode0; break;
        default: rackTopologyPreset_ = daisyhost::LiveRackTopologyPreset::kNode0Only; break;
    }
    RefreshCoreStateFromIdleHostChange();
}

juce::String DaisyHostPatchAudioProcessor::GetRackNodeRoleLabel(std::size_t index) const
{
    if(index >= rackNodes_.size())
    {
        return {};
    }

    const auto nodeId = rackNodes_[index].nodeId;
    switch(rackTopologyPreset_)
    {
        case daisyhost::LiveRackTopologyPreset::kNode0Only:
            return nodeId == "node0" ? "Entry" : "Inactive";
        case daisyhost::LiveRackTopologyPreset::kNode1Only:
            return nodeId == "node1" ? "Entry" : "Inactive";
        case daisyhost::LiveRackTopologyPreset::kNode0ToNode1:
            return nodeId == "node0" ? "Entry" : "Output";
        case daisyhost::LiveRackTopologyPreset::kNode1ToNode0:
            return nodeId == "node1" ? "Entry" : "Output";
    }

    return {};
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
    std::lock_guard<std::recursive_mutex> coreLock(coreStateMutex_);
    return index < activeBindings_.knobDetailLabels.size()
               ? juce::String(activeBindings_.knobDetailLabels[index])
               : juce::String();
}

std::string DaisyHostPatchAudioProcessor::GetTopControlId(std::size_t index) const
{
    std::lock_guard<std::recursive_mutex> coreLock(coreStateMutex_);
    return index < activeBindings_.knobControlIds.size()
               ? activeBindings_.knobControlIds[index]
               : std::string();
}

bool DaisyHostPatchAudioProcessor::IsDaisyFieldBoard() const
{
    return boardId_ == "daisy_field";
}

float DaisyHostPatchAudioProcessor::GetFieldKnobValue(std::size_t index) const
{
    return index < fieldKnobValues_.size() ? fieldKnobValues_[index].load() : 0.0f;
}

juce::String DaisyHostPatchAudioProcessor::GetFieldKnobLabel(std::size_t index) const
{
    const auto mapping = BuildActiveFieldControlMapping();
    const auto* binding = FindFieldKnobBinding(mapping, index);
    return binding != nullptr ? juce::String(binding->label) : juce::String();
}

juce::String DaisyHostPatchAudioProcessor::GetFieldKnobDetailLabel(
    std::size_t index) const
{
    const auto mapping = BuildActiveFieldControlMapping();
    const auto* binding = FindFieldKnobBinding(mapping, index);
    return binding != nullptr ? juce::String(binding->detailLabel) : juce::String();
}

std::string DaisyHostPatchAudioProcessor::GetFieldKnobControlId(
    std::size_t index) const
{
    const auto mapping = BuildActiveFieldControlMapping();
    const auto* binding = FindFieldKnobBinding(mapping, index);
    return binding != nullptr ? binding->controlId : std::string();
}

void DaisyHostPatchAudioProcessor::SetFieldKnobValue(std::size_t index,
                                                     float       normalizedValue)
{
    if(index >= fieldKnobValues_.size())
    {
        return;
    }

    const float clamped = Clamp01(normalizedValue);
    fieldKnobValues_[index].store(clamped);
    if(index < topControlValues_.size())
    {
        topControlValues_[index].store(clamped);
    }
    RefreshCoreStateFromIdleHostChange();
}

int DaisyHostPatchAudioProcessor::GetFieldKeyMidiNote(std::size_t index) const
{
    const auto mapping = BuildActiveFieldControlMapping();
    if(index >= mapping.keys.size())
    {
        return -1;
    }
    return mapping.keys[index].midiNote;
}

bool DaisyHostPatchAudioProcessor::GetFieldKeyPressed(std::size_t index) const
{
    return index < fieldKeyPressed_.size() && fieldKeyPressed_[index].load();
}

void DaisyHostPatchAudioProcessor::SetFieldKeyPressed(std::size_t index,
                                                      bool        pressed)
{
    if(index >= fieldKeyPressed_.size() || !IsDaisyFieldBoard())
    {
        return;
    }

    const bool wasPressed = fieldKeyPressed_[index].exchange(pressed);
    if(wasPressed == pressed)
    {
        return;
    }

    const int midiNote = GetFieldKeyMidiNote(index);
    if(midiNote >= 0)
    {
        SetVirtualKeyboardNote(midiNote, pressed, pressed ? 1.0f : 0.0f);
    }
}

float DaisyHostPatchAudioProcessor::GetFieldCvOutputValue(std::size_t index) const
{
    const auto snapshot = BuildFieldSurfaceSnapshot();
    return index < snapshot.cvOutputs.size()
               ? snapshot.cvOutputs[index].normalizedValue
               : 0.0f;
}

float DaisyHostPatchAudioProcessor::GetFieldCvOutputVolts(std::size_t index) const
{
    const auto snapshot = BuildFieldSurfaceSnapshot();
    return index < snapshot.cvOutputs.size() ? snapshot.cvOutputs[index].volts
                                             : 0.0f;
}

bool DaisyHostPatchAudioProcessor::GetFieldSwitchPressed(std::size_t index) const
{
    return index < fieldSwitchPressed_.size()
           && fieldSwitchPressed_[index].load();
}

juce::String DaisyHostPatchAudioProcessor::GetFieldSwitchLabel(
    std::size_t index) const
{
    const auto mapping = BuildActiveFieldControlMapping();
    const auto* binding = FindFieldSwitchBinding(mapping, index);
    if(binding == nullptr)
    {
        return {};
    }
    if(!binding->detailLabel.empty())
    {
        return juce::String(binding->label + " " + binding->detailLabel);
    }
    return juce::String(binding->label);
}

bool DaisyHostPatchAudioProcessor::GetFieldSwitchAvailable(std::size_t index) const
{
    const auto mapping = BuildActiveFieldControlMapping();
    const auto* binding = FindFieldSwitchBinding(mapping, index);
    return binding != nullptr && binding->available;
}

void DaisyHostPatchAudioProcessor::SetFieldSwitchPressed(std::size_t index,
                                                         bool        pressed)
{
    if(index >= fieldSwitchPressed_.size() || !IsDaisyFieldBoard())
    {
        return;
    }

    fieldSwitchPressed_[index].store(pressed);
    if(pressed)
    {
        const auto mapping = BuildActiveFieldControlMapping();
        const auto* binding = FindFieldSwitchBinding(mapping, index);
        if(binding != nullptr && binding->available
           && binding->targetKind == daisyhost::BoardSurfaceTargetKind::kMenuItem)
        {
            SetMenuItemValue(binding->targetId, 1.0f);
        }
    }
    RefreshCoreStateFromIdleHostChange();
}

float DaisyHostPatchAudioProcessor::GetFieldLedValue(std::size_t index) const
{
    const auto snapshot = BuildFieldSurfaceSnapshot();
    return index < snapshot.leds.size() ? snapshot.leds[index].normalizedValue
                                        : 0.0f;
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
        const float clamped = Clamp01(normalizedValue);
        topControlValues_[index].store(clamped);
        if(IsDaisyFieldBoard() && index < fieldKnobValues_.size())
        {
            fieldKnobValues_[index].store(clamped);
        }
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

daisyhost::EffectiveHostFieldSurfaceSnapshot
DaisyHostPatchAudioProcessor::BuildFieldSurfaceSnapshot() const
{
    std::lock_guard<std::recursive_mutex> coreLock(coreStateMutex_);
    daisyhost::EffectiveHostFieldSurfaceSnapshot snapshot;
    if(!IsDaisyFieldBoard())
    {
        return snapshot;
    }

    const auto mapping = BuildActiveFieldControlMapping();
    const auto nodeId  = GetSelectedRackNode().nodeId;
    for(std::size_t index = 0; index < mapping.cvOutputs.size(); ++index)
    {
        const auto& binding = mapping.cvOutputs[index];
        auto&       output  = snapshot.cvOutputs[index];
        output.id           = daisyhost::MakeDaisyFieldCvOutputPortId(nodeId, index);
        output.label        = binding.label.empty()
                                  ? "CV OUT " + std::to_string(index + 1)
                                  : binding.label;
        output.available    = binding.available;
        if(binding.available && binding.targetKind == daisyhost::BoardSurfaceTargetKind::kParameter)
        {
            daisyhost::ParameterValueLookup value;
            if(core_ != nullptr)
            {
                value = core_->GetParameterValue(binding.targetId);
            }
            if(!value.hasValue)
            {
                std::lock_guard<std::mutex> lock(menuSnapshotMutex_);
                const auto it = std::find_if(
                    latestParameters_.begin(),
                    latestParameters_.end(),
                    [&binding](const daisyhost::ParameterDescriptor& parameter) {
                        return parameter.id == binding.targetId;
                    });
                if(it != latestParameters_.end())
                {
                    value.hasValue = true;
                    value.value    = it->normalizedValue;
                }
            }
            if(value.hasValue)
            {
                output.normalizedValue = Clamp01(value.value);
                output.volts           = output.normalizedValue * 5.0f;
            }
        }
    }

    for(std::size_t index = 0; index < mapping.switches.size(); ++index)
    {
        const auto& binding = mapping.switches[index];
        auto&       sw      = snapshot.switches[index];
        sw.id               = binding.controlId;
        sw.label            = binding.label.empty()
                                  ? "SW" + std::to_string(index + 1)
                                  : binding.label;
        sw.detailLabel      = binding.detailLabel;
        sw.available        = binding.available;
        sw.pressed          = fieldSwitchPressed_[index].load();
    }

    for(std::size_t index = 0; index < mapping.leds.size(); ++index)
    {
        const auto& binding = mapping.leds[index];
        auto&       led     = snapshot.leds[index];
        led.id              = binding.controlId;
        led.label           = binding.label;
        if(index < daisyhost::kDaisyFieldKeyCount)
        {
            led.normalizedValue = fieldKeyPressed_[index].load() ? 1.0f : 0.0f;
        }
        else if(index < daisyhost::kDaisyFieldKeyCount + daisyhost::kDaisyFieldSwitchCount)
        {
            const std::size_t switchIndex = index - daisyhost::kDaisyFieldKeyCount;
            led.normalizedValue
                = fieldSwitchPressed_[switchIndex].load() ? 1.0f : 0.0f;
        }
        else if(index == 18)
        {
            led.normalizedValue = gateValues_[0].load() ? 1.0f : 0.0f;
        }
        else if(index == 19 && core_ != nullptr
                && !activeBindings_.gateOutputPortId.empty())
        {
            const auto value = core_->GetPortOutput(activeBindings_.gateOutputPortId);
            led.normalizedValue = value.gate ? 1.0f : 0.0f;
        }
    }

    return snapshot;
}

daisyhost::EffectiveHostStateSnapshot
DaisyHostPatchAudioProcessor::GetEffectiveHostStateSnapshot() const
{
    std::vector<daisyhost::ParameterDescriptor> parameters;
    std::vector<daisyhost::MetaControllerDescriptor> metaControllers;
    daisyhost::HostAutomationSlotBindings       automationSlots;
    {
        std::lock_guard<std::mutex> lock(menuSnapshotMutex_);
        parameters      = latestParameters_;
        metaControllers = latestMetaControllers_;
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

    const auto topologyConfig
        = daisyhost::BuildLiveRackTopologyConfig(rackTopologyPreset_);
    std::vector<daisyhost::EffectiveHostNodeSummary> nodeSummaries;
    nodeSummaries.reserve(rackNodes_.size());
    for(const auto& node : rackNodes_)
    {
        daisyhost::EffectiveHostNodeSummary summary;
        summary.nodeId         = node.nodeId;
        summary.appId          = node.appId;
        summary.appDisplayName = node.core != nullptr ? node.core->GetAppDisplayName()
                                                      : std::string();
        summary.selected       = node.nodeId == rackNodes_[selectedRackNodeIndex_].nodeId;
        summary.entryNode      = node.nodeId == topologyConfig.entryNodeId;
        summary.outputNode     = node.nodeId == topologyConfig.outputNodeId;
        nodeSummaries.push_back(std::move(summary));
    }

    std::vector<daisyhost::EffectiveHostRouteSnapshot> routes;
    routes.reserve(topologyConfig.routes.size());
    for(const auto& route : topologyConfig.routes)
    {
        routes.push_back({route.sourcePortId, route.destPortId});
    }

    return daisyhost::BuildEffectiveHostStateSnapshot(
        boardId_,
        rackNodes_[selectedRackNodeIndex_].nodeId,
        rackNodes_.size(),
        topologyConfig.entryNodeId,
        topologyConfig.outputNodeId,
        activeAppId_,
        core_ != nullptr ? core_->GetAppDisplayName() : std::string(),
        nodeSummaries,
        routes,
        activeBindings_,
        parameters,
        automationSlots,
        cvInputs,
        gateInputs,
        audioInput,
        metaControllers,
        BuildFieldSurfaceSnapshot());
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
    const auto nodeId = GetSelectedRackNode().nodeId;
    {
        std::lock_guard<std::mutex> lock(pendingMenuValueMutex_);
        pendingMenuValues_[nodeId + "/menu/midi/computer_keys"] = enabled ? 1.0f : 0.0f;
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
    const auto nodeId = GetSelectedRackNode().nodeId;
    {
        std::lock_guard<std::mutex> lock(pendingMenuValueMutex_);
        pendingMenuValues_[nodeId + "/menu/midi/octave"]
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
    for(auto& pressed : fieldKeyPressed_)
    {
        pressed.store(false);
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
    const auto nodeId = GetSelectedRackNode().nodeId;
    {
        std::lock_guard<std::mutex> lock(pendingMenuValueMutex_);
        pendingMenuValues_[nodeId + "/menu/input/source"]
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
    const auto nodeId = GetSelectedRackNode().nodeId;
    {
        std::lock_guard<std::mutex> lock(pendingMenuValueMutex_);
        pendingMenuValues_[nodeId + "/menu/input/level"] = Clamp01(clamped / 10.0f);
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
    const auto nodeId = GetSelectedRackNode().nodeId;
    {
        std::lock_guard<std::mutex> lock(pendingMenuValueMutex_);
        pendingMenuValues_[nodeId + "/menu/input/fire_impulse"] = 1.0f;
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
    if(core_ == nullptr)
    {
        return;
    }

    for(std::size_t i = 0; i < activeBindings_.knobControlIds.size(); ++i)
    {
        if(!activeBindings_.knobControlIds[i].empty())
        {
            core_->SetControl(activeBindings_.knobControlIds[i],
                              Clamp01(topControlValues_[i].load()));
        }
        if(IsDaisyFieldBoard() && i < fieldKnobValues_.size())
        {
            fieldKnobValues_[i].store(Clamp01(topControlValues_[i].load()));
        }
    }
    ApplyFieldControlStateToCore();
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

void DaisyHostPatchAudioProcessor::ApplyFieldControlStateToCore()
{
    if(!IsDaisyFieldBoard() || core_ == nullptr)
    {
        return;
    }

    const auto mapping = BuildActiveFieldControlMapping();
    for(std::size_t i = 0; i < mapping.knobs.size(); ++i)
    {
        const auto& binding = mapping.knobs[i];
        if(!binding.available)
        {
            continue;
        }
        ApplySurfaceBinding(*core_, binding, Clamp01(fieldKnobValues_[i].load()));
    }
}

void DaisyHostPatchAudioProcessor::SyncFieldHostStateFromCore()
{
    if(!IsDaisyFieldBoard() || core_ == nullptr)
    {
        return;
    }

    const auto mapping = BuildActiveFieldControlMapping();
    for(std::size_t i = 0; i < mapping.knobs.size(); ++i)
    {
        const auto& binding = mapping.knobs[i];
        if(!binding.available)
        {
            continue;
        }

        daisyhost::ParameterValueLookup value;
        switch(binding.targetKind)
        {
            case daisyhost::BoardSurfaceTargetKind::kControl:
                value = core_->GetControlValue(binding.targetId);
                break;
            case daisyhost::BoardSurfaceTargetKind::kParameter:
                value = core_->GetParameterValue(binding.targetId);
                break;
            case daisyhost::BoardSurfaceTargetKind::kUnavailable:
            case daisyhost::BoardSurfaceTargetKind::kCvInput:
            case daisyhost::BoardSurfaceTargetKind::kGateInput:
            case daisyhost::BoardSurfaceTargetKind::kMidiNote:
            case daisyhost::BoardSurfaceTargetKind::kMenuItem:
            case daisyhost::BoardSurfaceTargetKind::kLed: break;
        }

        if(value.hasValue)
        {
            const float clamped = Clamp01(value.value);
            fieldKnobValues_[i].store(clamped);
            if(i < topControlValues_.size())
            {
                topControlValues_[i].store(clamped);
            }
        }
    }
}

void DaisyHostPatchAudioProcessor::ReleaseFieldKeys()
{
    for(std::size_t i = 0; i < fieldKeyPressed_.size(); ++i)
    {
        if(!fieldKeyPressed_[i].exchange(false))
        {
            continue;
        }
        const int midiNote = GetFieldKeyMidiNote(i);
        if(midiNote >= 0)
        {
            SetVirtualKeyboardNote(midiNote, false, 0.0f);
        }
    }
}

void DaisyHostPatchAudioProcessor::ReleaseFieldSwitches()
{
    for(auto& pressed : fieldSwitchPressed_)
    {
        pressed.store(false);
    }
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
    FlushSelectedNodeStateToRack();
    SyncAutomationParametersFromCore();
}

void DaisyHostPatchAudioProcessor::ApplyHubStartupRequestIfNeeded()
{
    if(hubStartupRequestApplied_ || !pendingHubStartupRequest_.has_value())
    {
        return;
    }

    if(!pendingHubStartupRequest_->boardId.empty())
    {
        TryApplyBoardId(pendingHubStartupRequest_->boardId);
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
    SyncFieldHostStateFromCore();
}

void DaisyHostPatchAudioProcessor::UpdateCoreSnapshots()
{
    activeBindings_ = core_->GetPatchBindings();
    UpdateDisplaySnapshot();

    {
        std::lock_guard<std::mutex> lock(menuSnapshotMutex_);
        latestMenu_            = core_->GetMenuModel();
        latestParameters_      = core_->GetParameters();
        latestMetaControllers_ = core_->GetMetaControllers();
        automationSlotBindings_
            = daisyhost::BuildHostAutomationSlotBindings(latestParameters_);
    }

    FlushSelectedNodeStateToRack();
}

void DaisyHostPatchAudioProcessor::LoadSession(
    const daisyhost::HostSessionState& state)
{
    TryApplyBoardId(state.boardId);
    std::vector<daisyhost::LiveRackTopologyRoute> routes;
    routes.reserve(state.routes.size());
    for(const auto& route : state.routes)
    {
        routes.push_back({route.sourcePortId, route.destPortId});
    }
    const daisyhost::LiveRackTopologyConfig topologyConfig{
        state.entryNodeId.empty() ? "node0" : state.entryNodeId,
        state.outputNodeId.empty() ? "node0" : state.outputNodeId,
        routes};
    if(!daisyhost::TryInferLiveRackTopologyPreset(topologyConfig,
                                                  &rackTopologyPreset_,
                                                  nullptr))
    {
        rackTopologyPreset_ = daisyhost::LiveRackTopologyPreset::kNode0Only;
    }

    hasRestoredTestInputMode_ = false;

    for(std::size_t index = 0; index < rackNodes_.size(); ++index)
    {
        auto& node = rackNodes_[index];
        const auto stateNodeIt
            = std::find_if(state.nodes.begin(),
                           state.nodes.end(),
                           [&node](const daisyhost::HostSessionNodeState& stateNode) {
                               return stateNode.nodeId == node.nodeId;
                           });
        const bool hasStateNode = stateNodeIt != state.nodes.end();
        const std::string requestedAppId
            = hasStateNode
                  ? stateNodeIt->appId
                  : (node.nodeId == "node0" && !state.appId.empty()
                         ? state.appId
                         : daisyhost::GetDefaultHostedAppId());

        RecreateRackNode(index, requestedAppId);
        node.randomSeed = hasStateNode ? stateNodeIt->randomSeed
                                       : (node.nodeId == "node0" ? state.randomSeed : 0u);
        node.restoredParameterValues.clear();
        node.pendingMenuValues.clear();
        node.impulseRequested = false;
        node.testPhase        = 0.0f;
        node.noiseState       = node.randomSeed == 0 ? 1u : node.randomSeed;

        for(const auto& entry : state.parameterValues)
        {
            if(entry.first.rfind(node.nodeId + "/", 0) == 0)
            {
                node.restoredParameterValues[entry.first] = entry.second;
            }
        }

        if(node.core != nullptr)
        {
            node.core->ResetToDefaultState(node.randomSeed);
            if(!node.restoredParameterValues.empty())
            {
                node.core->RestoreStatefulParameterValues(node.restoredParameterValues);
            }
        }

        const bool hasCanonicalParameterState = !node.restoredParameterValues.empty();
        for(std::size_t i = 0; i < node.bindings.knobControlIds.size(); ++i)
        {
            if(!hasCanonicalParameterState
               && !node.bindings.knobControlIds[i].empty())
            {
                if(const auto controlIt
                   = state.controlValues.find(node.bindings.knobControlIds[i]);
                   controlIt != state.controlValues.end())
                {
                    node.topControlValues[i] = Clamp01(controlIt->second);
                }
            }
            node.fieldKnobValues[i] = node.topControlValues[i];
            if(const auto fieldIt = state.controlValues.find(
                   daisyhost::MakeDaisyFieldKnobControlId(node.nodeId, i));
               fieldIt != state.controlValues.end())
            {
                node.fieldKnobValues[i] = Clamp01(fieldIt->second);
                if(i < node.topControlValues.size())
                {
                    node.topControlValues[i] = node.fieldKnobValues[i];
                }
            }

            if(!node.bindings.cvInputPortIds[i].empty())
            {
                if(const auto cvIt = state.cvValues.find(node.bindings.cvInputPortIds[i]);
                   cvIt != state.cvValues.end())
                {
                    node.cvValues[i]   = Clamp01(cvIt->second);
                    node.cvVoltages[i] = daisyhost::NormalizedToCvVolts(cvIt->second);
                    node.cvGeneratorStates[i].manualVolts
                        = daisyhost::NormalizedToCvVolts(cvIt->second);
                }
            }

            if(const auto it = state.controlValues.find(
                   MakeCvHostStateId(node.nodeId, i, "mode"));
               it != state.controlValues.end())
            {
                node.cvGeneratorStates[i].mode
                    = ClampCvMode(static_cast<int>(std::round(it->second)));
            }
            if(const auto it = state.controlValues.find(
                   MakeCvHostStateId(node.nodeId, i, "waveform"));
               it != state.controlValues.end())
            {
                node.cvGeneratorStates[i].waveform
                    = static_cast<daisyhost::BasicWaveform>(
                        daisyhost::ClampBasicWaveform(
                            static_cast<int>(std::round(it->second))));
            }
            if(const auto it = state.controlValues.find(
                   MakeCvHostStateId(node.nodeId, i, "frequency_hz"));
               it != state.controlValues.end())
            {
                node.cvGeneratorStates[i].frequencyHz
                    = std::clamp(it->second, 0.01f, 20.0f);
            }
            if(const auto it = state.controlValues.find(
                   MakeCvHostStateId(node.nodeId, i, "amplitude_volts"));
               it != state.controlValues.end())
            {
                node.cvGeneratorStates[i].amplitudeVolts
                    = daisyhost::ClampCvAmplitudeVolts(it->second);
            }
            if(const auto it = state.controlValues.find(
                   MakeCvHostStateId(node.nodeId, i, "bias_volts"));
               it != state.controlValues.end())
            {
                node.cvGeneratorStates[i].biasVolts
                    = daisyhost::ClampCvVoltage(it->second);
            }
            if(const auto it = state.controlValues.find(
                   MakeCvHostStateId(node.nodeId, i, "manual_volts"));
               it != state.controlValues.end())
            {
                node.cvGeneratorStates[i].manualVolts
                    = daisyhost::ClampCvVoltage(it->second);
            }
        }

        for(std::size_t i = node.bindings.knobControlIds.size();
            i < node.fieldKnobValues.size();
            ++i)
        {
            if(const auto fieldIt = state.controlValues.find(
                   daisyhost::MakeDaisyFieldKnobControlId(node.nodeId, i));
               fieldIt != state.controlValues.end())
            {
                node.fieldKnobValues[i] = Clamp01(fieldIt->second);
            }
            else
            {
                node.fieldKnobValues[i] = 0.0f;
            }
        }

        for(std::size_t i = 0; i < node.bindings.gateInputPortIds.size(); ++i)
        {
            if(!node.bindings.gateInputPortIds[i].empty())
            {
                if(const auto gateIt
                   = state.gateValues.find(node.bindings.gateInputPortIds[i]);
                   gateIt != state.gateValues.end())
                {
                    node.gateValues[i] = gateIt->second;
                }
            }
        }

        if(const auto it = state.controlValues.find(
               MakeComputerKeyboardEnabledStateId(node.nodeId));
           it != state.controlValues.end())
        {
            node.computerKeyboardEnabled = it->second >= 0.5f;
        }
        if(const auto it = state.controlValues.find(
               MakeComputerKeyboardOctaveStateId(node.nodeId));
           it != state.controlValues.end())
        {
            node.computerKeyboardOctave
                = daisyhost::ComputerKeyboardMidi::ClampOctave(
                    static_cast<int>(std::round(it->second)));
        }
        if(const auto it = state.controlValues.find(MakeTestInputModeStateId(node.nodeId));
           it != state.controlValues.end())
        {
            node.testInputMode
                = daisyhost::ClampTestInputSignalMode(static_cast<int>(std::round(it->second)));
            hasRestoredTestInputMode_ = true;
        }
        if(const auto it = state.controlValues.find(MakeTestInputLevelStateId(node.nodeId));
           it != state.controlValues.end())
        {
            node.testInputLevel
                = it->second <= 1.0f ? it->second * 10.0f : it->second;
        }
        if(const auto it = state.controlValues.find(
               MakeTestInputFrequencyStateId(node.nodeId));
           it != state.controlValues.end())
        {
            node.testInputFrequencyHz = it->second;
        }

        UpdateRackNodeSnapshots(node);
    }

    {
        std::lock_guard<std::mutex> lock(midiLearnMutex_);
        midiLearnMap_ = state.midiLearn;
        learningTargetId_.clear();
    }

    selectedRackNodeIndex_ = 0;
    for(std::size_t index = 0; index < rackNodes_.size(); ++index)
    {
        if(rackNodes_[index].nodeId == state.selectedNodeId)
        {
            selectedRackNodeIndex_ = index;
            break;
        }
    }

    SyncSelectedNodeStateFromRack();
    SyncAutomationParametersFromCore();
}

bool DaisyHostPatchAudioProcessor::RecreateRackNode(std::size_t        index,
                                                    const std::string& requestedAppId)
{
    std::lock_guard<std::recursive_mutex> coreLock(coreStateMutex_);
    if(index >= rackNodes_.size())
    {
        return false;
    }

    auto& node = rackNodes_[index];
    std::string resolvedAppId;
    auto newCore = daisyhost::CreateHostedAppCore(
        requestedAppId, node.nodeId, &resolvedAppId);
    if(newCore == nullptr)
    {
        return false;
    }

    node.core     = std::move(newCore);
    node.appId    = resolvedAppId;
    node.bindings = node.core->GetPatchBindings();

    if(currentSampleRate_ > 1.0 && currentBlockSize_ > 0)
    {
        node.core->Prepare(currentSampleRate_, currentBlockSize_);
    }

    UpdateRackNodeSnapshots(node);

    if(index == selectedRackNodeIndex_)
    {
        SyncSelectedNodeStateFromRack();
    }
    return true;
}

bool DaisyHostPatchAudioProcessor::RecreateHostedApp(const std::string& requestedAppId)
{
    if(!RecreateRackNode(selectedRackNodeIndex_, requestedAppId))
    {
        return false;
    }

    ClearPendingAutomationParameterState();

    return true;
}

bool DaisyHostPatchAudioProcessor::SetActiveAppId(const std::string& requestedAppId)
{
    std::lock_guard<std::recursive_mutex> coreLock(coreStateMutex_);
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

    std::lock_guard<std::recursive_mutex> coreLock(coreStateMutex_);
    ApplyPendingAutomationParametersToCore();
    ApplyControlStateToCore();
    ApplyPendingMenuInteractionsToCore();
    UpdateCvGeneratorOutputs(0.0);
    ApplyVirtualPortStateToCore(std::vector<daisyhost::MidiMessageEvent>{});
    UpdateCoreSnapshots();
    SyncHostStateFromCore();
    FlushSelectedNodeStateToRack();
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DaisyHostPatchAudioProcessor();
}
