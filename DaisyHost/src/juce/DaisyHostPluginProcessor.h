#pragma once

#include <array>
#include <atomic>
#include <cmath>
#include <mutex>
#include <string>
#include <vector>

#include <juce_audio_processors/juce_audio_processors.h>

#include "daisyhost/BoardProfile.h"
#include "daisyhost/HostSessionState.h"
#include "daisyhost/MidiEventTracker.h"
#include "daisyhost/MidiNotePreview.h"
#include "daisyhost/apps/MultiDelayCore.h"

class DaisyHostPatchAudioProcessorEditor;

class DaisyHostPatchAudioProcessor : public juce::AudioProcessor
{
  public:
    enum TestInputMode
    {
        kHostInput = 0,
        kSineInput,
        kNoiseInput,
        kImpulseInput,
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
    float                          GetKnobValue(std::size_t index) const;
    float                          GetDryWetValue() const;
    int                            GetDryWetPercent() const;
    bool                           GetEncoderPressed() const;
    void                           SetKnobValue(std::size_t index, float normalizedValue);
    void                           SetDryWetValue(float normalizedValue);
    void                           SetEncoderPressed(bool pressed);

    float GetCvValue(std::size_t index) const;
    void  SetCvValue(std::size_t index, float normalizedValue);
    bool  GetGateValue(std::size_t index) const;
    void  SetGateValue(std::size_t index, bool enabled);

    float GetAudioInputPeak(std::size_t index) const;
    float GetAudioOutputPeak(std::size_t index) const;
    float GetMidiActivity() const;
    juce::String GetMidiInputStatusText() const;
    juce::StringArray GetRecentMidiEventLines() const;
    void RefreshMidiInputStatus();
    daisyhost::DisplayModel GetDisplayModelSnapshot() const;
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
    void  TriggerImpulse();
    juce::String GetTestInputModeName(int mode) const;

    void        BeginMidiLearn(const std::string& controlId);
    void        CancelMidiLearn();
    bool        IsLearning(const std::string& controlId) const;
    std::string GetMidiBindingLabel(const std::string& controlId) const;

  private:
    float Clamp01(float value) const;
    void  ApplyControlStateToCore();
    void  ApplyVirtualPortStateToCore(const std::vector<daisyhost::MidiMessageEvent>& midiEvents);
    void  UpdateDisplaySnapshot();
    void  LoadSession(const daisyhost::HostSessionState& state);
    void  GenerateTestInput(float* destination, int numSamples);

    daisyhost::BoardProfile         boardProfile_;
    daisyhost::apps::MultiDelayCore core_;
    std::array<std::string, 4>      knobIds_;
    std::string                     dryWetId_;
    std::string                     encoderButtonId_;
    std::array<std::string, 4>      cvPortIds_;
    std::array<std::string, 2>      gatePortIds_;
    std::array<std::string, 4>      audioInputPortIds_;
    std::array<std::string, 4>      audioOutputPortIds_;
    std::string                     midiInputPortId_;

    std::array<std::atomic<float>, 4> knobValues_;
    std::atomic<float>                dryWetValue_;
    std::atomic<bool>                 encoderPressed_;
    std::array<std::atomic<float>, 4> cvValues_;
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
    std::atomic<bool>                 impulseRequested_;
    float                             testPhase_ = 0.0f;
    std::array<bool, 2>               previousGateValues_;
    juce::Random                      noise_;

    mutable std::mutex       displayMutex_;
    daisyhost::DisplayModel  latestDisplay_;
    mutable std::mutex       midiLearnMutex_;
    daisyhost::MidiLearnMap  midiLearnMap_;
    std::string              learningTargetId_;

    juce::AudioBuffer<float> scratchOutput_;
    std::vector<float>       zeroBuffer_;
    std::vector<float>       generatedInput_;

    const std::string testInputModeStateId_  = "node0/host/test_input_mode";
    const std::string testInputLevelStateId_ = "node0/host/test_input_level";
    const std::string computerKeyboardEnabledStateId_
        = "node0/host/computer_keyboard_enabled";
    const std::string computerKeyboardOctaveStateId_
        = "node0/host/computer_keyboard_octave";

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DaisyHostPatchAudioProcessor)
};
