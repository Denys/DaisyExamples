#include <gtest/gtest.h>

#include "daisyhost/HostSessionState.h"

namespace
{
TEST(HostSessionStateTest, RoundTripsSerializedControlAndMidiLearnState)
{
    daisyhost::HostSessionState state;
    state.controlValues["node0/control/ctrl_1"] = 0.25f;
    state.controlValues["node0/control/drywet"] = 0.60f;
    state.controlValues["node0/host/computer_keyboard_enabled"] = 1.0f;
    state.controlValues["node0/host/computer_keyboard_octave"]  = 5.0f;
    state.cvValues["node0/port/cv_in_1"]        = 0.75f;
    state.gateValues["node0/port/gate_in_1"]    = true;
    state.midiLearn.Assign(74, "node0/control/ctrl_1");
    state.midiLearn.Assign(71, "node0/control/drywet");

    const std::string serialized = state.Serialize();
    const auto        restored
        = daisyhost::HostSessionState::Deserialize(serialized);

    ASSERT_EQ(restored.controlValues.size(), 4u);
    ASSERT_EQ(restored.cvValues.size(), 1u);
    ASSERT_EQ(restored.gateValues.size(), 1u);
    EXPECT_FLOAT_EQ(restored.controlValues.at("node0/control/ctrl_1"), 0.25f);
    EXPECT_FLOAT_EQ(restored.controlValues.at("node0/control/drywet"), 0.60f);
    EXPECT_FLOAT_EQ(restored.controlValues.at("node0/host/computer_keyboard_enabled"),
                    1.0f);
    EXPECT_FLOAT_EQ(restored.controlValues.at("node0/host/computer_keyboard_octave"),
                    5.0f);
    EXPECT_FLOAT_EQ(restored.cvValues.at("node0/port/cv_in_1"), 0.75f);
    EXPECT_TRUE(restored.gateValues.at("node0/port/gate_in_1"));

    std::string mappedControl;
    ASSERT_TRUE(restored.midiLearn.TryLookup(74, &mappedControl));
    EXPECT_EQ(mappedControl, "node0/control/ctrl_1");
    ASSERT_TRUE(restored.midiLearn.TryLookup(71, &mappedControl));
    EXPECT_EQ(mappedControl, "node0/control/drywet");
}
} // namespace
