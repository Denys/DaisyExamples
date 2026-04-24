#include <array>
#include <cmath>

#include <gtest/gtest.h>

#include "daisyhost/apps/BraidsCore.h"

namespace
{
const daisyhost::MenuSection* FindSection(const daisyhost::MenuModel& menu,
                                          const char*                 sectionId)
{
    for(const auto& section : menu.sections)
    {
        if(section.id == sectionId)
        {
            return &section;
        }
    }
    return nullptr;
}

TEST(BraidsCoreTest, ExposesMetadataCapabilitiesAndDefaultDrumPage)
{
    daisyhost::apps::BraidsCore core("node0");

    EXPECT_EQ(core.GetAppId(), "braids");
    EXPECT_EQ(core.GetAppDisplayName(), "Braids");

    const auto capabilities = core.GetCapabilities();
    EXPECT_FALSE(capabilities.acceptsAudioInput);
    EXPECT_TRUE(capabilities.acceptsMidiInput);
    EXPECT_FALSE(capabilities.producesMidiOutput);

    const auto bindings = core.GetPatchBindings();
    EXPECT_EQ(bindings.knobDetailLabels[0], "Tune");
    EXPECT_EQ(bindings.knobDetailLabels[1], "Timbre");
    EXPECT_EQ(bindings.knobDetailLabels[2], "Color");
    EXPECT_EQ(bindings.knobDetailLabels[3], "Model");
    EXPECT_EQ(bindings.gateInputPortIds[0], "node0/port/gate_in_1");
    EXPECT_EQ(bindings.midiInputPortId, "node0/port/midi_in_1");
    EXPECT_EQ(bindings.audioOutputPortIds[0], "node0/port/audio_out_1");
    EXPECT_EQ(bindings.audioOutputPortIds[1], "node0/port/audio_out_2");

    const auto& menu = core.GetMenuModel();
    ASSERT_FALSE(menu.sections.empty());
    EXPECT_NE(FindSection(menu, "node0/menu/pages"), nullptr);
    EXPECT_NE(FindSection(menu, "node0/menu/model"), nullptr);
    EXPECT_NE(FindSection(menu, "node0/menu/utilities"), nullptr);
    EXPECT_NE(FindSection(menu, "node0/menu/info"), nullptr);
}

TEST(BraidsCoreTest, PageSwitchUpdatesKnobLabelsAndControlRouting)
{
    daisyhost::apps::BraidsCore core("node0");
    core.Prepare(48000.0, 48);
    core.ResetToDefaultState(7u);

    core.SetMenuItemValue("node0/menu/pages/page", 1.0f);
    const auto bindings = core.GetPatchBindings();
    EXPECT_EQ(bindings.knobDetailLabels[0], "Resolution");
    EXPECT_EQ(bindings.knobDetailLabels[1], "Rate");
    EXPECT_EQ(bindings.knobDetailLabels[2], "Signature");
    EXPECT_EQ(bindings.knobDetailLabels[3], "Level");

    core.SetControl(bindings.knobControlIds[2], 0.75f);
    const auto signature = core.GetParameterValue("node0/param/signature");
    const auto timbre    = core.GetParameterValue("node0/param/timbre");
    ASSERT_TRUE(signature.hasValue);
    ASSERT_TRUE(timbre.hasValue);
    EXPECT_NEAR(signature.value, 0.75f, 0.0001f);
    EXPECT_NE(signature.value, timbre.value);
}

TEST(BraidsCoreTest, UtilitiesAndTriggerInputsRemainVisible)
{
    daisyhost::apps::BraidsCore core("node0");
    core.Prepare(48000.0, 48);
    core.ResetToDefaultState(13u);

    daisyhost::PortValue midiValue;
    midiValue.type = daisyhost::VirtualPortType::kMidi;
    midiValue.midiEvents.push_back({0x90, 36, 100});
    core.SetPortInput("node0/port/midi_in_1", midiValue);

    std::array<float, 48> left{};
    std::array<float, 48> right{};
    float* outputChannels[] = {left.data(), right.data(), nullptr, nullptr};
    core.Process({}, {outputChannels, 4}, left.size());

    float energy = 0.0f;
    for(std::size_t i = 0; i < left.size(); ++i)
    {
        energy += std::abs(left[i]) + std::abs(right[i]);
    }
    EXPECT_GT(energy, 0.01f);

    const auto before = core.GetParameterValue("node0/param/model");
    ASSERT_TRUE(before.hasValue);
    core.SetMenuItemValue("node0/menu/utilities/randomize_model", 1.0f);
    const auto after = core.GetParameterValue("node0/param/model");
    ASSERT_TRUE(after.hasValue);
    EXPECT_NE(before.value, after.value);

    core.SetMenuItemValue("node0/menu/utilities/panic", 1.0f);
    const auto& display = core.GetDisplayModel();
    EXPECT_FALSE(display.texts.empty());
}
} // namespace
