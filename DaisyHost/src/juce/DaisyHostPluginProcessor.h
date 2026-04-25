#pragma once

#include <array>
#include <atomic>
#include <cmath>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <juce_audio_processors/juce_audio_processors.h>

#include "daisyhost/AppRegistry.h"
#include "daisyhost/BoardControlMapping.h"
#include "daisyhost/BoardProfile.h"
#include "daisyhost/EffectiveHostStateSnapshot.h"
#include "daisyhost/HostSessionState.h"
#include "daisyhost/HubSupport.h"
#include "daisyhost/LiveRackTopology.h"
#include "daisyhost/MidiEventTracker.h"
#include "daisyhost/MidiNotePreview.h"
#include "daisyhost/SignalGenerator.h"
#include "daisyhost/TestInputSignal.h"
#include "daisyhost/VersionInfo.h"

class DaisyHostPatchAudioProcessorEditor;
class DaisyHostAutomationParameter;

class DaisyHostPatchAudioProcessor : public juce::AudioProcessor
{
  public:
    enum TestInputMode
    {
        kHostInput = 0,
        kSineInput,
        kSawInput,
        kNoiseInput,
        kImpulseInput,
        kTriangleInput,
        kSquareInput,
        kNumTestInputModes
    };

    DaisyHostPatchAudioProcessor();
    ~DaisyHostPatchAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool                        hasEditor() const override;

    const juce::String getName() const override;
    bool               acceptsMidi() const override;
    bool               producesMidi() const override;
    bool               isMidiEffect() const override;
    double             getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    const daisyhost::BoardProfile& GetBoardProfile() const;
    const daisyhost::HostedAppPatchBindings& GetActivePatchBindings() const;
    juce::String GetActiveAppId() const;
    juce::String GetActiveAppDisplayName() const;
    bool SetActiveAppId(const std::string& requestedAppId);
    std::size_t GetRackNodeCount() const;
    juce::String GetSelectedRackNodeId() const;
    bool SetSelectedRackNodeId(const std::string& nodeId);
    juce::String GetRackNodeId(std::size_t index) const;
    juce::String GetRackNodeAppId(std::size_t index) const;
    juce::String GetRackNodeAppDisplayName(std::size_t index) const;
    bool SetRackNodeAppId(std::size_t index, const std::string& requestedAppId);
    int GetRackTopologyPreset() const;
    void SetRackTopologyPreset(int preset);
    juce::String GetRackNodeRoleLabel(std::size_t index) const;
    float GetTopControlValue(std::size_t index) const;
    juce::String GetTopControlLabel(std::size_t index) const;
    juce::String GetTopControlDetailLabel(std::size_t index) const;
    std::string GetTopControlId(std::size_t index) const;
    bool IsDaisyFieldBoard() const;
    float GetFieldKnobValue(std::size_t index) const;
    juce::String GetFieldKnobLabel(std::size_t index) const;
    juce::String GetFieldKnobDetailLabel(std::size_t index) const;
    std::string GetFieldKnobControlId(std::size_t index) const;
    void SetFieldKnobValue(std::size_t index, float normalizedValue);
    int  GetFieldKeyMidiNote(std::size_t index) const;
    bool GetFieldKeyPressed(std::size_t index) const;
    void SetFieldKeyPressed(std::size_t index, bool pressed);
    float GetFieldCvOutputValue(std::size_t index) const;
    float GetFieldCvOutputVolts(std::size_t index) const;
    bool GetFieldSwitchPressed(std::size_t index) const;
    juce::String GetFieldSwitchLabel(std::size_t index) const;
    bool GetFieldSwitchAvailable(std::size_t index) const;
    void SetFieldSwitchPressed(std::size_t index, bool pressed);
    float GetFieldLedValue(std::size_t index) const;
    bool                           GetEncoderPressed() const;
    void                           SetTopControlValue(std::size_t index, float normalizedValue);
    void                           SetEncoderPressed(bool pressed);

