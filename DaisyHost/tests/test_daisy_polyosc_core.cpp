#include <array>
#include <cmath>

#include <gtest/gtest.h>

#include "daisyhost/DaisyPolyOscCore.h"

namespace
{
const daisyhost::DaisyPolyOscParameter* RequireParameter(
    const daisyhost::DaisyPolyOscCore& core,
    const char*                        id)
{
    const auto* parameter = core.FindParameter(id);
    EXPECT_NE(parameter, nullptr) << id;
    return parameter;
}

TEST(DaisyPolyOscCoreTest, ExposesSourceParametersAndDefaults)
{
    daisyhost::DaisyPolyOscCore core;
    core.ResetToDefaultState(17u);

    const auto& parameters = core.GetParameters();
    ASSERT_EQ(parameters.size(), 5u);

    const auto* osc1 = RequireParameter(core, "osc1_freq");
    const auto* osc2 = RequireParameter(core, "osc2_freq");
    const auto* osc3 = RequireParameter(core, "osc3_freq");
    const auto* global = RequireParameter(core, "global_freq");
    const auto* waveform = RequireParameter(core, "waveform");

    ASSERT_NE(osc1, nullptr);
    ASSERT_NE(osc2, nullptr);
    ASSERT_NE(osc3, nullptr);
    ASSERT_NE(global, nullptr);
    ASSERT_NE(waveform, nullptr);

    EXPECT_FLOAT_EQ(osc1->normalizedValue, 0.0f);
    EXPECT_FLOAT_EQ(osc2->normalizedValue, 0.0f);
    EXPECT_FLOAT_EQ(osc3->normalizedValue, 0.0f);
    EXPECT_FLOAT_EQ(global->normalizedValue, 0.5f);
    EXPECT_FLOAT_EQ(waveform->normalizedValue, 0.0f);
    EXPECT_EQ(waveform->stepCount, 4);
    EXPECT_EQ(core.GetWaveformLabel(), "Sine");
}

TEST(DaisyPolyOscCoreTest, UsesPatchFrequencyFormulaAndWaveformSteps)
{
    daisyhost::DaisyPolyOscCore core;
    core.Prepare(48000.0, daisyhost::DaisyPolyOscCore::kPreferredBlockSize);
    core.ResetToDefaultState(19u);

    ASSERT_TRUE(core.SetParameterValue("osc1_freq", 0.25f));
    ASSERT_TRUE(core.SetParameterValue("global_freq", 0.5f));
    ASSERT_TRUE(core.SetParameterValue("waveform", 0.5f));

    const float expectedFrequency = std::pow(2.0f, (0.25f + 0.5f) * 5.0f) * 55.0f;
    EXPECT_NEAR(core.GetOscillatorFrequencyHz(0), expectedFrequency, 0.01f);
    EXPECT_EQ(core.GetWaveformLabel(), "Saw");
}

TEST(DaisyPolyOscCoreTest, RendersThreeOscillatorsAndMixedOutput)
{
    daisyhost::DaisyPolyOscCore core;
    core.Prepare(48000.0, daisyhost::DaisyPolyOscCore::kPreferredBlockSize);
    core.ResetToDefaultState(23u);

    ASSERT_TRUE(core.SetParameterValue("osc1_freq", 0.10f));
    ASSERT_TRUE(core.SetParameterValue("osc2_freq", 0.20f));
    ASSERT_TRUE(core.SetParameterValue("osc3_freq", 0.30f));
    ASSERT_TRUE(core.SetParameterValue("global_freq", 0.35f));
    ASSERT_TRUE(core.SetParameterValue("waveform", 0.25f));

    std::array<float, daisyhost::DaisyPolyOscCore::kPreferredBlockSize> out1{};
    std::array<float, daisyhost::DaisyPolyOscCore::kPreferredBlockSize> out2{};
    std::array<float, daisyhost::DaisyPolyOscCore::kPreferredBlockSize> out3{};
    std::array<float, daisyhost::DaisyPolyOscCore::kPreferredBlockSize> mix{};

    core.Process(out1.data(), out2.data(), out3.data(), mix.data(), out1.size());

    float energy = 0.0f;
    for(std::size_t i = 0; i < out1.size(); ++i)
    {
        energy += std::abs(out1[i]) + std::abs(out2[i]) + std::abs(out3[i]);
        EXPECT_NEAR(mix[i], (out1[i] + out2[i] + out3[i]) * 0.25f, 0.0001f);
    }
    EXPECT_GT(energy, 0.01f);
}
} // namespace
