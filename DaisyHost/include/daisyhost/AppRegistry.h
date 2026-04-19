#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "daisyhost/HostedAppCore.h"

namespace daisyhost
{
struct HostedAppRegistration
{
    std::string appId;
    std::string displayName;
    std::function<std::unique_ptr<HostedAppCore>(const std::string& nodeId)> create;
};

const std::vector<HostedAppRegistration>& GetHostedAppRegistrations();
std::string GetDefaultHostedAppId();
std::unique_ptr<HostedAppCore> CreateHostedAppCore(const std::string& requestedAppId,
                                                   const std::string& nodeId,
                                                   std::string* resolvedAppId = nullptr);
} // namespace daisyhost
