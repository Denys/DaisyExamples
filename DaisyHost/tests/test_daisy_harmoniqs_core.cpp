#include <array>
#include <cmath>

#include <gtest/gtest.h>

#include "daisyhost/DaisyHarmoniqsCore.h"

namespace
{
float RenderChecksum(daisyhost::DaisyHarmoniqsCore& core)
{
    std::array<float, daisyhost::DaisyHarmoniqsCore::kPreferredBlockSize> left{};
    std::array<float, daisyhost::DaisyHarmoniqsCore::kPreferredBlockSize> right{};

    core.Process(left.data(), right.data(), left.size());

    float checksum = 0.0f;
    for(std::size_t i = 0; i < left.size(); ++i)
    {
        checksum += left[i] * 0.37f + right[i] * 0.63f;
    }
    return checksum;
}

float OutputEnergy(daisyhost::DaisyHarmoniqsCore& core, int blockCount = 1)
{
    std::array<float, daisyhost::DaisyHarmoniqsCore::kPreferredBlockSize> left{};
    std::array<float, daisyhost::DaisyHarmoniqsCore::kPreferredBlockSize> right{};

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

const daisyhost::DaisyHarmoniqsParameter* RequireParameter(
    const daisyhost::DaisyHarmoniqsCore& core,
    const char*                          id)
{
    const auto* parameter = core.FindParameter(id);
    EXPECT_NE(parameter, nullptr) << id;
    return parameter;
}

TEST(DaisyHarmoniqsCoreTest, ExposesStableParametersAndSpectrumPageByDefault)
{
    daisyhost::DaisyHarmoniqsCore core;
    core.ResetToDefaultState(17u);

    const auto& parameters = core.GetParameters();
    ASSERT_EQ(parameters.size(), 16u);

    EXPECT_NE(RequireParameter(core, "brightness"), nullptr);
    EXPECT_NE(RequireParameter(core, "tilt"), nullptr);
    EXPECT_NE(RequireParameter(core, "odd_even"), nullptr);
    EXPECT_NE(RequireParameter(core, "spread"), nullptr);
    EXPECT_NE(RequireParameter(core, "attack"), nullptr);
    EXPECT_NE(RequireParameter(core, "release"), nullptr);
    EXPECT_NE(RequireParameter(core, "detune"), nullptr);
    EXPECT_NE(RequireParameter(core, "level"), nullptr);
    EXPECT_NE(RequireParameter(core, "harmonic_1"), nullptr);
    EXPECT_NE(RequireParameter(core, "harmonic_8"), nullptr);

    const auto pageBinding = core.GetActivePageBinding();
    EXPECT_EQ(pageBinding.pageLabel, "Spectrum");
    EXPECT_EQ(pageBinding.parameterIds[0], "brightness");
    EXPECT_EQ(pageBinding.parameterIds[1], "tilt");
    EXPECT_EQ(pageBinding.parameterIds[2], "odd_even");
    EXPECT_EQ(pageBinding.parameterIds[3], "spread");
}

TEST(DaisyHarmoniqsCoreTest,
     SupportsDeterministicResetCaptureRestoreAndSpectrumRandomization)
{
    daisyhost::DaisyHarmoniqsCore first;
    daisyhost::DaisyHarmoniqsCore second;

    first.Prepare(48000.0, daisyhost::DaisyHarmoniqsCore::kPreferredBlockSize);
    second.Prepare(48000.0, daisyhost::DaisyHarmoniqsCore::kPreferredBlockSize);

    first.ResetToDefaultState(1234u);
    second.ResetToDefaultState(1234u);

    ASSERT_TRUE(first.SetParameterValue("brightness", 0.7f));
    ASSERT_TRUE(first.SetParameterValue("tilt", 0.35f));
    ASSERT_TRUE(first.SetParameterValue("harmonic_3", 0.8f));

    first.TriggerMidiNote(48, 110);
    second.RestoreStatefulParameterValues(first.CaptureStatefulParameterValues());
    second.TriggerMidiNote(48, 110);

    EXPECT_FLOAT_EQ(RenderChecksum(first), RenderChecksum(second));

    const auto before = first.CaptureStatefulParameterValues();
    ASSERT_TRUE(first.TriggerMomentaryAction("randomize_spectrum"));
    const auto after = first.CaptureStatefulParameterValues();
    ASSERT_NE(before.at("harmonic_1"), after.at("harmonic_1"));
}

TEST(DaisyHarmoniqsCoreTest, ProducesSoundFromMidiAndGateTriggers)
{
    daisyhost::DaisyHarmoniqsCore core;
    core.Prepare(48000.0, daisyhost::DaisyHarmoniqsCore::kPreferredBlockSize);
    core.ResetToDefaultState(91u);

    EXPECT_LT(OutputEnergy(core, 1), 0.0001f);

    core.TriggerMidiNote(45, 100);
    EXPECT_GT(OutputEnergy(core, 4), 0.01f);

    core.TriggerMomentaryAction("panic");
    core.TriggerGate(true);
    EXPECT_GT(OutputEnergy(core, 4), 0.01f);
    core.TriggerGate(false);
}

TEST(DaisyHarmoniqsCoreTest, UsesEnvelopePageBinding)
{
    daisyhost::DaisyHarmoniqsCore core;
    core.Prepare(48000.0, daisyhost::DaisyHarmoniqsCore::kPreferredBlockSize);
    core.ResetToDefaultState(0u);

    ASSERT_TRUE(core.SetActivePage(daisyhost::DaisyHarmoniqsPage::kEnvelope));
    const auto pageBinding = core.GetActivePageBinding();
    EXPECT_EQ(pageBinding.pageLabel, "Envelope");
    EXPECT_EQ(pageBinding.parameterIds[0], "attack");
    EXPECT_EQ(pageBinding.parameterIds[1], "release");
    EXPECT_EQ(pageBinding.parameterIds[2], "detune");
    EXPECT_EQ(pageBinding.parameterIds[3], "level");
}
} // namespace
