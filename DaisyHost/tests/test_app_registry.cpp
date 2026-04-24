#include <gtest/gtest.h>

#include "daisyhost/AppRegistry.h"

namespace
{
TEST(AppRegistryTest, RegistersMultiDelayTorusCloudSeedBraidsHarmoniqsAndVASynthApps)
{
    const auto& registrations = daisyhost::GetHostedAppRegistrations();

    ASSERT_GE(registrations.size(), 6u);
    EXPECT_EQ(registrations.front().appId, "multidelay");

    bool sawMultiDelay = false;
    bool sawTorus      = false;
    bool sawCloudSeed  = false;
    bool sawBraids     = false;
    bool sawHarmoniqs  = false;
    bool sawVASynth    = false;
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
        if(registration.appId == "braids")
        {
            sawBraids = true;
        }
        if(registration.appId == "harmoniqs")
        {
            sawHarmoniqs = true;
        }
        if(registration.appId == "vasynth")
        {
            sawVASynth = true;
        }
    }

    EXPECT_TRUE(sawMultiDelay);
    EXPECT_TRUE(sawTorus);
    EXPECT_TRUE(sawCloudSeed);
    EXPECT_TRUE(sawBraids);
    EXPECT_TRUE(sawHarmoniqs);
    EXPECT_TRUE(sawVASynth);
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
