#pragma once

#include <array>
#include <cstddef>
#include <string>
#include <vector>

#include "daisyhost/HostedAppCore.h"

namespace daisyhost
{
inline constexpr std::size_t kDaisyFieldKnobCount   = 8;
inline constexpr std::size_t kDaisyFieldCvInputCount = 4;
inline constexpr std::size_t kDaisyFieldCvOutputCount = 2;
inline constexpr std::size_t kDaisyFieldSwitchCount = 2;
inline constexpr std::size_t kDaisyFieldKeyCount     = 16;
inline constexpr std::size_t kDaisyFieldLedCount     = 20;

enum class BoardSurfaceTargetKind
{
    kUnavailable,
    kControl,
    kParameter,
    kCvInput,
    kGateInput,
    kMidiNote,
    kMenuItem,
    kLed,
};

enum class DaisyFieldKnobLayoutMode
{
    kPatchPagePlusExtras,
    kControllableParameters,
};

enum class DaisyFieldDrawerPage
{
    kKeyboardMidiCv = 0,
    kPublicParameters,
    kRackAudio,
};

struct BoardSurfaceBinding
{
    std::string            controlId;
    std::string            label;
    std::string            detailLabel;
    BoardSurfaceTargetKind targetKind = BoardSurfaceTargetKind::kUnavailable;
    std::string            targetId;
    int                    midiNote  = -1;
    bool                   available = false;
};

struct DaisyFieldControlMapping
{
    std::array<BoardSurfaceBinding, kDaisyFieldKnobCount>    knobs{};
    std::array<BoardSurfaceBinding, kDaisyFieldCvInputCount> cvInputs{};
    std::array<BoardSurfaceBinding, kDaisyFieldCvOutputCount> cvOutputs{};
    std::array<BoardSurfaceBinding, kDaisyFieldSwitchCount>   switches{};
    BoardSurfaceBinding                                      gateInput{};
    std::array<BoardSurfaceBinding, kDaisyFieldKeyCount>     keys{};
    std::array<BoardSurfaceBinding, kDaisyFieldLedCount>     leds{};
};

std::string MakeDaisyFieldKnobControlId(const std::string& nodeId,
                                         std::size_t       zeroBasedIndex);

std::string MakeDaisyFieldKeyControlId(const std::string& nodeId,
                                        std::size_t       zeroBasedIndex);

std::string MakeDaisyFieldCvOutputControlId(const std::string& nodeId,
                                            std::size_t       zeroBasedIndex);

std::string MakeDaisyFieldCvOutputPortId(const std::string& nodeId,
                                         std::size_t       zeroBasedIndex);

std::string MakeDaisyFieldSwitchControlId(const std::string& nodeId,
                                          std::size_t       zeroBasedIndex);

int DaisyFieldKeyToMidiNote(std::size_t zeroBasedIndex, int keyboardOctave);

std::vector<BoardSurfaceBinding> BuildDaisyFieldPublicParameterList(
    const HostedAppPatchBindings&             patchBindings,
    const std::vector<ParameterDescriptor>&   parameters);

std::vector<BoardSurfaceBinding> BuildDaisyFieldCvTargetOptions(
    const DaisyFieldControlMapping& mapping,
    const std::string&              latchedTargetId);

std::vector<BoardSurfaceBinding> BuildDaisyFieldCvTargetOptions(
    const DaisyFieldControlMapping& defaultMapping,
    const DaisyFieldControlMapping& alternativeMapping,
    const std::string&              latchedTargetId);

bool IsDaisyFieldCvTargetSafe(const BoardSurfaceBinding& binding);

bool IsDaisyFieldCvTargetIdSafe(const std::string& targetId);

bool ShouldForwardDaisyFieldCvInput(const std::string& latchedTargetId);

DaisyFieldDrawerPage StepDaisyFieldDrawerPage(DaisyFieldDrawerPage page,
                                               int                  delta);

DaisyFieldControlMapping BuildDaisyFieldControlMapping(
    const HostedAppPatchBindings&             patchBindings,
    const std::vector<ParameterDescriptor>&   parameters,
    const MenuModel&                          menu,
    int                                       keyboardOctave,
    const std::string&                        nodeId,
    DaisyFieldKnobLayoutMode                  knobLayoutMode
    = DaisyFieldKnobLayoutMode::kPatchPagePlusExtras);

DaisyFieldControlMapping BuildDaisyFieldControlMapping(
    const HostedAppPatchBindings&             patchBindings,
    const std::vector<ParameterDescriptor>&   parameters,
    int                                       keyboardOctave,
    const std::string&                        nodeId,
    DaisyFieldKnobLayoutMode                  knobLayoutMode
    = DaisyFieldKnobLayoutMode::kPatchPagePlusExtras);
} // namespace daisyhost
