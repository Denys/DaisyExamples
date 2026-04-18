#include <array>
#include <vector>

#include <gtest/gtest.h>

#include "daisyhost/apps/MultiDelayCore.h"

namespace
{
TEST(MultiDelayCoreTest, EncoderAndDryWetControlUpdateDisplay)
{
    daisyhost::apps::MultiDelayCore core("node0");
    core.Prepare(48000.0, 64);

    core.SetEncoderDelta(3);
    EXPECT_EQ(core.GetDryWetPercent(), 65);
    EXPECT_EQ(core.GetDisplayModel().texts.at(2).text, "65%");

    core.SetControl(
        daisyhost::apps::MultiDelayCore::MakeDryWetControlId("node0"), 0.20f);
    EXPECT_EQ(core.GetDryWetPercent(), 20);
    EXPECT_EQ(core.GetDisplayModel().texts.at(2).text, "20%");
}

TEST(MultiDelayCoreTest, KnobControlsMapToDelayTargetsAndFeedback)
{
    daisyhost::apps::MultiDelayCore core("node0");
    core.Prepare(48000.0, 32);

    core.SetControl(
        daisyhost::apps::MultiDelayCore::MakeKnobControlId("node0", 1), 0.0f);
    core.SetControl(
        daisyhost::apps::MultiDelayCore::MakeKnobControlId("node0", 2), 1.0f);
    core.SetControl(
        daisyhost::apps::MultiDelayCore::MakeKnobControlId("node0", 4), 0.5f);

    EXPECT_NEAR(core.GetDelayTargetSamples(0), 2400.0f, 0.5f);
    EXPECT_NEAR(core.GetDelayTargetSamples(1), 48000.0f, 1.0f);
    EXPECT_FLOAT_EQ(core.GetFeedback(), 0.5f);
}

TEST(MultiDelayCoreTest, FixedInputProducesDeterministicOutputAndPortLevels)
{
    daisyhost::apps::MultiDelayCore core("node0");
    core.Prepare(48000.0, 64);
    core.SetControl(
        daisyhost::apps::MultiDelayCore::MakeDryWetControlId("node0"), 0.5f);
    core.SetControl(
        daisyhost::apps::MultiDelayCore::MakeKnobControlId("node0", 1), 0.0f);
    core.SetControl(
        daisyhost::apps::MultiDelayCore::MakeKnobControlId("node0", 2), 0.0f);
    core.SetControl(
        daisyhost::apps::MultiDelayCore::MakeKnobControlId("node0", 3), 0.0f);
    core.SetControl(
        daisyhost::apps::MultiDelayCore::MakeKnobControlId("node0", 4), 0.0f);

    std::vector<float> inputSignal(64, 1.0f);
    std::array<std::vector<float>, 4> outputs;
    for(auto& channel : outputs)
    {
        channel.assign(64, -1.0f);
    }

    std::array<const float*, 1> inputPtrs = {{inputSignal.data()}};
    std::array<float*, 4> outputPtrs = {
        {outputs[0].data(), outputs[1].data(), outputs[2].data(), outputs[3].data()}};

    core.Process({inputPtrs.data(), inputPtrs.size()},
                 {outputPtrs.data(), outputPtrs.size()},
                 64);

    EXPECT_NEAR(outputs[0][0], 0.0f, 0.0001f);
    EXPECT_NEAR(outputs[1][0], 0.0f, 0.0001f);
    EXPECT_NEAR(outputs[2][0], 0.0f, 0.0001f);
    EXPECT_NEAR(outputs[3][0], 0.5f, 0.0001f);
    EXPECT_NEAR(outputs[0][1], 0.95990407f, 0.0001f);
    EXPECT_NEAR(outputs[3][1], 0.93195683f, 0.0001f);

    float checksum = 0.0f;
    for(std::size_t frame = 0; frame < 64; ++frame)
    {
        checksum += outputs[3][frame];
    }

    EXPECT_NEAR(checksum, 60.331989f, 0.0001f);

    const auto mixPort = core.GetPortOutput(
        daisyhost::apps::MultiDelayCore::MakeAudioOutputPortId("node0", 4));
    EXPECT_NEAR(mixPort.scalar, 0.95000005f, 0.0001f);
}
} // namespace
