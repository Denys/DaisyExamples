#include <gtest/gtest.h>

#include "daisyhost/VersionInfo.h"

namespace
{
TEST(VersionInfoTest, ExposesConfiguredVersionAndHighlights)
{
    const auto& info = daisyhost::GetVersionInfo();

    EXPECT_EQ(info.version, "0.2.0");
    EXPECT_FALSE(info.buildIdentity.empty());
    EXPECT_FALSE(info.releaseHighlights.empty());
}
} // namespace
