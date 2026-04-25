#include <filesystem>

#include <gtest/gtest.h>

#include "daisyhost/AppRegistry.h"
#include "daisyhost/RenderRuntime.h"

namespace
{
std::string FirstParameterId(const std::string& appId)
{
    auto app = daisyhost::CreateHostedAppCore(appId, "node0");
    EXPECT_NE(app, nullptr);
    EXPECT_FALSE(app->GetParameters().empty());
    return app->GetParameters().front().id;
}

std::string FirstCvPortId(const std::string& appId)
{
    auto app = daisyhost::CreateHostedAppCore(appId, "node0");
    EXPECT_NE(app, nullptr);
    const auto bindings = app->GetPatchBindings();
    return bindings.cvInputPortIds[0];
}

std::string FirstGatePortId(const std::string& appId)
{
    auto app = daisyhost::CreateHostedAppCore(appId, "node0");
    EXPECT_NE(app, nullptr);
    const auto bindings = app->GetPatchBindings();
    return bindings.gateInputPortIds[0];
}

daisyhost::RenderScenario MakeBaseScenario(const std::string& appId)
{
    daisyhost::RenderScenario scenario;
    scenario.appId                           = appId;
    scenario.renderConfig.sampleRate         = 48000.0;
    scenario.renderConfig.blockSize          = 48;
    scenario.renderConfig.durationSeconds    = 0.25;
    scenario.renderConfig.outputChannelCount = 2;
    scenario.seed                            = 123u;
    scenario.audioInput.mode
        = static_cast<int>(daisyhost::TestInputSignalMode::kSineInput);
    scenario.audioInput.level       = 6.0f;
    scenario.audioInput.frequencyHz = 220.0f;
    return scenario;
}

TEST(RenderRuntimeTest, ParsesScenarioJsonWithTimeline)
{
    const auto parameterId = FirstParameterId("multidelay");
    const std::string json = R"({
  "appId": "multidelay",
  "renderConfig": {
    "sampleRate": 48000,
    "blockSize": 48,
    "durationSeconds": 0.5,
    "outputChannelCount": 2
  },
  "seed": 77,
  "initialParameterValues": {
    ")" + parameterId + R"(": 0.25
  },
  "audioInput": {
    "mode": "triangle",
    "level": 4.0,
    "frequencyHz": 110.0
  },
  "timeline": [
    {
      "timeSeconds": 0.1,
      "type": "parameter_set",
      "parameterId": ")" + parameterId + R"(",
      "normalizedValue": 0.75
    },
    {
      "timeSeconds": 0.2,
      "type": "impulse"
    }
  ]
})";

    daisyhost::RenderScenario scenario;
    std::string               errorMessage;
    ASSERT_TRUE(daisyhost::ParseRenderScenarioJson(json, &scenario, &errorMessage))
        << errorMessage;
    EXPECT_EQ(scenario.appId, "multidelay");
    EXPECT_EQ(scenario.renderConfig.outputChannelCount, 2);
    EXPECT_EQ(scenario.seed, 77u);
    ASSERT_EQ(scenario.initialParameterValues.size(), 1u);
    EXPECT_FLOAT_EQ(scenario.initialParameterValues.begin()->second, 0.25f);
    EXPECT_EQ(scenario.audioInput.mode,
              static_cast<int>(daisyhost::TestInputSignalMode::kTriangleInput));
    ASSERT_EQ(scenario.timeline.size(), 2u);
    EXPECT_EQ(scenario.timeline[0].type,
              daisyhost::RenderTimelineEventType::kParameterSet);
    EXPECT_EQ(scenario.timeline[1].type,
              daisyhost::RenderTimelineEventType::kImpulse);
}

TEST(RenderRuntimeTest, ParsesFieldSurfaceControlSetTimelineEvent)
{
    const std::string json = R"({
  "boardId": "daisy_field",
  "appId": "vasynth",
  "timeline": [
    {
      "timeSeconds": 0.1,
      "type": "surface_control_set",
      "controlId": "node0/control/field_knob_5",
      "normalizedValue": 0.82
    }
  ]
})";

    daisyhost::RenderScenario scenario;
    std::string               errorMessage;
    ASSERT_TRUE(daisyhost::ParseRenderScenarioJson(json, &scenario, &errorMessage))
        << errorMessage;

    ASSERT_EQ(scenario.timeline.size(), 1u);
    EXPECT_EQ(scenario.timeline[0].type,
              daisyhost::RenderTimelineEventType::kSurfaceControlSet);
    EXPECT_EQ(scenario.timeline[0].controlId, "node0/control/field_knob_5");
    EXPECT_FLOAT_EQ(scenario.timeline[0].normalizedValue, 0.82f);
}

TEST(RenderRuntimeTest, ParsesNodeAndRouteAwareScenarioJson)
{
    const std::string json = R"({
  "boardId": "daisy_patch",
  "selectedNodeId": "node1",
  "entryNodeId": "node0",
  "outputNodeId": "node1",
  "appId": "multidelay",
  "seed": 77,
  "nodes": [
    { "nodeId": "node0", "appId": "multidelay", "seed": 77 },
    { "nodeId": "node1", "appId": "cloudseed", "seed": 91 }
  ],
  "routes": [
    {
      "sourcePortId": "node0/port/audio_out_1",
      "destPortId": "node1/port/audio_in_1"
    },
    {
      "sourcePortId": "node0/port/audio_out_2",
      "destPortId": "node1/port/audio_in_2"
    }
  ]
})";

    daisyhost::RenderScenario scenario;
    std::string               errorMessage;
    ASSERT_TRUE(daisyhost::ParseRenderScenarioJson(json, &scenario, &errorMessage))
        << errorMessage;

    EXPECT_EQ(scenario.boardId, "daisy_patch");
    EXPECT_EQ(scenario.selectedNodeId, "node1");
    EXPECT_EQ(scenario.entryNodeId, "node0");
    EXPECT_EQ(scenario.outputNodeId, "node1");
    ASSERT_EQ(scenario.nodes.size(), 2u);
    EXPECT_EQ(scenario.nodes[0].nodeId, "node0");
    EXPECT_EQ(scenario.nodes[0].appId, "multidelay");
    EXPECT_EQ(scenario.nodes[0].seed, 77u);
    EXPECT_EQ(scenario.nodes[1].nodeId, "node1");
    EXPECT_EQ(scenario.nodes[1].appId, "cloudseed");
    EXPECT_EQ(scenario.nodes[1].seed, 91u);
    ASSERT_EQ(scenario.routes.size(), 2u);
    EXPECT_EQ(scenario.routes[0].sourcePortId, "node0/port/audio_out_1");
    EXPECT_EQ(scenario.routes[0].destPortId, "node1/port/audio_in_1");
}

