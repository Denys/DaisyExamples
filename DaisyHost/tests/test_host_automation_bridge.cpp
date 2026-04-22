#include <gtest/gtest.h>

#include "daisyhost/AppRegistry.h"
#include "daisyhost/HostAutomationBridge.h"

namespace
{
std::vector<daisyhost::ParameterDescriptor> ParametersForApp(const std::string& appId)
{
    auto app = daisyhost::CreateHostedAppCore(appId, "node0");
    EXPECT_NE(app, nullptr);
    return app != nullptr ? app->GetParameters()
                          : std::vector<daisyhost::ParameterDescriptor>{};
}

daisyhost::ParameterDescriptor MakeParameter(const std::string& id,
                                             const std::string& label,
                                             int                importanceRank,
                                             bool               automatable = true)
{
    daisyhost::ParameterDescriptor parameter;
    parameter.id             = id;
    parameter.label          = label;
    parameter.importanceRank = importanceRank;
    parameter.automatable    = automatable;
    return parameter;
}

TEST(HostAutomationBridgeTest, MapsMultiDelayToAllFiveCanonicalDspParameters)
{
    const auto bindings
        = daisyhost::BuildHostAutomationSlotBindings(ParametersForApp("multidelay"));

    ASSERT_EQ(bindings.size(), daisyhost::kHostAutomationSlotCount);
    EXPECT_EQ(bindings[0].slotId, "daisyhost.slot1");
    EXPECT_EQ(bindings[0].slotName, "Param 1");
    EXPECT_TRUE(bindings[0].available);
    EXPECT_EQ(bindings[0].parameterId, "node0/param/dry_wet");
    EXPECT_EQ(bindings[1].parameterId, "node0/param/delay_primary");
    EXPECT_EQ(bindings[2].parameterId, "node0/param/delay_secondary");
    EXPECT_EQ(bindings[3].parameterId, "node0/param/feedback");
    EXPECT_EQ(bindings[4].parameterId, "node0/param/delay_tertiary");
}

TEST(HostAutomationBridgeTest, MapsTorusToItsFirstFiveImportanceRankedPlayableParameters)
{
    const auto bindings
        = daisyhost::BuildHostAutomationSlotBindings(ParametersForApp("torus"));

    ASSERT_EQ(bindings.size(), daisyhost::kHostAutomationSlotCount);
    EXPECT_EQ(bindings[0].parameterId, "node0/param/frequency");
    EXPECT_EQ(bindings[1].parameterId, "node0/param/structure");
    EXPECT_EQ(bindings[2].parameterId, "node0/param/brightness");
    EXPECT_EQ(bindings[3].parameterId, "node0/param/damping");
    EXPECT_EQ(bindings[4].parameterId, "node0/param/position");
}

TEST(HostAutomationBridgeTest, MapsCloudSeedToItsPerformanceFirstAutomationSlots)
{
    const auto bindings
        = daisyhost::BuildHostAutomationSlotBindings(ParametersForApp("cloudseed"));

    ASSERT_EQ(bindings.size(), daisyhost::kHostAutomationSlotCount);
    EXPECT_EQ(bindings[0].parameterId, "node0/param/mix");
    EXPECT_EQ(bindings[1].parameterId, "node0/param/size");
    EXPECT_EQ(bindings[2].parameterId, "node0/param/decay");
    EXPECT_EQ(bindings[3].parameterId, "node0/param/diffusion");
    EXPECT_EQ(bindings[4].parameterId, "node0/param/pre_delay");
}

TEST(HostAutomationBridgeTest, LeavesUnusedSlotsUnavailableAndIgnoresNonAutomatableParameters)
{
    const std::vector<daisyhost::ParameterDescriptor> parameters = {
        MakeParameter("node0/param/alpha", "Alpha", 1),
        MakeParameter("node0/param/skip", "Skip", 0, false),
        MakeParameter("node0/param/bravo", "Bravo", 2),
    };

    const auto bindings = daisyhost::BuildHostAutomationSlotBindings(parameters);

    ASSERT_EQ(bindings.size(), daisyhost::kHostAutomationSlotCount);
    EXPECT_TRUE(bindings[0].available);
    EXPECT_EQ(bindings[0].parameterId, "node0/param/alpha");
    EXPECT_TRUE(bindings[1].available);
    EXPECT_EQ(bindings[1].parameterId, "node0/param/bravo");

    for(std::size_t index = 2; index < bindings.size(); ++index)
    {
        EXPECT_FALSE(bindings[index].available);
        EXPECT_TRUE(bindings[index].parameterId.empty());
    }
}

TEST(HostAutomationBridgeTest, KeepsSlotIdsStableAcrossAppMappings)
{
    const auto multiDelayBindings
        = daisyhost::BuildHostAutomationSlotBindings(ParametersForApp("multidelay"));
    const auto torusBindings
        = daisyhost::BuildHostAutomationSlotBindings(ParametersForApp("torus"));

    ASSERT_EQ(multiDelayBindings.size(), torusBindings.size());
    for(std::size_t index = 0; index < multiDelayBindings.size(); ++index)
    {
        EXPECT_EQ(multiDelayBindings[index].slotId, torusBindings[index].slotId);
        EXPECT_EQ(multiDelayBindings[index].slotName, torusBindings[index].slotName);
    }
}
} // namespace
