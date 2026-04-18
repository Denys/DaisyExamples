#pragma once

#include <string>

namespace daisyhost
{
enum class PortDirection
{
    kInput,
    kOutput,
};

enum class VirtualPortType
{
    kAudio,
    kCv,
    kGate,
    kMidi,
};

struct PanelRect
{
    float x      = 0.0f;
    float y      = 0.0f;
    float width  = 0.0f;
    float height = 0.0f;
};

struct VirtualPort
{
    std::string     id;
    std::string     nodeId;
    std::string     label;
    VirtualPortType type         = VirtualPortType::kAudio;
    PortDirection   direction    = PortDirection::kInput;
    int             channelCount = 1;
    PanelRect       panelBounds;
    bool            exposed = true;
};
} // namespace daisyhost
