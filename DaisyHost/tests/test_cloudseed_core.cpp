#include <array>

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
    const auto* root = FindSection(menu, "node0/menu/root");
    ASSERT_NE(root, nullptr);
    ASSERT_EQ(root->items.size(), 6u);
    EXPECT_NE(FindSection(menu, "node0/menu/pages"), nullptr);
    EXPECT_NE(FindSection(menu, "node0/menu/macros"), nullptr);
    EXPECT_NE(FindSection(menu, "node0/menu/arp"), nullptr);
    EXPECT_NE(FindSection(menu, "node0/menu/program"), nullptr);
    EXPECT_NE(FindSection(menu, "node0/menu/utilities"), nullptr);
    EXPECT_NE(FindSection(menu, "node0/menu/info"), nullptr);
}

TEST(CloudSeedCoreTest, MetaControllersReflectCanonicalStateAndWriteThrough)
{
    daisyhost::apps::CloudSeedCore core("node0");
    core.Prepare(48000.0, 48);
    core.ResetToDefaultState(11u);

    const auto& metaControllers = core.GetMetaControllers();
    ASSERT_EQ(metaControllers.size(), 4u);
    EXPECT_EQ(metaControllers[0].id, "node0/meta/blend");
    EXPECT_EQ(metaControllers[0].label, "Blend");
    EXPECT_TRUE(metaControllers[0].stateful);
    EXPECT_EQ(metaControllers[1].id, "node0/meta/space");
    EXPECT_EQ(metaControllers[1].label, "Space");
    EXPECT_TRUE(metaControllers[1].stateful);
    EXPECT_EQ(metaControllers[2].id, "node0/meta/motion");
    EXPECT_EQ(metaControllers[2].label, "Motion");
    EXPECT_TRUE(metaControllers[2].stateful);
    EXPECT_EQ(metaControllers[3].id, "node0/meta/tone");
    EXPECT_EQ(metaControllers[3].label, "Tone");
    EXPECT_TRUE(metaControllers[3].stateful);

    ASSERT_TRUE(core.SetMetaControllerValue("node0/meta/blend", 0.20f));
    ASSERT_TRUE(core.SetMetaControllerValue("node0/meta/space", 0.60f));
    ASSERT_TRUE(core.SetMetaControllerValue("node0/meta/motion", 0.35f));
    ASSERT_TRUE(core.SetMetaControllerValue("node0/meta/tone", 0.25f));

    const auto mix = core.GetParameterValue("node0/param/mix");
    const auto size = core.GetParameterValue("node0/param/size");
    const auto decay = core.GetParameterValue("node0/param/decay");
    const auto preDelay = core.GetParameterValue("node0/param/pre_delay");
    const auto modAmount = core.GetParameterValue("node0/param/mod_amount");
    const auto modRate = core.GetParameterValue("node0/param/mod_rate");
    const auto diffusion = core.GetParameterValue("node0/param/diffusion");
    const auto damping = core.GetParameterValue("node0/param/damping");
    ASSERT_TRUE(mix.hasValue);
    ASSERT_TRUE(size.hasValue);
    ASSERT_TRUE(decay.hasValue);
    ASSERT_TRUE(preDelay.hasValue);
    ASSERT_TRUE(modAmount.hasValue);
    ASSERT_TRUE(modRate.hasValue);
    ASSERT_TRUE(diffusion.hasValue);
    ASSERT_TRUE(damping.hasValue);
    EXPECT_NEAR(mix.value, 0.20f, 0.0001f);
    EXPECT_NEAR(size.value, 0.61f, 0.0001f);
    EXPECT_NEAR(decay.value, 0.59f, 0.0001f);
    EXPECT_NEAR(preDelay.value, 0.52f, 0.0001f);
    EXPECT_NEAR(modAmount.value, 0.35f, 0.0001f);
    EXPECT_NEAR(modRate.value, 0.38f, 0.0001f);
    EXPECT_NEAR(diffusion.value, 0.3125f, 0.0001f);
    EXPECT_NEAR(damping.value, 0.70f, 0.0001f);

    const auto blend = core.GetMetaControllerValue("node0/meta/blend");
    const auto space = core.GetMetaControllerValue("node0/meta/space");
    const auto motion = core.GetMetaControllerValue("node0/meta/motion");
    const auto tone = core.GetMetaControllerValue("node0/meta/tone");
    ASSERT_TRUE(blend.hasValue);
    ASSERT_TRUE(space.hasValue);
    ASSERT_TRUE(motion.hasValue);
    ASSERT_TRUE(tone.hasValue);
    EXPECT_NEAR(blend.value, 0.20f, 0.0001f);
    EXPECT_NEAR(space.value, 0.60f, 0.0001f);
    EXPECT_NEAR(motion.value, 0.35f, 0.0001f);
    EXPECT_NEAR(tone.value, 0.25f, 0.0001f);

    ASSERT_TRUE(core.SetParameterValue("node0/param/mod_amount", 0.55f));
    const auto derivedMotion = core.GetMetaControllerValue("node0/meta/motion");
    ASSERT_TRUE(derivedMotion.hasValue);
    EXPECT_NEAR(derivedMotion.value, 0.55f, 0.0001f);

    const auto captured = core.CaptureStatefulParameterValues();
    EXPECT_EQ(captured.count("node0/meta/blend"), 0u);
    EXPECT_EQ(captured.count("node0/meta/space"), 0u);
    EXPECT_EQ(captured.count("node0/meta/motion"), 0u);
    EXPECT_EQ(captured.count("node0/meta/tone"), 0u);
    EXPECT_FALSE(core.SetMetaControllerValue("node0/meta/unknown", 0.5f));
    EXPECT_FALSE(core.GetMetaControllerValue("node0/meta/unknown").hasValue);
}

