#include <algorithm>
#include <array>
#include <cmath>
#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include "daisyhost/AppRegistry.h"
#include "daisyhost/DaisyDelayFxCore.h"
#include "daisyhost/apps/DelayFxAdaptationCore.h"

namespace
{
constexpr std::array<daisyhost::DaisyDelayFxSource, 4> kSources = {{
    daisyhost::DaisyDelayFxSource::kMultiFxPedal,
    daisyhost::DaisyDelayFxSource::kReverbPlayground,
    daisyhost::DaisyDelayFxSource::kFunBox,
    daisyhost::DaisyDelayFxSource::kSdramDelaylines,
}};

float Energy(const std::vector<float>& buffer)
{
    float energy = 0.0f;
    for(float sample : buffer)
    {
        energy += std::abs(sample);
    }
    return energy;
}

void PrepareDelayCore(daisyhost::DaisyDelayFxCore& core,
                      std::vector<float>&          storage)
{
    storage.assign(daisyhost::DaisyDelayFxCore::kDelayLineCount
                       * daisyhost::DaisyDelayFxCore::kMaxDelaySamples,
                   0.0f);
    core.AttachDelayStorage(storage.data(),
                            daisyhost::DaisyDelayFxCore::kDelayLineCount,
                            daisyhost::DaisyDelayFxCore::kMaxDelaySamples);
    core.Prepare(48000.0, 48);
}

bool RegistryContains(const char* appId)
{
    for(const auto& registration : daisyhost::GetHostedAppRegistrations())
    {
        if(registration.appId == appId)
        {
            return true;
        }
    }
    return false;
}
} // namespace

TEST(DaisyDelayFxCoreTest, AllProfilesExposeThreeFieldLayersAndRender)
{
    for(auto source : kSources)
    {
        daisyhost::DaisyDelayFxCore core(source);
        std::vector<float> storage(daisyhost::DaisyDelayFxCore::kDelayLineCount
                                       * daisyhost::DaisyDelayFxCore::kMaxDelaySamples,
                                   0.0f);
        core.AttachDelayStorage(storage.data(),
                                daisyhost::DaisyDelayFxCore::kDelayLineCount,
                                daisyhost::DaisyDelayFxCore::kMaxDelaySamples);
        core.Prepare(48000.0, 48);

        EXPECT_EQ(core.GetParameters().size(), 24u);
        for(std::size_t layer = 0; layer < 3; ++layer)
        {
            for(std::size_t knob = 0; knob < 8; ++knob)
            {
                EXPECT_NE(std::string(core.GetParameterForLayerKnob(layer, knob)),
                          "");
            }
        }

        EXPECT_TRUE(core.SetParameterValue("mix", 0.6f));
        EXPECT_TRUE(core.TriggerFieldKeyAction(8, true));
        std::vector<float> outL(256, 0.0f);
        std::vector<float> outR(256, 0.0f);
        core.Process(nullptr, nullptr, outL.data(), outR.data(), outL.size());
        EXPECT_GT(Energy(outL) + Energy(outR), 0.001f);

        core.TriggerFieldKeyAction(8, false);
        EXPECT_TRUE(core.TriggerFieldKeyAction(1, true));
        EXPECT_EQ(core.GetButtonState(1), 1);
        EXPECT_FALSE(core.FormatParameterValue("time").empty());
    }
}

TEST(DaisyDelayFxCoreTest, BundleAlgorithmsUseTypeFirstLabels)
{
    const auto& algorithms = daisyhost::GetDaisyDelayFxAlgorithmDescriptors();
    ASSERT_EQ(algorithms.size(), 4u);

    EXPECT_EQ(algorithms[0].source, daisyhost::DaisyDelayFxSource::kMultiFxPedal);
    EXPECT_STREQ(algorithms[0].label, "Tape [multifx]");
    EXPECT_STREQ(algorithms[0].shortLabel, "Tape");

    EXPECT_EQ(algorithms[1].source,
              daisyhost::DaisyDelayFxSource::kReverbPlayground);
    EXPECT_STREQ(algorithms[1].label, "Tank [reverb]");

    EXPECT_EQ(algorithms[2].source, daisyhost::DaisyDelayFxSource::kFunBox);
    EXPECT_STREQ(algorithms[2].label, "Texture [FunBox]");

    EXPECT_EQ(algorithms[3].source,
              daisyhost::DaisyDelayFxSource::kSdramDelaylines);
    EXPECT_STREQ(algorithms[3].label, "Long [sdram]");

    EXPECT_EQ(daisyhost::DaisyDelayFxAlgorithmIndex(
                  daisyhost::DaisyDelayFxSource::kFunBox),
              2u);
    EXPECT_EQ(daisyhost::DaisyDelayFxSourceForAlgorithmIndex(3),
              daisyhost::DaisyDelayFxSource::kSdramDelaylines);
}

