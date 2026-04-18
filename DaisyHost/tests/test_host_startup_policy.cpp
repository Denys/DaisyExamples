#include <gtest/gtest.h>

#include "daisyhost/HostStartupPolicy.h"

namespace
{
TEST(HostStartupPolicyTest, DefaultsStandaloneHostInputToSineWhenNoSessionExists)
{
    EXPECT_EQ(daisyhost::ResolveStartupTestInputMode(true, false, 0), 1);
}

TEST(HostStartupPolicyTest, PreservesSavedOrNonStandaloneModes)
{
    EXPECT_EQ(daisyhost::ResolveStartupTestInputMode(false, false, 0), 0);
    EXPECT_EQ(daisyhost::ResolveStartupTestInputMode(true, true, 0), 1);
    EXPECT_EQ(daisyhost::ResolveStartupTestInputMode(true, false, 2), 2);
}
} // namespace
