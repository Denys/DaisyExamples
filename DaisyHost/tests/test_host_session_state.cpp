#include <gtest/gtest.h>

#include "daisyhost/HostSessionState.h"

namespace
{
TEST(HostSessionStateTest, RoundTripsSerializedControlAndMidiLearnState)
{
    daisyhost::HostSessionState state;
    state.appId                                  = "multidelay";
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

    EXPECT_EQ(restored.appId, "multidelay");
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

TEST(HostSessionStateTest, VersionTwoRoundTripsCanonicalParametersAndSeed)
{
    daisyhost::HostSessionState state;
    state.appId                                 = "torus";
    state.controlValues["node0/control/ctrl_1"]   = 0.25f;
    state.parameterValues["node0/param/dry_wet"]  = 0.60f;
    state.parameterValues["node0/param/feedback"] = 0.35f;
    state.randomSeed                              = 1234u;
    state.midiLearn.Assign(71, "node0/control/drywet");

    const std::string serialized = state.Serialize();
    const auto restored
        = daisyhost::HostSessionState::Deserialize(serialized);

    EXPECT_EQ(restored.appId, "torus");
    EXPECT_EQ(restored.randomSeed, 1234u);
    ASSERT_EQ(restored.parameterValues.size(), 2u);
    EXPECT_FLOAT_EQ(restored.parameterValues.at("node0/param/dry_wet"), 0.60f);
    EXPECT_FLOAT_EQ(restored.parameterValues.at("node0/param/feedback"), 0.35f);
    EXPECT_FLOAT_EQ(restored.controlValues.at("node0/control/ctrl_1"), 0.25f);
}

TEST(HostSessionStateTest, VersionOneSessionStillDeserializesWithoutParameterLoss)
{
    const std::string versionOneText
        = "version 1\n"
          "control node0/control/ctrl_1 0.25\n"
          "cv node0/port/cv_in_1 0.75\n"
          "gate node0/port/gate_in_1 1\n"
          "midi 74 node0/control/ctrl_1\n";

    const auto restored
        = daisyhost::HostSessionState::Deserialize(versionOneText);

    EXPECT_TRUE(restored.appId.empty());
    EXPECT_TRUE(restored.parameterValues.empty());
    EXPECT_EQ(restored.randomSeed, 0u);
    EXPECT_FLOAT_EQ(restored.controlValues.at("node0/control/ctrl_1"), 0.25f);
    EXPECT_FLOAT_EQ(restored.cvValues.at("node0/port/cv_in_1"), 0.75f);
    EXPECT_TRUE(restored.gateValues.at("node0/port/gate_in_1"));
}

TEST(HostSessionStateTest, VersionThreeRoundTripsAppIdAndCanonicalParameters)
{
    daisyhost::HostSessionState state;
    state.appId                                 = "torus";
    state.parameterValues["node0/param/frequency"] = 0.42f;
    state.parameterValues["node0/param/model"]     = 0.80f;
    state.randomSeed                               = 7788u;

    const std::string serialized = state.Serialize();
    const auto restored
        = daisyhost::HostSessionState::Deserialize(serialized);

    EXPECT_EQ(restored.appId, "torus");
    EXPECT_EQ(restored.randomSeed, 7788u);
    EXPECT_FLOAT_EQ(restored.parameterValues.at("node0/param/frequency"), 0.42f);
    EXPECT_FLOAT_EQ(restored.parameterValues.at("node0/param/model"), 0.80f);
}

TEST(HostSessionStateTest, RoundTripsHostGeneratorControlValues)
{
    daisyhost::HostSessionState state;
    state.controlValues["node0/host/audio_in_1_source"] = 3.0f;
    state.controlValues["node0/host/audio_in_1_frequency_hz"] = 440.0f;
    state.controlValues["node0/host/audio_in_1_level_volts"] = 7.5f;
    state.controlValues["node0/host/cv_1_mode"] = 1.0f;
    state.controlValues["node0/host/cv_1_waveform"] = 2.0f;
    state.controlValues["node0/host/cv_1_frequency_hz"] = 1.5f;
    state.controlValues["node0/host/cv_1_amplitude_volts"] = 1.25f;
    state.controlValues["node0/host/cv_1_bias_volts"] = 3.5f;
    state.controlValues["node0/host/cv_1_manual_volts"] = 4.0f;

    const auto restored = daisyhost::HostSessionState::Deserialize(state.Serialize());

    EXPECT_FLOAT_EQ(restored.controlValues.at("node0/host/audio_in_1_source"), 3.0f);
    EXPECT_FLOAT_EQ(restored.controlValues.at("node0/host/audio_in_1_frequency_hz"),
                    440.0f);
    EXPECT_FLOAT_EQ(restored.controlValues.at("node0/host/audio_in_1_level_volts"),
                    7.5f);
    EXPECT_FLOAT_EQ(restored.controlValues.at("node0/host/cv_1_mode"), 1.0f);
    EXPECT_FLOAT_EQ(restored.controlValues.at("node0/host/cv_1_waveform"), 2.0f);
    EXPECT_FLOAT_EQ(restored.controlValues.at("node0/host/cv_1_frequency_hz"), 1.5f);
    EXPECT_FLOAT_EQ(restored.controlValues.at("node0/host/cv_1_amplitude_volts"),
                    1.25f);
    EXPECT_FLOAT_EQ(restored.controlValues.at("node0/host/cv_1_bias_volts"), 3.5f);
    EXPECT_FLOAT_EQ(restored.controlValues.at("node0/host/cv_1_manual_volts"), 4.0f);
}
} // namespace