TEST(DaisyDelayFxCoreTest, BundleInternalSynthSupportsPluckPadAndLatch)
{
    daisyhost::DaisyDelayFxCore core(daisyhost::DaisyDelayFxSource::kMultiFxPedal);
    core.SetBundleMode(true);
    std::vector<float> storage;
    PrepareDelayCore(core, storage);

    EXPECT_EQ(core.GetInternalSynthMode(), 1);
    EXPECT_EQ(core.GetInternalSynthHoldMode(), 0);
    ASSERT_NE(core.FindParameter("freeze"), nullptr);
    EXPECT_EQ(core.FindParameter("freeze")->label, "Synth Bright");
    ASSERT_NE(core.FindParameter("tempo"), nullptr);
    EXPECT_EQ(core.FindParameter("tempo")->label, "Synth Level");

    EXPECT_TRUE(core.TriggerFieldKeyAction(8, true));
    auto leds = core.GetFieldKeyLedValues();
    EXPECT_GT(leds[8], 0.5f);

    std::vector<float> outL(512, 0.0f);
    std::vector<float> outR(512, 0.0f);
    core.Process(nullptr, nullptr, outL.data(), outR.data(), outL.size());
    EXPECT_GT(Energy(outL) + Energy(outR), 0.001f);

    EXPECT_TRUE(core.TriggerFieldKeyAction(8, false));
    leds = core.GetFieldKeyLedValues();
    EXPECT_LT(leds[8], 0.2f);

    core.SetInternalSynthHoldMode(1);
    EXPECT_TRUE(core.TriggerFieldKeyAction(9, true));
    EXPECT_TRUE(core.TriggerFieldKeyAction(9, false));
    leds = core.GetFieldKeyLedValues();
    EXPECT_GT(leds[9], 0.5f);
    EXPECT_TRUE(core.TriggerFieldKeyAction(9, true));
    leds = core.GetFieldKeyLedValues();
    EXPECT_LT(leds[9], 0.2f);

    core.SetInternalSynthMode(2);
    core.SetInternalSynthHoldMode(0);
    EXPECT_TRUE(core.TriggerFieldKeyAction(10, true));
    std::fill(outL.begin(), outL.end(), 0.0f);
    std::fill(outR.begin(), outR.end(), 0.0f);
    core.Process(nullptr, nullptr, outL.data(), outR.data(), outL.size());
    EXPECT_GT(Energy(outL) + Energy(outR), 0.001f);
}

TEST(DaisyDelayFxCoreTest, BundleInternalSynthIsIdleSilentAndReleaseDecays)
{
    daisyhost::DaisyDelayFxCore core(daisyhost::DaisyDelayFxSource::kMultiFxPedal);
    core.SetBundleMode(true);
    std::vector<float> storage;
    PrepareDelayCore(core, storage);
    ASSERT_TRUE(core.SetParameterValue("mix", 0.0f));
    ASSERT_TRUE(core.SetParameterValue("feedback", 0.0f));

    std::vector<float> outL(512, 0.0f);
    std::vector<float> outR(512, 0.0f);
    core.Process(nullptr, nullptr, outL.data(), outR.data(), outL.size());
    EXPECT_LT(Energy(outL) + Energy(outR), 0.00001f);

    EXPECT_TRUE(core.TriggerFieldKeyAction(8, true));
    std::fill(outL.begin(), outL.end(), 0.0f);
    std::fill(outR.begin(), outR.end(), 0.0f);
    core.Process(nullptr, nullptr, outL.data(), outR.data(), outL.size());
    EXPECT_GT(Energy(outL) + Energy(outR), 0.001f);

    EXPECT_TRUE(core.TriggerFieldKeyAction(8, false));
    for(int block = 0; block < 240; ++block)
    {
        std::fill(outL.begin(), outL.end(), 0.0f);
        std::fill(outR.begin(), outR.end(), 0.0f);
        core.Process(nullptr, nullptr, outL.data(), outR.data(), outL.size());
    }
    EXPECT_LT(Energy(outL) + Energy(outR), 0.01f);
}

