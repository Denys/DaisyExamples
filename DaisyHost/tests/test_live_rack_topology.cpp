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

TEST(LiveRackTopologyTest, BuildsRoutePlansForAllCurrentPresets)
{
    daisyhost::LiveRackRoutePlan plan;
    std::string                  errorMessage;

    ASSERT_TRUE(daisyhost::TryBuildLiveRackRoutePlan(
        daisyhost::BuildLiveRackTopologyConfig(
            daisyhost::LiveRackTopologyPreset::kNode0Only),
        &plan,
        &errorMessage))
        << errorMessage;
    ASSERT_EQ(plan.processingOrder.size(), 1u);
    EXPECT_EQ(plan.processingOrder[0], "node0");
    EXPECT_TRUE(plan.audioRoutes.empty());

    ASSERT_TRUE(daisyhost::TryBuildLiveRackRoutePlan(
        daisyhost::BuildLiveRackTopologyConfig(
            daisyhost::LiveRackTopologyPreset::kNode1Only),
        &plan,
        &errorMessage))
        << errorMessage;
    ASSERT_EQ(plan.processingOrder.size(), 1u);
    EXPECT_EQ(plan.processingOrder[0], "node1");
    EXPECT_TRUE(plan.audioRoutes.empty());

    ASSERT_TRUE(daisyhost::TryBuildLiveRackRoutePlan(
        daisyhost::BuildLiveRackTopologyConfig(
            daisyhost::LiveRackTopologyPreset::kNode0ToNode1),
        &plan,
        &errorMessage))
        << errorMessage;
    ASSERT_EQ(plan.processingOrder.size(), 2u);
    EXPECT_EQ(plan.processingOrder[0], "node0");
    EXPECT_EQ(plan.processingOrder[1], "node1");
    ASSERT_EQ(plan.audioRoutes.size(), 2u);
    EXPECT_EQ(plan.audioRoutes[0].source.nodeId, "node0");
    EXPECT_EQ(plan.audioRoutes[0].source.portId, "node0/port/audio_out_1");
    EXPECT_EQ(plan.audioRoutes[0].source.channelIndex, 0u);
    EXPECT_EQ(plan.audioRoutes[0].destination.nodeId, "node1");
    EXPECT_EQ(plan.audioRoutes[0].destination.portId, "node1/port/audio_in_1");
    EXPECT_EQ(plan.audioRoutes[0].destination.channelIndex, 0u);
    EXPECT_EQ(plan.audioRoutes[1].source.channelIndex, 1u);
    EXPECT_EQ(plan.audioRoutes[1].destination.channelIndex, 1u);

    ASSERT_TRUE(daisyhost::TryBuildLiveRackRoutePlan(
        daisyhost::BuildLiveRackTopologyConfig(
            daisyhost::LiveRackTopologyPreset::kNode1ToNode0),
        &plan,
        &errorMessage))
        << errorMessage;
    ASSERT_EQ(plan.processingOrder.size(), 2u);
    EXPECT_EQ(plan.processingOrder[0], "node1");
    EXPECT_EQ(plan.processingOrder[1], "node0");
    ASSERT_EQ(plan.audioRoutes.size(), 2u);
    EXPECT_EQ(plan.audioRoutes[0].source.nodeId, "node1");
    EXPECT_EQ(plan.audioRoutes[0].destination.nodeId, "node0");
}

TEST(LiveRackTopologyTest, SortsRoutePlansByDestinationChannel)
{
    daisyhost::LiveRackRoutePlan plan;
    std::string                  errorMessage;

    ASSERT_TRUE(daisyhost::TryBuildLiveRackRoutePlan(
        MakeConfig("node0",
                   "node1",
                   {MakeRoute("node0/port/audio_out_2", "node1/port/audio_in_2"),
                    MakeRoute("node0/port/audio_out_1", "node1/port/audio_in_1")}),
        &plan,
        &errorMessage))
        << errorMessage;

    ASSERT_EQ(plan.audioRoutes.size(), 2u);
    EXPECT_EQ(plan.audioRoutes[0].destination.channelIndex, 0u);
    EXPECT_EQ(plan.audioRoutes[1].destination.channelIndex, 1u);
}