TEST(RenderRuntimeTest, ParsesFieldBoardIdWithoutChangingDefaultRackGlobals)
{
    const std::string json = R"({
  "boardId": "daisy_field",
  "appId": "multidelay",
  "seed": 77
})";

    daisyhost::RenderScenario scenario;
    std::string               errorMessage;
    ASSERT_TRUE(daisyhost::ParseRenderScenarioJson(json, &scenario, &errorMessage))
        << errorMessage;

    EXPECT_EQ(scenario.boardId, "daisy_field");
    EXPECT_EQ(scenario.selectedNodeId, "node0");
    EXPECT_EQ(scenario.entryNodeId, "node0");
    EXPECT_EQ(scenario.outputNodeId, "node0");
    EXPECT_TRUE(scenario.nodes.empty());
    EXPECT_TRUE(scenario.routes.empty());
}

TEST(RenderRuntimeTest, ParsesTargetNodeIdForNodeScopedNonSignalEvents)
{
    const std::string json = R"({
  "appId": "multidelay",
  "nodes": [
    { "nodeId": "node0", "appId": "multidelay", "seed": 77 },
    { "nodeId": "node1", "appId": "cloudseed", "seed": 91 }
  ],
  "timeline": [
    {
      "timeSeconds": 0.0,
      "type": "audio_input_config",
      "targetNodeId": "node1",
      "audioMode": "sine",
      "audioLevel": 4.0
    },
    {
      "timeSeconds": 0.1,
      "type": "impulse",
      "targetNodeId": "node0"
    }
  ]
})";

    daisyhost::RenderScenario scenario;
    std::string               errorMessage;
    ASSERT_TRUE(daisyhost::ParseRenderScenarioJson(json, &scenario, &errorMessage))
        << errorMessage;

    ASSERT_EQ(scenario.timeline.size(), 2u);
    EXPECT_EQ(scenario.timeline[0].targetNodeId, "node1");
    EXPECT_EQ(scenario.timeline[1].targetNodeId, "node0");
}

TEST(RenderRuntimeTest, RejectsUnknownAppId)
{
    auto        scenario = MakeBaseScenario("definitely-not-an-app");
    std::string errorMessage;
    daisyhost::RenderResult result;
    EXPECT_FALSE(daisyhost::RunRenderScenario(scenario, &result, &errorMessage));
    EXPECT_NE(errorMessage.find("Unknown appId"), std::string::npos);
}

TEST(RenderRuntimeTest, RejectsUnknownBoardId)
{
    auto scenario = MakeBaseScenario("multidelay");
    scenario.boardId = "unknown_board";

    std::string errorMessage;
    daisyhost::RenderResult result;
    EXPECT_FALSE(daisyhost::RunRenderScenario(scenario, &result, &errorMessage));
    EXPECT_NE(errorMessage.find("boardId"), std::string::npos);
}

TEST(RenderRuntimeTest, RejectsUnknownParameterId)
{
    auto scenario = MakeBaseScenario("multidelay");
    scenario.initialParameterValues["node0/param/does_not_exist"] = 0.5f;

    std::string errorMessage;
    daisyhost::RenderResult result;
    EXPECT_FALSE(daisyhost::RunRenderScenario(scenario, &result, &errorMessage));
    EXPECT_NE(errorMessage.find("Unknown parameter id"), std::string::npos);
}

TEST(RenderRuntimeTest, ProducesDeterministicOfflineRenderForBraids)
{
    auto scenario = MakeBaseScenario("braids");
    scenario.renderConfig.durationSeconds = 0.5;
    scenario.audioInput.mode
        = static_cast<int>(daisyhost::TestInputSignalMode::kHostInput);
    scenario.initialParameterValues["node0/param/model"] = 0.0f;
    scenario.initialParameterValues["node0/param/timbre"] = 0.55f;
    scenario.initialParameterValues["node0/param/color"] = 0.35f;

    daisyhost::RenderTimelineEvent midiEvent;
    midiEvent.timeSeconds = 0.0;
    midiEvent.type = daisyhost::RenderTimelineEventType::kMidi;
    midiEvent.midiMessage.status = 0x90;
    midiEvent.midiMessage.data1 = 36;
    midiEvent.midiMessage.data2 = 110;
    scenario.timeline.push_back(midiEvent);

    daisyhost::RenderTimelineEvent pageEvent;
    pageEvent.timeSeconds = 0.1;
    pageEvent.type = daisyhost::RenderTimelineEventType::kMenuSetItem;
    pageEvent.menuItemId = "node0/menu/pages/page";
    pageEvent.normalizedValue = 1.0f;
    scenario.timeline.push_back(pageEvent);

    daisyhost::RenderTimelineEvent finishEvent;
    finishEvent.timeSeconds = 0.12;
    finishEvent.type = daisyhost::RenderTimelineEventType::kParameterSet;
    finishEvent.parameterId = "node0/param/signature";
    finishEvent.normalizedValue = 0.65f;
    scenario.timeline.push_back(finishEvent);

    daisyhost::RenderTimelineEvent modelEvent;
    modelEvent.timeSeconds = 0.2;
    modelEvent.type = daisyhost::RenderTimelineEventType::kMenuSetItem;
    modelEvent.menuItemId = "node0/menu/utilities/randomize_model";
    modelEvent.normalizedValue = 1.0f;
    scenario.timeline.push_back(modelEvent);

    daisyhost::RenderTimelineEvent gateEvent;
    gateEvent.timeSeconds = 0.3;
    gateEvent.type = daisyhost::RenderTimelineEventType::kGateSet;
    gateEvent.portId = "node0/port/gate_in_1";
    gateEvent.gateValue = true;
    scenario.timeline.push_back(gateEvent);

    daisyhost::RenderResult firstResult;
    daisyhost::RenderResult secondResult;
    std::string errorMessage;

    ASSERT_TRUE(daisyhost::RunRenderScenario(scenario, &firstResult, &errorMessage))
        << errorMessage;
    ASSERT_TRUE(daisyhost::RunRenderScenario(scenario, &secondResult, &errorMessage))
        << errorMessage;

    EXPECT_EQ(firstResult.manifest.appId, "braids");
    EXPECT_EQ(firstResult.manifest.audioChecksum, secondResult.manifest.audioChecksum);
    ASSERT_EQ(firstResult.audioChannels.size(), 2u);

    float energy = 0.0f;
    for(float sample : firstResult.audioChannels[0])
    {
        energy += std::abs(sample);
    }
    for(float sample : firstResult.audioChannels[1])
    {
        energy += std::abs(sample);
    }
    EXPECT_GT(energy, 0.01f);
}