TEST(CloudSeedCoreTest, MacroMenuItemsWriteIntoCanonicalParameters)
{
    daisyhost::apps::CloudSeedCore core("node0");
    core.Prepare(48000.0, 48);
    core.ResetToDefaultState(17u);

    core.SetMenuItemValue("node0/menu/macros/blend", 0.65f);
    core.SetMenuItemValue("node0/menu/macros/space", 0.40f);
    core.SetMenuItemValue("node0/menu/macros/motion", 0.75f);
    core.SetMenuItemValue("node0/menu/macros/tone", 0.10f);

    const auto mix = core.GetParameterValue("node0/param/mix");
    const auto size = core.GetParameterValue("node0/param/size");
    const auto decay = core.GetParameterValue("node0/param/decay");
    const auto preDelay = core.GetParameterValue("node0/param/pre_delay");
    const auto modAmount = core.GetParameterValue("node0/param/mod_amount");
    const auto modRate = core.GetParameterValue("node0/param/mod_rate");
    const auto diffusion = core.GetParameterValue("node0/param/diffusion");
    const auto damping = core.GetParameterValue("node0/param/damping");
    ASSERT_TRUE(mix.hasValue);
    ASSERT_TRUE(size.hasValue);
    ASSERT_TRUE(decay.hasValue);
    ASSERT_TRUE(preDelay.hasValue);
    ASSERT_TRUE(modAmount.hasValue);
    ASSERT_TRUE(modRate.hasValue);
    ASSERT_TRUE(diffusion.hasValue);
    ASSERT_TRUE(damping.hasValue);
    EXPECT_NEAR(mix.value, 0.65f, 0.0001f);
    EXPECT_NEAR(size.value, 0.44f, 0.0001f);
    EXPECT_NEAR(decay.value, 0.41f, 0.0001f);
    EXPECT_NEAR(preDelay.value, 0.38f, 0.0001f);
    EXPECT_NEAR(modAmount.value, 0.75f, 0.0001f);
    EXPECT_NEAR(modRate.value, 0.70f, 0.0001f);
    EXPECT_NEAR(diffusion.value, 0.185f, 0.0001f);
    EXPECT_NEAR(damping.value, 0.82f, 0.0001f);
}

