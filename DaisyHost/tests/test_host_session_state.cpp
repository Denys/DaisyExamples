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

TEST(HostSessionStateTest, VersionFourRoundTripsNodeMetadataAndRoutes)
{
    daisyhost::HostSessionState state;
    state.nodes.push_back({"node0", "multidelay", 123u});
    state.nodes.push_back({"node1", "cloudseed", 456u});
    state.routes.push_back({"node0/port/audio_out_1", "node1/port/audio_in_1"});
    state.routes.push_back({"node0/port/audio_out_2", "node1/port/audio_in_2"});
    state.parameterValues["node0/param/dry_wet"] = 0.60f;
    state.parameterValues["node1/param/mix"]     = 0.35f;
    state.cvValues["node0/port/cv_in_1"]         = 0.75f;
    state.gateValues["node1/port/gate_in_1"]     = true;

    const auto restored = daisyhost::HostSessionState::Deserialize(state.Serialize());

    ASSERT_EQ(restored.nodes.size(), 2u);
    EXPECT_EQ(restored.nodes[0].nodeId, "node0");
    EXPECT_EQ(restored.nodes[0].appId, "multidelay");
    EXPECT_EQ(restored.nodes[0].randomSeed, 123u);
    EXPECT_EQ(restored.nodes[1].nodeId, "node1");
    EXPECT_EQ(restored.nodes[1].appId, "cloudseed");
    EXPECT_EQ(restored.nodes[1].randomSeed, 456u);
    ASSERT_EQ(restored.routes.size(), 2u);
    EXPECT_EQ(restored.routes[0].sourcePortId, "node0/port/audio_out_1");
    EXPECT_EQ(restored.routes[0].destPortId, "node1/port/audio_in_1");
    EXPECT_EQ(restored.routes[1].sourcePortId, "node0/port/audio_out_2");
    EXPECT_EQ(restored.routes[1].destPortId, "node1/port/audio_in_2");
    EXPECT_EQ(restored.appId, "multidelay");
    EXPECT_EQ(restored.randomSeed, 123u);
}

TEST(HostSessionStateTest, VersionFiveRoundTripsRackGlobalsAlongsideNodesAndRoutes)
{
    daisyhost::HostSessionState state;
    state.boardId        = "daisy_patch";
    state.selectedNodeId = "node1";
    state.entryNodeId    = "node1";
    state.outputNodeId   = "node0";
    state.nodes.push_back({"node0", "multidelay", 123u});
    state.nodes.push_back({"node1", "cloudseed", 456u});
    state.routes.push_back({"node1/port/audio_out_1", "node0/port/audio_in_1"});
    state.routes.push_back({"node1/port/audio_out_2", "node0/port/audio_in_2"});

    const auto restored = daisyhost::HostSessionState::Deserialize(state.Serialize());

    EXPECT_EQ(restored.boardId, "daisy_patch");
    EXPECT_EQ(restored.selectedNodeId, "node1");
    EXPECT_EQ(restored.entryNodeId, "node1");
    EXPECT_EQ(restored.outputNodeId, "node0");
    ASSERT_EQ(restored.nodes.size(), 2u);
    ASSERT_EQ(restored.routes.size(), 2u);
    EXPECT_EQ(restored.routes[0].sourcePortId, "node1/port/audio_out_1");
    EXPECT_EQ(restored.routes[0].destPortId, "node0/port/audio_in_1");
}

TEST(HostSessionStateTest, VersionFiveRoundTripsFieldBoardIdWithoutChangingRackShape)
{
    daisyhost::HostSessionState state;
    state.boardId        = "daisy_field";
    state.selectedNodeId = "node1";
    state.entryNodeId    = "node0";
    state.outputNodeId   = "node1";
    state.nodes.push_back({"node0", "multidelay", 123u});
    state.nodes.push_back({"node1", "cloudseed", 456u});
    state.routes.push_back({"node0/port/audio_out_1", "node1/port/audio_in_1"});
    state.routes.push_back({"node0/port/audio_out_2", "node1/port/audio_in_2"});

    const auto restored = daisyhost::HostSessionState::Deserialize(state.Serialize());

    EXPECT_EQ(restored.boardId, "daisy_field");
    EXPECT_EQ(restored.selectedNodeId, "node1");
    EXPECT_EQ(restored.entryNodeId, "node0");
    EXPECT_EQ(restored.outputNodeId, "node1");
    ASSERT_EQ(restored.nodes.size(), 2u);
    ASSERT_EQ(restored.routes.size(), 2u);
}

TEST(HostSessionStateTest, LegacySessionsSynthesizeNodeZeroMetadata)
{
    const std::string versionThreeText
        = "version 3\n"
          "app torus\n"
          "param node0/param/frequency 0.42\n"
          "seed 7788\n";

    const auto restored = daisyhost::HostSessionState::Deserialize(versionThreeText);

    ASSERT_EQ(restored.nodes.size(), 1u);
    EXPECT_EQ(restored.nodes[0].nodeId, "node0");
    EXPECT_EQ(restored.nodes[0].appId, "torus");
    EXPECT_EQ(restored.nodes[0].randomSeed, 7788u);
    EXPECT_TRUE(restored.routes.empty());
}

TEST(HostSessionStateTest, LegacySessionsSynthesizeRackDefaults)
{
    const std::string versionFourText
        = "version 4\n"
          "node node0 torus 7788\n"
          "param node0/param/frequency 0.42\n";

    const auto restored = daisyhost::HostSessionState::Deserialize(versionFourText);

    EXPECT_EQ(restored.boardId, "daisy_patch");
    EXPECT_EQ(restored.selectedNodeId, "node0");
    EXPECT_EQ(restored.entryNodeId, "node0");
    EXPECT_EQ(restored.outputNodeId, "node0");
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