TEST(RenderRuntimeTest, ProducesDeterministicOfflineRenderForHarmoniqs)
{
    auto scenario = MakeBaseScenario("harmoniqs");
    scenario.renderConfig.durationSeconds = 0.5;
    scenario.audioInput.mode
        = static_cast<int>(daisyhost::TestInputSignalMode::kHostInput);
    scenario.initialParameterValues["node0/param/brightness"] = 0.62f;
    scenario.initialParameterValues["node0/param/tilt"] = 0.38f;
    scenario.initialParameterValues["node0/param/harmonic_3"] = 0.81f;

    daisyhost::RenderTimelineEvent midiEvent;
    midiEvent.timeSeconds = 0.0;
    midiEvent.type = daisyhost::RenderTimelineEventType::kMidi;
    midiEvent.midiMessage.status = 0x90;
    midiEvent.midiMessage.data1 = 48;
    midiEvent.midiMessage.data2 = 110;
    scenario.timeline.push_back(midiEvent);

    daisyhost::RenderTimelineEvent pageEvent;
    pageEvent.timeSeconds = 0.1;
    pageEvent.type = daisyhost::RenderTimelineEventType::kMenuSetItem;
    pageEvent.menuItemId = "node0/menu/pages/page";
    pageEvent.normalizedValue = 1.0f;
    scenario.timeline.push_back(pageEvent);

    daisyhost::RenderTimelineEvent detuneEvent;
    detuneEvent.timeSeconds = 0.12;
    detuneEvent.type = daisyhost::RenderTimelineEventType::kParameterSet;
    detuneEvent.parameterId = "node0/param/detune";
    detuneEvent.normalizedValue = 0.68f;
    scenario.timeline.push_back(detuneEvent);

    daisyhost::RenderTimelineEvent randomizeEvent;
    randomizeEvent.timeSeconds = 0.2;
    randomizeEvent.type = daisyhost::RenderTimelineEventType::kMenuSetItem;
    randomizeEvent.menuItemId = "node0/menu/utilities/randomize_spectrum";
    randomizeEvent.normalizedValue = 1.0f;
    scenario.timeline.push_back(randomizeEvent);

    daisyhost::RenderTimelineEvent gateEvent;
    gateEvent.timeSeconds = 0.3;
    gateEvent.type = daisyhost::RenderTimelineEventType::kGateSet;
    gateEvent.portId = "node0/port/gate_in_1";
    gateEvent.gateValue = true;
    scenario.timeline.push_back(gateEvent);

    daisyhost::RenderTimelineEvent gateReleaseEvent;
    gateReleaseEvent.timeSeconds = 0.35;
    gateReleaseEvent.type = daisyhost::RenderTimelineEventType::kGateSet;
    gateReleaseEvent.portId = "node0/port/gate_in_1";
    gateReleaseEvent.gateValue = false;
    scenario.timeline.push_back(gateReleaseEvent);

    daisyhost::RenderResult firstResult;
    daisyhost::RenderResult secondResult;
    std::string             errorMessage;

    ASSERT_TRUE(daisyhost::RunRenderScenario(scenario, &firstResult, &errorMessage))
        << errorMessage;
    ASSERT_TRUE(daisyhost::RunRenderScenario(scenario, &secondResult, &errorMessage))
        << errorMessage;

    EXPECT_EQ(firstResult.manifest.appId, "harmoniqs");
    EXPECT_EQ(firstResult.manifest.audioChecksum, secondResult.manifest.audioChecksum);
    ASSERT_EQ(firstResult.audioChannels.size(), 2u);
    EXPECT_GT(firstResult.manifest.channelSummaries[0].peak, 0.0f);
    EXPECT_GT(firstResult.manifest.channelSummaries[1].peak, 0.0f);
}

TEST(RenderRuntimeTest, ProducesDeterministicOfflineRenderForVASynth)
{
    auto scenario = MakeBaseScenario("vasynth");
    scenario.renderConfig.durationSeconds = 0.5;
    scenario.audioInput.mode
        = static_cast<int>(daisyhost::TestInputSignalMode::kHostInput);
    scenario.initialParameterValues["node0/param/osc_mix"] = 0.58f;
    scenario.initialParameterValues["node0/param/filter_cutoff"] = 0.64f;

    daisyhost::RenderTimelineEvent noteOnA;
    noteOnA.timeSeconds = 0.0;
    noteOnA.type = daisyhost::RenderTimelineEventType::kMidi;
    noteOnA.midiMessage = {0x90, 48, 110};
    scenario.timeline.push_back(noteOnA);

    daisyhost::RenderTimelineEvent noteOnB;
    noteOnB.timeSeconds = 0.05;
    noteOnB.type = daisyhost::RenderTimelineEventType::kMidi;
    noteOnB.midiMessage = {0x90, 55, 96};
    scenario.timeline.push_back(noteOnB);

    daisyhost::RenderTimelineEvent pageEvent;
    pageEvent.timeSeconds = 0.1;
    pageEvent.type = daisyhost::RenderTimelineEventType::kMenuSetItem;
    pageEvent.menuItemId = "node0/menu/pages/page";
    pageEvent.normalizedValue = 1.0f;
    scenario.timeline.push_back(pageEvent);

    daisyhost::RenderTimelineEvent resonanceEvent;
    resonanceEvent.timeSeconds = 0.12;
    resonanceEvent.type = daisyhost::RenderTimelineEventType::kParameterSet;
    resonanceEvent.parameterId = "node0/param/resonance";
    resonanceEvent.normalizedValue = 0.55f;
    scenario.timeline.push_back(resonanceEvent);

    daisyhost::RenderTimelineEvent stereoEvent;
    stereoEvent.timeSeconds = 0.18;
    stereoEvent.type = daisyhost::RenderTimelineEventType::kMenuSetItem;
    stereoEvent.menuItemId = "node0/menu/utilities/stereo_sim";
    stereoEvent.normalizedValue = 1.0f;
    scenario.timeline.push_back(stereoEvent);

    daisyhost::RenderTimelineEvent noteOffA;
    noteOffA.timeSeconds = 0.24;
    noteOffA.type = daisyhost::RenderTimelineEventType::kMidi;
    noteOffA.midiMessage = {0x80, 48, 0};
    scenario.timeline.push_back(noteOffA);

    daisyhost::RenderTimelineEvent gateEvent;
    gateEvent.timeSeconds = 0.3;
    gateEvent.type = daisyhost::RenderTimelineEventType::kGateSet;
    gateEvent.portId = "node0/port/gate_in_1";
    gateEvent.gateValue = true;
    scenario.timeline.push_back(gateEvent);

    daisyhost::RenderTimelineEvent gateReleaseEvent;
    gateReleaseEvent.timeSeconds = 0.35;
    gateReleaseEvent.type = daisyhost::RenderTimelineEventType::kGateSet;
    gateReleaseEvent.portId = "node0/port/gate_in_1";
    gateReleaseEvent.gateValue = false;
    scenario.timeline.push_back(gateReleaseEvent);

    daisyhost::RenderResult firstResult;
    daisyhost::RenderResult secondResult;
    std::string             errorMessage;

    ASSERT_TRUE(daisyhost::RunRenderScenario(scenario, &firstResult, &errorMessage))
        << errorMessage;
    ASSERT_TRUE(daisyhost::RunRenderScenario(scenario, &secondResult, &errorMessage))
        << errorMessage;

    EXPECT_EQ(firstResult.manifest.appId, "vasynth");
    EXPECT_EQ(firstResult.manifest.audioChecksum, secondResult.manifest.audioChecksum);
    ASSERT_EQ(firstResult.audioChannels.size(), 2u);
    EXPECT_GT(firstResult.manifest.channelSummaries[0].peak, 0.0f);
    EXPECT_GT(firstResult.manifest.channelSummaries[1].peak, 0.0f);
}

