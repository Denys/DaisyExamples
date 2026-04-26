#include "daisyhost/BoardProfile.h"

#include <stdexcept>

#include "daisyhost/apps/MultiDelayCore.h"

namespace daisyhost
{
std::vector<std::string> GetSupportedBoardIds()
{
    return {"daisy_patch", "daisy_field"};
}

std::optional<BoardProfile> TryCreateBoardProfile(const std::string& boardId,
                                                  const std::string& nodeId)
{
    if(boardId == "daisy_patch")
    {
        return MakeDaisyPatchProfile(nodeId);
    }
    if(boardId == "daisy_field")
    {
        return MakeDaisyFieldProfile(nodeId);
    }

    return std::nullopt;
}

BoardProfile CreateBoardProfile(const std::string& boardId, const std::string& nodeId)
{
    if(const auto profile = TryCreateBoardProfile(boardId, nodeId))
    {
        return *profile;
    }

    throw std::invalid_argument("Unsupported DaisyHost board id: " + boardId);
}

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

ControlSpec MakeControl(const std::string& id,
                        const std::string& nodeId,
                        const std::string& label,
                        ControlKind        kind,
                        const PanelRect&   bounds,
                        bool               automatable = true)
{
    ControlSpec control;
    control.id          = id;
    control.nodeId      = nodeId;
    control.label       = label;
    control.kind        = kind;
    control.panelBounds = bounds;
    control.automatable = automatable;
    return control;
}

PanelControlSlotSpec MakeSurfaceControl(const std::string& id,
                                        const std::string& nodeId,
                                        const std::string& targetId,
                                        const std::string& label,
                                        const std::string& detailLabel,
                                        ControlKind        kind,
                                        const PanelRect&   bounds,
                                        bool               primarySurface = true)
{
    PanelControlSlotSpec control;
    control.id             = id;
    control.nodeId         = nodeId;
    control.targetId       = targetId;
    control.label          = label;
    control.detailLabel    = detailLabel;
    control.kind           = kind;
    control.panelBounds    = bounds;
    control.primarySurface = primarySurface;
    return control;
}

PanelDecorationSpec MakeDecoration(const std::string& id,
                                   const std::string& nodeId,
                                   const std::string& label,
                                   PanelDecorationKind kind,
                                   const PanelRect&    bounds,
                                   float               cornerRadius,
                                   bool                emphasized = false)
{
    PanelDecorationSpec decoration;
    decoration.id           = id;
    decoration.nodeId       = nodeId;
    decoration.label        = label;
    decoration.kind         = kind;
    decoration.panelBounds  = bounds;
    decoration.cornerRadius = cornerRadius;
    decoration.emphasized   = emphasized;
    return decoration;
}

PanelTextSpec MakeText(const std::string& id,
                       const std::string& nodeId,
                       const std::string& text,
                       const PanelRect&   bounds,
                       float              pointSize,
                       bool               bold,
                       TextAlignment      alignment)
{
    PanelTextSpec spec;
    spec.id         = id;
    spec.nodeId     = nodeId;
    spec.text       = text;
    spec.panelBounds = bounds;
    spec.pointSize  = pointSize;
    spec.bold       = bold;
    spec.alignment  = alignment;
    return spec;
}

PanelIndicatorSpec MakeIndicator(const std::string& id,
                                 const std::string& nodeId,
                                 const std::string& targetId,
                                 const std::string& label,
                                 PanelIndicatorKind kind,
                                 const PanelRect&   bounds)
{
    PanelIndicatorSpec spec;
    spec.id          = id;
    spec.nodeId      = nodeId;
    spec.targetId    = targetId;
    spec.label       = label;
    spec.kind        = kind;
    spec.panelBounds = bounds;
    return spec;
}
} // namespace

