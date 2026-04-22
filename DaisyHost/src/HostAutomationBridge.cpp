#include "daisyhost/HostAutomationBridge.h"

#include <algorithm>

namespace daisyhost
{
namespace
{
struct IndexedParameter
{
    const ParameterDescriptor* parameter   = nullptr;
    std::size_t                sourceIndex = 0;
};
} // namespace

std::string MakeHostAutomationSlotId(std::size_t slotIndex)
{
    return "daisyhost.slot" + std::to_string(slotIndex + 1);
}

std::string MakeHostAutomationSlotName(std::size_t slotIndex)
{
    return "Param " + std::to_string(slotIndex + 1);
}

HostAutomationSlotBindings BuildHostAutomationSlotBindings(
    const std::vector<ParameterDescriptor>& parameters)
{
    std::vector<IndexedParameter> rankedParameters;
    rankedParameters.reserve(parameters.size());

    for(std::size_t index = 0; index < parameters.size(); ++index)
    {
        if(!parameters[index].automatable)
        {
            continue;
        }

        rankedParameters.push_back({&parameters[index], index});
    }

    std::sort(rankedParameters.begin(),
              rankedParameters.end(),
              [](const IndexedParameter& left, const IndexedParameter& right) {
                  if(left.parameter->importanceRank != right.parameter->importanceRank)
                  {
                      return left.parameter->importanceRank
                             < right.parameter->importanceRank;
                  }

                  if(left.sourceIndex != right.sourceIndex)
                  {
                      return left.sourceIndex < right.sourceIndex;
                  }

                  return left.parameter->id < right.parameter->id;
              });

    HostAutomationSlotBindings bindings{};
    for(std::size_t slotIndex = 0; slotIndex < bindings.size(); ++slotIndex)
    {
        auto& slot   = bindings[slotIndex];
        slot.slotId  = MakeHostAutomationSlotId(slotIndex);
        slot.slotName = MakeHostAutomationSlotName(slotIndex);

        if(slotIndex >= rankedParameters.size())
        {
            continue;
        }

        const auto& parameter = *rankedParameters[slotIndex].parameter;
        slot.available        = true;
        slot.parameterId      = parameter.id;
        slot.parameterLabel   = parameter.label;
        slot.unitLabel        = parameter.unitLabel;
    }

    return bindings;
}
} // namespace daisyhost
