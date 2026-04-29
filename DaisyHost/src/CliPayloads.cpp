#include "daisyhost/CliPayloads.h"

#include <algorithm>
#include <array>
#include <memory>

#include <juce_core/juce_core.h>

#include "daisyhost/AppRegistry.h"
#include "daisyhost/BoardControlMapping.h"
#include "daisyhost/BoardProfile.h"
#include "daisyhost/HostAutomationBridge.h"
#include "daisyhost/HostModulation.h"
#include "daisyhost/RenderAssertions.h"
#include "daisyhost/TestInputSignal.h"

namespace daisyhost
{
namespace cli
{
namespace
{
juce::var StringVar(const std::string& text)
{
    return juce::var(juce::String(text));
}

std::string ToJson(const juce::var& value)
{
    return juce::JSON::toString(value, true).toStdString();
}

bool IsKnownAppId(const std::string& appId)
{
    const auto& registrations = GetHostedAppRegistrations();
    return std::any_of(registrations.begin(),
                       registrations.end(),
                       [&appId](const HostedAppRegistration& registration) {
                           return registration.appId == appId;
                       });
}

const HostedAppRegistration* FindAppRegistration(const std::string& appId)
{
    const auto& registrations = GetHostedAppRegistrations();
    const auto  it = std::find_if(registrations.begin(),
                                 registrations.end(),
                                 [&appId](const HostedAppRegistration& registration) {
                                     return registration.appId == appId;
                                 });
    return it != registrations.end() ? &(*it) : nullptr;
}

std::string ParameterRoleName(ParameterRole role)
{
    switch(role)
    {
        case ParameterRole::kGeneric: return "generic";
        case ParameterRole::kMix: return "mix";
        case ParameterRole::kPrimaryDelay: return "primary_delay";
        case ParameterRole::kSecondaryDelay: return "secondary_delay";
        case ParameterRole::kFeedback: return "feedback";
        case ParameterRole::kTertiaryDelay: return "tertiary_delay";
    }
    return "generic";
}

std::string MenuActionKindName(MenuItemActionKind kind)
{
    switch(kind)
    {
        case MenuItemActionKind::kValue: return "value";
        case MenuItemActionKind::kMomentary: return "momentary";
        case MenuItemActionKind::kReadonly: return "readonly";
        case MenuItemActionKind::kBack: return "back";
        case MenuItemActionKind::kEnterSection: return "enter_section";
    }
    return "readonly";
}

std::string VirtualPortTypeName(VirtualPortType type)
{
    switch(type)
    {
        case VirtualPortType::kAudio: return "audio";
        case VirtualPortType::kCv: return "cv";
        case VirtualPortType::kGate: return "gate";
        case VirtualPortType::kMidi: return "midi";
    }
    return "audio";
}

std::string PortDirectionName(PortDirection direction)
{
    return direction == PortDirection::kInput ? "input" : "output";
}

std::string ControlKindName(ControlKind kind)
{
    switch(kind)
    {
        case ControlKind::kKnob: return "knob";
        case ControlKind::kEncoder: return "encoder";
        case ControlKind::kButton: return "button";
        case ControlKind::kKey: return "key";
        case ControlKind::kSwitch: return "switch";
    }
    return "knob";
}

std::string DecorationKindName(PanelDecorationKind kind)
{
    switch(kind)
    {
        case PanelDecorationKind::kCvBay: return "cv_bay";
        case PanelDecorationKind::kGateColumn: return "gate_column";
        case PanelDecorationKind::kAudioSection: return "audio_section";
        case PanelDecorationKind::kMidiSection: return "midi_section";
        case PanelDecorationKind::kSeedModule: return "seed_module";
        case PanelDecorationKind::kDisplayFrame: return "display_frame";
    }
    return "cv_bay";
}

std::string IndicatorKindName(PanelIndicatorKind kind)
{
    switch(kind)
    {
        case PanelIndicatorKind::kLed: return "led";
        case PanelIndicatorKind::kCvOutput: return "cv_output";
        case PanelIndicatorKind::kGateOutput: return "gate_output";
    }
    return "led";
}

std::string TextAlignmentName(TextAlignment alignment)
{
    switch(alignment)
    {
        case TextAlignment::kLeft: return "left";
        case TextAlignment::kCenter: return "center";
        case TextAlignment::kRight: return "right";
    }
    return "left";
}

std::string TestInputModeId(int mode)
{
    switch(static_cast<TestInputSignalMode>(ClampTestInputSignalMode(mode)))
    {
        case TestInputSignalMode::kHostInput: return "host_in";
        case TestInputSignalMode::kSineInput: return "sine";
        case TestInputSignalMode::kSawInput: return "saw";
        case TestInputSignalMode::kNoiseInput: return "noise";
        case TestInputSignalMode::kImpulseInput: return "impulse";
        case TestInputSignalMode::kTriangleInput: return "triangle";
        case TestInputSignalMode::kSquareInput: return "square";
    }
    return "host_in";
}

juce::var RectVar(const PanelRect& rect)
{
    auto object = std::make_unique<juce::DynamicObject>();
    object->setProperty("x", rect.x);
    object->setProperty("y", rect.y);
    object->setProperty("width", rect.width);
    object->setProperty("height", rect.height);
    return juce::var(object.release());
}

juce::Array<juce::var> StringArrayVar(const std::array<std::string, 2>& values)
{
    juce::Array<juce::var> array;
    for(const auto& value : values)
    {
        array.add(StringVar(value));
    }
    return array;
}

template <std::size_t Size>
juce::Array<juce::var> StringArrayVar(const std::array<std::string, Size>& values)
{
    juce::Array<juce::var> array;
    for(const auto& value : values)
    {
        array.add(StringVar(value));
    }
    return array;
}

juce::var CapabilitiesVar(const HostedAppCapabilities& capabilities)
{
    auto object = std::make_unique<juce::DynamicObject>();
    object->setProperty("acceptsAudioInput", capabilities.acceptsAudioInput);
    object->setProperty("acceptsMidiInput", capabilities.acceptsMidiInput);
    object->setProperty("producesMidiOutput", capabilities.producesMidiOutput);
    return juce::var(object.release());
}

juce::var PatchBindingsVar(const HostedAppPatchBindings& bindings)
{
    auto object = std::make_unique<juce::DynamicObject>();
    object->setProperty("knobControlIds",
                        juce::var(StringArrayVar(bindings.knobControlIds)));
    object->setProperty("knobParameterIds",
                        juce::var(StringArrayVar(bindings.knobParameterIds)));
    object->setProperty("fieldKnobControlIds",
                        juce::var(StringArrayVar(bindings.fieldKnobControlIds)));
    object->setProperty("fieldKnobParameterIds",
                        juce::var(StringArrayVar(bindings.fieldKnobParameterIds)));
    object->setProperty("fieldKeyMenuItemIds",
                        juce::var(StringArrayVar(bindings.fieldKeyMenuItemIds)));
    object->setProperty("fieldKeyDetailLabels",
                        juce::var(StringArrayVar(bindings.fieldKeyDetailLabels)));
    object->setProperty("encoderControlId", StringVar(bindings.encoderControlId));
    object->setProperty("encoderButtonControlId",
                        StringVar(bindings.encoderButtonControlId));
    object->setProperty("cvInputPortIds",
                        juce::var(StringArrayVar(bindings.cvInputPortIds)));
    object->setProperty("gateInputPortIds",
                        juce::var(StringArrayVar(bindings.gateInputPortIds)));
    object->setProperty("gateOutputPortId", StringVar(bindings.gateOutputPortId));
    object->setProperty("audioInputPortIds",
                        juce::var(StringArrayVar(bindings.audioInputPortIds)));
    object->setProperty("audioOutputPortIds",
                        juce::var(StringArrayVar(bindings.audioOutputPortIds)));
    object->setProperty("midiInputPortId", StringVar(bindings.midiInputPortId));
    object->setProperty("midiOutputPortId", StringVar(bindings.midiOutputPortId));
    return juce::var(object.release());
}

juce::var ParameterVar(const ParameterDescriptor& parameter)
{
    auto object = std::make_unique<juce::DynamicObject>();
    object->setProperty("id", StringVar(parameter.id));
    object->setProperty("label", StringVar(parameter.label));
    object->setProperty("normalizedValue", parameter.normalizedValue);
    object->setProperty("defaultNormalizedValue",
                        parameter.defaultNormalizedValue);
    object->setProperty("effectiveNormalizedValue",
                        parameter.effectiveNormalizedValue);
    object->setProperty("unitLabel", StringVar(parameter.unitLabel));
    object->setProperty("stepCount", parameter.stepCount);
    object->setProperty("role", StringVar(ParameterRoleName(parameter.role)));
    object->setProperty("importanceRank", parameter.importanceRank);
    object->setProperty("automatable", parameter.automatable);
    object->setProperty("stateful", parameter.stateful);
    object->setProperty("menuEditable", parameter.menuEditable);
    object->setProperty("nativeMinimum", parameter.nativeMinimum);
    object->setProperty("nativeMaximum", parameter.nativeMaximum);
    object->setProperty("nativeDefault", parameter.nativeDefault);
    object->setProperty("nativePrecision", parameter.nativePrecision);
    return juce::var(object.release());
}

juce::var MetaControllerVar(const MetaControllerDescriptor& controller)
{
    auto object = std::make_unique<juce::DynamicObject>();
    object->setProperty("id", StringVar(controller.id));
    object->setProperty("label", StringVar(controller.label));
    object->setProperty("normalizedValue", controller.normalizedValue);
    object->setProperty("defaultNormalizedValue",
                        controller.defaultNormalizedValue);
    object->setProperty("stateful", controller.stateful);
    return juce::var(object.release());
}

juce::var MenuItemVar(const MenuItem& item)
{
    auto object = std::make_unique<juce::DynamicObject>();
    object->setProperty("id", StringVar(item.id));
    object->setProperty("label", StringVar(item.label));
    object->setProperty("editable", item.editable);
    object->setProperty("actionKind", StringVar(MenuActionKindName(item.actionKind)));
    object->setProperty("normalizedValue", item.normalizedValue);
    object->setProperty("valueText", StringVar(item.valueText));
    object->setProperty("targetSectionId", StringVar(item.targetSectionId));
    return juce::var(object.release());
}

juce::var MenuSectionVar(const MenuSection& section)
{
    auto object = std::make_unique<juce::DynamicObject>();
    object->setProperty("id", StringVar(section.id));
    object->setProperty("title", StringVar(section.title));
    object->setProperty("selectedIndex", section.selectedIndex);

    juce::Array<juce::var> items;
    for(const auto& item : section.items)
    {
        items.add(MenuItemVar(item));
    }
    object->setProperty("items", juce::var(items));
    return juce::var(object.release());
}

juce::var VirtualPortVar(const VirtualPort& port)
{
    auto object = std::make_unique<juce::DynamicObject>();
    object->setProperty("id", StringVar(port.id));
    object->setProperty("nodeId", StringVar(port.nodeId));
    object->setProperty("label", StringVar(port.label));
    object->setProperty("type", StringVar(VirtualPortTypeName(port.type)));
    object->setProperty("direction", StringVar(PortDirectionName(port.direction)));
    object->setProperty("channelCount", port.channelCount);
    object->setProperty("panelBounds", RectVar(port.panelBounds));
    object->setProperty("exposed", port.exposed);
    return juce::var(object.release());
}

juce::var ControlSpecVar(const ControlSpec& control)
{
    auto object = std::make_unique<juce::DynamicObject>();
    object->setProperty("id", StringVar(control.id));
    object->setProperty("nodeId", StringVar(control.nodeId));
    object->setProperty("label", StringVar(control.label));
    object->setProperty("kind", StringVar(ControlKindName(control.kind)));
    object->setProperty("panelBounds", RectVar(control.panelBounds));
    object->setProperty("automatable", control.automatable);
    return juce::var(object.release());
}

juce::var SurfaceControlVar(const PanelControlSlotSpec& control)
{
    auto object = std::make_unique<juce::DynamicObject>();
    object->setProperty("id", StringVar(control.id));
    object->setProperty("nodeId", StringVar(control.nodeId));
    object->setProperty("targetId", StringVar(control.targetId));
    object->setProperty("label", StringVar(control.label));
    object->setProperty("detailLabel", StringVar(control.detailLabel));
    object->setProperty("kind", StringVar(ControlKindName(control.kind)));
    object->setProperty("panelBounds", RectVar(control.panelBounds));
    object->setProperty("primarySurface", control.primarySurface);
    return juce::var(object.release());
}

juce::var DisplayVar(const DisplaySpec& display)
{
    auto object = std::make_unique<juce::DynamicObject>();
    object->setProperty("id", StringVar(display.id));
    object->setProperty("nodeId", StringVar(display.nodeId));
    object->setProperty("panelBounds", RectVar(display.panelBounds));
    object->setProperty("width", display.width);
    object->setProperty("height", display.height);
    return juce::var(object.release());
}

juce::var DecorationVar(const PanelDecorationSpec& decoration)
{
    auto object = std::make_unique<juce::DynamicObject>();
    object->setProperty("id", StringVar(decoration.id));
    object->setProperty("nodeId", StringVar(decoration.nodeId));
    object->setProperty("label", StringVar(decoration.label));
    object->setProperty("kind", StringVar(DecorationKindName(decoration.kind)));
    object->setProperty("panelBounds", RectVar(decoration.panelBounds));
    object->setProperty("cornerRadius", decoration.cornerRadius);
    object->setProperty("emphasized", decoration.emphasized);
    return juce::var(object.release());
}

juce::var TextVar(const PanelTextSpec& text)
{
    auto object = std::make_unique<juce::DynamicObject>();
    object->setProperty("id", StringVar(text.id));
    object->setProperty("nodeId", StringVar(text.nodeId));
    object->setProperty("text", StringVar(text.text));
    object->setProperty("panelBounds", RectVar(text.panelBounds));
    object->setProperty("pointSize", text.pointSize);
    object->setProperty("bold", text.bold);
    object->setProperty("alignment", StringVar(TextAlignmentName(text.alignment)));
    return juce::var(object.release());
}

juce::var IndicatorVar(const PanelIndicatorSpec& indicator)
{
    auto object = std::make_unique<juce::DynamicObject>();
    object->setProperty("id", StringVar(indicator.id));
    object->setProperty("nodeId", StringVar(indicator.nodeId));
    object->setProperty("targetId", StringVar(indicator.targetId));
    object->setProperty("label", StringVar(indicator.label));
    object->setProperty("kind", StringVar(IndicatorKindName(indicator.kind)));
    object->setProperty("panelBounds", RectVar(indicator.panelBounds));
    return juce::var(object.release());
}

template <typename Item, typename Mapper>
juce::Array<juce::var> VectorVar(const std::vector<Item>& values, Mapper mapper)
{
    juce::Array<juce::var> array;
    for(const auto& value : values)
    {
        array.add(mapper(value));
    }
    return array;
}

juce::var DebugNodeVar(const std::string& nodeId,
                       const std::string& appId,
                       const std::string& appDisplayName,
                       bool               selected,
                       bool               entryNode,
                       bool               outputNode);
juce::var DebugControlTargetsVar(const std::string& selectedNodeId);

juce::var ChannelSummaryVar(const RenderChannelSummary& summary)
{
    auto object = std::make_unique<juce::DynamicObject>();
    object->setProperty("peak", summary.peak);
    object->setProperty("rms", summary.rms);
    return juce::var(object.release());
}

juce::var RenderFloatMapVar(const std::map<std::string, float>& values)
{
    auto object = std::make_unique<juce::DynamicObject>();
    for(const auto& entry : values)
    {
        object->setProperty(juce::Identifier(entry.first), entry.second);
    }
    return juce::var(object.release());
}

juce::var RenderBoolMapVar(const std::map<std::string, bool>& values)
{
    auto object = std::make_unique<juce::DynamicObject>();
    for(const auto& entry : values)
    {
        object->setProperty(juce::Identifier(entry.first), entry.second);
    }
    return juce::var(object.release());
}

juce::var RenderNodeSummaryVar(const RenderNodeResultSummary& node)
{
    auto object = std::make_unique<juce::DynamicObject>();
    object->setProperty("nodeId", StringVar(node.nodeId));
    object->setProperty("appId", StringVar(node.appId));
    object->setProperty("appDisplayName", StringVar(node.appDisplayName));
    object->setProperty("seed", static_cast<juce::int64>(node.seed));
    object->setProperty("initialParameterValues",
                        RenderFloatMapVar(node.initialParameterValues));
    object->setProperty("finalParameterValues",
                        RenderFloatMapVar(node.finalParameterValues));
    object->setProperty("finalEffectiveParameterValues",
                        RenderFloatMapVar(node.finalEffectiveParameterValues));
    object->setProperty("finalCvInputs", RenderFloatMapVar(node.finalCvInputs));
    object->setProperty("finalGateInputs", RenderBoolMapVar(node.finalGateInputs));
    return juce::var(object.release());
}

juce::var RenderRouteVar(const RenderRoute& route)
{
    auto object = std::make_unique<juce::DynamicObject>();
    object->setProperty("sourcePortId", StringVar(route.sourcePortId));
    object->setProperty("destPortId", StringVar(route.destPortId));
    return juce::var(object.release());
}

juce::var RenderDebugStateVar(const RenderResultManifest& manifest)
{
    auto root = std::make_unique<juce::DynamicObject>();
    root->setProperty("boardId", StringVar(manifest.boardId));
    root->setProperty("selectedNodeId", StringVar(manifest.selectedNodeId));
    root->setProperty("entryNodeId", StringVar(manifest.entryNodeId));
    root->setProperty("outputNodeId", StringVar(manifest.outputNodeId));

    juce::Array<juce::var> nodes;
    for(const auto& node : manifest.nodes)
    {
        const bool selected   = node.nodeId == manifest.selectedNodeId;
        const bool entryNode  = node.nodeId == manifest.entryNodeId;
        const bool outputNode = node.nodeId == manifest.outputNodeId;
        nodes.add(DebugNodeVar(node.nodeId,
                               node.appId,
                               node.appDisplayName,
                               selected,
                               entryNode,
                               outputNode));
    }
    root->setProperty("nodes", juce::var(nodes));
    root->setProperty("routes",
                      juce::var(VectorVar(manifest.routes, RenderRouteVar)));
    root->setProperty("controlTargets",
                      DebugControlTargetsVar(manifest.selectedNodeId));

    int resolvedTargetEventCount = 0;
    for(const auto& event : manifest.executedTimeline)
    {
        if(!event.targetNodeId.empty())
        {
            ++resolvedTargetEventCount;
        }
    }

    auto timeline = std::make_unique<juce::DynamicObject>();
    timeline->setProperty(
        "executedEventCount",
        static_cast<int>(manifest.executedTimeline.size()));
    timeline->setProperty("resolvedTargetEventCount", resolvedTargetEventCount);
    root->setProperty("timeline", juce::var(timeline.release()));
    return juce::var(root.release());
}

juce::var RenderTimelineEventVar(const RenderTimelineEvent& event)
{
    auto object = std::make_unique<juce::DynamicObject>();
    object->setProperty("timeSeconds", event.timeSeconds);
    object->setProperty("type",
                        StringVar(GetRenderTimelineEventTypeName(event.type)));
    if(!event.targetNodeId.empty())
    {
        object->setProperty("targetNodeId", StringVar(event.targetNodeId));
    }

    switch(event.type)
    {
        case RenderTimelineEventType::kParameterSet:
            object->setProperty("parameterId", StringVar(event.parameterId));
            object->setProperty("normalizedValue", event.normalizedValue);
            break;
        case RenderTimelineEventType::kCvSet:
            object->setProperty("portId", StringVar(event.portId));
            object->setProperty("normalizedValue", event.normalizedValue);
            break;
        case RenderTimelineEventType::kGateSet:
            object->setProperty("portId", StringVar(event.portId));
            object->setProperty("gate", event.gateValue);
            break;
        case RenderTimelineEventType::kMidi:
            object->setProperty("status",
                                static_cast<int>(event.midiMessage.status));
            object->setProperty("data1",
                                static_cast<int>(event.midiMessage.data1));
            object->setProperty("data2",
                                static_cast<int>(event.midiMessage.data2));
            break;
        case RenderTimelineEventType::kAudioInputConfig:
            if(event.hasAudioMode)
            {
                object->setProperty(
                    "mode",
                    StringVar(GetRenderAudioInputModeName(event.audioMode)));
            }
            if(event.hasAudioLevel)
            {
                object->setProperty("level", event.audioLevel);
            }
            if(event.hasAudioFrequency)
            {
                object->setProperty("frequencyHz", event.audioFrequencyHz);
            }
            break;
        case RenderTimelineEventType::kImpulse: break;
        case RenderTimelineEventType::kMenuRotate:
            object->setProperty("delta", event.menuDelta);
            break;
        case RenderTimelineEventType::kMenuPress: break;
        case RenderTimelineEventType::kMenuSetItem:
            object->setProperty("itemId", StringVar(event.menuItemId));
            object->setProperty("normalizedValue", event.normalizedValue);
            break;
        case RenderTimelineEventType::kSurfaceControlSet:
            object->setProperty("controlId", StringVar(event.controlId));
            object->setProperty("normalizedValue", event.normalizedValue);
            break;
    }

    return juce::var(object.release());
}

juce::var RenderAssertionResultVar(const RenderAssertionResult& result)
{
    auto object = std::make_unique<juce::DynamicObject>();
    object->setProperty("id", StringVar(result.id));
    object->setProperty("expected", StringVar(result.expected));
    object->setProperty("actual", StringVar(result.actual));
    object->setProperty("passed", result.passed);
    object->setProperty("message", StringVar(result.message));
    return juce::var(object.release());
}

juce::var RenderAssertionReportVar(const RenderAssertionReport& report)
{
    auto object = std::make_unique<juce::DynamicObject>();
    object->setProperty("passed", report.passed);
    object->setProperty("results",
                        juce::var(VectorVar(report.results,
                                            RenderAssertionResultVar)));
    return juce::var(object.release());
}

juce::var AutomationSlotVar(const EffectiveHostAutomationSlotSnapshot& slot)
{
    auto object = std::make_unique<juce::DynamicObject>();
    object->setProperty("slotId", StringVar(slot.slotId));
    object->setProperty("slotName", StringVar(slot.slotName));
    object->setProperty("available", slot.available);
    object->setProperty("parameterId", StringVar(slot.parameterId));
    object->setProperty("parameterLabel", StringVar(slot.parameterLabel));
    object->setProperty("unitLabel", StringVar(slot.unitLabel));
    object->setProperty("normalizedValue", slot.normalizedValue);
    object->setProperty("effectiveNormalizedValue",
                        slot.effectiveNormalizedValue);
    return juce::var(object.release());
}

juce::var CvInputVar(const EffectiveHostCvInputSnapshot& input)
{
    auto object = std::make_unique<juce::DynamicObject>();
    object->setProperty("portId", StringVar(input.portId));
    object->setProperty("normalizedValue", input.normalizedValue);
    object->setProperty("volts", input.volts);
    object->setProperty("sourceMode", input.sourceMode);
    object->setProperty("waveform", input.waveform);
    object->setProperty("frequencyHz", input.frequencyHz);
    object->setProperty("amplitudeVolts", input.amplitudeVolts);
    object->setProperty("biasVolts", input.biasVolts);
    object->setProperty("manualVolts", input.manualVolts);
    return juce::var(object.release());
}

juce::var GateInputVar(const EffectiveHostGateInputSnapshot& input)
{
    auto object = std::make_unique<juce::DynamicObject>();
    object->setProperty("portId", StringVar(input.portId));
    object->setProperty("value", input.value);
    return juce::var(object.release());
}

juce::var AudioInputVar(const EffectiveHostAudioInputSnapshot& input)
{
    auto object = std::make_unique<juce::DynamicObject>();
    object->setProperty("mode", input.mode);
    object->setProperty("modeName", StringVar(input.modeName));
    object->setProperty("level", input.level);
    object->setProperty("frequencyHz", input.frequencyHz);
    return juce::var(object.release());
}

juce::var NodeSummaryVar(const EffectiveHostNodeSummary& node)
{
    auto object = std::make_unique<juce::DynamicObject>();
    object->setProperty("nodeId", StringVar(node.nodeId));
    object->setProperty("appId", StringVar(node.appId));
    object->setProperty("appDisplayName", StringVar(node.appDisplayName));
    object->setProperty("selected", node.selected);
    object->setProperty("entryNode", node.entryNode);
    object->setProperty("outputNode", node.outputNode);
    return juce::var(object.release());
}

juce::var RouteVar(const EffectiveHostRouteSnapshot& route)
{
    auto object = std::make_unique<juce::DynamicObject>();
    object->setProperty("sourcePortId", StringVar(route.sourcePortId));
    object->setProperty("destPortId", StringVar(route.destPortId));
    return juce::var(object.release());
}

std::string DebugRoleLabel(bool selected, bool entryNode, bool outputNode)
{
    std::string role;
    if(entryNode && outputNode)
    {
        role = "Entry + output";
    }
    else if(entryNode)
    {
        role = "Audio entry";
    }
    else if(outputNode)
    {
        role = "Audio output";
    }
    else
    {
        role = "Not in route";
    }

    return selected ? "Selected - " + role : role;
}

juce::var DebugNodeVar(const std::string& nodeId,
                       const std::string& appId,
                       const std::string& appDisplayName,
                       bool               selected,
                       bool               entryNode,
                       bool               outputNode)
{
    auto object = std::make_unique<juce::DynamicObject>();
    object->setProperty("nodeId", StringVar(nodeId));
    object->setProperty("appId", StringVar(appId));
    object->setProperty("appDisplayName", StringVar(appDisplayName));
    object->setProperty("selected", selected);
    object->setProperty("entryNode", entryNode);
    object->setProperty("outputNode", outputNode);
    object->setProperty("roleLabel",
                        StringVar(DebugRoleLabel(selected, entryNode, outputNode)));
    return juce::var(object.release());
}

juce::var DebugControlTargetsVar(const std::string& selectedNodeId)
{
    auto object = std::make_unique<juce::DynamicObject>();
    object->setProperty("liveControlsTargetNodeId", StringVar(selectedNodeId));
    object->setProperty("cvGateTargetNodeId", StringVar(selectedNodeId));
    object->setProperty("modulationTargetNodeId", StringVar(selectedNodeId));
    object->setProperty("fieldSurfaceTargetNodeId", StringVar(selectedNodeId));
    object->setProperty("liveControlsCue",
                        StringVar("Live controls target selected node "
                                  + selectedNodeId));
    object->setProperty("cvGateCue",
                        StringVar("CV and gate controls target selected node "
                                  + selectedNodeId));
    object->setProperty("modulationCue",
                        StringVar("Modulation controls target selected node "
                                  + selectedNodeId));
    object->setProperty("fieldSurfaceCue",
                        StringVar("Field surface controls target selected node "
                                  + selectedNodeId));
    return juce::var(object.release());
}

juce::var SnapshotDebugStateVar(const EffectiveHostStateSnapshot& snapshot)
{
    auto root = std::make_unique<juce::DynamicObject>();
    root->setProperty("boardId", StringVar(snapshot.boardId));
    root->setProperty("selectedNodeId", StringVar(snapshot.selectedNodeId));
    root->setProperty("entryNodeId", StringVar(snapshot.entryNodeId));
    root->setProperty("outputNodeId", StringVar(snapshot.outputNodeId));

    juce::Array<juce::var> nodes;
    for(const auto& node : snapshot.nodeSummaries)
    {
        nodes.add(DebugNodeVar(node.nodeId,
                               node.appId,
                               node.appDisplayName,
                               node.selected,
                               node.entryNode,
                               node.outputNode));
    }
    root->setProperty("nodes", juce::var(nodes));
    root->setProperty("routes", juce::var(VectorVar(snapshot.routes, RouteVar)));
    root->setProperty("controlTargets",
                      DebugControlTargetsVar(snapshot.selectedNodeId));
    return juce::var(root.release());
}

juce::var ModulationLaneVar(const EffectiveHostModulationLaneSnapshot& lane)
{
    auto object = std::make_unique<juce::DynamicObject>();
    object->setProperty("slotIndex", lane.slotIndex);
    object->setProperty("enabled", lane.enabled);
    object->setProperty("source",
                        StringVar(HostModulationSourceToString(lane.source)));
    object->setProperty("cvTargetMinimum", lane.cvTargetMinimum);
    object->setProperty("cvTargetMaximum", lane.cvTargetMaximum);
    object->setProperty("bipolarDepth", lane.bipolarDepth);
    object->setProperty("liveSourceValue", lane.liveSourceValue);
    object->setProperty("nativeContribution", lane.nativeContribution);
    return juce::var(object.release());
}

juce::var ModulationDestinationVar(
    const EffectiveHostModulationDestinationSnapshot& destination)
{
    auto object = std::make_unique<juce::DynamicObject>();
    object->setProperty("nodeId", StringVar(destination.nodeId));
    object->setProperty("parameterId", StringVar(destination.parameterId));
    object->setProperty("parameterLabel", StringVar(destination.parameterLabel));
    object->setProperty("unitLabel", StringVar(destination.unitLabel));
    object->setProperty("baseNativeValue", destination.baseNativeValue);
    object->setProperty("resultNativeValue", destination.resultNativeValue);
    object->setProperty("resultNormalizedValue",
                        destination.resultNormalizedValue);
    object->setProperty("clamped", destination.clamped);
    object->setProperty("lanes",
                        juce::var(VectorVar(destination.lanes,
                                            ModulationLaneVar)));
    return juce::var(object.release());
}

juce::var FieldSurfaceVar(const EffectiveHostFieldSurfaceSnapshot& surface)
{
    auto object = std::make_unique<juce::DynamicObject>();
    juce::Array<juce::var> cvOutputs;
    for(const auto& output : surface.cvOutputs)
    {
        auto item = std::make_unique<juce::DynamicObject>();
        item->setProperty("id", StringVar(output.id));
        item->setProperty("label", StringVar(output.label));
        item->setProperty("available", output.available);
        item->setProperty("normalizedValue", output.normalizedValue);
        item->setProperty("volts", output.volts);
        cvOutputs.add(juce::var(item.release()));
    }
    object->setProperty("cvOutputs", juce::var(cvOutputs));

    juce::Array<juce::var> switches;
    for(const auto& sw : surface.switches)
    {
        auto item = std::make_unique<juce::DynamicObject>();
        item->setProperty("id", StringVar(sw.id));
        item->setProperty("label", StringVar(sw.label));
        item->setProperty("detailLabel", StringVar(sw.detailLabel));
        item->setProperty("available", sw.available);
        item->setProperty("pressed", sw.pressed);
        switches.add(juce::var(item.release()));
    }
    object->setProperty("switches", juce::var(switches));

    juce::Array<juce::var> leds;
    for(const auto& led : surface.leds)
    {
        auto item = std::make_unique<juce::DynamicObject>();
        item->setProperty("id", StringVar(led.id));
        item->setProperty("label", StringVar(led.label));
        item->setProperty("normalizedValue", led.normalizedValue);
        leds.add(juce::var(item.release()));
    }
    object->setProperty("leds", juce::var(leds));
    return juce::var(object.release());
}

EffectiveHostFieldSurfaceSnapshot BuildDefaultFieldSurface(HostedAppCore& app,
                                                           const std::string& nodeId)
{
    EffectiveHostFieldSurfaceSnapshot snapshot;
    const auto mapping = BuildDaisyFieldControlMapping(
        app.GetPatchBindings(), app.GetParameters(), app.GetMenuModel(), 4, nodeId);

    for(std::size_t index = 0; index < mapping.cvOutputs.size(); ++index)
    {
        const auto& binding = mapping.cvOutputs[index];
        auto&       output  = snapshot.cvOutputs[index];
        output.id           = MakeDaisyFieldCvOutputPortId(nodeId, index);
        output.label        = binding.label;
        output.available    = binding.available;
        if(binding.available && binding.targetKind == BoardSurfaceTargetKind::kParameter)
        {
            const auto value = app.GetParameterValue(binding.targetId);
            output.normalizedValue = value.hasValue ? value.value : 0.0f;
            output.volts           = output.normalizedValue * 5.0f;
        }
    }

    for(std::size_t index = 0; index < mapping.switches.size(); ++index)
    {
        const auto& binding = mapping.switches[index];
        auto&       sw      = snapshot.switches[index];
        sw.id               = binding.controlId;
        sw.label            = binding.label;
        sw.detailLabel      = binding.detailLabel;
        sw.available        = binding.available;
        sw.pressed          = false;
    }

    for(std::size_t index = 0; index < mapping.leds.size(); ++index)
    {
        const auto& binding = mapping.leds[index];
        auto&       led     = snapshot.leds[index];
        led.id              = binding.controlId;
        led.label           = binding.label;
        led.normalizedValue = 0.0f;
    }
    return snapshot;
}
} // namespace

std::string SerializeAppsPayloadJson()
{
    auto root = std::make_unique<juce::DynamicObject>();
    root->setProperty("defaultAppId", StringVar(GetDefaultHostedAppId()));

    juce::Array<juce::var> apps;
    for(const auto& registration : GetHostedAppRegistrations())
    {
        auto item = std::make_unique<juce::DynamicObject>();
        item->setProperty("appId", StringVar(registration.appId));
        item->setProperty("displayName", StringVar(registration.displayName));
        apps.add(juce::var(item.release()));
    }
    root->setProperty("apps", juce::var(apps));
    return ToJson(juce::var(root.release()));
}

bool SerializeAppDescriptionPayloadJson(const std::string& appId,
                                        std::string*       outputJson,
                                        std::string*       errorMessage)
{
    if(outputJson != nullptr)
    {
        outputJson->clear();
    }
    const auto* registration = FindAppRegistration(appId);
    if(registration == nullptr)
    {
        if(errorMessage != nullptr)
        {
            *errorMessage = "Unknown app: " + appId;
        }
        return false;
    }

    auto app = registration->create("node0");
    app->Prepare(48000.0, 64);
    app->ResetToDefaultState(0);

    auto root = std::make_unique<juce::DynamicObject>();
    root->setProperty("appId", StringVar(app->GetAppId()));
    root->setProperty("displayName", StringVar(app->GetAppDisplayName()));
    root->setProperty("capabilities", CapabilitiesVar(app->GetCapabilities()));
    root->setProperty("patchBindings", PatchBindingsVar(app->GetPatchBindings()));

    root->setProperty(
        "parameters",
        juce::var(VectorVar(app->GetParameters(), ParameterVar)));
    root->setProperty(
        "metaControllers",
        juce::var(VectorVar(app->GetMetaControllers(), MetaControllerVar)));

    const auto& menu = app->GetMenuModel();
    auto menuObject = std::make_unique<juce::DynamicObject>();
    menuObject->setProperty("isOpen", menu.isOpen);
    menuObject->setProperty("isEditing", menu.isEditing);
    menuObject->setProperty("currentSectionId", StringVar(menu.currentSectionId));
    menuObject->setProperty("currentSelection", menu.currentSelection);
    root->setProperty("menu", juce::var(menuObject.release()));
    root->setProperty("menuSections",
                      juce::var(VectorVar(menu.sections, MenuSectionVar)));

    if(outputJson != nullptr)
    {
        *outputJson = ToJson(juce::var(root.release()));
    }
    return true;
}

std::string SerializeBoardsPayloadJson()
{
    auto root = std::make_unique<juce::DynamicObject>();
    root->setProperty("defaultBoardId", StringVar("daisy_patch"));

    juce::Array<juce::var> boards;
    for(const auto& boardId : GetSupportedBoardIds())
    {
        auto item = std::make_unique<juce::DynamicObject>();
        item->setProperty("boardId", StringVar(boardId));
        if(const auto profile = TryCreateBoardProfile(boardId))
        {
            item->setProperty("displayName", StringVar(profile->displayName));
        }
        boards.add(juce::var(item.release()));
    }
    root->setProperty("boards", juce::var(boards));
    return ToJson(juce::var(root.release()));
}

bool SerializeBoardDescriptionPayloadJson(const std::string& boardId,
                                          std::string*       outputJson,
                                          std::string*       errorMessage)
{
    if(outputJson != nullptr)
    {
        outputJson->clear();
    }
    const auto profile = TryCreateBoardProfile(boardId, "node0");
    if(!profile)
    {
        if(errorMessage != nullptr)
        {
            *errorMessage = "Unknown board: " + boardId;
        }
        return false;
    }

    auto root = std::make_unique<juce::DynamicObject>();
    root->setProperty("boardId", StringVar(profile->boardId));
    root->setProperty("nodeId", StringVar(profile->nodeId));
    root->setProperty("displayName", StringVar(profile->displayName));
    root->setProperty("controls",
                      juce::var(VectorVar(profile->controls, ControlSpecVar)));
    root->setProperty("ports",
                      juce::var(VectorVar(profile->ports, VirtualPortVar)));
    root->setProperty("display", DisplayVar(profile->display));
    root->setProperty("surfaceControls",
                      juce::var(VectorVar(profile->surfaceControls,
                                          SurfaceControlVar)));
    root->setProperty("decorations",
                      juce::var(VectorVar(profile->decorations, DecorationVar)));
    root->setProperty("texts", juce::var(VectorVar(profile->texts, TextVar)));
    root->setProperty("indicators",
                      juce::var(VectorVar(profile->indicators, IndicatorVar)));

    if(outputJson != nullptr)
    {
        *outputJson = ToJson(juce::var(root.release()));
    }
    return true;
}

std::string SerializeInputsPayloadJson()
{
    auto root = std::make_unique<juce::DynamicObject>();
    juce::Array<juce::var> inputs;
    const int maxMode = static_cast<int>(TestInputSignalMode::kSquareInput);
    for(int mode = 0; mode <= maxMode; ++mode)
    {
        auto item = std::make_unique<juce::DynamicObject>();
        item->setProperty("id", StringVar(TestInputModeId(mode)));
        item->setProperty("mode", mode);
        item->setProperty("displayName",
                          StringVar(GetTestInputSignalModeName(mode)));
        inputs.add(juce::var(item.release()));
    }
    root->setProperty("inputs", juce::var(inputs));
    return ToJson(juce::var(root.release()));
}

std::string SerializeRenderResultPayloadJson(
    const RenderResultManifest& manifest,
    const RenderAssertionReport* assertions)
{
    auto root = std::make_unique<juce::DynamicObject>();
    root->setProperty("appId", StringVar(manifest.appId));
    root->setProperty("boardId", StringVar(manifest.boardId));
    root->setProperty("selectedNodeId", StringVar(manifest.selectedNodeId));
    root->setProperty("entryNodeId", StringVar(manifest.entryNodeId));
    root->setProperty("outputNodeId", StringVar(manifest.outputNodeId));
    root->setProperty("audioPath", StringVar(manifest.audioPath));
    root->setProperty("manifestPath", StringVar(manifest.manifestPath));
    root->setProperty("audioChecksum", StringVar(manifest.audioChecksum));
    root->setProperty("channelSummaries",
                      juce::var(VectorVar(manifest.channelSummaries,
                                          ChannelSummaryVar)));
    root->setProperty("nodes",
                      juce::var(VectorVar(manifest.nodes,
                                          RenderNodeSummaryVar)));
    root->setProperty("routes",
                      juce::var(VectorVar(manifest.routes,
                                          RenderRouteVar)));
    root->setProperty("executedTimeline",
                      juce::var(VectorVar(manifest.executedTimeline,
                                          RenderTimelineEventVar)));
    root->setProperty("debugState", RenderDebugStateVar(manifest));
    if(assertions != nullptr)
    {
        root->setProperty("assertions", RenderAssertionReportVar(*assertions));
    }
    return ToJson(juce::var(root.release()));
}

bool BuildDefaultSnapshot(const std::string&        appId,
                          const std::string&        boardId,
                          const std::string&        selectedNodeId,
                          EffectiveHostStateSnapshot* snapshot,
                          std::string*             errorMessage)
{
    if(snapshot == nullptr)
    {
        if(errorMessage != nullptr)
        {
            *errorMessage = "Snapshot output is null";
        }
        return false;
    }
    if(!IsKnownAppId(appId))
    {
        if(errorMessage != nullptr)
        {
            *errorMessage = "Unknown app: " + appId;
        }
        return false;
    }
    if(!TryCreateBoardProfile(boardId, selectedNodeId))
    {
        if(errorMessage != nullptr)
        {
            *errorMessage = "Unknown board: " + boardId;
        }
        return false;
    }

    auto app = CreateHostedAppCore(appId, selectedNodeId);
    app->Prepare(48000.0, 64);
    app->ResetToDefaultState(0);

    const auto parameters      = app->GetParameters();
    const auto automationSlots = BuildHostAutomationSlotBindings(parameters);
    std::array<HostCvInputState, 4> cvInputs{};
    for(auto& input : cvInputs)
    {
        input.normalizedValue = 0.5f;
        input.volts           = 2.5f;
    }
    std::array<HostGateInputState, 2> gateInputs{};
    HostAudioInputState audioInput;
    audioInput.mode        = static_cast<int>(TestInputSignalMode::kHostInput);
    audioInput.level       = 0.0f;
    audioInput.frequencyHz = 220.0f;

    const std::vector<EffectiveHostNodeSummary> nodeSummaries = {
        {selectedNodeId, app->GetAppId(), app->GetAppDisplayName(), true, true, true},
    };
    const std::vector<EffectiveHostRouteSnapshot> routes;
    const auto fieldSurface = boardId == "daisy_field"
                                  ? BuildDefaultFieldSurface(*app, selectedNodeId)
                                  : EffectiveHostFieldSurfaceSnapshot{};

    *snapshot = BuildEffectiveHostStateSnapshot(boardId,
                                                selectedNodeId,
                                                1u,
                                                selectedNodeId,
                                                selectedNodeId,
                                                app->GetAppId(),
                                                app->GetAppDisplayName(),
                                                nodeSummaries,
                                                routes,
                                                app->GetPatchBindings(),
                                                parameters,
                                                automationSlots,
                                                cvInputs,
                                                gateInputs,
                                                audioInput,
                                                app->GetMetaControllers(),
                                                fieldSurface);
    return true;
}

std::string SerializeSnapshotPayloadJson(const EffectiveHostStateSnapshot& snapshot)
{
    auto root = std::make_unique<juce::DynamicObject>();
    root->setProperty("boardId", StringVar(snapshot.boardId));
    root->setProperty("selectedNodeId", StringVar(snapshot.selectedNodeId));
    root->setProperty("nodeCount", static_cast<int>(snapshot.nodeCount));
    root->setProperty("entryNodeId", StringVar(snapshot.entryNodeId));
    root->setProperty("outputNodeId", StringVar(snapshot.outputNodeId));
    root->setProperty("appId", StringVar(snapshot.appId));
    root->setProperty("appDisplayName", StringVar(snapshot.appDisplayName));
    root->setProperty("nodeSummaries",
                      juce::var(VectorVar(snapshot.nodeSummaries,
                                          NodeSummaryVar)));
    root->setProperty("routes", juce::var(VectorVar(snapshot.routes, RouteVar)));
    root->setProperty("parameters",
                      juce::var(VectorVar(snapshot.parameters, ParameterVar)));
    root->setProperty("metaControllers",
                      juce::var(VectorVar(snapshot.metaControllers,
                                          [](const EffectiveHostMetaControllerSnapshot& c) {
                                              return MetaControllerVar({c.id,
                                                                        c.label,
                                                                        c.normalizedValue,
                                                                        c.defaultNormalizedValue,
                                                                        c.stateful});
                                          })));

    juce::Array<juce::var> automationSlots;
    for(const auto& slot : snapshot.automationSlots)
    {
        automationSlots.add(AutomationSlotVar(slot));
    }
    root->setProperty("automationSlots", juce::var(automationSlots));

    juce::Array<juce::var> cvInputs;
    for(const auto& input : snapshot.cvInputs)
    {
        cvInputs.add(CvInputVar(input));
    }
    root->setProperty("cvInputs", juce::var(cvInputs));

    juce::Array<juce::var> gateInputs;
    for(const auto& input : snapshot.gateInputs)
    {
        gateInputs.add(GateInputVar(input));
    }
    root->setProperty("gateInputs", juce::var(gateInputs));
    root->setProperty("audioInput", AudioInputVar(snapshot.audioInput));
    root->setProperty("fieldSurface", FieldSurfaceVar(snapshot.fieldSurface));
    root->setProperty("selectedModulationDestinationId",
                      StringVar(snapshot.selectedModulationDestinationId));
    root->setProperty("modulationDestinations",
                      juce::var(VectorVar(snapshot.modulationDestinations,
                                          ModulationDestinationVar)));
    root->setProperty("debugState", SnapshotDebugStateVar(snapshot));
    return ToJson(juce::var(root.release()));
}
} // namespace cli
} // namespace daisyhost