    float GetCvValue(std::size_t index) const;
    void  SetCvValue(std::size_t index, float normalizedValue);
    float GetCvVoltage(std::size_t index) const;
    void  SetCvManualVoltage(std::size_t index, float volts);
    int   GetCvSourceMode(std::size_t index) const;
    void  SetCvSourceMode(std::size_t index, int mode);
    int   GetCvWaveform(std::size_t index) const;
    void  SetCvWaveform(std::size_t index, int waveform);
    float GetCvFrequencyHz(std::size_t index) const;
    void  SetCvFrequencyHz(std::size_t index, float frequencyHz);
    float GetCvAmplitudeVolts(std::size_t index) const;
    void  SetCvAmplitudeVolts(std::size_t index, float volts);
    float GetCvBiasVolts(std::size_t index) const;
    void  SetCvBiasVolts(std::size_t index, float volts);
    bool  GetGateValue(std::size_t index) const;
    void  SetGateValue(std::size_t index, bool enabled);

    float GetAudioInputPeak(std::size_t index) const;
    float GetAudioOutputPeak(std::size_t index) const;
    float GetMidiActivity() const;
    juce::String GetMidiInputStatusText() const;
    juce::StringArray GetRecentMidiEventLines() const;
    void RefreshMidiInputStatus();
    daisyhost::DisplayModel GetDisplayModelSnapshot() const;
    daisyhost::MenuModel GetMenuModelSnapshot() const;
    std::vector<daisyhost::ParameterDescriptor> GetParameterSnapshot() const;
    daisyhost::EffectiveHostStateSnapshot GetEffectiveHostStateSnapshot() const;
    juce::MidiKeyboardState& GetVirtualKeyboardState();
    bool  GetComputerKeyboardEnabled() const;
    void  SetComputerKeyboardEnabled(bool enabled);
    int   GetComputerKeyboardOctave() const;
    void  SetComputerKeyboardOctave(int octave);
    void  SetVirtualKeyboardNote(int midiNote, bool isDown, float velocity = 1.0f);
    void  AllVirtualKeyboardNotesOff();
    int   GetTestInputMode() const;
    void  SetTestInputMode(int mode);
    float GetTestInputLevel() const;
    void  SetTestInputLevel(float level);
    float GetTestInputFrequencyHz() const;
    void  SetTestInputFrequencyHz(float frequencyHz);
    void  TriggerImpulse();
    juce::String GetTestInputModeName(int mode) const;
    juce::String GetVersionText() const;
    juce::String GetBuildIdentityText() const;
    juce::StringArray GetReleaseHighlightLines() const;
    bool IsStandaloneWrapper() const;
    void RotateEncoder(int delta);
    void PulseEncoderPress();
    void SetMenuItemValue(const std::string& itemId, float normalizedValue);

    void        BeginMidiLearn(const std::string& controlId);
    void        CancelMidiLearn();
    bool        IsLearning(const std::string& controlId) const;
    std::string GetMidiBindingLabel(const std::string& controlId) const;

  private:
    struct RackNodeState
    {
        std::string                                nodeId;
        std::string                                appId;
        std::unique_ptr<daisyhost::HostedAppCore>  core;
        daisyhost::HostedAppPatchBindings          bindings;
        std::array<float, 4>                       topControlValues{};
        std::array<float, daisyhost::kDaisyFieldKnobCount> fieldKnobValues{};
        std::array<bool, daisyhost::kDaisyFieldKeyCount> fieldKeyPressed{};
        std::array<bool, daisyhost::kDaisyFieldSwitchCount> fieldSwitchPressed{};
        std::array<float, 4>                       cvValues{};
        std::array<float, 4>                       cvVoltages{};
        std::array<bool, 2>                        gateValues{};
        std::array<bool, 2>                        previousGateValues{};
        std::array<float, 4>                       audioInputPeaks{};
        std::array<float, 4>                       audioOutputPeaks{};
        std::array<daisyhost::CvInputGeneratorState, 4> cvGeneratorStates{};
        bool                                       computerKeyboardEnabled = true;
        int                                        computerKeyboardOctave  = 4;
        int                                        testInputMode           = kHostInput;
        float                                      testInputLevel          = 3.5f;
        float                                      testInputFrequencyHz    = 220.0f;
        bool                                       impulseRequested        = false;
        float                                      testPhase               = 0.0f;
        std::uint32_t                              noiseState              = 1u;
        std::uint32_t                              randomSeed              = 0u;
        std::unordered_map<std::string, float>     restoredParameterValues;
        daisyhost::DisplayModel                    latestDisplay;
        daisyhost::MenuModel                       latestMenu;
        std::vector<daisyhost::ParameterDescriptor> latestParameters;
        std::vector<daisyhost::MetaControllerDescriptor> latestMetaControllers;
        daisyhost::HostAutomationSlotBindings      automationSlotBindings{};
        std::unordered_map<std::string, float>     pendingMenuValues;
    };

