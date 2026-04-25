#include "daisyhost/BoardControlMapping.h"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <string>
#include <unordered_set>

#include "daisyhost/ComputerKeyboardMidi.h"

namespace daisyhost
{
namespace
{
std::string MakeFieldCvInputControlId(const std::string& nodeId,
                                      std::size_t       zeroBasedIndex)
{
    return nodeId + "/control/field_cv_in_"
           + std::to_string(zeroBasedIndex + 1);
}

std::string MakeFieldGateInputControlId(const std::string& nodeId)
{
    return nodeId + "/control/field_gate_in";
}

std::string MakeFieldGateOutputPortId(const std::string& nodeId)
{
    return nodeId + "/port/gate_out_1";
}

std::string FieldKeyLabel(std::size_t zeroBasedIndex)
{
    const char row = zeroBasedIndex < 8 ? 'A' : 'B';
    return std::string(1, row) + std::to_string((zeroBasedIndex % 8) + 1);
}

std::string ParameterIdForControlId(const std::string& controlId)
{
    constexpr const char* kControlSegment   = "/control/";
    constexpr const char* kParameterSegment = "/param/";

    const auto segmentPos = controlId.find(kControlSegment);
    if(segmentPos == std::string::npos)
    {
        return controlId;
    }

    std::string parameterId = controlId;
    parameterId.replace(segmentPos,
                        std::string(kControlSegment).size(),
                        kParameterSegment);
    return parameterId;
}

BoardSurfaceBinding MakeUnavailableBinding(const std::string& controlId,
                                           const std::string& label)
{
    BoardSurfaceBinding binding;
    binding.controlId = controlId;
    binding.label     = label;
    return binding;
}

bool IsUtilitySection(const MenuSection& section)
{
    std::string haystack = section.id + " " + section.title;
    std::transform(haystack.begin(),
                   haystack.end(),
                   haystack.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return haystack.find("util") != std::string::npos;
}

std::string MakeFieldLedId(const std::string& nodeId,
                           const std::string& suffix)
{
    return nodeId + "/led/" + suffix;
}

void AssignPortBinding(BoardSurfaceBinding& binding,
                       BoardSurfaceTargetKind targetKind,
                       const std::string& targetId)
{
    if(targetId.empty())
    {
        return;
    }

    binding.available  = true;
    binding.targetKind = targetKind;
    binding.targetId   = targetId;
}
} // namespace

std::string MakeDaisyFieldKnobControlId(const std::string& nodeId,
                                         std::size_t       zeroBasedIndex)
{
    if(zeroBasedIndex >= kDaisyFieldKnobCount)
    {
        throw std::out_of_range("Daisy Field knob index out of range");
    }
    return nodeId + "/control/field_knob_"
           + std::to_string(zeroBasedIndex + 1);
}

std::string MakeDaisyFieldKeyControlId(const std::string& nodeId,
                                        std::size_t       zeroBasedIndex)
{
    if(zeroBasedIndex >= kDaisyFieldKeyCount)
    {
        throw std::out_of_range("Daisy Field key index out of range");
    }

    const char row = zeroBasedIndex < 8 ? 'a' : 'b';
    return nodeId + "/control/field_key_" + std::string(1, row) + "_"
           + std::to_string((zeroBasedIndex % 8) + 1);
}

std::string MakeDaisyFieldCvOutputControlId(const std::string& nodeId,
                                            std::size_t       zeroBasedIndex)
{
    if(zeroBasedIndex >= kDaisyFieldCvOutputCount)
    {
        throw std::out_of_range("Daisy Field CV output index out of range");
    }
    return nodeId + "/control/field_cv_out_"
           + std::to_string(zeroBasedIndex + 1);
}

std::string MakeDaisyFieldCvOutputPortId(const std::string& nodeId,
                                         std::size_t       zeroBasedIndex)
{
    if(zeroBasedIndex >= kDaisyFieldCvOutputCount)
    {
        throw std::out_of_range("Daisy Field CV output index out of range");
    }

    return nodeId + "/port/field_cv_out_"
           + std::to_string(zeroBasedIndex + 1);
}

std::string MakeDaisyFieldSwitchControlId(const std::string& nodeId,
                                          std::size_t       zeroBasedIndex)
{
    if(zeroBasedIndex >= kDaisyFieldSwitchCount)
    {
        throw std::out_of_range("Daisy Field switch index out of range");
    }
    return nodeId + "/control/field_sw_"
           + std::to_string(zeroBasedIndex + 1);
}

int DaisyFieldKeyToMidiNote(std::size_t zeroBasedIndex, int keyboardOctave)
{
    if(zeroBasedIndex >= kDaisyFieldKeyCount)
    {
        throw std::out_of_range("Daisy Field key index out of range");
    }

    const int clampedOctave = ComputerKeyboardMidi::ClampOctave(keyboardOctave);
    return ((clampedOctave + 1) * 12) + static_cast<int>(zeroBasedIndex);
}

DaisyFieldControlMapping BuildDaisyFieldControlMapping(
    const HostedAppPatchBindings&           patchBindings,
    const std::vector<ParameterDescriptor>& parameters,
    const MenuModel&                        menu,
    int                                     keyboardOctave,
    const std::string&                      nodeId)
{
    DaisyFieldControlMapping mapping;

    std::unordered_set<std::string> mirroredParameterIds;
    for(std::size_t index = 0; index < 4; ++index)
    {
        const std::string label = "K" + std::to_string(index + 1);
        auto&             knob  = mapping.knobs[index];
        knob = MakeUnavailableBinding(MakeDaisyFieldKnobControlId(nodeId, index),
                                      label);
        knob.detailLabel = patchBindings.knobDetailLabels[index];

        const auto& targetId = patchBindings.knobControlIds[index];
        if(!targetId.empty())
        {
            knob.available  = true;
            knob.targetKind = BoardSurfaceTargetKind::kControl;
            knob.targetId   = targetId;
            mirroredParameterIds.insert(targetId);
            if(!patchBindings.knobParameterIds[index].empty())
            {
                mirroredParameterIds.insert(patchBindings.knobParameterIds[index]);
            }
            else
            {
                mirroredParameterIds.insert(ParameterIdForControlId(targetId));
            }
        }
    }

    struct RankedParameter
    {
        const ParameterDescriptor* parameter = nullptr;
        std::size_t                sourceIndex = 0;
    };

    std::vector<RankedParameter> rankedParameters;
    rankedParameters.reserve(parameters.size());
    for(std::size_t index = 0; index < parameters.size(); ++index)
    {
        const auto& parameter = parameters[index];
        if(!parameter.automatable || mirroredParameterIds.count(parameter.id) > 0)
        {
            continue;
        }
        rankedParameters.push_back({&parameter, index});
    }

    std::sort(rankedParameters.begin(),
              rankedParameters.end(),
              [](const RankedParameter& lhs, const RankedParameter& rhs) {
                  if(lhs.parameter->importanceRank != rhs.parameter->importanceRank)
                  {
                      return lhs.parameter->importanceRank
                             < rhs.parameter->importanceRank;
                  }
                  return lhs.sourceIndex < rhs.sourceIndex;
              });

    for(std::size_t index = 4; index < kDaisyFieldKnobCount; ++index)
    {
        const std::string label = "K" + std::to_string(index + 1);
        auto&             knob  = mapping.knobs[index];
        knob = MakeUnavailableBinding(MakeDaisyFieldKnobControlId(nodeId, index),
                                      label);

        const std::size_t rankedIndex = index - 4;
        if(rankedIndex >= rankedParameters.size())
        {
            continue;
        }

        const auto& parameter = *rankedParameters[rankedIndex].parameter;
        knob.available       = true;
        knob.targetKind      = BoardSurfaceTargetKind::kParameter;
        knob.targetId        = parameter.id;
        knob.detailLabel     = parameter.label;
    }

    for(std::size_t index = 0; index < kDaisyFieldCvOutputCount; ++index)
    {
        auto& cvOutput = mapping.cvOutputs[index];
        cvOutput       = MakeUnavailableBinding(
            MakeDaisyFieldCvOutputControlId(nodeId, index),
            "CV OUT " + std::to_string(index + 1));

        const auto& sourceKnob = mapping.knobs[4 + index];
        if(sourceKnob.available
           && sourceKnob.targetKind == BoardSurfaceTargetKind::kParameter)
        {
            cvOutput.available  = true;
            cvOutput.targetKind = BoardSurfaceTargetKind::kParameter;
            cvOutput.targetId   = sourceKnob.targetId;
            cvOutput.detailLabel = sourceKnob.detailLabel;
        }
    }

    std::vector<const MenuItem*> utilityItems;
    for(const auto& section : menu.sections)
    {
        if(!IsUtilitySection(section))
        {
            continue;
        }
        for(const auto& item : section.items)
        {
            if(item.editable && item.actionKind == MenuItemActionKind::kMomentary)
            {
                utilityItems.push_back(&item);
            }
        }
    }

    for(std::size_t index = 0; index < kDaisyFieldSwitchCount; ++index)
    {
        auto& sw = mapping.switches[index];
        sw       = MakeUnavailableBinding(MakeDaisyFieldSwitchControlId(nodeId, index),
                                    "SW" + std::to_string(index + 1));
        if(index < utilityItems.size())
        {
            sw.available   = true;
            sw.targetKind  = BoardSurfaceTargetKind::kMenuItem;
            sw.targetId    = utilityItems[index]->id;
            sw.detailLabel = utilityItems[index]->label;
        }
    }

    for(std::size_t index = 0; index < kDaisyFieldCvInputCount; ++index)
    {
        auto& cvInput = mapping.cvInputs[index];
        cvInput       = MakeUnavailableBinding(
            MakeFieldCvInputControlId(nodeId, index),
            "CV" + std::to_string(index + 1));
        AssignPortBinding(cvInput,
                          BoardSurfaceTargetKind::kCvInput,
                          patchBindings.cvInputPortIds[index]);
    }

    mapping.gateInput = MakeUnavailableBinding(MakeFieldGateInputControlId(nodeId),
                                               "Gate In");
    AssignPortBinding(mapping.gateInput,
                      BoardSurfaceTargetKind::kGateInput,
                      patchBindings.gateInputPortIds[0]);

    for(std::size_t index = 0; index < kDaisyFieldKeyCount; ++index)
    {
        auto& key     = mapping.keys[index];
        key           = MakeUnavailableBinding(MakeDaisyFieldKeyControlId(nodeId, index),
                                     FieldKeyLabel(index));
        key.available = true;
        key.targetKind = BoardSurfaceTargetKind::kMidiNote;
        key.midiNote   = DaisyFieldKeyToMidiNote(index, keyboardOctave);
    }

    for(std::size_t index = 0; index < kDaisyFieldKeyCount; ++index)
    {
        auto& led      = mapping.leds[index];
        led            = MakeUnavailableBinding(
            MakeFieldLedId(nodeId, "field_key_"
                                       + std::string(1, index < 8 ? 'a' : 'b')
                                       + "_" + std::to_string((index % 8) + 1)),
            FieldKeyLabel(index) + " LED");
        led.available  = true;
        led.targetKind = BoardSurfaceTargetKind::kLed;
        led.targetId   = mapping.keys[index].controlId;
    }

    for(std::size_t index = 0; index < kDaisyFieldSwitchCount; ++index)
    {
        auto& led      = mapping.leds[kDaisyFieldKeyCount + index];
        led            = MakeUnavailableBinding(
            MakeFieldLedId(nodeId, "field_sw_" + std::to_string(index + 1)),
            "SW" + std::to_string(index + 1) + " LED");
        led.available  = true;
        led.targetKind = BoardSurfaceTargetKind::kLed;
        led.targetId   = mapping.switches[index].controlId;
    }

    mapping.leds[18] = MakeUnavailableBinding(MakeFieldLedId(nodeId, "field_gate_in"),
                                              "Gate In");
    mapping.leds[18].available  = true;
    mapping.leds[18].targetKind = BoardSurfaceTargetKind::kLed;
    mapping.leds[18].targetId   = MakeFieldGateInputControlId(nodeId);

    mapping.leds[19] = MakeUnavailableBinding(MakeFieldLedId(nodeId, "field_gate_out"),
                                              "Gate Out");
    mapping.leds[19].available  = true;
    mapping.leds[19].targetKind = BoardSurfaceTargetKind::kLed;
    mapping.leds[19].targetId   = MakeFieldGateOutputPortId(nodeId);

    return mapping;
}

DaisyFieldControlMapping BuildDaisyFieldControlMapping(
    const HostedAppPatchBindings&           patchBindings,
    const std::vector<ParameterDescriptor>& parameters,
    int                                     keyboardOctave,
    const std::string&                      nodeId)
{
    return BuildDaisyFieldControlMapping(patchBindings,
                                         parameters,
                                         MenuModel{},
                                         keyboardOctave,
                                         nodeId);
}
} // namespace daisyhost
