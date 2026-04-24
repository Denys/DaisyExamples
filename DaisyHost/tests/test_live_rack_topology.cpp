#include <gtest/gtest.h>

#include "daisyhost/LiveRackTopology.h"

namespace
{
daisyhost::LiveRackTopologyRoute MakeRoute(const char* sourcePortId,
                                           const char* destPortId)
{
    return {sourcePortId, destPortId};
}

daisyhost::LiveRackTopologyConfig MakeConfig(
    const char*                                       entryNodeId,
    const char*                                       outputNodeId,
    std::initializer_list<daisyhost::LiveRackTopologyRoute> routes)
{
    daisyhost::LiveRackTopologyConfig config;
    config.entryNodeId  = entryNodeId;
    config.outputNodeId = outputNodeId;
    config.routes.assign(routes.begin(), routes.end());
    return config;
}

TEST(LiveRackTopologyTest, ExpandsNodeOnlyPresetsWithoutRoutes)
{
    const auto node0Only = daisyhost::BuildLiveRackTopologyConfig(
        daisyhost::LiveRackTopologyPreset::kNode0Only);
    EXPECT_EQ(node0Only.entryNodeId, "node0");
    EXPECT_EQ(node0Only.outputNodeId, "node0");
    EXPECT_TRUE(node0Only.routes.empty());

    const auto node1Only = daisyhost::BuildLiveRackTopologyConfig(
        daisyhost::LiveRackTopologyPreset::kNode1Only);
    EXPECT_EQ(node1Only.entryNodeId, "node1");
    EXPECT_EQ(node1Only.outputNodeId, "node1");
    EXPECT_TRUE(node1Only.routes.empty());
}

TEST(LiveRackTopologyTest, ExpandsSerialPresetsToCanonicalStereoRoutes)
{
    const auto node0ToNode1 = daisyhost::BuildLiveRackTopologyConfig(
        daisyhost::LiveRackTopologyPreset::kNode0ToNode1);
    EXPECT_EQ(node0ToNode1.entryNodeId, "node0");
    EXPECT_EQ(node0ToNode1.outputNodeId, "node1");
    ASSERT_EQ(node0ToNode1.routes.size(), 2u);
    EXPECT_EQ(node0ToNode1.routes[0].sourcePortId, "node0/port/audio_out_1");
    EXPECT_EQ(node0ToNode1.routes[0].destPortId, "node1/port/audio_in_1");
    EXPECT_EQ(node0ToNode1.routes[1].sourcePortId, "node0/port/audio_out_2");
    EXPECT_EQ(node0ToNode1.routes[1].destPortId, "node1/port/audio_in_2");

    const auto node1ToNode0 = daisyhost::BuildLiveRackTopologyConfig(
        daisyhost::LiveRackTopologyPreset::kNode1ToNode0);
    EXPECT_EQ(node1ToNode0.entryNodeId, "node1");
    EXPECT_EQ(node1ToNode0.outputNodeId, "node0");
    ASSERT_EQ(node1ToNode0.routes.size(), 2u);
    EXPECT_EQ(node1ToNode0.routes[0].sourcePortId, "node1/port/audio_out_1");
    EXPECT_EQ(node1ToNode0.routes[0].destPortId, "node0/port/audio_in_1");
    EXPECT_EQ(node1ToNode0.routes[1].sourcePortId, "node1/port/audio_out_2");
    EXPECT_EQ(node1ToNode0.routes[1].destPortId, "node0/port/audio_in_2");
}

TEST(LiveRackTopologyTest, InfersPresetFromStoredRackState)
{
    daisyhost::LiveRackTopologyPreset preset;

    ASSERT_TRUE(daisyhost::TryInferLiveRackTopologyPreset(
        MakeConfig("node0", "node0", {}),
        &preset));
    EXPECT_EQ(preset, daisyhost::LiveRackTopologyPreset::kNode0Only);

    ASSERT_TRUE(daisyhost::TryInferLiveRackTopologyPreset(
        MakeConfig("node1",
                   "node1",
                   {}),
        &preset));
    EXPECT_EQ(preset, daisyhost::LiveRackTopologyPreset::kNode1Only);

    ASSERT_TRUE(daisyhost::TryInferLiveRackTopologyPreset(
        MakeConfig("node0",
                   "node1",
                   {MakeRoute("node0/port/audio_out_2", "node1/port/audio_in_2"),
                    MakeRoute("node0/port/audio_out_1", "node1/port/audio_in_1")}),
        &preset));
    EXPECT_EQ(preset, daisyhost::LiveRackTopologyPreset::kNode0ToNode1);

    ASSERT_TRUE(daisyhost::TryInferLiveRackTopologyPreset(
        MakeConfig("node1",
                   "node0",
                   {MakeRoute("node1/port/audio_out_2", "node0/port/audio_in_2"),
                    MakeRoute("node1/port/audio_out_1", "node0/port/audio_in_1")}),
        &preset));
    EXPECT_EQ(preset, daisyhost::LiveRackTopologyPreset::kNode1ToNode0);
}

TEST(LiveRackTopologyTest, ValidatesSupportedShapes)
{
    std::string errorMessage;
    EXPECT_TRUE(daisyhost::ValidateLiveRackTopologyConfig(
        MakeConfig("node0", "node0", {}),
        &errorMessage))
        << errorMessage;
    EXPECT_TRUE(daisyhost::ValidateLiveRackTopologyConfig(
        MakeConfig("node0",
                   "node1",
                   {MakeRoute("node0/port/audio_out_1", "node1/port/audio_in_1"),
                    MakeRoute("node0/port/audio_out_2", "node1/port/audio_in_2")}),
        &errorMessage))
        << errorMessage;
}

TEST(LiveRackTopologyTest, RejectsUnknownNodeIds)
{
    std::string errorMessage;
    EXPECT_FALSE(daisyhost::ValidateLiveRackTopologyConfig(
        MakeConfig("node2", "node2", {}),
        &errorMessage));
    EXPECT_NE(errorMessage.find("node0"), std::string::npos);
    EXPECT_NE(errorMessage.find("node1"), std::string::npos);
}

TEST(LiveRackTopologyTest, RejectsPartialStereoRouteSets)
{
    std::string errorMessage;
    EXPECT_FALSE(daisyhost::ValidateLiveRackTopologyConfig(
        MakeConfig("node0",
                   "node1",
                   {MakeRoute("node0/port/audio_out_1", "node1/port/audio_in_1")}),
        &errorMessage));
    EXPECT_NE(errorMessage.find("exactly two"), std::string::npos);
}

TEST(LiveRackTopologyTest, RejectsNonAudioRoutes)
{
    std::string errorMessage;
    EXPECT_FALSE(daisyhost::ValidateLiveRackTopologyConfig(
        MakeConfig("node0",
                   "node1",
                   {MakeRoute("node0/port/audio_out_1", "node1/port/gate_in_1"),
                    MakeRoute("node0/port/audio_out_2", "node1/port/audio_in_2")}),
        &errorMessage));
    EXPECT_NE(errorMessage.find("audio routes only"), std::string::npos);
}

TEST(LiveRackTopologyTest, RejectsShapesThatDoNotMatchSprintPresets)
{
    std::string errorMessage;
    EXPECT_FALSE(daisyhost::ValidateLiveRackTopologyConfig(
        MakeConfig("node0", "node1", {}),
        &errorMessage));
    EXPECT_NE(errorMessage.find("unsupported"), std::string::npos);

    errorMessage.clear();
    EXPECT_FALSE(daisyhost::TryInferLiveRackTopologyPreset(
        MakeConfig("node0",
                   "node1",
                   {MakeRoute("node1/port/audio_out_1", "node0/port/audio_in_1"),
                    MakeRoute("node1/port/audio_out_2", "node0/port/audio_in_2")}),
        nullptr,
        &errorMessage));
    EXPECT_NE(errorMessage.find("unsupported"), std::string::npos);
}
} // namespace
