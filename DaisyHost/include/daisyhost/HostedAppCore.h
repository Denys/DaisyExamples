#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "daisyhost/DisplayModel.h"
#include "daisyhost/VirtualPort.h"

namespace daisyhost
{
struct AudioBufferView
{
    const float* const* channels     = nullptr;
    std::size_t         channelCount = 0;
};

struct AudioBufferWriteView
{
    float* const* channels     = nullptr;
    std::size_t   channelCount = 0;
};

struct MidiMessageEvent
{
    std::uint8_t status = 0;
    std::uint8_t data1  = 0;
    std::uint8_t data2  = 0;
};

struct PortValue
{
    VirtualPortType               type   = VirtualPortType::kAudio;
    float                         scalar = 0.0f;
    bool                          gate   = false;
    std::vector<MidiMessageEvent> midiEvents;
};

enum class ParameterRole
{
    kGeneric,
    kMix,
    kPrimaryDelay,
    kSecondaryDelay,
    kFeedback,
    kTertiaryDelay,
};

struct HostedAppCapabilities
{
    bool acceptsAudioInput  = false;
    bool acceptsMidiInput   = false;
    bool producesMidiOutput = false;
};

struct HostedAppPatchBindings
{
    std::array<std::string, 4> knobControlIds{};
    std::array<std::string, 4> knobDetailLabels{};
    std::string                encoderControlId;
    std::string                encoderButtonControlId;
    std::array<std::string, 4> cvInputPortIds{};
    std::array<std::string, 2> gateInputPortIds{};
    std::string                gateOutputPortId;
    std::array<std::string, 4> audioInputPortIds{};
    std::array<std::string, 4> audioOutputPortIds{};
    std::string                midiInputPortId;
    std::string                midiOutputPortId;
    std::array<int, 2>         mainOutputChannels{{0, 1}};
};

struct ParameterDescriptor
{
    std::string   id;
    std::string   label;
    float         normalizedValue = 0.0f;
    float         defaultNormalizedValue = 0.0f;
    float         effectiveNormalizedValue = 0.0f;
    std::string   unitLabel;
    int           stepCount       = 0;
    ParameterRole role            = ParameterRole::kGeneric;
    int           importanceRank  = 0;
    bool          automatable     = false;
    bool          stateful        = false;
    bool          menuEditable    = false;
};

struct MetaControllerDescriptor
{
    std::string id;
    std::string label;
    float       normalizedValue        = 0.0f;
    float       defaultNormalizedValue = 0.0f;
    bool        stateful               = false;
};

struct ParameterValueLookup
{
    bool  hasValue = false;
    float value    = 0.0f;
};

enum class MenuItemActionKind
{
    kValue,
    kMomentary,
    kReadonly,
    kBack,
    kEnterSection,
};

struct MenuItem
{
    std::string        id;
    std::string        label;
    bool               editable      = false;
    MenuItemActionKind actionKind    = MenuItemActionKind::kReadonly;
    float              normalizedValue = 0.0f;
    std::string        valueText;
    std::string        targetSectionId;
};

struct MenuSection
{
    std::string           id;
    std::string           title;
    std::vector<MenuItem> items;
    int                   selectedIndex = 0;
};

struct MenuModel
{
    bool                    isOpen          = false;
    bool                    isEditing       = false;
    std::vector<std::string> sectionStack;
    std::vector<MenuSection> sections;
    std::string             currentSectionId = "root";
    int                     currentSelection = 0;
};

class HostedAppCore
{
  public:
    virtual ~HostedAppCore() {}

    virtual std::string GetAppId() const          = 0;
    virtual std::string GetAppDisplayName() const = 0;
    virtual HostedAppCapabilities GetCapabilities() const = 0;
    virtual HostedAppPatchBindings GetPatchBindings() const = 0;

    virtual void Prepare(double sampleRate, std::size_t maxBlockSize) = 0;
    virtual void Process(const AudioBufferView& input,
                         const AudioBufferWriteView& output,
                         std::size_t frameCount)
        = 0;
    virtual void SetControl(const std::string& controlId, float normalizedValue)
        = 0;
    virtual void SetEncoderDelta(int delta)                = 0;
    virtual void SetEncoderPress(bool pressed)             = 0;
    virtual void SetPortInput(const std::string& portId, const PortValue& value)
        = 0;
    virtual PortValue GetPortOutput(const std::string& portId) const = 0;
    virtual void TickUi(double deltaMs)                             = 0;
    virtual bool SetParameterValue(const std::string& parameterId,
                                   float              normalizedValue)
        = 0;
    virtual ParameterValueLookup GetControlValue(
        const std::string& controlId) const = 0;
    virtual ParameterValueLookup GetParameterValue(
        const std::string& parameterId) const = 0;
    virtual ParameterValueLookup GetEffectiveParameterValue(
        const std::string& parameterId) const = 0;
    virtual const std::vector<MetaControllerDescriptor>& GetMetaControllers() const
    {
        static const std::vector<MetaControllerDescriptor> kEmptyMetaControllers;
        return kEmptyMetaControllers;
    }
    virtual bool SetMetaControllerValue(const std::string&, float)
    {
        return false;
    }
    virtual ParameterValueLookup GetMetaControllerValue(
        const std::string&) const
    {
        return {};
    }
    virtual void ResetToDefaultState(std::uint32_t seed = 0) = 0;
    virtual std::unordered_map<std::string, float>
    CaptureStatefulParameterValues() const = 0;
    virtual void RestoreStatefulParameterValues(
        const std::unordered_map<std::string, float>& values)
        = 0;
    virtual const std::vector<ParameterDescriptor>& GetParameters() const
        = 0;
    virtual const MenuModel& GetMenuModel() const = 0;
    virtual void MenuRotate(int delta)            = 0;
    virtual void MenuPress()                      = 0;
    virtual void SetMenuItemValue(const std::string& itemId,
                                  float              normalizedValue)
        = 0;
    virtual const DisplayModel& GetDisplayModel() const             = 0;
};
} // namespace daisyhost
