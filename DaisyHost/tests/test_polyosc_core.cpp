#include <array>
#include <cmath>

#include <gtest/gtest.h>

#include "daisyhost/apps/PolyOscCore.h"

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

TEST(PolyOscCoreTest, ExposesMetadataPatchBindingsAndMenu)
{
    daisyhost::apps::PolyOscCore core("node0");

    EXPECT_EQ(core.GetAppId(), "polyosc");
    EXPECT_EQ(core.GetAppDisplayName(), "PolyOsc");

    const auto capabilities = core.GetCapabilities();
    EXPECT_FALSE(capabilities.acceptsAudioInput);
    EXPECT_FALSE(capabilities.acceptsMidiInput);
    EXPECT_FALSE(capabilities.producesMidiOutput);

    const auto bindings = core.GetPatchBindings();
    EXPECT_EQ(bindings.knobDetailLabels[0], "Osc 1");
    EXPECT_EQ(bindings.knobDetailLabels[1], "Osc 2");
    EXPECT_EQ(bindings.knobDetailLabels[2], "Osc 3");
    EXPECT_EQ(bindings.knobDetailLabels[3], "Global");
    EXPECT_EQ(bindings.knobParameterIds[0], "node0/param/osc1_freq");
    EXPECT_EQ(bindings.knobParameterIds[1], "node0/param/osc2_freq");
    EXPECT_EQ(bindings.knobParameterIds[2], "node0/param/osc3_freq");
    EXPECT_EQ(bindings.knobParameterIds[3], "node0/param/global_freq");
    EXPECT_EQ(bindings.audioOutputPortIds[0], "node0/port/audio_out_1");
    EXPECT_EQ(bindings.audioOutputPortIds[1], "node0/port/audio_out_2");
    EXPECT_EQ(bindings.audioOutputPortIds[2], "node0/port/audio_out_3");
    EXPECT_EQ(bindings.audioOutputPortIds[3], "node0/port/audio_out_4");
    EXPECT_EQ(bindings.mainOutputChannels[0], 3);
    EXPECT_EQ(bindings.mainOutputChannels[1], 3);

    const auto& menu = core.GetMenuModel();
    ASSERT_FALSE(menu.sections.empty());
    EXPECT_NE(FindSection(menu, "node0/menu/params"), nullptr);
    EXPECT_NE(FindSection(menu, "node0/menu/waveform"), nullptr);
    EXPECT_NE(FindSection(menu, "node0/menu/info"), nullptr);
}

TEST(PolyOscCoreTest, ControlsEncoderAndMenuEditAllParameters)
{
    daisyhost::apps::PolyOscCore core("node0");
    core.Prepare(48000.0, 48);
    core.ResetToDefaultState(29u);

    const auto bindings = core.GetPatchBindings();
    core.SetControl(bindings.knobControlIds[0], 0.4f);
    core.SetControl(bindings.knobControlIds[3], 0.6f);
    core.SetEncoderDelta(2);

    auto osc1 = core.GetParameterValue("node0/param/osc1_freq");
    auto global = core.GetParameterValue("node0/param/global_freq");
    auto waveform = core.GetParameterValue("node0/param/waveform");
    ASSERT_TRUE(osc1.hasValue);
    ASSERT_TRUE(global.hasValue);
    ASSERT_TRUE(waveform.hasValue);
    EXPECT_NEAR(osc1.value, 0.4f, 0.0001f);
    EXPECT_NEAR(global.value, 0.6f, 0.0001f);
    EXPECT_NEAR(waveform.value, 0.5f, 0.0001f);

    core.SetMenuItemValue("node0/menu/params/osc2_freq", 0.7f);
    core.SetMenuItemValue("node0/menu/params/osc3_freq", 0.8f);
    core.SetMenuItemValue("node0/menu/waveform/waveform", 1.0f);

    auto osc2 = core.GetParameterValue("node0/param/osc2_freq");
    auto osc3 = core.GetParameterValue("node0/param/osc3_freq");
    waveform = core.GetParameterValue("node0/param/waveform");
    ASSERT_TRUE(osc2.hasValue);
    ASSERT_TRUE(osc3.hasValue);
    ASSERT_TRUE(waveform.hasValue);
    EXPECT_NEAR(osc2.value, 0.7f, 0.0001f);
    EXPECT_NEAR(osc3.value, 0.8f, 0.0001f);
    EXPECT_NEAR(waveform.value, 1.0f, 0.0001f);
}

TEST(PolyOscCoreTest, RendersIndividualOutputsAndHostMix)
{
    daisyhost::apps::PolyOscCore core("node0");
    core.Prepare(48000.0, 48);
    core.ResetToDefaultState(31u);

    core.SetParameterValue("node0/param/osc1_freq", 0.1f);
    core.SetParameterValue("node0/param/osc2_freq", 0.2f);
    core.SetParameterValue("node0/param/osc3_freq", 0.3f);
    core.SetParameterValue("node0/param/global_freq", 0.35f);

    std::array<float, 48> out1{};
    std::array<float, 48> out2{};
    std::array<float, 48> out3{};
    std::array<float, 48> mix{};
    float* outputChannels[] = {out1.data(), out2.data(), out3.data(), mix.data()};

    core.Process({}, {outputChannels, 4}, out1.size());

    float energy = 0.0f;
    for(std::size_t i = 0; i < out1.size(); ++i)
    {
        energy += std::abs(mix[i]);
        EXPECT_NEAR(mix[i], (out1[i] + out2[i] + out3[i]) * 0.25f, 0.0001f);
    }
    EXPECT_GT(energy, 0.001f);

    const auto output = core.GetPortOutput("node0/port/audio_out_4");
    EXPECT_EQ(output.type, daisyhost::VirtualPortType::kAudio);
    EXPECT_GT(output.scalar, 0.0f);
}
} // namespace
