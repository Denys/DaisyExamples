#pragma once

#include <array>
#include <memory>
#include <string>
#include <unordered_map>

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
    struct DelayState
    {
        DelayLineType* line         = nullptr;
        float          currentDelay = 0.0f;
        float          targetDelay  = 0.0f;
    };

    float Clamp01(float value) const;
    float MapLogControl(float normalized, float min, float max) const;
    void  UpdateMappedStateFromControls();
    void  UpdateDisplay();

    std::string                                  nodeId_;
    std::array<float, 4>                         controlsNormalized_;
    std::array<DelayState, kDelayCount>          delays_;
    DelayLineType*                               delayLineStorage_ = nullptr;
    std::size_t                                  delayLineCount_   = 0;
    std::unique_ptr<DelayLineType[]>             ownedDelayLines_;
    std::unordered_map<std::string, PortValue>   portInputs_;
    std::unordered_map<std::string, PortValue>   portOutputs_;
    DisplayModel                                 display_;
    double                                       sampleRate_     = 48000.0;
    std::size_t                                  maxBlockSize_   = 48;
    float                                        feedback_       = 0.0f;
    int                                          dryWetPercent_  = 50;
    bool                                         encoderPressed_ = false;
};
} // namespace apps
} // namespace daisyhost
