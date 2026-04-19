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
#include "daisyhost/BoardProfile.h"
#include "daisyhost/HostSessionState.h"
#include "daisyhost/HubSupport.h"
#include "daisyhost/MidiEventTracker.h"
#include "daisyhost/MidiNotePreview.h"
#include "daisyhost/SignalGenerator.h"
#include "daisyhost/TestInputSignal.h"
#include "daisyhost/VersionInfo.h"

class DaisyHostPatchAudioProcessorEditor;

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
    float GetTopControlValue(std::size_t index) const;
    juce::String GetTopControlLabel(std::size_t index) const;
    juce::String GetTopControlDetailLabel(std::size_t index) const;
    std::string GetTopControlId(std::size_t index) const;
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
    float Clamp01(float value) const;
    void  ApplyControlStateToCore();
    void  ApplyPendingMenuInteractionsToCore();
    void  ApplyVirtualPortStateToCore(const std::vector<daisyhost::MidiMessageEvent>& midiEvents);
    void  ApplyHubStartupRequestIfNeeded();
    void  UpdateCvGeneratorOutputs(double blockDurationSeconds);
    void  SyncHostStateFromCore();
    void  UpdateCoreSnapshots();
    void  UpdateDisplaySnapshot();
    void  LoadSession(const daisyhost::HostSessionState& state);
    void  ApplyCanonicalSessionStateToCore();
    bool  RecreateHostedApp(const std::string& requestedAppId);
    void  GenerateTestInput(float* destination, int numSamples);

    daisyhost::BoardProfile         boardProfile_;
    std::unique_ptr<daisyhost::HostedAppCore> core_;
    std::string                     activeAppId_;
    daisyhost::HostedAppPatchBindings activeBindings_;

    std::array<std::atomic<float>, 4> topControlValues_;
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

    mutable std::mutex       displayMutex_;
    daisyhost::DisplayModel  latestDisplay_;
    mutable std::mutex       menuSnapshotMutex_;
    daisyhost::MenuModel     latestMenu_;
    std::vector<daisyhost::ParameterDescriptor> latestParameters_;
    mutable std::mutex       midiLearnMutex_;
    daisyhost::MidiLearnMap  midiLearnMap_;
    std::string              learningTargetId_;
    mutable std::mutex       pendingMenuValueMutex_;
    std::unordered_map<std::string, float> pendingMenuValues_;

    juce::AudioBuffer<float> scratchOutput_;
    std::vector<float>       zeroBuffer_;
    std::vector<float>       generatedInput_;

    const std::string testInputModeStateId_  = "node0/host/test_input_mode";
    const std::string testInputLevelStateId_ = "node0/host/test_input_level";
    const std::string testInputFrequencyStateId_
        = "node0/host/test_input_frequency_hz";
    const std::string computerKeyboardEnabledStateId_
        = "node0/host/computer_keyboard_enabled";
    const std::string computerKeyboardOctaveStateId_
        = "node0/host/computer_keyboard_octave";
    std::uint32_t appRandomSeed_ = 0;
    std::unordered_map<std::string, float> restoredParameterValues_;
    std::optional<daisyhost::HubStartupRequest> pendingHubStartupRequest_;
    bool                                        hubStartupRequestApplied_ = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DaisyHostPatchAudioProcessor)
};