TEST(RenderRuntimeTest, FieldSurfaceControlSetAppliesMappedParameter)
{
    auto scenario = MakeBaseScenario("vasynth");
    scenario.boardId = "daisy_field";
    scenario.renderConfig.durationSeconds = 0.25;
    scenario.audioInput.mode
        = static_cast<int>(daisyhost::TestInputSignalMode::kHostInput);

    daisyhost::RenderTimelineEvent controlEvent;
    controlEvent.timeSeconds     = 0.0;
    controlEvent.type            = daisyhost::RenderTimelineEventType::kSurfaceControlSet;
    controlEvent.controlId       = "node0/control/field_knob_5";
    controlEvent.normalizedValue = 0.82f;
    scenario.timeline.push_back(controlEvent);

    std::string             errorMessage;
    daisyhost::RenderResult result;
    ASSERT_TRUE(daisyhost::RunRenderScenario(scenario, &result, &errorMessage))
        << errorMessage;

    ASSERT_TRUE(result.manifest.finalParameterValues.count(
                    "node0/param/filter_cutoff")
                > 0);
    EXPECT_FLOAT_EQ(result.manifest.finalParameterValues.at(
                        "node0/param/filter_cutoff"),
                    0.82f);
    ASSERT_FALSE(result.manifest.executedTimeline.empty());
    EXPECT_EQ(result.manifest.executedTimeline[0].type,
              daisyhost::RenderTimelineEventType::kSurfaceControlSet);
}

TEST(RenderRuntimeTest, FieldExtendedSurfaceStateMirrorsOutputsSwitchesAndLeds)
{
    daisyhost::RenderScenario scenario;
    scenario.boardId                 = "daisy_field";
    scenario.appId                   = "vasynth";
    scenario.renderConfig.durationSeconds = 0.03;
    scenario.renderConfig.blockSize  = 16;

    daisyhost::RenderTimelineEvent switchEvent;
    switchEvent.timeSeconds     = 0.0;
    switchEvent.type            = daisyhost::RenderTimelineEventType::kSurfaceControlSet;
    switchEvent.controlId       = "node0/control/field_sw_1";
    switchEvent.normalizedValue = 1.0f;
    scenario.timeline.push_back(switchEvent);

    daisyhost::RenderTimelineEvent cutoffEvent;
    cutoffEvent.timeSeconds     = 0.0;
    cutoffEvent.type            = daisyhost::RenderTimelineEventType::kSurfaceControlSet;
    cutoffEvent.controlId       = "node0/control/field_knob_5";
    cutoffEvent.normalizedValue = 0.82f;
    scenario.timeline.push_back(cutoffEvent);

    daisyhost::RenderTimelineEvent resonanceEvent;
    resonanceEvent.timeSeconds     = 0.0;
    resonanceEvent.type            = daisyhost::RenderTimelineEventType::kSurfaceControlSet;
    resonanceEvent.controlId       = "node0/control/field_knob_6";
    resonanceEvent.normalizedValue = 0.44f;
    scenario.timeline.push_back(resonanceEvent);

    daisyhost::RenderTimelineEvent gateEvent;
    gateEvent.timeSeconds = 0.0;
    gateEvent.type        = daisyhost::RenderTimelineEventType::kGateSet;
    gateEvent.portId      = "node0/port/gate_in_1";
    gateEvent.gateValue   = true;
    scenario.timeline.push_back(gateEvent);

    daisyhost::RenderTimelineEvent noteEvent;
    noteEvent.timeSeconds         = 0.0;
    noteEvent.type                = daisyhost::RenderTimelineEventType::kMidi;
    noteEvent.midiMessage.status  = 0x90;
    noteEvent.midiMessage.data1   = 60;
    noteEvent.midiMessage.data2   = 100;
    scenario.timeline.push_back(noteEvent);

    daisyhost::RenderResult result;
    std::string             errorMessage;
    ASSERT_TRUE(daisyhost::RunRenderScenario(scenario, &result, &errorMessage))
        << errorMessage;

    ASSERT_EQ(result.manifest.fieldSurface.cvOutputs.size(),
              daisyhost::kDaisyFieldCvOutputCount);
    EXPECT_TRUE(result.manifest.fieldSurface.cvOutputs[0].available);
    EXPECT_EQ(result.manifest.fieldSurface.cvOutputs[0].id,
              "node0/port/field_cv_out_1");
    EXPECT_FLOAT_EQ(result.manifest.fieldSurface.cvOutputs[0].normalizedValue,
                    0.82f);
    EXPECT_FLOAT_EQ(result.manifest.fieldSurface.cvOutputs[0].volts, 4.10f);
    EXPECT_EQ(result.manifest.fieldSurface.cvOutputs[1].id,
              "node0/port/field_cv_out_2");
    EXPECT_FLOAT_EQ(result.manifest.fieldSurface.cvOutputs[1].normalizedValue,
                    0.44f);

    ASSERT_EQ(result.manifest.fieldSurface.switches.size(),
              daisyhost::kDaisyFieldSwitchCount);
    EXPECT_TRUE(result.manifest.fieldSurface.switches[0].available);
    EXPECT_TRUE(result.manifest.fieldSurface.switches[0].pressed);
    EXPECT_EQ(result.manifest.fieldSurface.switches[0].detailLabel, "Audition");

    ASSERT_EQ(result.manifest.fieldSurface.leds.size(), daisyhost::kDaisyFieldLedCount);
    EXPECT_EQ(result.manifest.fieldSurface.leds[0].id, "node0/led/field_key_a_1");
    EXPECT_FLOAT_EQ(result.manifest.fieldSurface.leds[0].normalizedValue, 1.0f);
    EXPECT_EQ(result.manifest.fieldSurface.leds[16].id, "node0/led/field_sw_1");
    EXPECT_FLOAT_EQ(result.manifest.fieldSurface.leds[16].normalizedValue, 1.0f);
    EXPECT_EQ(result.manifest.fieldSurface.leds[18].id, "node0/led/field_gate_in");
    EXPECT_FLOAT_EQ(result.manifest.fieldSurface.leds[18].normalizedValue, 1.0f);
}

