#include "daisyhost/HostStartupPolicy.h"

namespace daisyhost
{
int ResolveStartupTestInputMode(bool isStandalone,
                                bool hasSavedTestInputMode,
                                int  currentMode)
{
    (void) isStandalone;
    (void) hasSavedTestInputMode;
    return currentMode;
}
} // namespace daisyhost
