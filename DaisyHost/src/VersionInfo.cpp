#include "daisyhost/VersionInfo.h"

#ifndef DAISYHOST_VERSION
#define DAISYHOST_VERSION "0.0.0"
#endif

#ifndef DAISYHOST_BUILD_IDENTITY
#define DAISYHOST_BUILD_IDENTITY "DaisyHost"
#endif

namespace daisyhost
{
const VersionInfo& GetVersionInfo()
{
    static const VersionInfo kInfo = {
        DAISYHOST_VERSION,
        DAISYHOST_BUILD_IDENTITY,
        {
            "Patch-faithful panel refresh",
            "Shared menu with parameter editing",
            "Saw input and Host In default",
        },
    };

    return kInfo;
}
} // namespace daisyhost
