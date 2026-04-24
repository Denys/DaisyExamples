#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "daisyhost/HostedAppCore.h"
#include "daisysp.h"

namespace daisyhost
{
namespace apps
{
class MultiDelayCore : public HostedAppCore
{
  public:
    static const std::size_t kDelayCount      = 3;
    static const std::size_t kMaxDelaySamples = 48000;
    using DelayLineType = daisysp::DelayLine<float, kMaxDelaySamples>;

    explicit MultiDelayCore(const std::string& nodeId = "node0");

    void AttachDelayStorage(DelayLineType* delayLines, std::size_t count);

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
    const std::vector<MetaControllerDescriptor>& GetMetaControllers() const override;
    bool SetMetaControllerValue(const std::string& controllerId,
                                float              normalizedValue) override;
    ParameterValueLookup GetMetaControllerValue(
        const std::string& controllerId) const override;
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

    int   GetDryWetPercent() const;
    float GetFeedback() const;
    float GetDelayTargetSamples(std::size_t index) const;

    static std::string MakeKnobControlId(const std::string& nodeId,
                                         std::size_t       oneBasedIndex);
    static std::string MakeEncoderControlId(const std::string& nodeId);
    static std::string MakeEncoderButtonControlId(const std::string& nodeId);
    static std::string MakeDryWetControlId(const std::string& nodeId);
    static std::string MakeAudioInputPortId(const std::string& nodeId,
                                            std::size_t       oneBasedIndex);
    static std::string MakeAudioOutputPortId(const std::string& nodeId,
                                             std::size_t       oneBasedIndex);
    static std::string MakeCvInputPortId(const std::string& nodeId,
                                         std::size_t       oneBasedIndex);
    static std::string MakeGateInputPortId(const std::string& nodeId,
                                           std::size_t       oneBasedIndex);
    static std::string MakeGateOutputPortId(const std::string& nodeId,
                                            std::size_t       oneBasedIndex);
    static std::string MakeMidiInputPortId(const std::string& nodeId,
                                           std::size_t       oneBasedIndex);
    static std::string MakeMidiOutputPortId(const std::string& nodeId,
                                            std::size_t       oneBasedIndex);

  private:
    enum class ParameterIndex : std::size_t
    {
        kDryWet = 0,
        kDelayPrimary,
        kDelaySecondary,
        kFeedback,
        kDelayTertiary,
        kCount,
    };

    enum class ParameterSource
    {
        kNone,
        kKnob,
        kCv,
        kMenu,
        kMacro,
    };

    enum class MetaControllerIndex : std::size_t
    {
        kBlend = 0,
        kSpace,
        kRegen,
        kCount,
    };

    struct DelayState
    {
        DelayLineType* line         = nullptr;
        float          currentDelay = 0.0f;
        float          targetDelay  = 0.0f;
    };

    float Clamp01(float value) const;
    float MapLogControl(float normalized, float min, float max) const;
    bool  HasMeaningfulChange(float value, float previous, bool initialized) const;
    void  ApplyParameterValue(ParameterIndex index,
                              float          normalizedValue,
                              ParameterSource source);
    ParameterDescriptor* FindParameterById(const std::string& parameterId);
    const ParameterDescriptor* FindParameterById(
        const std::string& parameterId) const;
    MetaControllerDescriptor* FindMetaControllerById(
        const std::string& controllerId);
    const MetaControllerDescriptor* FindMetaControllerById(
        const std::string& controllerId) const;
    void  UpdateMappedStateFromParameters();
    void  UpdateMetaControllersFromParameters();
    float DeriveMetaControllerValue(MetaControllerIndex index) const;
    void  SyncMenuModel();
    void  UpdateDisplay();
    std::string FormatPercent(float normalized) const;
    std::string FormatDelaySamples(float normalized) const;
    std::string FormatFeedback(float normalized) const;
    std::string FormatInputSource(float normalized) const;
    std::string FormatOctave(float normalized) const;
    std::string FormatMidiStatus() const;
    std::string FormatTrackerLine(std::size_t index) const;
    std::string FormatAboutLine(std::size_t index) const;
    float       GetMenuStepSize(const std::string& itemId) const;
    float       GetCurrentMenuItemValue(const std::string& itemId) const;
    void        SetCurrentSection(const std::string& sectionId);
    void        MoveSelection(int delta);
    const MenuItem* GetSelectedMenuItem() const;
    MenuItem*       GetSelectedMenuItem();
    void            PushTrackerLine(const std::string& line);

    std::string                                  nodeId_;
    std::array<float, 4>                         controlInputs_;
    std::array<bool, 4>                          controlInputInitialized_;
    std::array<float, 4>                         cvInputs_;
    std::array<bool, 4>                          cvInputInitialized_;
    std::array<DelayState, kDelayCount>          delays_;
    DelayLineType*                               delayLineStorage_ = nullptr;
    std::size_t                                  delayLineCount_   = 0;
    std::unique_ptr<DelayLineType[]>             ownedDelayLines_;
    std::vector<ParameterDescriptor>             parameters_;
    std::vector<MetaControllerDescriptor>        metaControllers_;
    std::array<std::uint64_t, static_cast<std::size_t>(ParameterIndex::kCount)>
        parameterTouchSerials_;
    std::array<ParameterSource, static_cast<std::size_t>(ParameterIndex::kCount)>
        parameterSources_;
    std::unordered_map<std::string, PortValue>   portInputs_;
    std::unordered_map<std::string, PortValue>   portOutputs_;
    std::unordered_map<std::string, int>         menuSelections_;
    MenuModel                                    menu_;
    DisplayModel                                 display_;
    double                                       sampleRate_     = 48000.0;
    std::size_t                                  maxBlockSize_   = 48;
    float                                        feedback_       = 0.0f;
    int                                          dryWetPercent_  = 50;
    float                                        inputSourceNormalized_ = 0.0f;
    float                                        inputLevelNormalized_  = 1.0f;
    float                                        computerKeysNormalized_ = 1.0f;
    float                                        midiOctaveNormalized_ = 0.5f;
    std::array<std::string, 3>                   trackerLines_;
    std::array<std::string, 3>                   aboutLines_;
    std::uint64_t                                nextTouchSerial_ = 0;
    std::uint32_t                                randomSeed_      = 0;
    bool                                         encoderPressed_ = false;
};
} // namespace apps
} // namespace daisyhost
