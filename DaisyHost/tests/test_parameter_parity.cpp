#include <array>
#include <vector>

#include <gtest/gtest.h>

#include "daisyhost/apps/MultiDelayCore.h"

namespace
{
float RenderChecksum(daisyhost::apps::MultiDelayCore& core)
{
    std::vector<float> inputSignal(64, 1.0f);
    std::array<std::vector<float>, 4> outputs;
    for(auto& channel : outputs)
    {
        channel.assign(64, 0.0f);
    }

    std::array<const float*, 1> inputPtrs = {{inputSignal.data()}};
    std::array<float*, 4> outputPtrs = {
        {outputs[0].data(), outputs[1].data(), outputs[2].data(), outputs[3].data()}};

    core.Process({inputPtrs.data(), inputPtrs.size()},
                 {outputPtrs.data(), outputPtrs.size()},
                 64);

    float checksum = 0.0f;
    for(float sample : outputs[3])
    {
        checksum += sample;
    }
    return checksum;
}

TEST(ParameterParityTest, DirectParameterMatchesMenuEditForSameStateAndOutput)
{
    daisyhost::apps::MultiDelayCore menuCore("node0");
    daisyhost::apps::MultiDelayCore directCore("node0");
    menuCore.Prepare(48000.0, 64);
    directCore.Prepare(48000.0, 64);

    menuCore.ResetToDefaultState(99u);
    directCore.ResetToDefaultState(99u);

    menuCore.SetMenuItemValue("node0/menu/params/delay_primary", 0.64f);
    ASSERT_TRUE(directCore.SetParameterValue("node0/param/delay_primary", 0.64f));

    const auto menuSnapshot = menuCore.CaptureStatefulParameterValues();
    const auto directSnapshot = directCore.CaptureStatefulParameterValues();
    EXPECT_EQ(menuSnapshot, directSnapshot);

    EXPECT_NEAR(RenderChecksum(menuCore), RenderChecksum(directCore), 0.0001f);
}
} // namespace
