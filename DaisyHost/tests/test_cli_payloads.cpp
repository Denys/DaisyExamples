#include <string>

#include <gtest/gtest.h>
#include <juce_core/juce_core.h>

#include "daisyhost/CliPayloads.h"
#include "daisyhost/HostModulation.h"
#include "daisyhost/RenderAssertions.h"
#include "daisyhost/RenderTypes.h"

namespace
{
juce::var ParseJson(const std::string& json)
{
    return juce::JSON::parse(juce::String(json));
}

bool ArrayContainsObjectWithStringProperty(const juce::Array<juce::var>& values,
                                           const juce::String& propertyName,
                                           const juce::String& expectedValue)
{
    for(const auto& value : values)
    {
        if(auto* object = value.getDynamicObject())
        {
            if(object->getProperty(propertyName).toString() == expectedValue)
            {
                return true;
            }
        }
    }
    return false;
}

TEST(CliPayloadsTest, ListsHostedAppsAsJson)
{
    const auto json = daisyhost::cli::SerializeAppsPayloadJson();

    auto parsed = ParseJson(json);
    auto* root = parsed.getDynamicObject();
    ASSERT_NE(root, nullptr);
    auto* apps = root->getProperty("apps").getArray();
    ASSERT_NE(apps, nullptr);
    EXPECT_TRUE(
        ArrayContainsObjectWithStringProperty(*apps, "appId", "multidelay"));
    EXPECT_TRUE(
        ArrayContainsObjectWithStringProperty(*apps, "appId", "subharmoniq"));
}

TEST(CliPayloadsTest, DescribesCloudSeedAppWithoutFallingBackForUnknownIds)
{
    std::string json;
    std::string error;
    ASSERT_TRUE(daisyhost::cli::SerializeAppDescriptionPayloadJson(
        "cloudseed", &json, &error))
        << error;

    auto parsed = ParseJson(json);
    auto* root = parsed.getDynamicObject();
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->getProperty("appId").toString(), "cloudseed");
    EXPECT_EQ(root->getProperty("displayName").toString(), "CloudSeed");

    auto* parameters = root->getProperty("parameters").getArray();
    ASSERT_NE(parameters, nullptr);
    EXPECT_GT(parameters->size(), 0);

    auto* menuSections = root->getProperty("menuSections").getArray();
    ASSERT_NE(menuSections, nullptr);
    EXPECT_GT(menuSections->size(), 0);

    json.clear();
    error.clear();
    EXPECT_FALSE(daisyhost::cli::SerializeAppDescriptionPayloadJson(
        "not-a-real-app", &json, &error));
    EXPECT_TRUE(json.empty());
    EXPECT_NE(error.find("Unknown app: not-a-real-app"), std::string::npos);
}

TEST(CliPayloadsTest, DescribesDaisyFieldBoardAsJson)
{
    std::string json;
    std::string error;
    ASSERT_TRUE(daisyhost::cli::SerializeBoardDescriptionPayloadJson(
        "daisy_field", &json, &error))
        << error;

    auto parsed = ParseJson(json);
    auto* root = parsed.getDynamicObject();
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->getProperty("boardId").toString(), "daisy_field");
    EXPECT_EQ(root->getProperty("displayName").toString(), "Daisy Field");

    auto* surfaceControls = root->getProperty("surfaceControls").getArray();
    ASSERT_NE(surfaceControls, nullptr);
    EXPECT_GT(surfaceControls->size(), 0);

    auto* indicators = root->getProperty("indicators").getArray();
    ASSERT_NE(indicators, nullptr);
    EXPECT_GT(indicators->size(), 0);
}

TEST(CliPayloadsTest, ListsScenarioInputModesAsJson)
{
    const auto json = daisyhost::cli::SerializeInputsPayloadJson();

    auto parsed = ParseJson(json);
    auto* root = parsed.getDynamicObject();
    ASSERT_NE(root, nullptr);
    auto* inputs = root->getProperty("inputs").getArray();
    ASSERT_NE(inputs, nullptr);
    EXPECT_TRUE(ArrayContainsObjectWithStringProperty(*inputs, "id", "host_in"));
    EXPECT_TRUE(ArrayContainsObjectWithStringProperty(*inputs, "id", "saw"));
    EXPECT_TRUE(ArrayContainsObjectWithStringProperty(*inputs, "id", "impulse"));
}

