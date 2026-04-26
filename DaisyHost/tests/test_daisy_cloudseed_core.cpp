#include <array>
#include <algorithm>
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
    ASSERT_GE(parameters.size(), 55u);

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

    const auto* arpEnabled = RequireParameter(core, "arp_enabled");
    ASSERT_NE(arpEnabled, nullptr);
    EXPECT_EQ(arpEnabled->label, "Arp Enable");
    EXPECT_EQ(arpEnabled->groupLabel, "Arp");
    EXPECT_EQ(arpEnabled->stepCount, 2);
    EXPECT_FALSE(arpEnabled->automatable);
    EXPECT_TRUE(arpEnabled->stateful);

    const auto* arpTarget = RequireParameter(core, "arp_target");
    ASSERT_NE(arpTarget, nullptr);
    EXPECT_EQ(arpTarget->label, "Arp Target");
    EXPECT_EQ(arpTarget->stepCount, 2);

    const auto binding = core.GetActivePageBinding();
    EXPECT_EQ(binding.pageLabel, "Space");
    EXPECT_EQ(binding.parameterIds[0], "mix");
    EXPECT_EQ(binding.parameterIds[1], "size");
    EXPECT_EQ(binding.parameterIds[2], "decay");
    EXPECT_EQ(binding.parameterIds[3], "diffusion");
    EXPECT_EQ(binding.fieldParameterIds[0], "mix");
    EXPECT_EQ(binding.fieldParameterIds[7], "mod_rate");
}

TEST(DaisyCloudSeedCoreTest, AppPagesExposePerformanceArpAndSafeAdvancedFieldBindings)
{
    daisyhost::DaisyCloudSeedCore core;
    core.ResetToDefaultState(123u);

    ASSERT_TRUE(core.SetActivePage(daisyhost::DaisyCloudSeedPage::kArp));
    auto binding = core.GetActivePageBinding();
    EXPECT_EQ(binding.pageLabel, "Arp");
    EXPECT_EQ(binding.parameterIds[0], "arp_enabled");
    EXPECT_EQ(binding.parameterIds[1], "arp_rate");
    EXPECT_EQ(binding.parameterIds[2], "arp_pattern");
    EXPECT_EQ(binding.parameterIds[3], "arp_target");
    EXPECT_EQ(binding.fieldParameterIds[0], "arp_enabled");
    EXPECT_EQ(binding.fieldParameterIds[4], "arp_depth");
    EXPECT_TRUE(binding.fieldParameterIds[5].empty());

    ASSERT_TRUE(core.SetActivePage(daisyhost::DaisyCloudSeedPage::kAdvanced));
    binding = core.GetActivePageBinding();
    EXPECT_EQ(binding.pageLabel, "Advanced");
    const std::array<std::string, 8> expected = {{
        "eq_low_freq",
        "eq_high_freq",
        "eq_cutoff",
        "eq_low_gain",
        "eq_high_gain",
        "eq_cross_seed",
        "seed_diffusion",
        "seed_delay",
    }};
    EXPECT_EQ(binding.fieldParameterIds, expected);

    const std::array<const char*, 10> riskyIds = {{
        "global_input_mix",
        "global_dry_out",
        "global_early_out",
        "global_late_out",
        "bypass",
        "global_low_cut_enabled",
        "global_high_cut_enabled",
        "tap_enabled",
        "early_diffusion_enabled",
        "late_diffusion_enabled",
    }};
    for(const auto* riskyId : riskyIds)
    {
        EXPECT_EQ(std::find(binding.fieldParameterIds.begin(),
                            binding.fieldParameterIds.end(),
                            riskyId),
                  binding.fieldParameterIds.end())
            << riskyId;
    }

    for(const auto& id : binding.fieldParameterIds)
    {
        const auto* parameter = RequireParameter(core, id.c_str());
        ASSERT_NE(parameter, nullptr);
        EXPECT_TRUE(parameter->automatable) << id;
    }
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

TEST(DaisyCloudSeedCoreTest, ArpeggiatorStepsEffectivePerformanceStateDeterministically)
{
    auto effectiveSequence = [](daisyhost::DaisyCloudSeedCore& core) {
        std::array<float, 48> inLeft{};
        std::array<float, 48> inRight{};
        std::array<float, 48> outLeft{};
        std::array<float, 48> outRight{};
        std::array<float, 8>  sequence{};

        for(std::size_t block = 0; block < sequence.size(); ++block)
        {
            core.Process(inLeft.data(),
                         inRight.data(),
                         outLeft.data(),
                         outRight.data(),
                         inLeft.size());
            for(int extra = 0; extra < 39; ++extra)
            {
                core.Process(inLeft.data(),
                             inRight.data(),
                             outLeft.data(),
                             outRight.data(),
                             inLeft.size());
            }

            float mix = 0.0f;
            float size = 0.0f;
            core.GetEffectiveParameterValue("mix", &mix);
            core.GetEffectiveParameterValue("size", &size);
            sequence[block] = (mix * 0.37f) + (size * 0.63f);
        }

        return sequence;
    };

    daisyhost::DaisyCloudSeedCore first;
    daisyhost::DaisyCloudSeedCore second;
    first.Prepare(48000.0, 48);
    second.Prepare(48000.0, 48);
    first.ResetToDefaultState(2026u);
    second.ResetToDefaultState(2026u);

    ASSERT_TRUE(first.SetParameterValue("mix", 0.40f));
    ASSERT_TRUE(first.SetParameterValue("size", 0.40f));
    ASSERT_TRUE(first.SetParameterValue("decay", 0.40f));
    ASSERT_TRUE(first.SetParameterValue("diffusion", 0.40f));
    ASSERT_TRUE(first.SetParameterValue("arp_enabled", 1.0f));
    ASSERT_TRUE(first.SetParameterValue("arp_rate", 0.0f));
    ASSERT_TRUE(first.SetParameterValue("arp_pattern", 0.0f));
    ASSERT_TRUE(first.SetParameterValue("arp_target", 0.0f));
    ASSERT_TRUE(first.SetParameterValue("arp_depth", 0.40f));

    second.RestoreStatefulParameterValues(first.CaptureStatefulParameterValues());

    float storedMix = 0.0f;
    float effectiveMix = 0.0f;
    ASSERT_TRUE(first.GetParameterValue("mix", &storedMix));
    ASSERT_TRUE(first.GetEffectiveParameterValue("mix", &effectiveMix));
    EXPECT_FLOAT_EQ(storedMix, 0.40f);
    EXPECT_GT(effectiveMix, storedMix);

    const auto firstSequence = effectiveSequence(first);
    const auto secondSequence = effectiveSequence(second);
    EXPECT_EQ(firstSequence, secondSequence);

    float finalStoredMix = 0.0f;
    ASSERT_TRUE(first.GetParameterValue("mix", &finalStoredMix));
    EXPECT_FLOAT_EQ(finalStoredMix, 0.40f);
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
