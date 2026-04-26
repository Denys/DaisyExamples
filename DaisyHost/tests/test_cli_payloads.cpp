#include <string>

#include <gtest/gtest.h>
#include <juce_core/juce_core.h>

#include "daisyhost/CliPayloads.h"
#include "daisyhost/HostModulation.h"

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
} // namespace