TEST(CliPayloadsTest, SerializesModulationReadbackInSnapshots)
{
    daisyhost::EffectiveHostStateSnapshot snapshot;
    snapshot.boardId = "daisy_field";
    snapshot.selectedNodeId = "node0";
    snapshot.selectedModulationDestinationId = "node0/param/mix";

    daisyhost::EffectiveHostModulationDestinationSnapshot destination;
    destination.nodeId = "node0";
    destination.parameterId = "node0/param/mix";
    destination.parameterLabel = "Mix";
    destination.unitLabel = "%";
    destination.baseNativeValue = 50.0f;
    destination.resultNativeValue = 72.0f;
    destination.resultNormalizedValue = 0.72f;
    destination.clamped = false;
    destination.lanes.push_back({0,
                                 true,
                                 daisyhost::HostModulationSource::kLfo2,
                                 0.0f,
                                 1.0f,
                                 25.0f,
                                 -0.4f,
                                 -10.0f});
    snapshot.modulationDestinations.push_back(destination);

    auto parsed = ParseJson(daisyhost::cli::SerializeSnapshotPayloadJson(snapshot));
    auto* root = parsed.getDynamicObject();
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->getProperty("selectedModulationDestinationId").toString(),
              "node0/param/mix");
    auto* destinations
        = root->getProperty("modulationDestinations").getArray();
    ASSERT_NE(destinations, nullptr);
    ASSERT_EQ(destinations->size(), 1);
    auto* destinationObject = (*destinations)[0].getDynamicObject();
    ASSERT_NE(destinationObject, nullptr);
    EXPECT_EQ(destinationObject->getProperty("parameterId").toString(),
              "node0/param/mix");
    EXPECT_DOUBLE_EQ(static_cast<double>(
                         destinationObject->getProperty("baseNativeValue")),
                     50.0);

    auto* lanes = destinationObject->getProperty("lanes").getArray();
    ASSERT_NE(lanes, nullptr);
    ASSERT_EQ(lanes->size(), 1);
    auto* laneObject = (*lanes)[0].getDynamicObject();
    ASSERT_NE(laneObject, nullptr);
    EXPECT_EQ(laneObject->getProperty("source").toString(), "lfo2");
    EXPECT_DOUBLE_EQ(static_cast<double>(
                         laneObject->getProperty("nativeContribution")),
                     -10.0);
}

TEST(CliPayloadsTest, SnapshotPayloadIncludesDebugStateNodeRolesAndTargets)
{
    daisyhost::EffectiveHostStateSnapshot snapshot;
    snapshot.boardId        = "daisy_field";
    snapshot.selectedNodeId = "node0";
    snapshot.entryNodeId    = "node0";
    snapshot.outputNodeId   = "node0";
    snapshot.nodeCount      = 1u;
    snapshot.nodeSummaries.push_back(
        {"node0", "multidelay", "Multi Delay", true, true, true});

    auto parsed = ParseJson(daisyhost::cli::SerializeSnapshotPayloadJson(snapshot));
    auto* root = parsed.getDynamicObject();
    ASSERT_NE(root, nullptr);

    auto* debugState = root->getProperty("debugState").getDynamicObject();
    ASSERT_NE(debugState, nullptr);
    EXPECT_EQ(debugState->getProperty("boardId").toString(), "daisy_field");
    EXPECT_EQ(debugState->getProperty("selectedNodeId").toString(), "node0");

    auto* nodes = debugState->getProperty("nodes").getArray();
    ASSERT_NE(nodes, nullptr);
    ASSERT_EQ(nodes->size(), 1);
    auto* node = (*nodes)[0].getDynamicObject();
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->getProperty("nodeId").toString(), "node0");
    EXPECT_EQ(node->getProperty("roleLabel").toString(),
              "Selected - Entry + output");

    auto* controlTargets
        = debugState->getProperty("controlTargets").getDynamicObject();
    ASSERT_NE(controlTargets, nullptr);
    EXPECT_EQ(controlTargets->getProperty("liveControlsTargetNodeId").toString(),
              "node0");
    EXPECT_EQ(controlTargets->getProperty("cvGateTargetNodeId").toString(),
              "node0");
    EXPECT_EQ(controlTargets->getProperty("modulationTargetNodeId").toString(),
              "node0");
    EXPECT_EQ(controlTargets->getProperty("fieldSurfaceTargetNodeId").toString(),
              "node0");
}

TEST(CliPayloadsTest, RenderPayloadIncludesNodeDebugReadback)
{
    daisyhost::RenderResultManifest manifest;
    manifest.appId          = "multidelay";
    manifest.boardId        = "daisy_patch";
    manifest.selectedNodeId = "node1";
    manifest.entryNodeId    = "node0";
    manifest.outputNodeId   = "node0";
    manifest.audioChecksum  = "abcdef";

    daisyhost::RenderNodeResultSummary node0;
    node0.nodeId         = "node0";
    node0.appId          = "multidelay";
    node0.appDisplayName = "Multi Delay";
    manifest.nodes.push_back(node0);

    daisyhost::RenderNodeResultSummary node1;
    node1.nodeId         = "node1";
    node1.appId          = "cloudseed";
    node1.appDisplayName = "CloudSeed";
    manifest.nodes.push_back(node1);

    manifest.routes.push_back({"node0/port/audio_out_1", "node1/port/audio_in_1"});

    daisyhost::RenderTimelineEvent event;
    event.timeSeconds     = 0.125;
    event.type            = daisyhost::RenderTimelineEventType::kParameterSet;
    event.targetNodeId    = "node1";
    event.parameterId     = "node1/param/mix";
    event.normalizedValue = 0.70f;
    manifest.executedTimeline.push_back(event);

    auto parsed = ParseJson(daisyhost::cli::SerializeRenderResultPayloadJson(manifest));
    auto* root = parsed.getDynamicObject();
    ASSERT_NE(root, nullptr);

    auto* nodes = root->getProperty("nodes").getArray();
    ASSERT_NE(nodes, nullptr);
    ASSERT_EQ(nodes->size(), 2);
    EXPECT_EQ((*nodes)[1].getDynamicObject()->getProperty("nodeId").toString(),
              "node1");

    auto* routes = root->getProperty("routes").getArray();
    ASSERT_NE(routes, nullptr);
    ASSERT_EQ(routes->size(), 1);

    auto* timeline = root->getProperty("executedTimeline").getArray();
    ASSERT_NE(timeline, nullptr);
    ASSERT_EQ(timeline->size(), 1);
    auto* eventObject = (*timeline)[0].getDynamicObject();
    ASSERT_NE(eventObject, nullptr);
    EXPECT_EQ(eventObject->getProperty("targetNodeId").toString(), "node1");
    EXPECT_EQ(eventObject->getProperty("parameterId").toString(),
              "node1/param/mix");
}

