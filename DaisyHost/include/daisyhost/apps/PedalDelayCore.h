#pragma once

// Host app wrapper around PedalDelayEngine: the compact multi-delay pedal as a
// selectable DaisyHost app. Owns the parameter descriptors (musician label +
// engineering label + unit + range + DSP targets), the menu/display model, the
// board bindings, and the delay/freeze storage.

#include <array>
#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

#include "daisyhost/HostedAppCore.h"
#include "daisyhost/PedalDelayEngine.h"

namespace daisyhost
{
namespace apps
{
    class PedalDelayCore : public HostedAppCore
    {
      public:
        explicit PedalDelayCore(const std::string& nodeId = "node0");

        std::string            GetAppId() const override;
        std::string            GetAppDisplayName() const override;
        HostedAppCapabilities  GetCapabilities() const override;
        HostedAppPatchBindings GetPatchBindings() const override;

        void      Prepare(double sampleRate, std::size_t maxBlockSize) override;
        void      Process(const AudioBufferView&      input,
                          const AudioBufferWriteView& output,
                          std::size_t                 frameCount) override;
        void      SetControl(const std::string& controlId,
                             float              normalizedValue) override;
        void      SetEncoderDelta(int delta) override;
        void      SetEncoderPress(bool pressed) override;
        void      SetPortInput(const std::string& portId,
                               const PortValue&   value) override;
        PortValue GetPortOutput(const std::string& portId) const override;
        void      TickUi(double deltaMs) override;
        bool      SetParameterValue(const std::string& parameterId,
                                    float              normalizedValue) override;
        bool      SetEffectiveParameterValue(const std::string& parameterId,
                                             float normalizedValue) override;
        void      ClearEffectiveParameterOverrides() override;
        ParameterValueLookup
        GetControlValue(const std::string& controlId) const override;
        ParameterValueLookup
                             GetParameterValue(const std::string& parameterId) const override;
        ParameterValueLookup GetEffectiveParameterValue(
            const std::string& parameterId) const override;
        std::array<float, 16> GetFieldKeyLedValues() const override;
        void ResetToDefaultState(std::uint32_t seed = 0) override;
        std::unordered_map<std::string, float>
             CaptureStatefulParameterValues() const override;
        void RestoreStatefulParameterValues(
            const std::unordered_map<std::string, float>& values) override;
        const std::vector<ParameterDescriptor>& GetParameters() const override;
        const MenuModel&                        GetMenuModel() const override;
        void                                    MenuRotate(int delta) override;
        void                                    MenuPress() override;
        void                SetMenuItemValue(const std::string& itemId,
                                             float              normalizedValue) override;
        const DisplayModel& GetDisplayModel() const override;

        // Host/debug surface used by the tests and by the CLI.
        PedalDelayEngine&       GetEngine() { return engine_; }
        const PedalDelayEngine& GetEngine() const { return engine_; }
        // Parameter ids of exactly the five performance slots, in slot order.
        std::array<std::string, kPedalSlotCount>
                    GetPerformanceSlotParameterIds() const;
        std::string FormatSlotValue(PedalSlot slot) const;

        static std::string MakeParameterId(const std::string& nodeId,
                                           const std::string& suffix);
        static std::string MakeControlId(const std::string& nodeId,
                                         const std::string& suffix);
        static std::string MakeMenuSectionId(const std::string& nodeId,
                                             const std::string& suffix);
        static std::string MakeMenuItemId(const std::string& nodeId,
                                          const std::string& section,
                                          const std::string& item);
        static std::string MakeAudioInputPortId(const std::string& nodeId,
                                                std::size_t oneBasedIndex);
        static std::string MakeAudioOutputPortId(const std::string& nodeId,
                                                 std::size_t oneBasedIndex);
        static std::string MakeCvInputPortId(const std::string& nodeId,
                                             std::size_t        oneBasedIndex);
        static std::string MakeGateInputPortId(const std::string& nodeId);
        static std::string MakeMidiInputPortId(const std::string& nodeId);

      private:
        static const char* SlotSuffix(PedalSlot slot);
        static const char* ModeSuffix(PedalDelayMode mode);

        void ApplySlot(PedalSlot slot, float normalizedValue, bool updateBase);
        void SelectMode(PedalDelayMode mode);
        void RefreshSnapshots();
        void RefreshParameters();
        void BuildMenuModel();
        void BuildDisplay();
        void MoveSelection(int delta);
        void PressSelectedItem();
        void ConsumeGateInput();
        std::string StripPrefix(const std::string& value,
                                const std::string& prefix) const;
        std::string StripMenuItemPrefix(const std::string& itemId,
                                        const char*        section) const;
        const ParameterDescriptor*
        FindParameter(const std::string& parameterId) const;

        std::string nodeId_;
        // Cached so Process() never constructs a port-id string.
        std::string        gateInputPortId_;
        std::string        audioOutputPortId1_;
        std::string        audioOutputPortId2_;
        PedalDelayEngine   engine_;
        std::vector<float> historyStorage_;
        std::vector<float> freezeStorage_;
        std::vector<float> scratchLeft_;
        std::vector<float> scratchRight_;

        // UI-facing base values per mode; the engine always holds the
        // effective (post-modulation) value that the DSP actually uses.
        std::array<std::array<float, kPedalSlotCount>, kPedalDelayModeCount>
            baseNormalized_{};

        std::vector<ParameterDescriptor>           parameters_;
        MenuModel                                  menu_;
        DisplayModel                               display_;
        std::unordered_map<std::string, PortValue> portInputs_;
        std::unordered_map<std::string, PortValue> portOutputs_;

        PedalSlot lastTouchedSlot_ = PedalSlot::kTime;
        double    uiClockMs_       = 0.0;
        bool      encoderPressed_  = false;
        bool      gateHigh_        = false;
    };
} // namespace apps
} // namespace daisyhost