BoardProfile MakeDaisyPatchProfile(const std::string& nodeId)
{
    BoardProfile profile;
    profile.boardId     = "daisy_patch";
    profile.nodeId      = nodeId;
    profile.displayName = "Daisy Patch";

    profile.controls.push_back(MakeControl(
        apps::MultiDelayCore::MakeKnobControlId(nodeId, 1),
        nodeId,
        "Ctrl 2",
        ControlKind::kKnob,
        {0.27f, 0.17f, 0.12f, 0.16f}));
    profile.controls.push_back(MakeControl(
        apps::MultiDelayCore::MakeKnobControlId(nodeId, 2),
        nodeId,
        "Ctrl 3",
        ControlKind::kKnob,
        {0.46f, 0.17f, 0.12f, 0.16f}));
    profile.controls.push_back(MakeControl(
        apps::MultiDelayCore::MakeKnobControlId(nodeId, 3),
        nodeId,
        "Delay 3",
        ControlKind::kKnob,
        {0.73f, 0.72f, 0.10f, 0.13f}));
    profile.controls.push_back(MakeControl(
        apps::MultiDelayCore::MakeKnobControlId(nodeId, 4),
        nodeId,
        "Ctrl 4",
        ControlKind::kKnob,
        {0.65f, 0.17f, 0.12f, 0.16f}));
    profile.controls.push_back(MakeControl(
        apps::MultiDelayCore::MakeEncoderControlId(nodeId),
        nodeId,
        "Enc 1",
        ControlKind::kEncoder,
        {0.08f, 0.51f, 0.16f, 0.20f}));
    profile.controls.push_back(MakeControl(
        apps::MultiDelayCore::MakeEncoderButtonControlId(nodeId),
        nodeId,
        "Enc 1 Push",
        ControlKind::kButton,
        {0.09f, 0.41f, 0.14f, 0.05f},
        false));

    profile.display.id          = nodeId + "/display/oled";
    profile.display.nodeId      = nodeId;
    profile.display.panelBounds = {0.34f, 0.39f, 0.30f, 0.17f};
    profile.display.width       = 128;
    profile.display.height      = 64;

    profile.surfaceControls.push_back(MakeSurfaceControl(
        nodeId + "/surface/ctrl1_mix",
        nodeId,
        apps::MultiDelayCore::MakeDryWetControlId(nodeId),
        "CTRL 1",
        "Mix",
        ControlKind::kKnob,
        {0.08f, 0.17f, 0.12f, 0.16f}));
    profile.surfaceControls.push_back(MakeSurfaceControl(
        nodeId + "/surface/ctrl2_primary",
        nodeId,
        apps::MultiDelayCore::MakeKnobControlId(nodeId, 1),
        "CTRL 2",
        "Delay 1",
        ControlKind::kKnob,
        {0.27f, 0.17f, 0.12f, 0.16f}));
    profile.surfaceControls.push_back(MakeSurfaceControl(
        nodeId + "/surface/ctrl3_secondary",
        nodeId,
        apps::MultiDelayCore::MakeKnobControlId(nodeId, 2),
        "CTRL 3",
        "Delay 2",
        ControlKind::kKnob,
        {0.46f, 0.17f, 0.12f, 0.16f}));
    profile.surfaceControls.push_back(MakeSurfaceControl(
        nodeId + "/surface/ctrl4_feedback",
        nodeId,
        apps::MultiDelayCore::MakeKnobControlId(nodeId, 4),
        "CTRL 4",
        "Feedback",
        ControlKind::kKnob,
        {0.65f, 0.17f, 0.12f, 0.16f}));
    profile.surfaceControls.push_back(MakeSurfaceControl(
        nodeId + "/surface/enc1",
        nodeId,
        apps::MultiDelayCore::MakeEncoderControlId(nodeId),
        "ENC 1",
        "Menu",
        ControlKind::kEncoder,
        {0.08f, 0.51f, 0.16f, 0.20f}));
    profile.surfaceControls.push_back(MakeSurfaceControl(
        nodeId + "/surface/enc1_push",
        nodeId,
        apps::MultiDelayCore::MakeEncoderButtonControlId(nodeId),
        "PUSH",
        "Enter",
        ControlKind::kButton,
        {0.09f, 0.41f, 0.14f, 0.05f}));

    profile.decorations.push_back(MakeDecoration(
        nodeId + "/decoration/cv_bay",
        nodeId,
        "CV INPUTS",
        PanelDecorationKind::kCvBay,
        {0.05f, 0.05f, 0.76f, 0.27f},
        0.03f,
        true));
    profile.decorations.push_back(MakeDecoration(
        nodeId + "/decoration/gate_column",
        nodeId,
        "GATE",
        PanelDecorationKind::kGateColumn,
        {0.83f, 0.08f, 0.12f, 0.33f},
        0.02f));
    profile.decorations.push_back(MakeDecoration(
        nodeId + "/decoration/display_frame",
        nodeId,
        "DISPLAY",
        PanelDecorationKind::kDisplayFrame,
        {0.32f, 0.37f, 0.34f, 0.22f},
        0.02f));
    profile.decorations.push_back(MakeDecoration(
        nodeId + "/decoration/audio_section",
        nodeId,
        "AUDIO",
        PanelDecorationKind::kAudioSection,
        {0.05f, 0.72f, 0.55f, 0.20f},
        0.02f));
    profile.decorations.push_back(MakeDecoration(
        nodeId + "/decoration/midi_section",
        nodeId,
        "MIDI",
        PanelDecorationKind::kMidiSection,
        {0.62f, 0.72f, 0.13f, 0.20f},
        0.02f));
    profile.decorations.push_back(MakeDecoration(
        nodeId + "/decoration/seed_module",
        nodeId,
        "SEED",
        PanelDecorationKind::kSeedModule,
        {0.78f, 0.52f, 0.16f, 0.34f},
        0.02f,
        true));

    profile.texts.push_back(MakeText(nodeId + "/text/title",
                                     nodeId,
                                     "DAISY PATCH",
                                     {0.07f, 0.01f, 0.40f, 0.05f},
                                     22.0f,
                                     true,
                                     TextAlignment::kLeft));
    profile.texts.push_back(MakeText(nodeId + "/text/subtitle",
                                     nodeId,
                                     "DAISYHOST",
                                     {0.51f, 0.015f, 0.16f, 0.03f},
                                     12.0f,
                                     true,
                                     TextAlignment::kCenter));
    profile.texts.push_back(MakeText(nodeId + "/text/cv",
                                     nodeId,
                                     "CV INPUTS",
                                     {0.27f, 0.055f, 0.30f, 0.03f},
                                     11.0f,
                                     false,
                                     TextAlignment::kCenter));
    profile.texts.push_back(MakeText(nodeId + "/text/audio",
                                     nodeId,
                                     "AUDIO",
                                     {0.26f, 0.70f, 0.16f, 0.03f},
                                     12.0f,
                                     false,
                                     TextAlignment::kCenter));
    profile.texts.push_back(MakeText(nodeId + "/text/midi",
                                     nodeId,
                                     "MIDI",
                                     {0.685f, 0.70f, 0.08f, 0.03f},
                                     12.0f,
                                     false,
                                     TextAlignment::kCenter));
    profile.texts.push_back(MakeText(nodeId + "/text/electrosmith",
                                     nodeId,
                                     "ELECTROSMITH",
                                     {0.28f, 0.93f, 0.26f, 0.04f},
                                     16.0f,
                                     true,
                                     TextAlignment::kCenter));

    for(std::size_t i = 1; i <= 4; ++i)
    {
        const float x = 0.10f + static_cast<float>(i - 1) * 0.19f;
        profile.ports.push_back(MakePort(
            apps::MultiDelayCore::MakeCvInputPortId(nodeId, i),
            nodeId,
            "CV " + std::to_string(i),
            VirtualPortType::kCv,
            PortDirection::kInput,
            {x, 0.085f, 0.07f, 0.06f}));
    }

    for(std::size_t i = 1; i <= 4; ++i)
    {
        const float x = 0.06f + static_cast<float>(i - 1) * 0.125f;
        profile.ports.push_back(MakePort(
            apps::MultiDelayCore::MakeAudioInputPortId(nodeId, i),
            nodeId,
            "IN " + std::to_string(i),
            VirtualPortType::kAudio,
            PortDirection::kInput,
            {x, 0.77f, 0.07f, 0.06f}));
        profile.ports.push_back(MakePort(
            apps::MultiDelayCore::MakeAudioOutputPortId(nodeId, i),
            nodeId,
            "OUT " + std::to_string(i),
            VirtualPortType::kAudio,
            PortDirection::kOutput,
            {x, 0.86f, 0.07f, 0.06f}));
    }

    profile.ports.push_back(MakePort(
        apps::MultiDelayCore::MakeMidiInputPortId(nodeId, 1),
        nodeId,
        "MIDI IN",
        VirtualPortType::kMidi,
        PortDirection::kInput,
        {0.64f, 0.77f, 0.08f, 0.06f}));
    profile.ports.push_back(MakePort(
        apps::MultiDelayCore::MakeMidiOutputPortId(nodeId, 1),
        nodeId,
        "MIDI OUT",
        VirtualPortType::kMidi,
        PortDirection::kOutput,
        {0.64f, 0.86f, 0.08f, 0.06f}));

    profile.ports.push_back(MakePort(
        apps::MultiDelayCore::MakeGateInputPortId(nodeId, 1),
        nodeId,
        "GATE IN 1",
        VirtualPortType::kGate,
        PortDirection::kInput,
        {0.865f, 0.12f, 0.065f, 0.06f}));
    profile.ports.push_back(MakePort(
        apps::MultiDelayCore::MakeGateInputPortId(nodeId, 2),
        nodeId,
        "GATE IN 2",
        VirtualPortType::kGate,
        PortDirection::kInput,
        {0.865f, 0.22f, 0.065f, 0.06f}));
    profile.ports.push_back(MakePort(
        apps::MultiDelayCore::MakeGateOutputPortId(nodeId, 1),
        nodeId,
        "GATE OUT",
        VirtualPortType::kGate,
        PortDirection::kOutput,
        {0.865f, 0.32f, 0.065f, 0.06f}));

    return profile;
}