TEST(RenderRuntimeTest, FieldSurfaceSnapshotFollowsSelectedNodeInTwoNodeRack)
{
    daisyhost::RenderScenario scenario;
    scenario.boardId                         = "daisy_field";
    scenario.appId                           = "vasynth";
    scenario.selectedNodeId                  = "node1";
    scenario.entryNodeId                     = "node1";
    scenario.outputNodeId                    = "node1";
    scenario.renderConfig.sampleRate         = 48000.0;
    scenario.renderConfig.blockSize          = 48;
    scenario.renderConfig.durationSeconds    = 0.25;
    scenario.renderConfig.outputChannelCount = 2;
    scenario.nodes.push_back({"node0", "vasynth", 123u});
    scenario.nodes.push_back({"node1", "vasynth", 456u});

    daisyhost::RenderTimelineEvent cutoffEvent;
    cutoffEvent.timeSeconds     = 0.0;
    cutoffEvent.type            = daisyhost::RenderTimelineEventType::kSurfaceControlSet;
    cutoffEvent.controlId       = "node1/control/field_knob_5";
    cutoffEvent.normalizedValue = 0.82f;
    scenario.timeline.push_back(cutoffEvent);

    daisyhost::RenderTimelineEvent resonanceEvent;
    resonanceEvent.timeSeconds     = 0.0;
    resonanceEvent.type            = daisyhost::RenderTimelineEventType::kSurfaceControlSet;
    resonanceEvent.controlId       = "node1/control/field_knob_6";
    resonanceEvent.normalizedValue = 0.44f;
    scenario.timeline.push_back(resonanceEvent);

    daisyhost::RenderResult result;
    std::string             errorMessage;
    ASSERT_TRUE(daisyhost::RunRenderScenario(scenario, &result, &errorMessage))
        << errorMessage;

    EXPECT_EQ(result.manifest.selectedNodeId, "node1");
    ASSERT_EQ(result.manifest.nodes.size(), 2u);
    ASSERT_EQ(result.manifest.fieldSurface.cvOutputs.size(),
              daisyhost::kDaisyFieldCvOutputCount);
    EXPECT_EQ(result.manifest.fieldSurface.cvOutputs[0].id,
              "node1/port/field_cv_out_1");
    EXPECT_FLOAT_EQ(result.manifest.fieldSurface.cvOutputs[0].normalizedValue,
                    0.82f);
    EXPECT_EQ(result.manifest.fieldSurface.cvOutputs[1].id,
              "node1/port/field_cv_out_2");
    EXPECT_FLOAT_EQ(result.manifest.fieldSurface.cvOutputs[1].normalizedValue,
                    0.44f);
    ASSERT_TRUE(result.manifest.nodes[1].finalParameterValues.count(
                    "node1/param/filter_cutoff")
                > 0);
    EXPECT_FLOAT_EQ(result.manifest.nodes[1].finalParameterValues.at(
                        "node1/param/filter_cutoff"),
                    0.82f);
}

TEST(RenderRuntimeTest, RejectsUnknownFieldSurfaceControlId)
{
    auto scenario = MakeBaseScenario("vasynth");
    scenario.boardId = "daisy_field";

    daisyhost::RenderTimelineEvent controlEvent;
    controlEvent.type            = daisyhost::RenderTimelineEventType::kSurfaceControlSet;
    controlEvent.controlId       = "node0/control/field_knob_99";
    controlEvent.normalizedValue = 0.5f;
    scenario.timeline.push_back(controlEvent);

    std::string             errorMessage;
    daisyhost::RenderResult result;
    EXPECT_FALSE(daisyhost::RunRenderScenario(scenario, &result, &errorMessage));
    EXPECT_NE(errorMessage.find("Unknown surface control id"), std::string::npos);
}

TEST(RenderRuntimeTest, RejectsUnknownPortId)
{
    auto scenario = MakeBaseScenario("multidelay");
    daisyhost::RenderTimelineEvent event;
    event.timeSeconds = 0.0;
    event.type = daisyhost::RenderTimelineEventType::kCvSet;
    event.portId = "node0/cv/does_not_exist";
    event.normalizedValue = 0.25f;
    scenario.timeline.push_back(event);

    std::string errorMessage;
    daisyhost::RenderResult result;
    EXPECT_FALSE(daisyhost::RunRenderScenario(scenario, &result, &errorMessage));
    EXPECT_NE(errorMessage.find("Unknown CV port id"), std::string::npos);
}

TEST(RenderRuntimeTest, OrdersSameTimestampEventsByPriority)
{
    auto scenario = MakeBaseScenario("multidelay");
    const auto parameterId = FirstParameterId("multidelay");
    const auto cvPortId    = FirstCvPortId("multidelay");
    const auto gatePortId = FirstGatePortId("multidelay");

    auto impulseEvent         = daisyhost::RenderTimelineEvent{};
    impulseEvent.timeSeconds  = 0.0;
    impulseEvent.type         = daisyhost::RenderTimelineEventType::kImpulse;

    auto midiEvent                 = daisyhost::RenderTimelineEvent{};
    midiEvent.timeSeconds          = 0.0;
    midiEvent.type                 = daisyhost::RenderTimelineEventType::kMidi;
    midiEvent.midiMessage.status   = 0x90;
    midiEvent.midiMessage.data1    = 60;
    midiEvent.midiMessage.data2    = 100;

    auto gateEvent        = daisyhost::RenderTimelineEvent{};
    gateEvent.timeSeconds = 0.0;
    gateEvent.type        = daisyhost::RenderTimelineEventType::kGateSet;
    gateEvent.portId      = gatePortId;
    gateEvent.gateValue   = true;

    auto audioConfigEvent          = daisyhost::RenderTimelineEvent{};
    audioConfigEvent.timeSeconds   = 0.0;
    audioConfigEvent.type          = daisyhost::RenderTimelineEventType::kAudioInputConfig;
    audioConfigEvent.hasAudioMode  = true;
    audioConfigEvent.audioMode
        = static_cast<int>(daisyhost::TestInputSignalMode::kTriangleInput);

    auto cvEvent             = daisyhost::RenderTimelineEvent{};
    cvEvent.timeSeconds      = 0.0;
    cvEvent.type             = daisyhost::RenderTimelineEventType::kCvSet;
    cvEvent.portId           = cvPortId;
    cvEvent.normalizedValue  = 0.7f;

    auto parameterEvent            = daisyhost::RenderTimelineEvent{};
    parameterEvent.timeSeconds     = 0.0;
    parameterEvent.type            = daisyhost::RenderTimelineEventType::kParameterSet;
    parameterEvent.parameterId     = parameterId;
    parameterEvent.normalizedValue = 0.6f;

    scenario.timeline = {
        impulseEvent,
        midiEvent,
        gateEvent,
        audioConfigEvent,
        cvEvent,
        parameterEvent,
    };
    scenario.timeline[3].hasAudioLevel = true;
    scenario.timeline[3].audioLevel = 4.0f;

    std::string errorMessage;
    daisyhost::RenderResult result;
    ASSERT_TRUE(daisyhost::RunRenderScenario(scenario, &result, &errorMessage))
        << errorMessage;

    ASSERT_GE(result.manifest.executedTimeline.size(), 6u);
    EXPECT_EQ(result.manifest.executedTimeline[0].type,
              daisyhost::RenderTimelineEventType::kParameterSet);
    EXPECT_TRUE(result.manifest.executedTimeline[1].type
                    == daisyhost::RenderTimelineEventType::kCvSet
                || result.manifest.executedTimeline[1].type
                       == daisyhost::RenderTimelineEventType::kGateSet);
    EXPECT_TRUE(result.manifest.executedTimeline[2].type
                    == daisyhost::RenderTimelineEventType::kCvSet
                || result.manifest.executedTimeline[2].type
                       == daisyhost::RenderTimelineEventType::kGateSet);
    EXPECT_NE(result.manifest.executedTimeline[1].type,
              result.manifest.executedTimeline[2].type);
    EXPECT_EQ(result.manifest.executedTimeline[3].type,
              daisyhost::RenderTimelineEventType::kMidi);
    EXPECT_EQ(result.manifest.executedTimeline[4].type,
              daisyhost::RenderTimelineEventType::kAudioInputConfig);
    EXPECT_EQ(result.manifest.executedTimeline[5].type,
              daisyhost::RenderTimelineEventType::kImpulse);
}

