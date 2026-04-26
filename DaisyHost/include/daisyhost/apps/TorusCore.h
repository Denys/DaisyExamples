#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "daisyhost/HostedAppCore.h"

namespace daisyhost
{
namespace apps
{
class TorusCore : public HostedAppCore
{
  public:
    struct Impl;

    static constexpr std::size_t kPreferredBlockSize = 48;

    explicit TorusCore(const std::string& nodeId = "node0");
    ~TorusCore() override;

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
    bool SetEffectiveParameterValue(const std::string& parameterId,
                                    float normalizedValue) override;
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

    static std::string MakeKnobControlId(const std::string& nodeId,
                                         std::size_t       oneBasedIndex);
    static std::string MakeEncoderControlId(const std::string& nodeId);
    static std::string MakeEncoderButtonControlId(const std::string& nodeId);
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
    std::string               nodeId_;
    std::unique_ptr<Impl>     impl_;
};
} // namespace apps
} // namespace daisyhost