    friend class DaisyHostAutomationParameter;

    float Clamp01(float value) const;
    RackNodeState& GetSelectedRackNode();
    const RackNodeState& GetSelectedRackNode() const;
    RackNodeState* GetRackNodeById(const std::string& nodeId);
    const RackNodeState* GetRackNodeById(const std::string& nodeId) const;
    void FlushSelectedNodeStateToRack();
    void SyncSelectedNodeStateFromRack();
    bool TryApplyBoardId(const std::string& requestedBoardId);
    void UpdateSelectedBoardProfile();
    void UpdateRackNodeSnapshots(RackNodeState& node);
    void StepRackNodeCvGenerators(RackNodeState& node, double blockDurationSeconds);
    void ApplyRackNodeVirtualPortStateToCore(
        RackNodeState& node,
        const std::vector<daisyhost::MidiMessageEvent>& midiEvents);
    bool RecreateRackNode(std::size_t index, const std::string& requestedAppId);
    std::string MakeCvHostStateId(const std::string& nodeId,
                                  std::size_t        index,
                                  const char*        suffix) const;
    std::string MakeTestInputModeStateId(const std::string& nodeId) const;
    std::string MakeTestInputLevelStateId(const std::string& nodeId) const;
    std::string MakeTestInputFrequencyStateId(const std::string& nodeId) const;
    std::string MakeComputerKeyboardEnabledStateId(const std::string& nodeId) const;
    std::string MakeComputerKeyboardOctaveStateId(const std::string& nodeId) const;
    daisyhost::DaisyFieldControlMapping BuildActiveFieldControlMapping() const;
    daisyhost::EffectiveHostFieldSurfaceSnapshot BuildFieldSurfaceSnapshot() const;
    void  ApplyFieldControlStateToCore();
    void  SyncFieldHostStateFromCore();
    void  ReleaseFieldKeys();
    void  ReleaseFieldSwitches();
    void  ApplyControlStateToCore();
    void  ApplyPendingAutomationParametersToCore();
    void  ApplyPendingMenuInteractionsToCore();
    void  ApplyVirtualPortStateToCore(const std::vector<daisyhost::MidiMessageEvent>& midiEvents);
    void  ApplyHubStartupRequestIfNeeded();
    void  ClearPendingAutomationParameterState();
    void  HandleAutomationParameterValueChanged(std::size_t slotIndex,
                                                float       normalizedValue);
    void  RefreshCoreStateFromIdleHostChange();
    void  SyncAutomationParametersFromCore();
    void  UpdateCvGeneratorOutputs(double blockDurationSeconds);
    void  SyncHostStateFromCore();
    void  UpdateCoreSnapshots();
    void  UpdateDisplaySnapshot();
    void  LoadSession(const daisyhost::HostSessionState& state);
    void  ApplyCanonicalSessionStateToCore();
    bool  RecreateHostedApp(const std::string& requestedAppId);
    void  GenerateTestInput(float* destination, int numSamples);

    daisyhost::BoardProfile         boardProfile_;
    std::string                     boardId_ = "daisy_patch";
    std::array<RackNodeState, 2>    rackNodes_;
    std::size_t                     selectedRackNodeIndex_ = 0;
    daisyhost::LiveRackTopologyPreset rackTopologyPreset_
        = daisyhost::LiveRackTopologyPreset::kNode0Only;
    daisyhost::HostedAppCore*       core_ = nullptr;
    std::string                     activeAppId_;
    daisyhost::HostedAppPatchBindings activeBindings_;