TEST(RenderRuntimeTest, ProducesDeterministicOfflineRenderForMultiDelay)
{
    auto scenario = MakeBaseScenario("multidelay");
    scenario.initialParameterValues[FirstParameterId("multidelay")] = 0.35f;

    daisyhost::RenderTimelineEvent parameterEvent;
    parameterEvent.timeSeconds = 0.1;
    parameterEvent.type = daisyhost::RenderTimelineEventType::kParameterSet;
    parameterEvent.parameterId = FirstParameterId("multidelay");
    parameterEvent.normalizedValue = 0.75f;
    scenario.timeline.push_back(parameterEvent);

    daisyhost::RenderResult firstResult;
    daisyhost::RenderResult secondResult;
    std::string             errorMessage;

    ASSERT_TRUE(daisyhost::RunRenderScenario(scenario, &firstResult, &errorMessage))
        << errorMessage;
    ASSERT_TRUE(daisyhost::RunRenderScenario(scenario, &secondResult, &errorMessage))
        << errorMessage;

    EXPECT_EQ(firstResult.manifest.audioChecksum, secondResult.manifest.audioChecksum);
    EXPECT_EQ(firstResult.audioChannels.size(), secondResult.audioChannels.size());
    ASSERT_EQ(firstResult.audioChannels.size(), 2u);
    EXPECT_EQ(firstResult.audioChannels[0], secondResult.audioChannels[0]);
    EXPECT_EQ(firstResult.audioChannels[1], secondResult.audioChannels[1]);
}

TEST(RenderRuntimeTest, ManifestCapturesStateSnapshotsAndFinalInputStates)
{
    auto scenario = MakeBaseScenario("multidelay");
    const auto parameterId = FirstParameterId("multidelay");
    const auto cvPortId = FirstCvPortId("multidelay");
    const auto gatePortId = FirstGatePortId("multidelay");
    scenario.initialParameterValues[parameterId] = 0.2f;

    daisyhost::RenderTimelineEvent cvEvent;
    cvEvent.timeSeconds = 0.0;
    cvEvent.type = daisyhost::RenderTimelineEventType::kCvSet;
    cvEvent.portId = cvPortId;
    cvEvent.normalizedValue = 0.9f;
    scenario.timeline.push_back(cvEvent);

    daisyhost::RenderTimelineEvent gateEvent;
    gateEvent.timeSeconds = 0.0;
    gateEvent.type = daisyhost::RenderTimelineEventType::kGateSet;
    gateEvent.portId = gatePortId;
    gateEvent.gateValue = true;
    scenario.timeline.push_back(gateEvent);

    daisyhost::RenderTimelineEvent audioEvent;
    audioEvent.timeSeconds = 0.05;
    audioEvent.type = daisyhost::RenderTimelineEventType::kAudioInputConfig;
    audioEvent.hasAudioMode = true;
    audioEvent.audioMode = static_cast<int>(daisyhost::TestInputSignalMode::kSawInput);
    audioEvent.hasAudioFrequency = true;
    audioEvent.audioFrequencyHz = 330.0f;
    scenario.timeline.push_back(audioEvent);

    std::string errorMessage;
    daisyhost::RenderResult result;
    ASSERT_TRUE(daisyhost::RunRenderScenario(scenario, &result, &errorMessage))
        << errorMessage;

    EXPECT_EQ(result.manifest.appId, "multidelay");
    EXPECT_EQ(result.manifest.frameCount,
              static_cast<std::uint64_t>(0.25 * 48000.0));
    EXPECT_TRUE(result.manifest.initialParameterValues.count(parameterId) > 0);
    EXPECT_TRUE(result.manifest.finalParameterValues.count(parameterId) > 0);
    EXPECT_TRUE(result.manifest.finalEffectiveParameterValues.count(parameterId) > 0);
    EXPECT_FLOAT_EQ(result.manifest.finalCvInputs.at(cvPortId), 0.9f);
    EXPECT_TRUE(result.manifest.finalGateInputs.at(gatePortId));
    EXPECT_EQ(result.manifest.finalAudioInput.mode,
              static_cast<int>(daisyhost::TestInputSignalMode::kSawInput));
    EXPECT_FLOAT_EQ(result.manifest.finalAudioInput.frequencyHz, 330.0f);
    ASSERT_EQ(result.manifest.channelSummaries.size(), 2u);
    EXPECT_FALSE(result.manifest.audioChecksum.empty());
}

TEST(RenderRuntimeTest, AppliesMidiAudioAndImpulseEvents)
{
    auto scenario = MakeBaseScenario("multidelay");
    scenario.audioInput.mode
        = static_cast<int>(daisyhost::TestInputSignalMode::kImpulseInput);
    scenario.audioInput.level = 8.0f;

    daisyhost::RenderTimelineEvent midiEvent;
    midiEvent.timeSeconds = 0.05;
    midiEvent.type = daisyhost::RenderTimelineEventType::kMidi;
    midiEvent.midiMessage = {0x90, 60, 100};
    scenario.timeline.push_back(midiEvent);

    daisyhost::RenderTimelineEvent impulseEvent;
    impulseEvent.timeSeconds = 0.05;
    impulseEvent.type = daisyhost::RenderTimelineEventType::kImpulse;
    scenario.timeline.push_back(impulseEvent);

    std::string errorMessage;
    daisyhost::RenderResult result;
    ASSERT_TRUE(daisyhost::RunRenderScenario(scenario, &result, &errorMessage))
        << errorMessage;

    EXPECT_EQ(result.manifest.executedTimeline.size(), 2u);
    EXPECT_EQ(result.manifest.executedTimeline[0].type,
              daisyhost::RenderTimelineEventType::kMidi);
    EXPECT_EQ(result.manifest.executedTimeline[1].type,
              daisyhost::RenderTimelineEventType::kImpulse);
    EXPECT_GT(result.manifest.channelSummaries[0].peak, 0.0f);
}

