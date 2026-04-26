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

std::string LowerCopy(std::string text)
{
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return text;
}

bool ContainsText(const std::string& haystack, const std::string& needle)
{
    return haystack.find(needle) != std::string::npos;
}

BoardSurfaceBinding MakeUnavailableBinding(const std::string& controlId,
                                           const std::string& label)
{
    BoardSurfaceBinding binding;
    binding.controlId = controlId;
    binding.label     = label;
    return binding;
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

bool IsPublicControllableParameter(const ParameterDescriptor& parameter,
                                   bool includeMenuEditable)
{
    return parameter.automatable
           || (includeMenuEditable && parameter.menuEditable);
}

struct RankedParameter
{
    const ParameterDescriptor* parameter   = nullptr;
    std::size_t                sourceIndex = 0;
};

std::unordered_set<std::string>
BuildMirroredParameterIds(const HostedAppPatchBindings& patchBindings)
{
    std::unordered_set<std::string> mirroredParameterIds;
    for(std::size_t index = 0; index < patchBindings.knobControlIds.size();
        ++index)
    {
        const auto& targetId = patchBindings.knobControlIds[index];
        if(targetId.empty())
        {
            continue;
        }

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

    for(std::size_t index = 0; index < patchBindings.fieldKnobControlIds.size();
        ++index)
    {
        const auto& controlId = patchBindings.fieldKnobControlIds[index];
        if(!controlId.empty())
        {
            mirroredParameterIds.insert(controlId);
            if(!patchBindings.fieldKnobParameterIds[index].empty())
            {
                mirroredParameterIds.insert(
                    patchBindings.fieldKnobParameterIds[index]);
            }
            else
            {
                mirroredParameterIds.insert(ParameterIdForControlId(controlId));
            }
        }

        if(!patchBindings.fieldKnobParameterIds[index].empty())
        {
            mirroredParameterIds.insert(patchBindings.fieldKnobParameterIds[index]);
        }
    }

    return mirroredParameterIds;
}

std::vector<RankedParameter> BuildRankedPublicParameters(
    const std::vector<ParameterDescriptor>& parameters,
    const std::unordered_set<std::string>&  excludedParameterIds,
    bool                                    includeMenuEditable)
{
    std::vector<RankedParameter> rankedParameters;
    rankedParameters.reserve(parameters.size());
    for(std::size_t index = 0; index < parameters.size(); ++index)
    {
        const auto& parameter = parameters[index];
        if(!IsPublicControllableParameter(parameter, includeMenuEditable)
           || excludedParameterIds.count(parameter.id) > 0)
        {
            continue;
        }
        rankedParameters.push_back({&parameter, index});
    }

    std::sort(rankedParameters.begin(),
              rankedParameters.end(),
              [](const RankedParameter& lhs, const RankedParameter& rhs) {
                  if(lhs.parameter->importanceRank
                     != rhs.parameter->importanceRank)
                  {
                      return lhs.parameter->importanceRank
                             < rhs.parameter->importanceRank;
                  }
                  return lhs.sourceIndex < rhs.sourceIndex;
              });

    return rankedParameters;
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

std::vector<BoardSurfaceBinding> BuildDaisyFieldPublicParameterList(
    const HostedAppPatchBindings&,
    const std::vector<ParameterDescriptor>& parameters)
{
    const std::unordered_set<std::string> noExclusions;
    const auto rankedParameters
        = BuildRankedPublicParameters(parameters, noExclusions, true);

    std::vector<BoardSurfaceBinding> bindings;
    bindings.reserve(rankedParameters.size());
    for(std::size_t index = 0; index < rankedParameters.size(); ++index)
    {
        const auto& parameter = *rankedParameters[index].parameter;
        BoardSurfaceBinding binding;
        binding.controlId   = "field/public_parameter/"
                            + std::to_string(index + 1);
        binding.label       = "P" + std::to_string(index + 1);
        binding.detailLabel = parameter.label;
        binding.targetKind  = BoardSurfaceTargetKind::kParameter;
        binding.targetId    = parameter.id;
        binding.available   = true;
        bindings.push_back(binding);
    }

    return bindings;
}

std::vector<BoardSurfaceBinding> BuildDaisyFieldCvTargetOptions(
    const DaisyFieldControlMapping& mapping,
    const std::string&              latchedTargetId)
{
    std::vector<BoardSurfaceBinding> options;
    options.reserve(kDaisyFieldKnobCount + 2);
    options.push_back(MakeUnavailableBinding("", "App CV In"));
    options.back().detailLabel = "Unassigned";

    bool foundLatched = latchedTargetId.empty();
    for(std::size_t index = 0; index < mapping.knobs.size(); ++index)
    {
        const auto& knob = mapping.knobs[index];
        if(!knob.available
           || (knob.targetKind != BoardSurfaceTargetKind::kControl
               && knob.targetKind != BoardSurfaceTargetKind::kParameter)
           || !IsDaisyFieldCvTargetSafe(knob))
        {
            continue;
        }

        BoardSurfaceBinding option = knob;
        option.label = "K" + std::to_string(index + 1);
        option.detailLabel = option.label
                             + (knob.detailLabel.empty()
                                    ? std::string()
                                    : " " + knob.detailLabel);
        options.push_back(option);
        foundLatched = foundLatched || knob.targetId == latchedTargetId;
    }

    if(!foundLatched && IsDaisyFieldCvTargetIdSafe(latchedTargetId))
    {
        BoardSurfaceBinding latched;
        latched.available   = true;
        latched.label       = "Latched";
        latched.detailLabel = "Latched K target";
        latched.targetKind  = BoardSurfaceTargetKind::kControl;
        latched.targetId    = latchedTargetId;
        options.push_back(latched);
    }

    return options;
}

std::vector<BoardSurfaceBinding> BuildDaisyFieldCvTargetOptions(
    const DaisyFieldControlMapping& defaultMapping,
    const DaisyFieldControlMapping& alternativeMapping,
    const std::string&              latchedTargetId)
{
    std::vector<BoardSurfaceBinding> options;
    options.reserve((kDaisyFieldKnobCount * 2) + 2);
    options.push_back(MakeUnavailableBinding("", "App CV In"));
    options.back().detailLabel = "Unassigned";

    std::unordered_set<std::string> seenTargetIds;
    bool foundLatched = latchedTargetId.empty();

    auto appendMapping = [&](const DaisyFieldControlMapping& mapping,
                             const char*                     suffix) {
        for(std::size_t index = 0; index < mapping.knobs.size(); ++index)
        {
            const auto& knob = mapping.knobs[index];
            if(!knob.available
               || (knob.targetKind != BoardSurfaceTargetKind::kControl
                   && knob.targetKind != BoardSurfaceTargetKind::kParameter)
               || !IsDaisyFieldCvTargetSafe(knob)
               || seenTargetIds.count(knob.targetId) > 0)
            {
                continue;
            }

            seenTargetIds.insert(knob.targetId);
            BoardSurfaceBinding option = knob;
            option.label = "K" + std::to_string(index + 1) + suffix;
            option.detailLabel = option.label
                                 + (knob.detailLabel.empty()
                                        ? std::string()
                                        : " " + knob.detailLabel);
            options.push_back(option);
            foundLatched = foundLatched || knob.targetId == latchedTargetId;
        }
    };

    appendMapping(defaultMapping, ".1");
    appendMapping(alternativeMapping, ".2");

    if(!foundLatched && IsDaisyFieldCvTargetIdSafe(latchedTargetId))
    {
        BoardSurfaceBinding latched;
        latched.available   = true;
        latched.label       = "Latched";
        latched.detailLabel = "Latched K target";
        latched.targetKind  = BoardSurfaceTargetKind::kControl;
        latched.targetId    = latchedTargetId;
        options.push_back(latched);
    }

    return options;
}

bool IsDaisyFieldCvTargetSafe(const BoardSurfaceBinding& binding)
{
    if(!binding.available
       || (binding.targetKind != BoardSurfaceTargetKind::kControl
           && binding.targetKind != BoardSurfaceTargetKind::kParameter))
    {
        return false;
    }

    if(!IsDaisyFieldCvTargetIdSafe(binding.targetId))
    {
        return false;
    }

    const auto detail = LowerCopy(binding.detailLabel + " " + binding.label);
    if(ContainsText(detail, "input mix") || detail == "mix"
       || ContainsText(detail, " output") || ContainsText(detail, "volume"))
    {
        return false;
    }

    return true;
}

bool IsDaisyFieldCvTargetIdSafe(const std::string& targetId)
{
    if(targetId.empty())
    {
        return false;
    }

    const auto id = LowerCopy(targetId);
    if(ContainsText(id, "bypass") || ContainsText(id, "mute")
       || ContainsText(id, "enabled") || ContainsText(id, "input_mix")
       || ContainsText(id, "global_input") || ContainsText(id, "/control/mix")
       || ContainsText(id, "/param/mix") || ContainsText(id, "dry_out")
       || ContainsText(id, "early_out") || ContainsText(id, "late_out")
       || ContainsText(id, "/output") || ContainsText(id, "level")
       || ContainsText(id, "volume"))
    {
        return false;
    }

    return true;
}

bool ShouldForwardDaisyFieldCvInput(const std::string& latchedTargetId)
{
    return latchedTargetId.empty();
}

DaisyFieldDrawerPage StepDaisyFieldDrawerPage(DaisyFieldDrawerPage page,
                                               int                  delta)
{
    constexpr int kPageCount = 3;
    int current = static_cast<int>(page);
    if(current < 0 || current >= kPageCount)
    {
        current = 0;
    }

    int next = (current + delta) % kPageCount;
    if(next < 0)
    {
        next += kPageCount;
    }
    return static_cast<DaisyFieldDrawerPage>(next);
}

DaisyFieldControlMapping BuildDaisyFieldControlMapping(
    const HostedAppPatchBindings&           patchBindings,
    const std::vector<ParameterDescriptor>& parameters,
    const MenuModel&                        menu,
    int                                     keyboardOctave,
    const std::string&                      nodeId,
    DaisyFieldKnobLayoutMode                knobLayoutMode)
{
    (void)menu;
    DaisyFieldControlMapping mapping;

    const auto mirroredParameterIds = BuildMirroredParameterIds(patchBindings);

    for(std::size_t index = 0; index < kDaisyFieldKnobCount; ++index)
    {
        const std::string label = "K" + std::to_string(index + 1);
        auto&             knob  = mapping.knobs[index];
        knob = MakeUnavailableBinding(MakeDaisyFieldKnobControlId(nodeId, index),
                                      label);
    }

    if(knobLayoutMode == DaisyFieldKnobLayoutMode::kPatchPagePlusExtras)
    {
        const bool hasFieldKnobOverride = std::any_of(
            patchBindings.fieldKnobControlIds.begin(),
            patchBindings.fieldKnobControlIds.end(),
            [](const std::string& id) { return !id.empty(); })
                                          || std::any_of(
                                              patchBindings.fieldKnobParameterIds.begin(),
                                              patchBindings.fieldKnobParameterIds.end(),
                                              [](const std::string& id) {
                                                  return !id.empty();
                                              });

        if(hasFieldKnobOverride)
        {
            for(std::size_t index = 0; index < kDaisyFieldKnobCount; ++index)
            {
                auto& knob = mapping.knobs[index];
                knob.detailLabel = patchBindings.fieldKnobDetailLabels[index];
                if(!patchBindings.fieldKnobControlIds[index].empty())
                {
                    knob.available  = true;
                    knob.targetKind = BoardSurfaceTargetKind::kControl;
                    knob.targetId   = patchBindings.fieldKnobControlIds[index];
                }
                else if(!patchBindings.fieldKnobParameterIds[index].empty())
                {
                    knob.available  = true;
                    knob.targetKind = BoardSurfaceTargetKind::kParameter;
                    knob.targetId   = patchBindings.fieldKnobParameterIds[index];
                }
            }
        }
        else
        {
            for(std::size_t index = 0; index < 4; ++index)
            {
                auto& knob       = mapping.knobs[index];
                knob.detailLabel = patchBindings.knobDetailLabels[index];

                const auto& targetId = patchBindings.knobControlIds[index];
                if(!targetId.empty())
                {
                    knob.available  = true;
                    knob.targetKind = BoardSurfaceTargetKind::kControl;
                    knob.targetId   = targetId;
                }
            }
        }
    }
    const auto rankedParameters
        = BuildRankedPublicParameters(
            parameters,
            mirroredParameterIds,
            knobLayoutMode
                == DaisyFieldKnobLayoutMode::kControllableParameters);

    const std::size_t firstParameterKnob
        = knobLayoutMode == DaisyFieldKnobLayoutMode::kControllableParameters
              ? 0
              : 4;
    for(std::size_t index = firstParameterKnob; index < kDaisyFieldKnobCount;
        ++index)
    {
        if(mapping.knobs[index].available)
        {
            continue;
        }
        const std::size_t rankedIndex = index - firstParameterKnob;
        if(rankedIndex >= rankedParameters.size())
        {
            continue;
        }

        auto&       knob      = mapping.knobs[index];
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

        const auto& sourceKnob = mapping.knobs[firstParameterKnob + index];
        if(sourceKnob.available
           && sourceKnob.targetKind == BoardSurfaceTargetKind::kParameter)
        {
            cvOutput.available  = true;
            cvOutput.targetKind = BoardSurfaceTargetKind::kParameter;
            cvOutput.targetId   = sourceKnob.targetId;
            cvOutput.detailLabel = sourceKnob.detailLabel;
        }
    }

    const std::array<std::pair<const char*, const char*>, kDaisyFieldSwitchCount>
        navigationTargets = {{{"back", "Back"}, {"forward", "Forward"}}};

    for(std::size_t index = 0; index < kDaisyFieldSwitchCount; ++index)
    {
        auto& sw = mapping.switches[index];
        sw       = MakeUnavailableBinding(MakeDaisyFieldSwitchControlId(nodeId, index),
                                    "SW" + std::to_string(index + 1));
        sw.available   = true;
        sw.targetKind  = BoardSurfaceTargetKind::kMenuItem;
        sw.targetId    = nodeId + "/menu/navigation/"
                       + navigationTargets[index].first;
        sw.detailLabel = navigationTargets[index].second;
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
        if(!patchBindings.fieldKeyMenuItemIds[index].empty())
        {
            key.targetKind = BoardSurfaceTargetKind::kMenuItem;
            key.targetId   = patchBindings.fieldKeyMenuItemIds[index];
            key.detailLabel = patchBindings.fieldKeyDetailLabels[index];
        }
        else
        {
            key.targetKind = BoardSurfaceTargetKind::kMidiNote;
            key.midiNote   = DaisyFieldKeyToMidiNote(index, keyboardOctave);
        }
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
    const std::string&                      nodeId,
    DaisyFieldKnobLayoutMode                knobLayoutMode)
{
    return BuildDaisyFieldControlMapping(patchBindings,
                                         parameters,
                                         MenuModel{},
                                         keyboardOctave,
                                         nodeId,
                                         knobLayoutMode);
}
} // namespace daisyhost