TEST(CliPayloadsTest, RenderPayloadIncludesDebugStateForRackReadback)
{
    daisyhost::RenderResultManifest manifest;
    manifest.appId          = "multidelay";
    manifest.boardId        = "daisy_patch";
    manifest.selectedNodeId = "node1";
    manifest.entryNodeId    = "node0";
    manifest.outputNodeId   = "node1";
    manifest.audioChecksum  = "abcdef";

    daisyhost::RenderNodeResultSummary node0;
    node0.nodeId         = "node0";
    node0.appId          = "multidelay";
    node0.appDisplayName = "Multi Delay";
    manifest.nodes.push_back(node0);

    daisyhost::RenderNodeResultSummary node1;
    node1.nodeId         = "node1";
    node1.appId          = "cloudseed";
    node1.appDisplayName = "CloudSeed";
    manifest.nodes.push_back(node1);

    manifest.routes.push_back({"node0/port/audio_out_1", "node1/port/audio_in_1"});
    manifest.routes.push_back({"node0/port/audio_out_2", "node1/port/audio_in_2"});

    daisyhost::RenderTimelineEvent resolvedEvent;
    resolvedEvent.type         = daisyhost::RenderTimelineEventType::kMidi;
    resolvedEvent.targetNodeId = "node1";
    manifest.executedTimeline.push_back(resolvedEvent);

    daisyhost::RenderTimelineEvent unresolvedEvent;
    unresolvedEvent.type = daisyhost::RenderTimelineEventType::kAudioInputConfig;
    manifest.executedTimeline.push_back(unresolvedEvent);

    auto parsed = ParseJson(daisyhost::cli::SerializeRenderResultPayloadJson(manifest));
    auto* root = parsed.getDynamicObject();
    ASSERT_NE(root, nullptr);

    auto* debugState = root->getProperty("debugState").getDynamicObject();
    ASSERT_NE(debugState, nullptr);

    auto* nodes = debugState->getProperty("nodes").getArray();
    ASSERT_NE(nodes, nullptr);
    ASSERT_EQ(nodes->size(), 2);
    auto* entryNode = (*nodes)[0].getDynamicObject();
    auto* outputNode = (*nodes)[1].getDynamicObject();
    ASSERT_NE(entryNode, nullptr);
    ASSERT_NE(outputNode, nullptr);
    EXPECT_EQ(entryNode->getProperty("roleLabel").toString(), "Audio entry");
    EXPECT_EQ(outputNode->getProperty("roleLabel").toString(),
              "Selected - Audio output");

    auto* routes = debugState->getProperty("routes").getArray();
    ASSERT_NE(routes, nullptr);
    ASSERT_EQ(routes->size(), 2);

    auto* timeline = debugState->getProperty("timeline").getDynamicObject();
    ASSERT_NE(timeline, nullptr);
    EXPECT_EQ(static_cast<int>(timeline->getProperty("executedEventCount")), 2);
    EXPECT_EQ(static_cast<int>(timeline->getProperty("resolvedTargetEventCount")),
              1);
}

TEST(CliPayloadsTest, RenderPayloadCanIncludeAssertionReport)
{
    daisyhost::RenderResultManifest manifest;
    manifest.appId         = "multidelay";
    manifest.audioChecksum = "abcdef";

    daisyhost::cli::RenderAssertionReport report;
    report.passed = false;
    report.results.push_back({"checksum", "expected", "actual", false, "checksum mismatch"});

    auto parsed = ParseJson(
        daisyhost::cli::SerializeRenderResultPayloadJson(manifest, &report));
    auto* root = parsed.getDynamicObject();
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->getProperty("audioChecksum").toString(), "abcdef");

    auto* assertions = root->getProperty("assertions").getDynamicObject();
    ASSERT_NE(assertions, nullptr);
    EXPECT_FALSE(static_cast<bool>(assertions->getProperty("passed")));

    auto* results = assertions->getProperty("results").getArray();
    ASSERT_NE(results, nullptr);
    ASSERT_EQ(results->size(), 1);
    auto* result = (*results)[0].getDynamicObject();
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->getProperty("id").toString(), "checksum");
    EXPECT_EQ(result->getProperty("message").toString(), "checksum mismatch");
}
} // namespace
