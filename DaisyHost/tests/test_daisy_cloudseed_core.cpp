#include <array>
#include <cmath>

#include <gtest/gtest.h>

#include "daisyhost/DaisyCloudSeedCore.h"

namespace
{
float RenderChecksum(daisyhost::DaisyCloudSeedCore& core)
{
    std::array<float, 48> inLeft{};
    std::array<float, 48> inRight{};
    std::array<float, 48> outLeft{};
    std::array<float, 48> outRight{};
    inLeft[0]  = 0.9f;
    inRight[0] = 0.7f;

    core.Process(inLeft.data(),
                 inRight.data(),
                 outLeft.data(),
                 outRight.data(),
                 inLeft.size());

    float checksum = 0.0f;
    for(std::size_t i = 0; i < outLeft.size(); ++i)
    {
        checksum += outLeft[i] * 0.37f + outRight[i] * 0.63f;
    }
    return checksum;
}

float TailEnergyAfterZeroInput(daisyhost::DaisyCloudSeedCore& core,
                               int                            blockCount = 1)
{
    std::array<float, 48> inLeft{};
    std::array<float, 48> inRight{};
    std::array<float, 48> outLeft{};
    std::array<float, 48> outRight{};

    float energy = 0.0f;
    for(int block = 0; block < blockCount; ++block)
    {
        core.Process(inLeft.data(),
                     inRight.data(),
                     outLeft.data(),
                     outRight.data(),
                     inLeft.size());

        for(std::size_t i = 0; i < outLeft.size(); ++i)
        {
            energy += std::abs(outLeft[i]) + std::abs(outRight[i]);
        }
    }
    return energy;
}

const daisyhost::DaisyCloudSeedParameter* RequireParameter(
    const daisyhost::DaisyCloudSeedCore& core,
    const char*                          id)
{
    const auto* parameter = core.FindParameter(id);
    EXPECT_NE(parameter, nullptr) << id;
    return parameter;
}

TEST(DaisyCloudSeedCoreTest, ExposesStablePerformanceAndExpertParameters)
{
    daisyhost::DaisyCloudSeedCore core;
    core.ResetToDefaultState(123u);

    const auto& parameters = core.GetParameters();
    ASSERT_GE(parameters.size(), 50u);

    const auto* mix = RequireParameter(core, "mix");
    ASSERT_NE(mix, nullptr);
    EXPECT_EQ(mix->label, "Mix");
    EXPECT_TRUE(mix->automatable);
    EXPECT_TRUE(mix->stateful);
    EXPECT_TRUE(mix->performanceTier);

    const auto* interpolation = RequireParameter(core, "global_interpolation");
    ASSERT_NE(interpolation, nullptr);
    EXPECT_EQ(interpolation->groupLabel, "Global");
    EXPECT_FALSE(interpolation->automatable);
    EXPECT_TRUE(interpolation->stateful);

    const auto binding = core.GetActivePageBinding();
    EXPECT_EQ(binding.pageLabel, "Space");
    EXPECT_EQ(binding.parameterIds[0], "mix");
    EXPECT_EQ(binding.parameterIds[1], "size");
    EXPECT_EQ(binding.parameterIds[2], "decay");
    EXPECT_EQ(binding.parameterIds[3], "diffusion");
}

TEST(DaisyCloudSeedCoreTest, SupportsDeterministicResetCaptureRestoreAndSeedRandomization)
{
    daisyhost::DaisyCloudSeedCore first;
    daisyhost::DaisyCloudSeedCore second;

    first.Prepare(48000.0, 48);
    second.Prepare(48000.0, 48);

    first.ResetToDefaultState(777u);
    second.ResetToDefaultState(777u);

    ASSERT_TRUE(first.SetParameterValue("mix", 0.61f));
    ASSERT_TRUE(first.SetParameterValue("size", 0.73f));
    ASSERT_TRUE(first.SetParameterValue("eq_high_gain", 0.48f));

    second.RestoreStatefulParameterValues(first.CaptureStatefulParameterValues());

    EXPECT_EQ(first.GetSeedSummary(), second.GetSeedSummary());
    EXPECT_FLOAT_EQ(RenderChecksum(first), RenderChecksum(second));

    const auto before = first.CaptureStatefulParameterValues();
    ASSERT_TRUE(first.TriggerMomentaryAction("randomize_seeds"));
    const auto after = first.CaptureStatefulParameterValues();

    const bool anySeedChanged = before.at("seed_tap") != after.at("seed_tap")
                                || before.at("seed_diffusion")
                                       != after.at("seed_diffusion")
                                || before.at("seed_delay") != after.at("seed_delay")
                                || before.at("seed_post_diffusion")
                                       != after.at("seed_post_diffusion");
    EXPECT_TRUE(anySeedChanged);
}

TEST(DaisyCloudSeedCoreTest, BypassAndClearTailsAffectAudioState)
{
    daisyhost::DaisyCloudSeedCore core;
    core.Prepare(48000.0, 48);
    core.ResetToDefaultState(19u);

    std::array<float, 48> inLeft{};
    std::array<float, 48> inRight{};
    std::array<float, 48> outLeft{};
    std::array<float, 48> outRight{};
    std::fill(inLeft.begin(), inLeft.end(), 0.35f);
    std::fill(inRight.begin(), inRight.end(), 0.28f);

    for(int block = 0; block < 24; ++block)
    {
        core.Process(inLeft.data(),
                     inRight.data(),
                     outLeft.data(),
                     outRight.data(),
                     inLeft.size());
    }

    const float tailBeforeClear = TailEnergyAfterZeroInput(core, 12);
    EXPECT_GT(tailBeforeClear, 0.0001f);

    ASSERT_TRUE(core.SetParameterValue("bypass", 1.0f));
    core.Process(inLeft.data(),
                 inRight.data(),
                 outLeft.data(),
                 outRight.data(),
                 inLeft.size());
    EXPECT_NEAR(outLeft[0], inLeft[0], 0.0001f);
    EXPECT_NEAR(outRight[0], inRight[0], 0.0001f);

    ASSERT_TRUE(core.SetParameterValue("bypass", 0.0f));
    ASSERT_TRUE(core.TriggerMomentaryAction("clear_tails"));
    const float tailAfterClear = TailEnergyAfterZeroInput(core, 4);
    EXPECT_LT(tailAfterClear, tailBeforeClear * 0.25f);
}
} // namespace
