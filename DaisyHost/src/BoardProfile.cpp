#include "daisyhost/BoardProfile.h"

#include "daisyhost/apps/MultiDelayCore.h"

namespace daisyhost
{
namespace
{
VirtualPort MakePort(const std::string& id,
                     const std::string& nodeId,
                     const std::string& label,
                     VirtualPortType    type,
                     PortDirection      direction,
                     const PanelRect&   bounds)
{
    VirtualPort port;
    port.id           = id;
    port.nodeId       = nodeId;
    port.label        = label;
    port.type         = type;
    port.direction    = direction;
    port.channelCount = 1;
    port.panelBounds  = bounds;
    return port;
}
} // namespace

BoardProfile MakeDaisyPatchProfile(const std::string& nodeId)
{
    BoardProfile profile;
    profile.boardId     = "daisy_patch";
    profile.nodeId      = nodeId;
    profile.displayName = "Daisy Patch";

    profile.controls.push_back(
        {apps::MultiDelayCore::MakeKnobControlId(nodeId, 1),
         nodeId,
         "Ctrl 1",
         ControlKind::kKnob,
         {0.08f, 0.08f, 0.14f, 0.18f},
         true});
    profile.controls.push_back(
        {apps::MultiDelayCore::MakeKnobControlId(nodeId, 2),
         nodeId,
         "Ctrl 2",
         ControlKind::kKnob,
         {0.29f, 0.08f, 0.14f, 0.18f},
         true});
    profile.controls.push_back(
        {apps::MultiDelayCore::MakeKnobControlId(nodeId, 3),
         nodeId,
         "Ctrl 3",
         ControlKind::kKnob,
         {0.50f, 0.08f, 0.14f, 0.18f},
         true});
    profile.controls.push_back(
        {apps::MultiDelayCore::MakeKnobControlId(nodeId, 4),
         nodeId,
         "Ctrl 4",
         ControlKind::kKnob,
         {0.71f, 0.08f, 0.14f, 0.18f},
         true});
    profile.controls.push_back(
        {apps::MultiDelayCore::MakeEncoderControlId(nodeId),
         nodeId,
         "Encoder",
         ControlKind::kEncoder,
         {0.08f, 0.52f, 0.16f, 0.18f},
         true});
    profile.controls.push_back(
        {apps::MultiDelayCore::MakeEncoderButtonControlId(nodeId),
         nodeId,
         "Encoder Btn",
         ControlKind::kButton,
         {0.10f, 0.72f, 0.12f, 0.06f},
         false});

    profile.display.id          = nodeId + "/display/oled";
    profile.display.nodeId      = nodeId;
    profile.display.panelBounds = {0.34f, 0.34f, 0.28f, 0.16f};
    profile.display.width       = 128;
    profile.display.height      = 64;

    for(std::size_t i = 1; i <= 4; ++i)
    {
        const float x = 0.11f + static_cast<float>(i - 1) * 0.21f;
        profile.ports.push_back(MakePort(
            apps::MultiDelayCore::MakeCvInputPortId(nodeId, i),
            nodeId,
            "CV " + std::to_string(i),
            VirtualPortType::kCv,
            PortDirection::kInput,
            {x, 0.27f, 0.06f, 0.06f}));
    }

    for(std::size_t i = 1; i <= 4; ++i)
    {
        const float x = 0.07f + static_cast<float>(i - 1) * 0.19f;
        profile.ports.push_back(MakePort(
            apps::MultiDelayCore::MakeAudioInputPortId(nodeId, i),
            nodeId,
            "Audio In " + std::to_string(i),
            VirtualPortType::kAudio,
            PortDirection::kInput,
            {x, 0.79f, 0.08f, 0.07f}));
        profile.ports.push_back(MakePort(
            apps::MultiDelayCore::MakeAudioOutputPortId(nodeId, i),
            nodeId,
            "Audio Out " + std::to_string(i),
            VirtualPortType::kAudio,
            PortDirection::kOutput,
            {x, 0.89f, 0.08f, 0.07f}));
    }

    profile.ports.push_back(MakePort(
        apps::MultiDelayCore::MakeGateInputPortId(nodeId, 1),
        nodeId,
        "Gate In 1",
        VirtualPortType::kGate,
        PortDirection::kInput,
        {0.89f, 0.11f, 0.07f, 0.07f}));
    profile.ports.push_back(MakePort(
        apps::MultiDelayCore::MakeGateInputPortId(nodeId, 2),
        nodeId,
        "Gate In 2",
        VirtualPortType::kGate,
        PortDirection::kInput,
        {0.89f, 0.21f, 0.07f, 0.07f}));
    profile.ports.push_back(MakePort(
        apps::MultiDelayCore::MakeGateOutputPortId(nodeId, 1),
        nodeId,
        "Gate Out",
        VirtualPortType::kGate,
        PortDirection::kOutput,
        {0.89f, 0.31f, 0.07f, 0.07f}));
    profile.ports.push_back(MakePort(
        apps::MultiDelayCore::MakeMidiInputPortId(nodeId, 1),
        nodeId,
        "MIDI In",
        VirtualPortType::kMidi,
        PortDirection::kInput,
        {0.87f, 0.79f, 0.10f, 0.07f}));
    profile.ports.push_back(MakePort(
        apps::MultiDelayCore::MakeMidiOutputPortId(nodeId, 1),
        nodeId,
        "MIDI Out",
        VirtualPortType::kMidi,
        PortDirection::kOutput,
        {0.87f, 0.89f, 0.10f, 0.07f}));

    return profile;
}
} // namespace daisyhost
