#include <array>
#include <cmath>

#include <gtest/gtest.h>

#include "daisyhost/DaisyVASynthCore.h"

namespace
{
float RenderChecksum(daisyhost::DaisyVASynthCore& core)
{
    std::array<float, daisyhost::DaisyVASynthCore::kPreferredBlockSize> left{};
    std::array<float, daisyhost::DaisyVASynthCore::kPreferredBlockSize> right{};

    core.Process(left.data(), right.data(), left.size());

    float checksum = 0.0f;
    for(std::size_t i = 0; i < left.size(); ++i)
    {
        checksum += left[i] * 0.41f + right[i] * 0.59f;
    }
    return checksum;
}

float OutputEnergy(daisyhost::DaisyVASynthCore& core, int blockCount = 1)
{
    std::array<float, daisyhost::DaisyVASynthCore::kPreferredBlockSize> left{};
    std::array<float, daisyhost::DaisyVASynthCore::kPreferredBlockSize> right{};

    float energy = 0.0f;
    for(int block = 0; block < blockCount; ++block)
    {
        core.Process(left.data(), right.data(), left.size());
        for(std::size_t i = 0; i < left.size(); ++i)
        {
            energy += std::abs(left[i]) + std::abs(right[i]);
        }
    }
    return energy;
}

const daisyhost::DaisyVASynthParameter* RequireParameter(
    const daisyhost::DaisyVASynthCore& core,
    const char*                        id)
{
    const auto* parameter = core.FindParameter(id);
    EXPECT_NE(parameter, nullptr) << id;
    return parameter;
}

TEST(DaisyVASynthCoreTest, ExposesStableParametersAndOscPageByDefault)
{
    daisyhost::DaisyVASynthCore core;
    core.ResetToDefaultState(17u);

    const auto& parameters = core.GetParameters();
    ASSERT_EQ(parameters.size(), 13u);

    EXPECT_NE(RequireParameter(core, "osc_mix"), nullptr);
    EXPECT_NE(RequireParameter(core, "detune"), nullptr);
    EXPECT_NE(RequireParameter(core, "osc1_wave"), nullptr);
    EXPECT_NE(RequireParameter(core, "osc2_wave"), nullptr);
    EXPECT_NE(RequireParameter(core, "filter_cutoff"), nullptr);
    EXPECT_NE(RequireParameter(core, "resonance"), nullptr);
    EXPECT_NE(RequireParameter(core, "filter_env_amount"), nullptr);
    EXPECT_NE(RequireParameter(core, "level"), nullptr);
    EXPECT_NE(RequireParameter(core, "lfo_rate"), nullptr);
    EXPECT_NE(RequireParameter(core, "lfo_amount"), nullptr);
    EXPECT_NE(RequireParameter(core, "attack"), nullptr);
    EXPECT_NE(RequireParameter(core, "release"), nullptr);
    EXPECT_NE(RequireParameter(core, "stereo_sim"), nullptr);

    const auto pageBinding = core.GetActivePageBinding();
    EXPECT_EQ(pageBinding.pageLabel, "Osc");
    EXPECT_EQ(pageBinding.parameterIds[0], "osc_mix");
    EXPECT_EQ(pageBinding.parameterIds[1], "detune");
    EXPECT_EQ(pageBinding.parameterIds[2], "osc1_wave");
    EXPECT_EQ(pageBinding.parameterIds[3], "osc2_wave");
}

TEST(DaisyVASynthCoreTest,
     SupportsDeterministicResetCaptureRestoreAndStereoSimState)
{
    daisyhost::DaisyVASynthCore first;
    daisyhost::DaisyVASynthCore second;

    first.Prepare(48000.0, daisyhost::DaisyVASynthCore::kPreferredBlockSize);
    second.Prepare(48000.0, daisyhost::DaisyVASynthCore::kPreferredBlockSize);

    first.ResetToDefaultState(1234u);
    second.ResetToDefaultState(1234u);

    ASSERT_TRUE(first.SetParameterValue("osc_mix", 0.7f));
    ASSERT_TRUE(first.SetParameterValue("filter_cutoff", 0.55f));
    ASSERT_TRUE(first.SetParameterValue("stereo_sim", 1.0f));

    first.TriggerMidiNote(48, 110);
    second.RestoreStatefulParameterValues(first.CaptureStatefulParameterValues());
    second.TriggerMidiNote(48, 110);

    EXPECT_FLOAT_EQ(RenderChecksum(first), RenderChecksum(second));
}

TEST(DaisyVASynthCoreTest, ProducesSoundAndReleasesVoicesFromMidiAndGate)
{
    daisyhost::DaisyVASynthCore core;
    core.Prepare(48000.0, daisyhost::DaisyVASynthCore::kPreferredBlockSize);
    core.ResetToDefaultState(91u);

    EXPECT_LT(OutputEnergy(core, 1), 0.0001f);

    core.TriggerMidiNote(48, 100);
    core.TriggerMidiNote(55, 90);
    EXPECT_GT(OutputEnergy(core, 4), 0.01f);

    core.ReleaseMidiNote(48);
    core.ReleaseMidiNote(55);
    core.TriggerGate(true);
    EXPECT_GT(OutputEnergy(core, 4), 0.01f);
    core.TriggerGate(false);
}

TEST(DaisyVASynthCoreTest, UsesMotionPageBinding)
{
    daisyhost::DaisyVASynthCore core;
    core.Prepare(48000.0, daisyhost::DaisyVASynthCore::kPreferredBlockSize);
    core.ResetToDefaultState(0u);

    ASSERT_TRUE(core.SetActivePage(daisyhost::DaisyVASynthPage::kMotion));
    const auto pageBinding = core.GetActivePageBinding();
    EXPECT_EQ(pageBinding.pageLabel, "Motion");
    EXPECT_EQ(pageBinding.parameterIds[0], "lfo_rate");
    EXPECT_EQ(pageBinding.parameterIds[1], "lfo_amount");
    EXPECT_EQ(pageBinding.parameterIds[2], "attack");
    EXPECT_EQ(pageBinding.parameterIds[3], "release");
}
} // namespace
