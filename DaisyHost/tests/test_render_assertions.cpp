#include <gtest/gtest.h>

#include "daisyhost/RenderAssertions.h"

namespace
{
daisyhost::RenderResultManifest MakeManifest()
{
    daisyhost::RenderResultManifest manifest;
    manifest.audioChecksum = "abc123";
    manifest.channelSummaries.push_back({0.25f, 0.10f});
    manifest.routes.push_back({"node0/port/audio_out_1", "node1/port/audio_in_1"});

    daisyhost::RenderNodeResultSummary node0;
    node0.nodeId = "node0";
    manifest.nodes.push_back(node0);

    daisyhost::RenderNodeResultSummary node1;
    node1.nodeId = "node1";
    manifest.nodes.push_back(node1);

    daisyhost::RenderTimelineEvent event;
    event.targetNodeId = "node1";
    manifest.executedTimeline.push_back(event);

    return manifest;
}
} // namespace

TEST(RenderAssertionsTest, PassesWhenRequestedRenderEvidenceMatches)
{
    const auto manifest = MakeManifest();

    daisyhost::cli::RenderAssertionOptions options;
    options.expectedChecksum = "abc123";
    options.expectNonSilent = true;
    options.expectedRouteCount = 1;
    options.expectedNodeIds.push_back("node0");
    options.expectedNodeIds.push_back("node1");
    options.expectedTimelineTargetNodeIds.push_back("node1");

    const auto report = daisyhost::cli::EvaluateRenderAssertions(manifest, options);

    EXPECT_TRUE(report.passed);
    ASSERT_EQ(report.results.size(), 6u);
    for(const auto& result : report.results)
    {
        EXPECT_TRUE(result.passed) << result.message;
    }
}

TEST(RenderAssertionsTest, ReportsEveryFailedRenderEvidenceAssertion)
{
    const auto manifest = MakeManifest();

    daisyhost::cli::RenderAssertionOptions options;
    options.expectedChecksum = "different";
    options.expectNonSilent = true;
    options.expectedRouteCount = 2;
    options.expectedNodeIds.push_back("node9");
    options.expectedTimelineTargetNodeIds.push_back("node0");

    auto silentManifest = manifest;
    silentManifest.channelSummaries.clear();

    const auto report = daisyhost::cli::EvaluateRenderAssertions(silentManifest, options);

    EXPECT_FALSE(report.passed);
    ASSERT_EQ(report.results.size(), 5u);
    for(const auto& result : report.results)
    {
        EXPECT_FALSE(result.passed);
        EXPECT_FALSE(result.message.empty());
    }
}
