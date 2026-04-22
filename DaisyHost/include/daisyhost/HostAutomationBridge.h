#pragma once

#include <array>
#include <cstddef>
#include <string>
#include <vector>

#include "daisyhost/HostedAppCore.h"

namespace daisyhost
{
inline constexpr std::size_t kHostAutomationSlotCount = 5;

struct HostAutomationSlotBinding
{
    std::string slotId;
    std::string slotName;
    bool        available = false;
    std::string parameterId;
    std::string parameterLabel;
    std::string unitLabel;
};

using HostAutomationSlotBindings
    = std::array<HostAutomationSlotBinding, kHostAutomationSlotCount>;

std::string MakeHostAutomationSlotId(std::size_t slotIndex);
std::string MakeHostAutomationSlotName(std::size_t slotIndex);

HostAutomationSlotBindings BuildHostAutomationSlotBindings(
    const std::vector<ParameterDescriptor>& parameters);
} // namespace daisyhost