TEST(LiveRackTopologyTest, SupportsValidationOnlyRoutePlanBuilds)
{
    std::string errorMessage;
    EXPECT_TRUE(daisyhost::TryBuildLiveRackRoutePlan(
        daisyhost::BuildLiveRackTopologyConfig(
            daisyhost::LiveRackTopologyPreset::kNode0ToNode1),
        nullptr,
        &errorMessage))
        << errorMessage;
}

TEST(LiveRackTopologyTest, CopiesAcceptedConfigIntoRoutePlan)
{
    daisyhost::LiveRackRoutePlan plan;
    std::string                  errorMessage;
    const auto                   config = MakeConfig(
        "node0",
        "node1",
        {MakeRoute("node0/port/audio_out_2", "node1/port/audio_in_2"),
         MakeRoute("node0/port/audio_out_1", "node1/port/audio_in_1")});

    ASSERT_TRUE(daisyhost::TryBuildLiveRackRoutePlan(config, &plan, &errorMessage))
        << errorMessage;

    EXPECT_EQ(plan.config.entryNodeId, config.entryNodeId);
    EXPECT_EQ(plan.config.outputNodeId, config.outputNodeId);
    ASSERT_EQ(plan.config.routes.size(), config.routes.size());
    EXPECT_EQ(plan.config.routes[0].sourcePortId, config.routes[0].sourcePortId);
    EXPECT_EQ(plan.config.routes[0].destPortId, config.routes[0].destPortId);
    EXPECT_EQ(plan.config.routes[1].sourcePortId, config.routes[1].sourcePortId);
    EXPECT_EQ(plan.config.routes[1].destPortId, config.routes[1].destPortId);
}