BoardProfile MakeDaisyFieldProfile(const std::string& nodeId)
{
    BoardProfile profile;
    profile.boardId     = "daisy_field";
    profile.nodeId      = nodeId;
    profile.displayName = "Daisy Field";

    profile.display.id          = nodeId + "/display/oled";
    profile.display.nodeId      = nodeId;
    profile.display.panelBounds = {0.34f, 0.51f, 0.32f, 0.12f};
    profile.display.width       = 128;
    profile.display.height      = 64;

    profile.decorations.push_back(MakeDecoration(
        nodeId + "/decoration/field_knob_bay",
        nodeId,
        "KNOBS",
        PanelDecorationKind::kCvBay,
        {0.06f, 0.10f, 0.74f, 0.42f},
        0.025f,
        true));
    profile.decorations.push_back(MakeDecoration(
        nodeId + "/decoration/field_display_frame",
        nodeId,
        "DISPLAY",
        PanelDecorationKind::kDisplayFrame,
        {0.32f, 0.49f, 0.36f, 0.16f},
        0.02f));
    profile.decorations.push_back(MakeDecoration(
        nodeId + "/decoration/field_key_grid",
        nodeId,
        "KEYS",
        PanelDecorationKind::kGateColumn,
        {0.07f, 0.64f, 0.76f, 0.18f},
        0.02f));
    profile.decorations.push_back(MakeDecoration(
        nodeId + "/decoration/field_io_row",
        nodeId,
        "CV I/O",
        PanelDecorationKind::kAudioSection,
        {0.48f, 0.84f, 0.46f, 0.11f},
        0.02f));
    profile.decorations.push_back(MakeDecoration(
        nodeId + "/decoration/field_midi_gate",
        nodeId,
        "AUDIO / GATE / MIDI",
        PanelDecorationKind::kMidiSection,
        {0.80f, 0.055f, 0.17f, 0.25f},
        0.02f));
    profile.decorations.push_back(MakeDecoration(
        nodeId + "/decoration/field_seed_module",
        nodeId,
        "SEED",
        PanelDecorationKind::kSeedModule,
        {0.82f, 0.34f, 0.12f, 0.20f},
        0.02f,
        true));

    profile.texts.push_back(MakeText(nodeId + "/text/title",
                                     nodeId,
                                     "DAISY FIELD",
                                     {0.07f, 0.015f, 0.38f, 0.05f},
                                     22.0f,
                                     true,
                                     TextAlignment::kLeft));
    profile.texts.push_back(MakeText(nodeId + "/text/native_controls",
                                     nodeId,
                                     "FIELD NATIVE CONTROLS",
                                     {0.47f, 0.02f, 0.30f, 0.04f},
                                     11.0f,
                                     true,
                                     TextAlignment::kCenter));
    profile.texts.push_back(MakeText(nodeId + "/text/knobs",
                                     nodeId,
                                     "K1-K8",
                                     {0.34f, 0.085f, 0.18f, 0.03f},
                                     11.0f,
                                     false,
                                     TextAlignment::kCenter));
    profile.texts.push_back(MakeText(nodeId + "/text/keys",
                                     nodeId,
                                     "A/B KEYS",
                                     {0.36f, 0.615f, 0.16f, 0.03f},
                                     11.0f,
                                     false,
                                     TextAlignment::kCenter));
    profile.texts.push_back(MakeText(nodeId + "/text/io",
                                     nodeId,
                                     "CV IN / CV OUT",
                                     {0.54f, 0.825f, 0.34f, 0.03f},
                                     11.0f,
                                     false,
                                     TextAlignment::kCenter));

    for(std::size_t row = 0; row < 2; ++row)
    {
        for(std::size_t col = 0; col < 4; ++col)
        {
            const std::size_t index = row * 4 + col + 1;
            const auto        id = nodeId + "/control/field_knob_"
                            + std::to_string(index);
            const PanelRect bounds{0.10f + static_cast<float>(col) * 0.19f,
                                   0.15f + static_cast<float>(row) * 0.20f,
                                   0.105f,
                                   0.14f};
            profile.controls.push_back(MakeControl(id,
                                                   nodeId,
                                                   "K" + std::to_string(index),
                                                   ControlKind::kKnob,
                                                   bounds,
                                                   false));
            profile.surfaceControls.push_back(MakeSurfaceControl(
                nodeId + "/surface/field_knob_" + std::to_string(index),
                nodeId,
                id,
                "K" + std::to_string(index),
                "",
                ControlKind::kKnob,
                bounds,
                false));
        }
    }

    for(std::size_t row = 0; row < 2; ++row)
    {
        const char bank = row == 0 ? 'A' : 'B';
        for(std::size_t col = 0; col < 8; ++col)
        {
            const std::size_t index = col + 1;
            const auto        label = std::string(1, bank) + std::to_string(index);
            const auto        id = nodeId + "/control/field_key_"
                            + static_cast<char>(bank + ('a' - 'A')) + "_"
                            + std::to_string(index);
            const PanelRect bounds{0.105f + static_cast<float>(col) * 0.082f,
                                   0.665f + static_cast<float>(row) * 0.085f,
                                   0.052f,
                                   0.055f};
            profile.controls.push_back(MakeControl(
                id, nodeId, label, ControlKind::kKey, bounds, false));
            profile.surfaceControls.push_back(MakeSurfaceControl(
                nodeId + "/surface/field_key_"
                    + static_cast<char>(bank + ('a' - 'A')) + "_"
                    + std::to_string(index),
                nodeId,
                id,
                label,
                "",
                ControlKind::kKey,
                bounds,
                false));
            profile.indicators.push_back(MakeIndicator(
                nodeId + "/led/field_key_"
                    + static_cast<char>(bank + ('a' - 'A')) + "_"
                    + std::to_string(index),
                nodeId,
                id,
                label,
                PanelIndicatorKind::kLed,
                {bounds.x + 0.014f, bounds.y - 0.025f, 0.024f, 0.018f}));
        }
    }

    for(std::size_t index = 1; index <= 2; ++index)
    {
        const PanelRect bounds{0.845f,
                               0.635f + static_cast<float>(index - 1) * 0.12f,
                               0.075f,
                               0.07f};
        const auto id = nodeId + "/control/field_sw_" + std::to_string(index);
        profile.controls.push_back(MakeControl(id,
                                               nodeId,
                                               "SW" + std::to_string(index),
                                               ControlKind::kSwitch,
                                               bounds,
                                               false));
        profile.surfaceControls.push_back(MakeSurfaceControl(
            nodeId + "/surface/field_sw_" + std::to_string(index),
            nodeId,
            id,
            "SW" + std::to_string(index),
            "",
            ControlKind::kSwitch,
            bounds,
            false));
        profile.indicators.push_back(MakeIndicator(
            nodeId + "/led/field_sw_" + std::to_string(index),
            nodeId,
            nodeId + "/control/field_sw_" + std::to_string(index),
            "SW" + std::to_string(index),
            PanelIndicatorKind::kLed,
            {bounds.x + 0.026f, bounds.y - 0.027f, 0.023f, 0.019f}));
    }

    for(std::size_t i = 1; i <= 2; ++i)
    {
        const float x = 0.815f + static_cast<float>(i - 1) * 0.060f;
        profile.ports.push_back(MakePort(nodeId + "/port/field_audio_in_"
                                             + std::to_string(i),
                                         nodeId,
                                         "IN " + std::to_string(i),
                                         VirtualPortType::kAudio,
                                         PortDirection::kInput,
                                         {x, 0.105f, 0.045f, 0.040f}));
        profile.ports.push_back(MakePort(nodeId + "/port/field_audio_out_"
                                             + std::to_string(i),
                                         nodeId,
                                         "OUT " + std::to_string(i),
                                         VirtualPortType::kAudio,
                                         PortDirection::kOutput,
                                         {x, 0.185f, 0.045f, 0.040f}));
    }

    for(std::size_t i = 1; i <= 4; ++i)
    {
        const float x = 0.49f + static_cast<float>(i - 1) * 0.075f;
        profile.ports.push_back(MakePort(nodeId + "/port/field_cv_in_"
                                             + std::to_string(i),
                                         nodeId,
                                         "CV IN " + std::to_string(i),
                                         VirtualPortType::kCv,
                                         PortDirection::kInput,
                                         {x, 0.88f, 0.05f, 0.045f}));
    }

    for(std::size_t i = 1; i <= 2; ++i)
    {
        const float x = 0.79f + static_cast<float>(i - 1) * 0.075f;
        profile.ports.push_back(MakePort(nodeId + "/port/field_cv_out_"
                                             + std::to_string(i),
                                         nodeId,
                                         "CV OUT " + std::to_string(i),
                                         VirtualPortType::kCv,
                                         PortDirection::kOutput,
                                         {x, 0.88f, 0.05f, 0.045f}));
        profile.indicators.push_back(MakeIndicator(
            nodeId + "/indicator/field_cv_out_" + std::to_string(i),
            nodeId,
            nodeId + "/port/field_cv_out_" + std::to_string(i),
            "CV OUT " + std::to_string(i),
            PanelIndicatorKind::kCvOutput,
            {x, 0.855f, 0.05f, 0.018f}));
    }

    profile.ports.push_back(MakePort(nodeId + "/port/field_gate_in_1",
                                     nodeId,
                                     "GATE IN",
                                     VirtualPortType::kGate,
                                     PortDirection::kInput,
                                     {0.815f, 0.260f, 0.045f, 0.040f}));
    profile.indicators.push_back(MakeIndicator(nodeId + "/led/field_gate_in",
                                               nodeId,
                                               nodeId + "/control/field_gate_in",
                                               "Gate In",
                                               PanelIndicatorKind::kLed,
                                               {0.828f, 0.235f, 0.020f, 0.017f}));
    profile.ports.push_back(MakePort(nodeId + "/port/field_gate_out_1",
                                     nodeId,
                                     "GATE OUT",
                                     VirtualPortType::kGate,
                                     PortDirection::kOutput,
                                     {0.875f, 0.260f, 0.045f, 0.040f}));
    profile.indicators.push_back(MakeIndicator(nodeId + "/led/field_gate_out",
                                               nodeId,
                                               nodeId + "/port/gate_out_1",
                                               "Gate Out",
                                               PanelIndicatorKind::kLed,
                                               {0.888f, 0.235f, 0.020f, 0.017f}));
    profile.ports.push_back(MakePort(nodeId + "/port/field_midi_in_1",
                                     nodeId,
                                     "MIDI IN",
                                     VirtualPortType::kMidi,
                                     PortDirection::kInput,
                                     {0.935f, 0.105f, 0.040f, 0.040f}));
    profile.ports.push_back(MakePort(nodeId + "/port/field_midi_out_1",
                                     nodeId,
                                     "MIDI OUT",
                                     VirtualPortType::kMidi,
                                     PortDirection::kOutput,
                                     {0.935f, 0.185f, 0.040f, 0.040f}));

    return profile;
}
} // namespace daisyhost
