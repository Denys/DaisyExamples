#include <string>

#include <gtest/gtest.h>
#include <juce_core/juce_core.h>

#include "daisyhost/CliPayloads.h"

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
} // namespace
