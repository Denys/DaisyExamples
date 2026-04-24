#include <array>
#include <cmath>
#include <string_view>

#include <gtest/gtest.h>

#include "daisyhost/DaisyBraidsCore.h"

namespace
{
float RenderChecksum(daisyhost::DaisyBraidsCore& core)
{
    std::array<float, daisyhost::DaisyBraidsCore::kPreferredBlockSize> left{};
    std::array<float, daisyhost::DaisyBraidsCore::kPreferredBlockSize> right{};

    core.Process(left.data(), right.data(), left.size());

    float checksum = 0.0f;
    for(std::size_t i = 0; i < left.size(); ++i)
    {
        checksum += left[i] * 0.37f + right[i] * 0.63f;
    }
    return checksum;
}

float OutputEnergy(daisyhost::DaisyBraidsCore& core, int blockCount = 1)
{
    std::array<float, daisyhost::DaisyBraidsCore::kPreferredBlockSize> left{};
    std::array<float, daisyhost::DaisyBraidsCore::kPreferredBlockSize> right{};

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

const daisyhost::DaisyBraidsParameter* RequireParameter(
    const daisyhost::DaisyBraidsCore& core,
    const char*                       id)
{
    const auto* parameter = core.FindParameter(id);
    EXPECT_NE(parameter, nullptr) << id;
    return parameter;
}

TEST(DaisyBraidsCoreTest, ExposesStableParametersAndDrumPageByDefault)
{
    daisyhost::DaisyBraidsCore core;
    core.ResetToDefaultState(17u);

    const auto& parameters = core.GetParameters();
    ASSERT_EQ(parameters.size(), 8u);

    const auto* model = RequireParameter(core, "model");
    const auto* tune  = RequireParameter(core, "tune");
    const auto* timbre = RequireParameter(core, "timbre");
    const auto* color = RequireParameter(core, "color");
    const auto* resolution = RequireParameter(core, "resolution");
    const auto* sampleRate = RequireParameter(core, "sample_rate");
    const auto* signature = RequireParameter(core, "signature");
    const auto* level = RequireParameter(core, "level");

    ASSERT_NE(model, nullptr);
    ASSERT_NE(tune, nullptr);
    ASSERT_NE(timbre, nullptr);
    ASSERT_NE(color, nullptr);
    ASSERT_NE(resolution, nullptr);
    ASSERT_NE(sampleRate, nullptr);
    ASSERT_NE(signature, nullptr);
    ASSERT_NE(level, nullptr);

    EXPECT_TRUE(model->automatable);
    EXPECT_TRUE(tune->automatable);
    EXPECT_TRUE(timbre->automatable);
    EXPECT_TRUE(color->automatable);
    EXPECT_TRUE(signature->automatable);
    EXPECT_TRUE(level->stateful);

    const auto pageBinding = core.GetActivePageBinding();
    EXPECT_EQ(pageBinding.pageLabel, "Drum");
    EXPECT_EQ(pageBinding.parameterIds[0], "tune");
    EXPECT_EQ(pageBinding.parameterIds[1], "timbre");
    EXPECT_EQ(pageBinding.parameterIds[2], "color");
    EXPECT_EQ(pageBinding.parameterIds[3], "model");

    const auto modelLabels = core.GetModelLabels();
    ASSERT_EQ(modelLabels.size(), 6u);
    EXPECT_EQ(modelLabels[0], "Kick");
    EXPECT_EQ(modelLabels[1], "Snare");
    EXPECT_EQ(modelLabels[2], "Cymbal");
    EXPECT_EQ(modelLabels[3], "Drum");
    EXPECT_EQ(modelLabels[4], "Bell");
    EXPECT_EQ(modelLabels[5], "Filtered Noise");
}

TEST(DaisyBraidsCoreTest, SupportsDeterministicResetCaptureRestoreAndModelRandomization)
{
    daisyhost::DaisyBraidsCore first;
    daisyhost::DaisyBraidsCore second;

    first.Prepare(48000.0, daisyhost::DaisyBraidsCore::kPreferredBlockSize);
    second.Prepare(48000.0, daisyhost::DaisyBraidsCore::kPreferredBlockSize);

    first.ResetToDefaultState(1234u);
    second.ResetToDefaultState(1234u);

    ASSERT_TRUE(first.SetParameterValue("model", 0.8f));
    ASSERT_TRUE(first.SetParameterValue("tune", 0.75f));
    ASSERT_TRUE(first.SetParameterValue("timbre", 0.55f));
    ASSERT_TRUE(first.SetParameterValue("signature", 0.25f));

    first.TriggerMidiNote(48, 100);
    second.RestoreStatefulParameterValues(first.CaptureStatefulParameterValues());
    second.TriggerMidiNote(48, 100);

    EXPECT_FLOAT_EQ(RenderChecksum(first), RenderChecksum(second));

    const auto before = first.CaptureStatefulParameterValues();
    ASSERT_TRUE(first.TriggerMomentaryAction("randomize_model"));
    const auto after = first.CaptureStatefulParameterValues();
    ASSERT_NE(before.at("model"), after.at("model"));
    EXPECT_GE(after.at("model"), 0.0f);
    EXPECT_LE(after.at("model"), 1.0f);
}

TEST(DaisyBraidsCoreTest, ProducesSoundFromMidiAndGateTriggers)
{
    daisyhost::DaisyBraidsCore core;
    core.Prepare(48000.0, daisyhost::DaisyBraidsCore::kPreferredBlockSize);
    core.ResetToDefaultState(91u);

    EXPECT_LT(OutputEnergy(core, 1), 0.0001f);

    core.TriggerMidiNote(36, 110);
    EXPECT_GT(OutputEnergy(core, 4), 0.01f);

    core.Panic();
    core.TriggerGate(true);
    EXPECT_GT(OutputEnergy(core, 4), 0.01f);
    core.TriggerGate(false);
}

TEST(DaisyBraidsCoreTest, UsesFinishPageAndSupportsInnerBlockProcessing)
{
    daisyhost::DaisyBraidsCore core;
    core.Prepare(48000.0, daisyhost::DaisyBraidsCore::kPreferredBlockSize);
    core.ResetToDefaultState(0u);

    ASSERT_TRUE(core.SetActivePage(daisyhost::DaisyBraidsPage::kFinish));
    const auto pageBinding = core.GetActivePageBinding();
    EXPECT_EQ(pageBinding.pageLabel, "Finish");
    EXPECT_EQ(pageBinding.parameterIds[0], "resolution");
    EXPECT_EQ(pageBinding.parameterIds[1], "sample_rate");
    EXPECT_EQ(pageBinding.parameterIds[2], "signature");
    EXPECT_EQ(pageBinding.parameterIds[3], "level");

    core.TriggerMidiNote(43, 96);
    const float energy = OutputEnergy(core, 2);
    EXPECT_GT(energy, 0.01f);
}
} // namespace