TEST(RenderRuntimeTest, RendersTorusSmokeScenario)
{
    auto scenario = MakeBaseScenario("torus");
    scenario.audioInput.level = 8.0f;
    scenario.audioInput.frequencyHz = 110.0f;

    auto gateEvent = daisyhost::RenderTimelineEvent{};
    gateEvent.timeSeconds = 0.02;
    gateEvent.type = daisyhost::RenderTimelineEventType::kGateSet;
    gateEvent.portId = FirstGatePortId("torus");
    gateEvent.gateValue = true;
    scenario.timeline.push_back(gateEvent);

    std::string errorMessage;
    daisyhost::RenderResult result;
    ASSERT_TRUE(daisyhost::RunRenderScenario(scenario, &result, &errorMessage))
        << errorMessage;

    EXPECT_EQ(result.manifest.appId, "torus");
    EXPECT_EQ(result.audioChannels.size(), 2u);
    EXPECT_GT(result.manifest.channelSummaries[0].peak, 0.0f);
}

TEST(RenderRuntimeTest, ProducesDeterministicCloudSeedRenderWithMenuActions)
{
    auto scenario = MakeBaseScenario("cloudseed");
    scenario.audioInput.level = 7.0f;
    scenario.audioInput.frequencyHz = 196.0f;
    scenario.initialParameterValues["node0/param/mix"] = 0.42f;
    scenario.initialParameterValues["node0/param/size"] = 0.65f;

    daisyhost::RenderTimelineEvent pageEvent;
    pageEvent.timeSeconds = 0.0;
    pageEvent.type        = daisyhost::RenderTimelineEventType::kMenuSetItem;
    pageEvent.menuItemId  = "node0/menu/pages/page";
    pageEvent.normalizedValue = 1.0f;
    scenario.timeline.push_back(pageEvent);

    daisyhost::RenderTimelineEvent preDelayEvent;
    preDelayEvent.timeSeconds = 0.05;
    preDelayEvent.type        = daisyhost::RenderTimelineEventType::kParameterSet;
    preDelayEvent.parameterId = "node0/param/pre_delay";
    preDelayEvent.normalizedValue = 0.68f;
    scenario.timeline.push_back(preDelayEvent);

    daisyhost::RenderTimelineEvent interpolationEvent;
    interpolationEvent.timeSeconds = 0.10;
    interpolationEvent.type        = daisyhost::RenderTimelineEventType::kMenuSetItem;
    interpolationEvent.menuItemId  = "node0/menu/utilities/interpolation";
    interpolationEvent.normalizedValue = 0.0f;
    scenario.timeline.push_back(interpolationEvent);

    daisyhost::RenderTimelineEvent randomizeEvent;
    randomizeEvent.timeSeconds = 0.15;
    randomizeEvent.type        = daisyhost::RenderTimelineEventType::kMenuSetItem;
    randomizeEvent.menuItemId  = "node0/menu/utilities/randomize_seeds";
    randomizeEvent.normalizedValue = 1.0f;
    scenario.timeline.push_back(randomizeEvent);

    daisyhost::RenderTimelineEvent clearEvent;
    clearEvent.timeSeconds = 0.20;
    clearEvent.type        = daisyhost::RenderTimelineEventType::kMenuSetItem;
    clearEvent.menuItemId  = "node0/menu/utilities/clear_tails";
    clearEvent.normalizedValue = 1.0f;
    scenario.timeline.push_back(clearEvent);

    daisyhost::RenderResult firstResult;
    daisyhost::RenderResult secondResult;
    std::string             errorMessage;

    ASSERT_TRUE(daisyhost::RunRenderScenario(scenario, &firstResult, &errorMessage))
        << errorMessage;
    ASSERT_TRUE(daisyhost::RunRenderScenario(scenario, &secondResult, &errorMessage))
        << errorMessage;

    EXPECT_EQ(firstResult.manifest.appId, "cloudseed");
    EXPECT_EQ(firstResult.manifest.audioChecksum, secondResult.manifest.audioChecksum);
    ASSERT_EQ(firstResult.audioChannels.size(), 2u);
    EXPECT_GT(firstResult.manifest.channelSummaries[0].peak, 0.0f);
    EXPECT_GT(firstResult.manifest.channelSummaries[1].peak, 0.0f);
    EXPECT_TRUE(firstResult.manifest.finalParameterValues.count("node0/param/mix") > 0);
    EXPECT_TRUE(firstResult.manifest.finalEffectiveParameterValues.count(
                    "node0/param/late_line_size")
                > 0);
    ASSERT_GE(firstResult.manifest.executedTimeline.size(), 5u);
    EXPECT_EQ(firstResult.manifest.executedTimeline[0].type,
              daisyhost::RenderTimelineEventType::kMenuSetItem);
}

TEST(RenderRuntimeTest, RunsTwoNodeAudioChainScenario)
{
    daisyhost::RenderScenario scenario;
    scenario.appId                           = "multidelay";
    scenario.renderConfig.sampleRate         = 48000.0;
    scenario.renderConfig.blockSize          = 48;
    scenario.renderConfig.durationSeconds    = 0.25;
    scenario.renderConfig.outputChannelCount = 2;
    scenario.audioInput.mode
        = static_cast<int>(daisyhost::TestInputSignalMode::kSineInput);
    scenario.audioInput.level       = 6.0f;
    scenario.audioInput.frequencyHz = 220.0f;
    scenario.selectedNodeId         = "node0";
    scenario.entryNodeId            = "node0";
    scenario.outputNodeId           = "node1";
    scenario.nodes.push_back({"node0", "multidelay", 123u});
    scenario.nodes.push_back({"node1", "cloudseed", 456u});
    scenario.routes.push_back({"node0/port/audio_out_1", "node1/port/audio_in_1"});
    scenario.routes.push_back({"node0/port/audio_out_2", "node1/port/audio_in_2"});

    daisyhost::RenderResult result;
    std::string             errorMessage;
    ASSERT_TRUE(daisyhost::RunRenderScenario(scenario, &result, &errorMessage))
        << errorMessage;

    EXPECT_EQ(result.manifest.appId, "multidelay");
    ASSERT_EQ(result.manifest.nodes.size(), 2u);
    EXPECT_EQ(result.manifest.nodes[0].nodeId, "node0");
    EXPECT_EQ(result.manifest.nodes[0].appId, "multidelay");
    EXPECT_EQ(result.manifest.nodes[1].nodeId, "node1");
    EXPECT_EQ(result.manifest.nodes[1].appId, "cloudseed");
    ASSERT_EQ(result.manifest.routes.size(), 2u);
    EXPECT_EQ(result.manifest.routes[0].sourcePortId, "node0/port/audio_out_1");
    EXPECT_EQ(result.manifest.routes[0].destPortId, "node1/port/audio_in_1");
    ASSERT_EQ(result.audioChannels.size(), 2u);
    EXPECT_GT(result.manifest.channelSummaries[0].peak, 0.0f);
    EXPECT_GT(result.manifest.channelSummaries[1].peak, 0.0f);
}

