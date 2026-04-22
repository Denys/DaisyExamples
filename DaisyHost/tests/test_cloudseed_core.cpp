#include <gtest/gtest.h>

#include "daisyhost/apps/CloudSeedCore.h"

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

TEST(CloudSeedCoreTest, ExposesMetadataCapabilitiesAndDefaultSpacePage)
{
    daisyhost::apps::CloudSeedCore core("node0");

    EXPECT_EQ(core.GetAppId(), "cloudseed");
    EXPECT_EQ(core.GetAppDisplayName(), "CloudSeed");

    const auto capabilities = core.GetCapabilities();
    EXPECT_TRUE(capabilities.acceptsAudioInput);
    EXPECT_FALSE(capabilities.acceptsMidiInput);
    EXPECT_FALSE(capabilities.producesMidiOutput);

    const auto bindings = core.GetPatchBindings();
    EXPECT_EQ(bindings.knobDetailLabels[0], "Mix");
    EXPECT_EQ(bindings.knobDetailLabels[1], "Size");
    EXPECT_EQ(bindings.knobDetailLabels[2], "Decay");
    EXPECT_EQ(bindings.knobDetailLabels[3], "Diffusion");
    EXPECT_EQ(bindings.audioInputPortIds[0], "node0/port/audio_in_1");
    EXPECT_EQ(bindings.audioInputPortIds[1], "node0/port/audio_in_2");
    EXPECT_EQ(bindings.mainOutputChannels[0], 0);
    EXPECT_EQ(bindings.mainOutputChannels[1], 1);

    const auto& menu = core.GetMenuModel();
    ASSERT_FALSE(menu.sections.empty());
    EXPECT_NE(FindSection(menu, "node0/menu/pages"), nullptr);
    EXPECT_NE(FindSection(menu, "node0/menu/program"), nullptr);
    EXPECT_NE(FindSection(menu, "node0/menu/utilities"), nullptr);
    EXPECT_NE(FindSection(menu, "node0/menu/info"), nullptr);
}

TEST(CloudSeedCoreTest, PageSwitchUpdatesKnobLabelsAndControlRouting)
{
    daisyhost::apps::CloudSeedCore core("node0");
    core.Prepare(48000.0, 48);
    core.ResetToDefaultState(5u);

    core.SetMenuItemValue("node0/menu/pages/page", 1.0f);

    const auto bindings = core.GetPatchBindings();
    EXPECT_EQ(bindings.knobDetailLabels[0], "Pre-Delay");
    EXPECT_EQ(bindings.knobDetailLabels[1], "Damping");
    EXPECT_EQ(bindings.knobDetailLabels[2], "Mod Amt");
    EXPECT_EQ(bindings.knobDetailLabels[3], "Mod Rate");

    core.SetControl(bindings.knobControlIds[0], 0.68f);
    const auto preDelayValue = core.GetParameterValue("node0/param/pre_delay");
    const auto mixValue      = core.GetParameterValue("node0/param/mix");
    EXPECT_TRUE(preDelayValue.hasValue);
    EXPECT_TRUE(mixValue.hasValue);
    EXPECT_NEAR(preDelayValue.value, 0.68f, 0.0001f);
    EXPECT_NE(mixValue.value, preDelayValue.value);
}

TEST(CloudSeedCoreTest, UtilitiesAndEffectiveStateRemainVisible)
{
    daisyhost::apps::CloudSeedCore core("node0");
    core.Prepare(48000.0, 48);
    core.ResetToDefaultState(21u);

    ASSERT_TRUE(core.SetParameterValue("node0/param/size", 0.83f));
    const auto storedLateLineSize
        = core.GetParameterValue("node0/param/late_line_size");
    const auto effectiveLateLineSize
        = core.GetEffectiveParameterValue("node0/param/late_line_size");
    EXPECT_TRUE(storedLateLineSize.hasValue);
    EXPECT_TRUE(effectiveLateLineSize.hasValue);
    EXPECT_NE(storedLateLineSize.value, effectiveLateLineSize.value);

    const auto before = core.CaptureStatefulParameterValues();
    core.SetMenuItemValue("node0/menu/utilities/interpolation", 0.0f);
    core.SetMenuItemValue("node0/menu/utilities/randomize_seeds", 1.0f);
    core.SetMenuItemValue("node0/menu/utilities/bypass", 1.0f);
    const auto after = core.CaptureStatefulParameterValues();

    EXPECT_FLOAT_EQ(after.at("node0/param/global_interpolation"), 0.0f);
    EXPECT_FLOAT_EQ(after.at("node0/param/bypass"), 1.0f);

    const bool anySeedChanged
        = before.at("node0/param/seed_tap") != after.at("node0/param/seed_tap")
          || before.at("node0/param/seed_diffusion")
                 != after.at("node0/param/seed_diffusion")
          || before.at("node0/param/seed_delay")
                 != after.at("node0/param/seed_delay")
          || before.at("node0/param/seed_post_diffusion")
                 != after.at("node0/param/seed_post_diffusion");
    EXPECT_TRUE(anySeedChanged);
}
} // namespace
