#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

#include "daisyhost/DaisySubharmoniqCore.h"
#include "daisyhost/HostedAppCore.h"

namespace daisyhost
{
namespace apps
{
class SubharmoniqCore : public HostedAppCore
{
  public:
    explicit SubharmoniqCore(const std::string& nodeId = "node0");

    std::string GetAppId() const override;
    std::string GetAppDisplayName() const override;
    HostedAppCapabilities GetCapabilities() const override;
    HostedAppPatchBindings GetPatchBindings() const override;

    void Prepare(double sampleRate, std::size_t maxBlockSize) override;
    void Process(const AudioBufferView& input,
                 const AudioBufferWriteView& output,
                 std::size_t frameCount) override;
    void SetControl(const std::string& controlId, float normalizedValue) override;
    void SetEncoderDelta(int delta) override;
    void SetEncoderPress(bool pressed) override;
    void SetPortInput(const std::string& portId, const PortValue& value) override;
    PortValue GetPortOutput(const std::string& portId) const override;
    void TickUi(double deltaMs) override;
    bool SetParameterValue(const std::string& parameterId,
                           float              normalizedValue) override;
    ParameterValueLookup GetControlValue(
        const std::string& controlId) const override;
    ParameterValueLookup GetParameterValue(
        const std::string& parameterId) const override;
    ParameterValueLookup GetEffectiveParameterValue(
        const std::string& parameterId) const override;
    void ResetToDefaultState(std::uint32_t seed = 0) override;
    std::unordered_map<std::string, float> CaptureStatefulParameterValues()
        const override;
    void RestoreStatefulParameterValues(
        const std::unordered_map<std::string, float>& values) override;
    const std::vector<ParameterDescriptor>& GetParameters() const override;
    const MenuModel& GetMenuModel() const override;
    void MenuRotate(int delta) override;
    void MenuPress() override;
    void SetMenuItemValue(const std::string& itemId,
                          float              normalizedValue) override;
    const DisplayModel& GetDisplayModel() const override;

    DaisySubharmoniqPageBinding GetActivePageBinding() const;
    bool SetActivePage(DaisySubharmoniqPage page);
    bool TriggerMomentaryAction(const std::string& actionId);
    void SetSequencerStepValue(std::size_t sequencer,
                               std::size_t step,
                               float       normalizedValue);
    void SetRhythmDivisor(std::size_t rhythm, int divisor);
    int  GetRhythmDivisor(std::size_t rhythm) const;
    void SetRhythmTarget(std::size_t rhythm,
                         DaisySubharmoniqRhythmTarget target);
    DaisySubharmoniqRhythmTarget GetRhythmTarget(std::size_t rhythm) const;
    void HandleMidiEvent(std::uint8_t status,
                         std::uint8_t data1,
                         std::uint8_t data2);
    void TriggerGate(bool high);
    void ProcessAudio(float* outputLeft,
                      float* outputRight,
                      std::size_t frameCount);
    bool SetParameterValue(const char* parameterId, float normalizedValue);
    void SetCvInput(std::size_t oneBasedIndex, float normalizedValue);
    float GetGateOutputPulse() const;
    DaisySubharmoniqPage GetActivePage() const;
    int  GetSequencerStepIndex(std::size_t sequencer) const;
    void SetQuantizeMode(DaisySubharmoniqQuantizeMode mode);
    DaisySubharmoniqQuantizeMode GetQuantizeMode() const;
    void SetSeqOctaveRange(int octaveRange);
    int  GetSeqOctaveRange() const;
    float GetSequencerCv(std::size_t sequencer) const;
    std::uint32_t GetTriggerCount() const;
    int   GetCurrentMidiNote() const;
    bool  IsPlaying() const;

    static std::string MakeParameterId(const std::string& nodeId,
                                       const std::string& suffix);
    static std::string MakeControlId(const std::string& nodeId,
                                     const std::string& suffix);
    static std::string MakeEncoderControlId(const std::string& nodeId);
    static std::string MakeEncoderButtonControlId(const std::string& nodeId);
    static std::string MakeCvInputPortId(const std::string& nodeId,
                                         std::size_t       oneBasedIndex);
    static std::string MakeGateInputPortId(const std::string& nodeId,
                                           std::size_t       oneBasedIndex);
    static std::string MakeGateOutputPortId(const std::string& nodeId,
                                            std::size_t       oneBasedIndex);
    static std::string MakeMidiInputPortId(const std::string& nodeId,
                                           std::size_t       oneBasedIndex);
    static std::string MakeAudioOutputPortId(const std::string& nodeId,
                                             std::size_t       oneBasedIndex);

  private:
    void RefreshSnapshots();
    void RefreshParameters();
    void BuildMenuModel();
    void BuildDisplay();
    bool TriggerFieldKeyAction(std::size_t zeroBasedIndex);
    std::string StripParameterId(const std::string& parameterId) const;
    std::string StripControlId(const std::string& controlId) const;

    std::string                                nodeId_;
    DaisySubharmoniqCore                       sharedCore_;
    std::vector<ParameterDescriptor>           parameters_;
    MenuModel                                  menu_;
    DisplayModel                               display_;
    std::unordered_map<std::string, PortValue> portInputs_;
    std::unordered_map<std::string, PortValue> portOutputs_;
    int                                        menuSectionIndex_ = 0;
    bool                                       encoderPressed_ = false;
};
} // namespace apps
} // namespace daisyhost
