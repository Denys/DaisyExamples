#pragma once

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

struct BoardProfile
{
    std::string              boardId;
    std::string              nodeId;
    std::string              displayName;
    std::vector<ControlSpec> controls;
    std::vector<VirtualPort> ports;
    DisplaySpec              display;
};

BoardProfile MakeDaisyPatchProfile(const std::string& nodeId = "node0");
} // namespace daisyhost