TEST(DelayFxAdaptationCoreTest, HostedAppsExposeFieldSurfaceAndMenu)
{
    for(auto source : kSources)
    {
        daisyhost::apps::DelayFxAdaptationCore app(source, "node0");
        app.Prepare(48000.0, 48);

        const auto capabilities = app.GetCapabilities();
        EXPECT_TRUE(capabilities.acceptsAudioInput);
        EXPECT_TRUE(capabilities.acceptsMidiInput);

        const auto bindings = app.GetPatchBindings();
        EXPECT_EQ(bindings.fieldKnobParameterIds[0], "node0/param/mix");
        EXPECT_EQ(bindings.fieldKnobParameterIds[7], "node0/param/output");
        EXPECT_EQ(bindings.fieldKeyMenuItemIds[0], "node0/menu/field_keys/a1");
        EXPECT_EQ(bindings.fieldKeyMenuItemIds[8], "node0/menu/field_keys/b1");
        EXPECT_EQ(bindings.midiInputPortId, "node0/port/midi_in");

        app.SetMenuItemValue("node0/menu/field_keys/a2", 1.0f);
        app.SetMenuItemValue("node0/menu/field_keys/a2", 0.0f);
        const auto leds = app.GetFieldKeyLedValues();
        EXPECT_GT(leds[1], 0.0f);

        app.SetMenuItemValue("node0/menu/base/time", 0.75f);
        auto time = app.GetParameterValue("node0/param/time");
        ASSERT_TRUE(time.hasValue);
        EXPECT_NEAR(time.value, 0.75f, 0.0001f);

        std::array<float, 48> outL{};
        std::array<float, 48> outR{};
        float* outputChannels[] = {outL.data(), outR.data()};
        daisyhost::PortValue midi;
        midi.type = daisyhost::VirtualPortType::kMidi;
        midi.midiEvents.push_back({0x90, 60, 100});
        app.SetPortInput("node0/port/midi_in", midi);
        app.Process({}, {outputChannels, 2}, outL.size());
        EXPECT_GT(Energy(std::vector<float>(outL.begin(), outL.end()))
                      + Energy(std::vector<float>(outR.begin(), outR.end())),
                  0.001f);
    }
}

TEST(DelayFxAdaptationCoreTest, RegistryCreatesAllDelayAdaptations)
{
    for(const char* appId :
        {"field_delay_multifx_pedal",
         "field_delay_reverb_playground",
         "field_delay_funbox",
         "field_delay_sdram_delaylines",
         "field_delay_bundle"})
    {
        EXPECT_TRUE(RegistryContains(appId));
        std::string resolved;
        std::unique_ptr<daisyhost::HostedAppCore> app
            = daisyhost::CreateHostedAppCore(appId, "node0", &resolved);
        ASSERT_NE(app, nullptr);
        EXPECT_EQ(resolved, appId);
        EXPECT_EQ(app->GetAppId(), appId);
    }
}

TEST(DelayFxAdaptationCoreTest, BundleSelectsAlgorithmsWithAKeysAndSnapshots)
{
    daisyhost::apps::DelayFxAdaptationCore app(
        daisyhost::DaisyDelayFxSource::kMultiFxPedal, "node0", true);
    app.Prepare(48000.0, 48);

    EXPECT_EQ(app.GetAppId(), "field_delay_bundle");
    EXPECT_EQ(app.GetAppDisplayName(), "Field Delay Bundle");
    EXPECT_EQ(app.GetDisplayModel().title, "Field Delay Bundle");
    ASSERT_FALSE(app.GetDisplayModel().texts.empty());
    EXPECT_NE(app.GetDisplayModel().texts.front().text.find("Tape [multifx]"),
              std::string::npos);

    const auto algorithm = app.GetParameterValue("node0/param/algorithm");
    ASSERT_TRUE(algorithm.hasValue);
    EXPECT_NEAR(algorithm.value, 0.0f, 0.0001f);

    app.SetParameterValue("node0/param/time", 0.71f);
    app.SetMenuItemValue("node0/menu/field_keys/a2", 1.0f);
    app.SetMenuItemValue("node0/menu/field_keys/a2", 0.0f);
    EXPECT_NE(app.GetDisplayModel().texts.front().text.find("Tank [reverb]"),
              std::string::npos);
    auto tankAlgorithm = app.GetParameterValue("node0/param/algorithm");
    ASSERT_TRUE(tankAlgorithm.hasValue);
    EXPECT_NEAR(tankAlgorithm.value, 1.0f / 3.0f, 0.0001f);

    app.SetParameterValue("node0/param/time", 0.22f);
    app.SetMenuItemValue("node0/menu/field_keys/a1", 1.0f);
    app.SetMenuItemValue("node0/menu/field_keys/a1", 0.0f);
    EXPECT_NE(app.GetDisplayModel().texts.front().text.find("Tape [multifx]"),
              std::string::npos);
    auto restoredTapeTime = app.GetParameterValue("node0/param/time");
    ASSERT_TRUE(restoredTapeTime.hasValue);
    EXPECT_NEAR(restoredTapeTime.value, 0.71f, 0.0001f);

    const auto leds = app.GetFieldKeyLedValues();
    EXPECT_GT(leds[0], leds[1]);
    EXPECT_GT(leds[0], leds[2]);
    EXPECT_GT(leds[0], leds[3]);
}
