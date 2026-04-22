#include <gtest/gtest.h>

#include "daisyhost/AppRegistry.h"

namespace
{
TEST(AppRegistryTest, RegistersMultiDelayTorusAndCloudSeedApps)
{
    const auto& registrations = daisyhost::GetHostedAppRegistrations();

    ASSERT_GE(registrations.size(), 3u);
    EXPECT_EQ(registrations.front().appId, "multidelay");

    bool sawMultiDelay = false;
    bool sawTorus      = false;
    bool sawCloudSeed  = false;
    for(const auto& registration : registrations)
    {
        if(registration.appId == "multidelay")
        {
            sawMultiDelay = true;
        }
        if(registration.appId == "torus")
        {
            sawTorus = true;
        }
        if(registration.appId == "cloudseed")
        {
            sawCloudSeed = true;
        }
    }

    EXPECT_TRUE(sawMultiDelay);
    EXPECT_TRUE(sawTorus);
    EXPECT_TRUE(sawCloudSeed);
}

TEST(AppRegistryTest, FallsBackToDefaultAppForUnknownId)
{
    std::string resolvedAppId;
    auto app = daisyhost::CreateHostedAppCore("definitely-not-an-app", "node0", &resolvedAppId);

    ASSERT_NE(app, nullptr);
    EXPECT_EQ(resolvedAppId, "multidelay");
    EXPECT_EQ(app->GetAppId(), "multidelay");
    EXPECT_EQ(app->GetAppDisplayName(), "Multi Delay");
}
} // namespace