TEST(RenderRuntimeTest, RejectsUnsupportedCrossNodeCvRoute)
{
    auto scenario = MakeBaseScenario("multidelay");
    scenario.nodes.push_back({"node0", "multidelay", 123u});
    scenario.nodes.push_back({"node1", "torus", 456u});
    scenario.routes.push_back({"node0/port/audio_out_1", "node1/port/gate_in_1"});

    std::string             errorMessage;
    daisyhost::RenderResult result;
    EXPECT_FALSE(daisyhost::RunRenderScenario(scenario, &result, &errorMessage));
    EXPECT_NE(errorMessage.find("audio routes only"), std::string::npos);
}

TEST(RenderRuntimeTest, RejectsAmbiguousMultiNodeImpulseWithoutTargetNodeId)
{
    auto scenario = MakeBaseScenario("multidelay");
    scenario.nodes.push_back({"node0", "multidelay", 123u});
    scenario.nodes.push_back({"node1", "cloudseed", 456u});
    scenario.entryNodeId  = "node0";
    scenario.outputNodeId = "node1";
    scenario.routes.push_back({"node0/port/audio_out_1", "node1/port/audio_in_1"});
    scenario.routes.push_back({"node0/port/audio_out_2", "node1/port/audio_in_2"});

    daisyhost::RenderTimelineEvent impulseEvent;
    impulseEvent.timeSeconds = 0.05;
    impulseEvent.type        = daisyhost::RenderTimelineEventType::kImpulse;
    scenario.timeline.push_back(impulseEvent);

    std::string             errorMessage;
    daisyhost::RenderResult result;
    EXPECT_FALSE(daisyhost::RunRenderScenario(scenario, &result, &errorMessage));
    EXPECT_NE(errorMessage.find("targetNodeId"), std::string::npos);
}

TEST(RenderRuntimeTest, RunsReverseTwoNodeAudioChainScenario)
{
    daisyhost::RenderScenario scenario;
    scenario.boardId                         = "daisy_patch";
    scenario.appId                           = "multidelay";
    scenario.renderConfig.sampleRate         = 48000.0;
    scenario.renderConfig.blockSize          = 48;
    scenario.renderConfig.durationSeconds    = 0.25;
    scenario.renderConfig.outputChannelCount = 2;
    scenario.audioInput.mode
        = static_cast<int>(daisyhost::TestInputSignalMode::kSineInput);
    scenario.audioInput.level       = 6.0f;
    scenario.audioInput.frequencyHz = 220.0f;
    scenario.selectedNodeId         = "node0";
    scenario.entryNodeId            = "node1";
    scenario.outputNodeId           = "node0";
    scenario.nodes.push_back({"node0", "multidelay", 123u});
    scenario.nodes.push_back({"node1", "cloudseed", 456u});
    scenario.routes.push_back({"node1/port/audio_out_1", "node0/port/audio_in_1"});
    scenario.routes.push_back({"node1/port/audio_out_2", "node0/port/audio_in_2"});

    daisyhost::RenderResult result;
    std::string             errorMessage;
    ASSERT_TRUE(daisyhost::RunRenderScenario(scenario, &result, &errorMessage))
        << errorMessage;

    EXPECT_EQ(result.manifest.boardId, "daisy_patch");
    EXPECT_EQ(result.manifest.selectedNodeId, "node0");
    EXPECT_EQ(result.manifest.entryNodeId, "node1");
    EXPECT_EQ(result.manifest.outputNodeId, "node0");
    ASSERT_EQ(result.manifest.routes.size(), 2u);
    EXPECT_EQ(result.manifest.routes[0].sourcePortId, "node1/port/audio_out_1");
    EXPECT_EQ(result.manifest.routes[0].destPortId, "node0/port/audio_in_1");
    ASSERT_EQ(result.audioChannels.size(), 2u);
}

TEST(RenderRuntimeTest, FieldBoardShellPreservesFrozenTwoNodeRackContract)
{
    daisyhost::RenderScenario scenario;
    scenario.boardId                         = "daisy_field";
    scenario.appId                           = "multidelay";
    scenario.renderConfig.sampleRate         = 48000.0;
    scenario.renderConfig.blockSize          = 48;
    scenario.renderConfig.durationSeconds    = 0.25;
    scenario.renderConfig.outputChannelCount = 2;
    scenario.audioInput.mode
        = static_cast<int>(daisyhost::TestInputSignalMode::kSineInput);
    scenario.audioInput.level       = 6.0f;
    scenario.audioInput.frequencyHz = 220.0f;
    scenario.selectedNodeId         = "node0";
    scenario.entryNodeId            = "node0";
    scenario.outputNodeId           = "node1";
    scenario.nodes.push_back({"node0", "multidelay", 123u});
    scenario.nodes.push_back({"node1", "cloudseed", 456u});
    scenario.routes.push_back({"node0/port/audio_out_1", "node1/port/audio_in_1"});
    scenario.routes.push_back({"node0/port/audio_out_2", "node1/port/audio_in_2"});

    daisyhost::RenderResult result;
    std::string             errorMessage;
    ASSERT_TRUE(daisyhost::RunRenderScenario(scenario, &result, &errorMessage))
        << errorMessage;

    EXPECT_EQ(result.manifest.boardId, "daisy_field");
    EXPECT_EQ(result.manifest.selectedNodeId, "node0");
    EXPECT_EQ(result.manifest.entryNodeId, "node0");
    EXPECT_EQ(result.manifest.outputNodeId, "node1");
    ASSERT_EQ(result.manifest.nodes.size(), 2u);
    ASSERT_EQ(result.manifest.routes.size(), 2u);
}

TEST(RenderRuntimeTest, WritesAudioAndManifestOutputs)
{
    auto scenario = MakeBaseScenario("multidelay");
    std::string errorMessage;
    daisyhost::RenderResult result;
    ASSERT_TRUE(daisyhost::RunRenderScenario(scenario, &result, &errorMessage))
        << errorMessage;

    const auto outputDir = std::filesystem::temp_directory_path()
                           / "daisyhost_render_runtime_test";
    std::filesystem::remove_all(outputDir);
    ASSERT_TRUE(daisyhost::WriteRenderOutputs(&result, outputDir, &errorMessage))
        << errorMessage;

    EXPECT_TRUE(std::filesystem::exists(outputDir / "audio.wav"));
    EXPECT_TRUE(std::filesystem::exists(outputDir / "manifest.json"));
    EXPECT_FALSE(result.manifest.audioPath.empty());
    EXPECT_FALSE(result.manifest.manifestPath.empty());

    std::filesystem::remove_all(outputDir);
}
} // namespace
