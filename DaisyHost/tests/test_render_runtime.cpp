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

TEST(RenderRuntimeTest, RejectsUnknownAppId)
{
    auto        scenario = MakeBaseScenario("definitely-not-an-app");
    std::string errorMessage;
    daisyhost::RenderResult result;
    EXPECT_FALSE(daisyhost::RunRenderScenario(scenario, &result, &errorMessage));
    EXPECT_NE(errorMessage.find("Unknown appId"), std::string::npos);
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
    const auto cvPortId = FirstCvPortId("multidelay");
    const auto gatePortId = FirstGatePortId("multidelay");

    scenario.timeline = {
        daisyhost::RenderTimelineEvent{0.0, daisyhost::RenderTimelineEventType::kImpulse},
        daisyhost::RenderTimelineEvent{0.0, daisyhost::RenderTimelineEventType::kMidi, "", "", "", 0.0f, false, {0x90, 60, 100}},
        daisyhost::RenderTimelineEvent{0.0, daisyhost::RenderTimelineEventType::kGateSet, "", gatePortId, "", 0.0f, true},
        daisyhost::RenderTimelineEvent{0.0, daisyhost::RenderTimelineEventType::kAudioInputConfig, "", "", "", 0.0f, false, {}, 0, true, static_cast<int>(daisyhost::TestInputSignalMode::kTriangleInput)},
        daisyhost::RenderTimelineEvent{0.0, daisyhost::RenderTimelineEventType::kCvSet, "", cvPortId, "", 0.7f},
        daisyhost::RenderTimelineEvent{0.0, daisyhost::RenderTimelineEventType::kParameterSet, parameterId, "", "", 0.6f},
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
