#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

#include "daisyhost/DaisyBraidsCore.h"
#include "daisyhost/HostedAppCore.h"

namespace daisyhost
{
namespace apps
{
class BraidsCore : public HostedAppCore
{
  public:
    static constexpr std::size_t kPreferredBlockSize
        = DaisyBraidsCore::kPreferredBlockSize;

    explicit BraidsCore(const std::string& nodeId = "node0");

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

    static std::string MakeParameterId(const std::string& nodeId,
                                       const std::string& suffix);
    static std::string MakeControlId(const std::string& nodeId,
                                     const std::string& suffix);
    static std::string MakeEncoderControlId(const std::string& nodeId);
    static std::string MakeEncoderButtonControlId(const std::string& nodeId);
    static std::string MakeGateInputPortId(const std::string& nodeId,
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
    void ResetMenuState();
    float GetMenuStepSize(const std::string& itemId) const;
    void  MoveSelection(int delta);
    const MenuItem* GetSelectedMenuItem() const;
    MenuItem*       GetSelectedMenuItem();
    std::string     StripParameterId(const std::string& parameterId) const;
    std::string     StripControlId(const std::string& controlId) const;

    std::string                                nodeId_;
    DaisyBraidsCore                            sharedCore_;
    std::vector<ParameterDescriptor>           parameters_;
    MenuModel                                  menu_;
    DisplayModel                               display_;
    std::unordered_map<std::string, int>       menuSelections_;
    std::unordered_map<std::string, PortValue> portInputs_;
    std::unordered_map<std::string, PortValue> portOutputs_;
    bool                                       encoderPressed_ = false;
};
} // namespace apps
} // namespace daisyhost
