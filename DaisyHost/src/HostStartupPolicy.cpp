#include "daisyhost/HostStartupPolicy.h"

namespace daisyhost
{
int ResolveStartupTestInputMode(bool isStandalone,
                                bool hasSavedTestInputMode,
                                int  currentMode)
{
    (void) hasSavedTestInputMode;

    constexpr int kHostInput = 0;
    constexpr int kSineInput = 1;

    if(isStandalone && currentMode == kHostInput)
    {
        return kSineInput;
    }

    return currentMode;
}
} // namespace daisyhost
