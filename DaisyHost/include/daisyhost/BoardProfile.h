#pragma once

#include <optional>
#include <string>
#include <vector>

#include "daisyhost/VirtualPort.h"

namespace daisyhost
{
enum class ControlKind
{
    kKnob,
    kEncoder,
    kButton,
    kKey,
    kSwitch,
};

enum class TextAlignment
{
    kLeft,
    kCenter,
    kRight,
};

enum class PanelDecorationKind
{
    kCvBay,
    kGateColumn,
    kAudioSection,
    kMidiSection,
    kSeedModule,
    kDisplayFrame,
};

enum class PanelIndicatorKind
{
    kLed,
    kCvOutput,
    kGateOutput,
};

struct ControlSpec
{
    std::string id;
    std::string nodeId;
    std::string label;
    ControlKind kind = ControlKind::kKnob;
    PanelRect   panelBounds;
    bool        automatable = true;
};

struct DisplaySpec
{
    std::string id;
    std::string nodeId;
    PanelRect   panelBounds;
    int         width  = 128;
    int         height = 64;
};

struct PanelControlSlotSpec
{
    std::string id;
    std::string nodeId;
    std::string targetId;
    std::string label;
    std::string detailLabel;
    ControlKind kind = ControlKind::kKnob;
    PanelRect   panelBounds;
    bool        primarySurface = true;
};

struct PanelDecorationSpec
{
    std::string         id;
    std::string         nodeId;
    std::string         label;
    PanelDecorationKind kind = PanelDecorationKind::kCvBay;
    PanelRect           panelBounds;
    float               cornerRadius = 0.0f;
    bool                emphasized   = false;
};

struct PanelTextSpec
{
    std::string   id;
    std::string   nodeId;
    std::string   text;
    PanelRect     panelBounds;
    float         pointSize = 12.0f;
    bool          bold      = false;
    TextAlignment alignment = TextAlignment::kLeft;
};

struct PanelIndicatorSpec
{
    std::string        id;
    std::string        nodeId;
    std::string        targetId;
    std::string        label;
    PanelIndicatorKind kind = PanelIndicatorKind::kLed;
    PanelRect          panelBounds;
};

struct BoardProfile
{
    std::string              boardId;
    std::string              nodeId;
    std::string              displayName;
    std::vector<ControlSpec> controls;
    std::vector<VirtualPort> ports;
    DisplaySpec              display;
    std::vector<PanelControlSlotSpec> surfaceControls;
    std::vector<PanelDecorationSpec>  decorations;
    std::vector<PanelTextSpec>        texts;
    std::vector<PanelIndicatorSpec>   indicators;
};

std::vector<std::string> GetSupportedBoardIds();
std::optional<BoardProfile> TryCreateBoardProfile(const std::string& boardId,
                                                  const std::string& nodeId = "node0");
BoardProfile CreateBoardProfile(const std::string& boardId,
                                const std::string& nodeId = "node0");
BoardProfile MakeDaisyPatchProfile(const std::string& nodeId = "node0");
BoardProfile MakeDaisyFieldProfile(const std::string& nodeId = "node0");
} // namespace daisyhost
