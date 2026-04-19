#include <array>

#include <gtest/gtest.h>

#include "daisyhost/apps/TorusCore.h"

namespace
{
float RenderChecksum(daisyhost::apps::TorusCore& core)
{
    std::array<float, daisyhost::apps::TorusCore::kPreferredBlockSize> input{};
    std::array<float, daisyhost::apps::TorusCore::kPreferredBlockSize> left{};
    std::array<float, daisyhost::apps::TorusCore::kPreferredBlockSize> right{};
    const float* inputChannels[] = {input.data()};
    float* outputChannels[]      = {left.data(), right.data(), nullptr, nullptr};

    for(std::size_t i = 0; i < input.size(); ++i)
    {
        input[i] = (i % 12 == 0) ? 0.6f : 0.0f;
    }

    core.Process({inputChannels, 1}, {outputChannels, 4}, input.size());

    float checksum = 0.0f;
    for(std::size_t i = 0; i < input.size(); ++i)
    {
        checksum += left[i] * 0.37f + right[i] * 0.63f;
    }
    return checksum;
}

float RenderAbsoluteSumWithZeroInput(daisyhost::apps::TorusCore& core)
{
    std::array<float, daisyhost::apps::TorusCore::kPreferredBlockSize> input{};
    std::array<float, daisyhost::apps::TorusCore::kPreferredBlockSize> left{};
    std::array<float, daisyhost::apps::TorusCore::kPreferredBlockSize> right{};
    const float* inputChannels[] = {input.data()};
    float* outputChannels[]      = {left.data(), right.data(), nullptr, nullptr};

    core.Process({inputChannels, 1}, {outputChannels, 4}, input.size());

    float absoluteSum = 0.0f;
    for(std::size_t i = 0; i < input.size(); ++i)
    {
        absoluteSum += std::abs(left[i]) + std::abs(right[i]);
    }
    return absoluteSum;
}

TEST(TorusCoreTest, ExposesStableMetadataParametersAndMenu)
{
    daisyhost::apps::TorusCore core("node0");

    EXPECT_EQ(core.GetAppId(), "torus");
    EXPECT_EQ(core.GetAppDisplayName(), "Torus");

    const auto capabilities = core.GetCapabilities();
    EXPECT_TRUE(capabilities.acceptsAudioInput);
    EXPECT_FALSE(capabilities.acceptsMidiInput);
    EXPECT_FALSE(capabilities.producesMidiOutput);

    const auto& parameters = core.GetParameters();
    ASSERT_GE(parameters.size(), 10u);
    EXPECT_EQ(parameters.front().id, "node0/param/frequency");
    EXPECT_TRUE(parameters.front().stateful);

    const auto& menu = core.GetMenuModel();
    ASSERT_FALSE(menu.sections.empty());
    EXPECT_EQ(menu.sections.front().id, "node0/menu/root");

    const auto* polyModelSection = [&]() -> const daisyhost::MenuSection* {
        for(const auto& section : menu.sections)
        {
            if(section.id == "node0/menu/poly_model")
            {
                return &section;
            }
        }
        return nullptr;
    }();
    ASSERT_NE(polyModelSection, nullptr);
    ASSERT_GE(polyModelSection->items.size(), 3u);
    EXPECT_EQ(polyModelSection->items[0].valueText, "One");
    EXPECT_EQ(polyModelSection->items[1].valueText, "Modal");
    EXPECT_EQ(polyModelSection->items[2].valueText, "Formant");
}

TEST(TorusCoreTest, SupportsDeterministicResetAndRender)
{
    daisyhost::apps::TorusCore first("node0");
    daisyhost::apps::TorusCore second("node0");

    first.Prepare(48000.0, daisyhost::apps::TorusCore::kPreferredBlockSize);
    second.Prepare(48000.0, daisyhost::apps::TorusCore::kPreferredBlockSize);

    first.ResetToDefaultState(1234u);
    second.ResetToDefaultState(1234u);

    ASSERT_TRUE(first.SetParameterValue("node0/param/frequency", 0.42f));
    ASSERT_TRUE(first.SetParameterValue("node0/param/brightness", 0.71f));
    ASSERT_TRUE(first.SetParameterValue("node0/param/model", 0.40f));

    second.RestoreStatefulParameterValues(first.CaptureStatefulParameterValues());

    EXPECT_FLOAT_EQ(RenderChecksum(first), RenderChecksum(second));
}

TEST(TorusCoreTest, StartsSilentWithoutInputOrGateByDefault)
{
    daisyhost::apps::TorusCore core("node0");
    core.Prepare(48000.0, daisyhost::apps::TorusCore::kPreferredBlockSize);
    core.ResetToDefaultState(0u);

    EXPECT_LT(RenderAbsoluteSumWithZeroInput(core), 0.0001f);
}

TEST(TorusCoreTest, DisplayUsesNamedModelAndEffectLabels)
{
    daisyhost::apps::TorusCore core("node0");
    core.Prepare(48000.0, daisyhost::apps::TorusCore::kPreferredBlockSize);
    core.ResetToDefaultState(0u);
    ASSERT_TRUE(core.SetParameterValue("node0/param/model", 1.0f));
    ASSERT_TRUE(core.SetParameterValue("node0/param/easter_fx", 0.4f));
    ASSERT_TRUE(core.SetParameterValue("node0/param/easter_egg", 1.0f));
    core.TickUi(16.0);

    const auto& display = core.GetDisplayModel();
    ASSERT_GE(display.texts.size(), 3u);
    EXPECT_NE(display.texts[1].text.find("Str+Reverb"), std::string::npos);
    EXPECT_NE(display.texts[2].text.find("Reverb"), std::string::npos);
    EXPECT_NE(display.texts[2].text.find("On"), std::string::npos);
}

TEST(TorusCoreTest, MenuModelKeepsFullNamesWhileDisplayCompactsThem)
{
    daisyhost::apps::TorusCore core("node0");
    core.Prepare(48000.0, daisyhost::apps::TorusCore::kPreferredBlockSize);
    core.ResetToDefaultState(0u);
    ASSERT_TRUE(core.SetParameterValue("node0/param/model", 1.0f));
    core.MenuPress();
    core.MenuRotate(1);
    core.MenuPress();
    core.TickUi(16.0);

    const auto& menu = core.GetMenuModel();
    const auto* polyModelSection = [&]() -> const daisyhost::MenuSection* {
        for(const auto& section : menu.sections)
        {
            if(section.id == "node0/menu/poly_model")
            {
                return &section;
            }
        }
        return nullptr;
    }();
    ASSERT_NE(polyModelSection, nullptr);
    EXPECT_EQ(polyModelSection->items[1].valueText, "String and Reverb");

    const auto& display = core.GetDisplayModel();
    bool foundCompactModel = false;
    for(const auto& text : display.texts)
    {
        if(text.text.find("Str+Reverb") != std::string::npos)
        {
            foundCompactModel = true;
            break;
        }
    }
    EXPECT_TRUE(foundCompactModel);
}
} // namespace
