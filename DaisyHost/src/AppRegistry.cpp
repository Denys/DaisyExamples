#include "daisyhost/AppRegistry.h"

#include <algorithm>

#include "daisyhost/apps/BraidsCore.h"
#include "daisyhost/apps/CloudSeedCore.h"
#include "daisyhost/apps/HarmoniqsCore.h"
#include "daisyhost/apps/MultiDelayCore.h"
#include "daisyhost/apps/TorusCore.h"
#include "daisyhost/apps/VASynthCore.h"

namespace daisyhost
{
namespace
{
const std::vector<HostedAppRegistration> kRegistrations = {
    {"multidelay",
     "Multi Delay",
     [](const std::string& nodeId) {
         return std::make_unique<apps::MultiDelayCore>(nodeId);
     }},
    {"torus",
     "Torus",
     [](const std::string& nodeId) {
         return std::make_unique<apps::TorusCore>(nodeId);
     }},
    {"cloudseed",
     "CloudSeed",
     [](const std::string& nodeId) {
         return std::make_unique<apps::CloudSeedCore>(nodeId);
     }},
    {"braids",
     "Braids",
     [](const std::string& nodeId) {
         return std::make_unique<apps::BraidsCore>(nodeId);
     }},
    {"harmoniqs",
     "Harmoniqs",
     [](const std::string& nodeId) {
         return std::make_unique<apps::HarmoniqsCore>(nodeId);
     }},
    {"vasynth",
     "VA Synth",
     [](const std::string& nodeId) {
         return std::make_unique<apps::VASynthCore>(nodeId);
     }},
};
} // namespace

const std::vector<HostedAppRegistration>& GetHostedAppRegistrations()
{
    return kRegistrations;
}

std::string GetDefaultHostedAppId()
{
    return "multidelay";
}

std::unique_ptr<HostedAppCore> CreateHostedAppCore(const std::string& requestedAppId,
                                                   const std::string& nodeId,
                                                   std::string*       resolvedAppId)
{
    const auto& registrations = GetHostedAppRegistrations();
    const auto  registrationIt
        = std::find_if(registrations.begin(),
                       registrations.end(),
                       [&requestedAppId](const HostedAppRegistration& registration) {
                           return registration.appId == requestedAppId;
                       });

    const auto& registration
        = registrationIt != registrations.end()
              ? *registrationIt
              : registrations.front();

    if(resolvedAppId != nullptr)
    {
        *resolvedAppId = registration.appId;
    }
    return registration.create(nodeId);
}
} // namespace daisyhost