TEST(CloudSeedCoreTest, PageSwitchUpdatesKnobLabelsAndControlRouting)
{
    daisyhost::apps::CloudSeedCore core("node0");
    core.Prepare(48000.0, 48);
    core.ResetToDefaultState(5u);

    core.SetMenuItemValue("node0/menu/pages/page", 0.33333334f);

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

TEST(CloudSeedCoreTest, AppMenuPagesIncludeArpAndAdvancedFieldKnobPage)
{
    daisyhost::apps::CloudSeedCore core("node0");
    core.Prepare(48000.0, 48);
    core.ResetToDefaultState(5u);

    const auto* pagesSection = FindSection(core.GetMenuModel(), "node0/menu/pages");
    ASSERT_NE(pagesSection, nullptr);
    ASSERT_GE(pagesSection->items.size(), 2u);
    EXPECT_EQ(pagesSection->items[1].valueText, "Space");

    core.SetMenuItemValue("node0/menu/pages/page", 0.6666667f);
    auto bindings = core.GetPatchBindings();
    EXPECT_EQ(bindings.knobDetailLabels[0], "Arp Enable");
    EXPECT_EQ(bindings.knobDetailLabels[1], "Arp Rate");
    EXPECT_EQ(bindings.knobDetailLabels[2], "Pattern");
    EXPECT_EQ(bindings.knobDetailLabels[3], "Target");
    EXPECT_EQ(bindings.fieldKnobDetailLabels[0], "Arp Enable");
    EXPECT_EQ(bindings.fieldKnobDetailLabels[4], "Arp Depth");
    EXPECT_TRUE(bindings.fieldKnobParameterIds[5].empty());

    core.SetMenuItemValue("node0/menu/pages/page", 1.0f);
    bindings = core.GetPatchBindings();
    EXPECT_EQ(bindings.knobDetailLabels[0], "Low Freq");
    EXPECT_EQ(bindings.knobDetailLabels[1], "High Freq");
    EXPECT_EQ(bindings.knobDetailLabels[2], "Cutoff");
    EXPECT_EQ(bindings.knobDetailLabels[3], "Low Gain");
    EXPECT_EQ(bindings.fieldKnobParameterIds[0], "node0/param/eq_low_freq");
    EXPECT_EQ(bindings.fieldKnobParameterIds[1], "node0/param/eq_high_freq");
    EXPECT_EQ(bindings.fieldKnobParameterIds[2], "node0/param/eq_cutoff");
    EXPECT_EQ(bindings.fieldKnobParameterIds[3], "node0/param/eq_low_gain");
    EXPECT_EQ(bindings.fieldKnobParameterIds[4], "node0/param/eq_high_gain");
    EXPECT_EQ(bindings.fieldKnobParameterIds[5], "node0/param/eq_cross_seed");
    EXPECT_EQ(bindings.fieldKnobParameterIds[6], "node0/param/seed_diffusion");
    EXPECT_EQ(bindings.fieldKnobParameterIds[7], "node0/param/seed_delay");

    core.SetControl(bindings.fieldKnobControlIds[0], 0.68f);
    const auto eqLow = core.GetParameterValue("node0/param/eq_low_freq");
    const auto inputMix = core.GetParameterValue("node0/param/global_input_mix");
    ASSERT_TRUE(eqLow.hasValue);
    ASSERT_TRUE(inputMix.hasValue);
    EXPECT_NEAR(eqLow.value, 0.68f, 0.0001f);
    EXPECT_NE(eqLow.value, inputMix.value);
}

TEST(CloudSeedCoreTest, ArpMenuControlsUpdateStateAndEffectiveRouting)
{
    daisyhost::apps::CloudSeedCore core("node0");
    core.Prepare(48000.0, 48);
    core.ResetToDefaultState(29u);

    const auto* arpSection = FindSection(core.GetMenuModel(), "node0/menu/arp");
    ASSERT_NE(arpSection, nullptr);
    ASSERT_EQ(arpSection->items.size(), 6u);
    EXPECT_EQ(arpSection->items[1].id, "node0/menu/arp/enabled");
    EXPECT_EQ(arpSection->items[2].id, "node0/menu/arp/rate");
    EXPECT_EQ(arpSection->items[3].id, "node0/menu/arp/pattern");
    EXPECT_EQ(arpSection->items[4].id, "node0/menu/arp/target");
    EXPECT_EQ(arpSection->items[5].id, "node0/menu/arp/depth");

    ASSERT_TRUE(core.SetParameterValue("node0/param/mix", 0.40f));
    ASSERT_TRUE(core.SetParameterValue("node0/param/size", 0.40f));
    core.SetMenuItemValue("node0/menu/arp/enabled", 1.0f);
    core.SetMenuItemValue("node0/menu/arp/rate", 0.0f);
    core.SetMenuItemValue("node0/menu/arp/pattern", 0.0f);
    core.SetMenuItemValue("node0/menu/arp/target", 0.0f);
    core.SetMenuItemValue("node0/menu/arp/depth", 0.40f);

    const auto arpEnabled = core.GetParameterValue("node0/param/arp_enabled");
    const auto arpDepth = core.GetParameterValue("node0/param/arp_depth");
    ASSERT_TRUE(arpEnabled.hasValue);
    ASSERT_TRUE(arpDepth.hasValue);
    EXPECT_FLOAT_EQ(arpEnabled.value, 1.0f);
    EXPECT_NEAR(arpDepth.value, 0.40f, 0.0001f);

    const auto storedMix = core.GetParameterValue("node0/param/mix");
    const auto effectiveMix = core.GetEffectiveParameterValue("node0/param/mix");
    ASSERT_TRUE(storedMix.hasValue);
    ASSERT_TRUE(effectiveMix.hasValue);
    EXPECT_FLOAT_EQ(storedMix.value, 0.40f);
    EXPECT_GT(effectiveMix.value, storedMix.value);

    std::array<float, 48> inputLeft{};
    std::array<float, 48> inputRight{};
    std::array<float, 48> outputLeft{};
    std::array<float, 48> outputRight{};
    std::array<const float*, 2> inputChannels = {inputLeft.data(), inputRight.data()};
    std::array<float*, 2> outputChannels = {outputLeft.data(), outputRight.data()};
    for(int block = 0; block < 40; ++block)
    {
        core.Process({inputChannels.data(), inputChannels.size()},
                     {outputChannels.data(), outputChannels.size()},
                     inputLeft.size());
    }

    const auto effectiveSize = core.GetEffectiveParameterValue("node0/param/size");
    ASSERT_TRUE(effectiveSize.hasValue);
    EXPECT_GT(effectiveSize.value, storedMix.value);
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
