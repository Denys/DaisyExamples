#include <gtest/gtest.h>

#include "daisyhost/BoardProfile.h"

namespace
{
TEST(BoardProfileTest, DaisyPatchProfileHasExpectedControlsAndPorts)
{
    const daisyhost::BoardProfile profile
        = daisyhost::MakeDaisyPatchProfile("nodeA");

    EXPECT_EQ(profile.boardId, "daisy_patch");
    EXPECT_EQ(profile.nodeId, "nodeA");
    EXPECT_EQ(profile.controls.size(), 6u);
    EXPECT_EQ(profile.display.width, 128);
    EXPECT_EQ(profile.display.height, 64);

    std::size_t audioInputs  = 0;
    std::size_t audioOutputs = 0;
    std::size_t cvInputs     = 0;
    std::size_t gateInputs   = 0;
    std::size_t midiPorts    = 0;

    for(const auto& port : profile.ports)
    {
        switch(port.type)
        {
            case daisyhost::VirtualPortType::kAudio:
                if(port.direction == daisyhost::PortDirection::kInput)
                {
                    ++audioInputs;
                }
                else
                {
                    ++audioOutputs;
                }
                break;
            case daisyhost::VirtualPortType::kCv: ++cvInputs; break;
            case daisyhost::VirtualPortType::kGate:
                if(port.direction == daisyhost::PortDirection::kInput)
                {
                    ++gateInputs;
                }
                break;
            case daisyhost::VirtualPortType::kMidi: ++midiPorts; break;
        }
    }

    EXPECT_EQ(audioInputs, 4u);
    EXPECT_EQ(audioOutputs, 4u);
    EXPECT_EQ(cvInputs, 4u);
    EXPECT_EQ(gateInputs, 2u);
    EXPECT_EQ(midiPorts, 2u);
}
} // namespace