    std::array<std::atomic<float>, 4> topControlValues_;
    std::array<std::atomic<float>, daisyhost::kDaisyFieldKnobCount>
        fieldKnobValues_;
    std::array<std::atomic<bool>, daisyhost::kDaisyFieldKeyCount>
        fieldKeyPressed_;
    std::array<std::atomic<bool>, daisyhost::kDaisyFieldSwitchCount>
        fieldSwitchPressed_;
    std::array<juce::AudioParameterFloat*, daisyhost::kHostAutomationSlotCount>
        automationParameters_{};
    std::array<std::atomic<float>, daisyhost::kHostAutomationSlotCount>
        pendingAutomationParameterValues_{};
    std::array<std::atomic<bool>, daisyhost::kHostAutomationSlotCount>
        pendingAutomationParameterDirty_{};
    std::atomic<bool> isProcessingBlock_{false};
    std::atomic<bool> isSyncingAutomationParameters_{false};
    std::atomic<bool>                 encoderPressed_;
    std::array<std::atomic<float>, 4> cvValues_;
    std::array<std::atomic<float>, 4> cvVoltages_;
    std::array<std::atomic<bool>, 2>  gateValues_;
    std::array<std::atomic<float>, 4> audioInputPeaks_;
    std::array<std::atomic<float>, 4> audioOutputPeaks_;
    std::atomic<float>                midiActivity_;
    daisyhost::MidiEventTracker       midiEventTracker_;
    juce::MidiKeyboardState           virtualKeyboardState_;
    daisyhost::MidiNotePreview        midiNotePreview_;
    std::atomic<bool>                 computerKeyboardEnabled_;
    std::atomic<int>                  computerKeyboardOctave_;
    bool                              didApplyStartupTestInputPolicy_ = false;
    bool                              hasRestoredTestInputMode_       = false;
    std::atomic<int>                  testInputMode_;
    std::atomic<float>                testInputLevel_;
    std::atomic<float>                testInputFrequencyHz_;
    std::atomic<bool>                 impulseRequested_;
    std::atomic<int>                  pendingEncoderDelta_;
    std::atomic<int>                  pendingEncoderPressCount_;
    float                             testPhase_ = 0.0f;
    std::uint32_t                     noiseState_ = 1u;
    std::array<bool, 2>               previousGateValues_;
    std::array<daisyhost::CvInputGeneratorState, 4> cvGeneratorStates_;
    double                            currentSampleRate_ = 0.0;
    std::size_t                       currentBlockSize_  = 0;

    // HostedAppCore instances are not thread-safe; serialize audio-thread
    // processing with message-thread control refreshes and board/app switches.
    mutable std::recursive_mutex coreStateMutex_;
    mutable std::mutex       displayMutex_;
    daisyhost::DisplayModel  latestDisplay_;
    mutable std::mutex       menuSnapshotMutex_;
    daisyhost::MenuModel     latestMenu_;
    std::vector<daisyhost::ParameterDescriptor> latestParameters_;
    std::vector<daisyhost::MetaControllerDescriptor> latestMetaControllers_;
    daisyhost::HostAutomationSlotBindings automationSlotBindings_{};
    mutable std::mutex       midiLearnMutex_;
    daisyhost::MidiLearnMap  midiLearnMap_;
    std::string              learningTargetId_;
    mutable std::mutex       pendingMenuValueMutex_;
    std::unordered_map<std::string, float> pendingMenuValues_;

    juce::AudioBuffer<float> scratchOutput_;
    std::vector<float>       zeroBuffer_;
    std::vector<float>       generatedInput_;

    std::uint32_t appRandomSeed_ = 0;
    std::unordered_map<std::string, float> restoredParameterValues_;
    std::optional<daisyhost::HubStartupRequest> pendingHubStartupRequest_;
    bool                                        hubStartupRequestApplied_ = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DaisyHostPatchAudioProcessor)
};