TEST(LiveRackTopologyTest, LeavesExistingRoutePlanUnchangedOnFailure)
{
    daisyhost::LiveRackRoutePlan plan;
    std::string                  errorMessage;
    ASSERT_TRUE(daisyhost::TryBuildLiveRackRoutePlan(
        daisyhost::BuildLiveRackTopologyConfig(
            daisyhost::LiveRackTopologyPreset::kNode0Only),
        &plan,
        &errorMessage))
        << errorMessage;

    const auto originalConfigEntry = plan.config.entryNodeId;
    const auto originalConfigOutput = plan.config.outputNodeId;
    const auto originalOrder = plan.processingOrder;
    const auto originalRouteCount = plan.audioRoutes.size();

    EXPECT_FALSE(daisyhost::TryBuildLiveRackRoutePlan(
        MakeConfig("node0",
                   "node1",
                   {MakeRoute("node0/port/audio_out_1", "node1/port/audio_in_2"),
                    MakeRoute("node0/port/audio_out_2", "node1/port/audio_in_1")}),
        &plan,
        &errorMessage));

    EXPECT_EQ(plan.config.entryNodeId, originalConfigEntry);
    EXPECT_EQ(plan.config.outputNodeId, originalConfigOutput);
    EXPECT_EQ(plan.processingOrder, originalOrder);
    EXPECT_EQ(plan.audioRoutes.size(), originalRouteCount);
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

TEST(LiveRackTopologyTest, RejectsDuplicateRoutePlans)
{
    daisyhost::LiveRackRoutePlan plan;
    std::string                  errorMessage;
    EXPECT_FALSE(daisyhost::TryBuildLiveRackRoutePlan(
        MakeConfig("node0",
                   "node1",
                   {MakeRoute("node0/port/audio_out_1", "node1/port/audio_in_1"),
                    MakeRoute("node0/port/audio_out_1", "node1/port/audio_in_1")}),
        &plan,
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

TEST(LiveRackTopologyTest, RejectsCrossChannelAndSameNodeRoutePlans)
{
    daisyhost::LiveRackRoutePlan plan;
    std::string                  errorMessage;

    EXPECT_FALSE(daisyhost::TryBuildLiveRackRoutePlan(
        MakeConfig("node0",
                   "node1",
                   {MakeRoute("node0/port/audio_out_1", "node1/port/audio_in_2"),
                    MakeRoute("node0/port/audio_out_2", "node1/port/audio_in_1")}),
        &plan,
        &errorMessage));
    EXPECT_NE(errorMessage.find("unsupported"), std::string::npos);

    errorMessage.clear();
    EXPECT_FALSE(daisyhost::TryBuildLiveRackRoutePlan(
        MakeConfig("node0",
                   "node1",
                   {MakeRoute("node0/port/audio_out_1", "node0/port/audio_in_1"),
                    MakeRoute("node0/port/audio_out_2", "node1/port/audio_in_2")}),
        &plan,
        &errorMessage));
    EXPECT_NE(errorMessage.find("unsupported"), std::string::npos);
}

TEST(LiveRackTopologyTest, RejectsShapesOutsideCurrentTwoNodeAudioContract)
{
    std::string errorMessage;
    EXPECT_FALSE(daisyhost::ValidateLiveRackTopologyConfig(
        MakeConfig("node0", "node1", {}),
        &errorMessage));
    EXPECT_NE(errorMessage.find("current two-node audio contract"),
              std::string::npos);
    EXPECT_EQ(errorMessage.find("sprint"), std::string::npos);

    errorMessage.clear();
    EXPECT_FALSE(daisyhost::TryInferLiveRackTopologyPreset(
        MakeConfig("node0",
                   "node1",
                   {MakeRoute("node1/port/audio_out_1", "node0/port/audio_in_1"),
                    MakeRoute("node1/port/audio_out_2", "node0/port/audio_in_2")}),
        nullptr,
        &errorMessage));
    EXPECT_NE(errorMessage.find("current two-node audio contract"),
              std::string::npos);
    EXPECT_EQ(errorMessage.find("sprint"), std::string::npos);
}

TEST(LiveRackTopologyTest, DescribesTopologyPresetsForOperatorUi)
{
    EXPECT_EQ(daisyhost::GetLiveRackTopologyDisplayLabel(
                  daisyhost::LiveRackTopologyPreset::kNode0Only),
              "Node 0 only");
    EXPECT_EQ(daisyhost::GetLiveRackTopologyDisplayLabel(
                  daisyhost::LiveRackTopologyPreset::kNode1Only),
              "Node 1 only");
    EXPECT_EQ(daisyhost::GetLiveRackTopologyDisplayLabel(
                  daisyhost::LiveRackTopologyPreset::kNode0ToNode1),
              "Node 0 feeds Node 1");
    EXPECT_EQ(daisyhost::GetLiveRackTopologyDisplayLabel(
                  daisyhost::LiveRackTopologyPreset::kNode1ToNode0),
              "Node 1 feeds Node 0");
}

TEST(LiveRackTopologyTest, DescribesNodeRolesWithSelectedState)
{
    EXPECT_EQ(daisyhost::GetLiveRackNodeRoleDisplayLabel(
                  daisyhost::LiveRackTopologyPreset::kNode0Only, "node0", true),
              "Selected - Entry + output");
    EXPECT_EQ(daisyhost::GetLiveRackNodeRoleDisplayLabel(
                  daisyhost::LiveRackTopologyPreset::kNode0Only, "node1", false),
              "Not in route");

    EXPECT_EQ(daisyhost::GetLiveRackNodeRoleDisplayLabel(
                  daisyhost::LiveRackTopologyPreset::kNode0ToNode1,
                  "node0",
                  true),
              "Selected - Audio entry");
    EXPECT_EQ(daisyhost::GetLiveRackNodeRoleDisplayLabel(
                  daisyhost::LiveRackTopologyPreset::kNode0ToNode1,
                  "node1",
                  false),
              "Audio output");

    EXPECT_EQ(daisyhost::GetLiveRackNodeRoleDisplayLabel(
                  daisyhost::LiveRackTopologyPreset::kNode1ToNode0,
                  "node0",
                  false),
              "Audio output");
    EXPECT_EQ(daisyhost::GetLiveRackNodeRoleDisplayLabel(
                  daisyhost::LiveRackTopologyPreset::kNode1ToNode0,
                  "node1",
                  true),
              "Selected - Audio entry");
}
} // namespace
