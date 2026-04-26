#include <array>
#include <cmath>
#include <string>

#include <gtest/gtest.h>

#include "daisyhost/AppRegistry.h"
#include "daisyhost/DaisySubharmoniqCore.h"
#include "daisyhost/apps/SubharmoniqCore.h"

namespace
{
const daisyhost::MenuSection* FindSection(const daisyhost::MenuModel& menu,
                                          const std::string&          sectionId)
{
    for(const auto& section : menu.sections)
    {
        if(section.id == sectionId)
        {
            return &section;
        }
    }
    return nullptr;
}

TEST(SubharmoniqCoreTest, DefaultsExposeSixSourcesPagesAndQuantization)
{
    daisyhost::DaisySubharmoniqCore core;
    core.Prepare(48000.0, 48);

    EXPECT_EQ(core.GetSourceCount(), 6u);
    EXPECT_EQ(core.GetQuantizeMode(),
              daisyhost::DaisySubharmoniqQuantizeMode::kTwelveEqual);
    EXPECT_EQ(core.GetSeqOctaveRange(), 2);

    const auto& parameters = core.GetParameters();
    ASSERT_GE(parameters.size(), 16u);
    EXPECT_EQ(parameters[0].id, "vco1_pitch");
    EXPECT_EQ(parameters[0].label, "VCO 1");
    EXPECT_TRUE(parameters[0].automatable);

    const auto binding = core.GetActivePageBinding();
    EXPECT_EQ(binding.page, daisyhost::DaisySubharmoniqPage::kHome);
    EXPECT_EQ(binding.pageLabel, "Home");
    EXPECT_EQ(binding.parameterLabels[0], "Tempo");
    EXPECT_EQ(binding.parameterLabels[1], "Cutoff");
    EXPECT_EQ(binding.parameterLabels[2], "Res");
    EXPECT_EQ(binding.parameterLabels[3], "VCA Decay");
}

TEST(SubharmoniqCoreTest, RhythmDividersAdvanceAssignedSequencers)
{
    daisyhost::DaisySubharmoniqCore core;
    core.Prepare(48000.0, 48);
    core.ResetToDefaultState(3u);
    core.SetRhythmDivisor(0, 1);
    core.SetRhythmTarget(0, daisyhost::DaisySubharmoniqRhythmTarget::kSeq1);
    core.SetRhythmDivisor(1, 2);
    core.SetRhythmTarget(1, daisyhost::DaisySubharmoniqRhythmTarget::kSeq2);

    core.TriggerMomentaryAction("play_toggle");
    core.AdvanceClockPulse();
    core.AdvanceClockPulse();

    EXPECT_EQ(core.GetSequencerStepIndex(0), 2);
    EXPECT_EQ(core.GetSequencerStepIndex(1), 1);
    EXPECT_GE(core.GetTriggerCount(), 2u);

    core.TriggerMomentaryAction("reset");
    EXPECT_EQ(core.GetSequencerStepIndex(0), 0);
    EXPECT_EQ(core.GetSequencerStepIndex(1), 0);
}

TEST(SubharmoniqCoreTest, SequencerStepsUseSelectedOctaveAndQuantization)
{
    daisyhost::DaisySubharmoniqCore core;
    core.Prepare(48000.0, 48);
    core.ResetToDefaultState(5u);

    core.SetSeqOctaveRange(2);
    core.SetQuantizeMode(daisyhost::DaisySubharmoniqQuantizeMode::kTwelveEqual);
    core.SetSequencerStepValue(0, 0, 0.75f);
    core.SetSequencerStepValue(1, 0, 0.25f);

    EXPECT_EQ(core.GetSequencerStepSemitones(0, 0), 12);
    EXPECT_EQ(core.GetSequencerStepSemitones(1, 0), -12);

    core.SetQuantizeMode(daisyhost::DaisySubharmoniqQuantizeMode::kEightJust);
    core.SetSequencerStepValue(0, 1, 0.625f);
    EXPECT_NEAR(core.GetSequencerStepRatio(0, 1), 1.5f, 0.0001f);
}

TEST(SubharmoniqCoreTest, MidiTriggerProducesFiniteAudioAndPanicSilences)
{
    daisyhost::DaisySubharmoniqCore core;
    core.Prepare(48000.0, 48);
    core.ResetToDefaultState(7u);
    core.HandleMidiEvent(0x90, 48, 100);

    std::array<float, 256> left{};
    std::array<float, 256> right{};
    core.Process(left.data(), right.data(), left.size());

    float energy = 0.0f;
    for(std::size_t i = 0; i < left.size(); ++i)
    {
        ASSERT_TRUE(std::isfinite(left[i]));
        ASSERT_TRUE(std::isfinite(right[i]));
        energy += std::abs(left[i]) + std::abs(right[i]);
    }
    EXPECT_GT(energy, 0.01f);

    ASSERT_TRUE(core.TriggerMomentaryAction("panic"));
    left.fill(1.0f);
    right.fill(1.0f);
    core.Process(left.data(), right.data(), left.size());
    for(std::size_t i = 0; i < left.size(); ++i)
    {
        EXPECT_FLOAT_EQ(left[i], 0.0f);
        EXPECT_FLOAT_EQ(right[i], 0.0f);
    }
}

TEST(SubharmoniqCoreTest, PlayToggleRunsInternalClockAndProducesAudio)
{
    daisyhost::DaisySubharmoniqCore core;
    core.Prepare(48000.0, 48);
    core.ResetToDefaultState(0);

    ASSERT_TRUE(core.TriggerMomentaryAction("play_toggle"));

    std::array<float, 48000> left{};
    std::array<float, 48000> right{};
    core.Process(left.data(), right.data(), left.size());

    float peak = 0.0f;
    float energy = 0.0f;
    for(float sample : left)
    {
        EXPECT_TRUE(std::isfinite(sample));
        peak = std::max(peak, std::abs(sample));
        energy += std::abs(sample);
    }

    EXPECT_GT(core.GetTriggerCount(), 0u);
    EXPECT_GT(peak, 0.02f);
    EXPECT_GT(energy, 1.0f);
}

TEST(SubharmoniqCoreTest, HostedWrapperExposesMenuBindingsAndRegistry)
{
    daisyhost::apps::SubharmoniqCore core("node0");
    core.Prepare(48000.0, 48);

    EXPECT_EQ(core.GetAppId(), "subharmoniq");
    EXPECT_EQ(core.GetAppDisplayName(), "Subharmoniq");
    EXPECT_TRUE(core.GetCapabilities().acceptsMidiInput);
    EXPECT_FALSE(core.GetCapabilities().acceptsAudioInput);

    const auto bindings = core.GetPatchBindings();
    EXPECT_EQ(bindings.knobDetailLabels[0], "Tempo");
    EXPECT_EQ(bindings.knobDetailLabels[1], "Cutoff");
    EXPECT_EQ(bindings.knobDetailLabels[2], "Res");
    EXPECT_EQ(bindings.knobDetailLabels[3], "VCA Decay");
    EXPECT_EQ(bindings.gateInputPortIds[0], "node0/port/gate_in_1");
    EXPECT_EQ(bindings.midiInputPortId, "node0/port/midi_in_1");

    const auto& menu = core.GetMenuModel();
    EXPECT_NE(FindSection(menu, "node0/menu/pages"), nullptr);
    EXPECT_NE(FindSection(menu, "node0/menu/filter"), nullptr);
    const auto* about = FindSection(menu, "node0/menu/about");
    ASSERT_NE(about, nullptr);
    ASSERT_FALSE(about->items.empty());
    EXPECT_EQ(about->items.back().valueText, "Round 2: SVF LPF/BPF + ladder LPF");

    core.MenuRotate(1);
    const auto voiceBindings = core.GetPatchBindings();
    EXPECT_EQ(voiceBindings.knobDetailLabels[0], "VCO 1");
    EXPECT_EQ(voiceBindings.knobDetailLabels[1], "VCO 2");
    EXPECT_EQ(voiceBindings.knobDetailLabels[2], "VCO1 Sub1");
    EXPECT_EQ(voiceBindings.knobDetailLabels[3], "VCO2 Sub1");

    bool sawSubharmoniq = false;
    for(const auto& registration : daisyhost::GetHostedAppRegistrations())
    {
        if(registration.appId == "subharmoniq")
        {
            sawSubharmoniq = true;
        }
    }
    EXPECT_TRUE(sawSubharmoniq);
}

TEST(SubharmoniqCoreTest, HostedWrapperProvidesAllocationLightFirmwareMidiPath)
{
    daisyhost::apps::SubharmoniqCore core("node0");
    core.Prepare(48000.0, 48);
    core.ResetToDefaultState(0);

    core.HandleMidiEvent(0x90, 60, 100);
    EXPECT_EQ(core.GetCurrentMidiNote(), 60);

    const auto beforeActiveSensing = core.GetTriggerCount();
    core.HandleMidiEvent(0xFE, 0, 0);
    EXPECT_EQ(core.GetTriggerCount(), beforeActiveSensing);

    core.TriggerMomentaryAction("play_toggle");
    core.HandleMidiEvent(0xF8, 0, 0);
    EXPECT_GT(core.GetTriggerCount(), beforeActiveSensing);

    core.TriggerGate(false);
    const auto beforeGate = core.GetTriggerCount();
    core.TriggerGate(true);
    EXPECT_GT(core.GetTriggerCount(), beforeGate);
}

TEST(SubharmoniqCoreTest, HostedWrapperProvidesAllocationLightFirmwareAudioAndCvPath)
{
    daisyhost::apps::SubharmoniqCore core("node0");
    core.Prepare(48000.0, 48);
    core.ResetToDefaultState(0);

    EXPECT_TRUE(core.SetParameterValue("output", 1.0f));
    core.SetCvInput(1, 0.75f);
    core.TriggerMomentaryAction("play_toggle");

    std::array<float, 256> left{};
    std::array<float, 256> right{};
    core.ProcessAudio(left.data(), right.data(), left.size());

    float energy = 0.0f;
    for(std::size_t i = 0; i < left.size(); ++i)
    {
        ASSERT_TRUE(std::isfinite(left[i]));
        ASSERT_TRUE(std::isfinite(right[i]));
        energy += std::abs(left[i]) + std::abs(right[i]);
    }

    EXPECT_EQ(core.GetActivePage(), daisyhost::DaisySubharmoniqPage::kHome);
    EXPECT_GE(core.GetSequencerStepIndex(0), 0);
    EXPECT_GE(core.GetGateOutputPulse(), 0.0f);
    EXPECT_GT(energy, 0.01f);
}

TEST(SubharmoniqCoreTest, HostedWrapperFieldKeyMenuActionsMatchFirmwareControls)
{
    daisyhost::apps::SubharmoniqCore core("node0");
    core.Prepare(48000.0, 48);
    core.ResetToDefaultState(0);

    const auto bindings = core.GetPatchBindings();
    ASSERT_EQ(bindings.fieldKeyMenuItemIds[14], "node0/menu/field_keys/b7");
    EXPECT_EQ(bindings.fieldKeyDetailLabels[14], "Play/Stop");
    EXPECT_EQ(bindings.fieldKeyMenuItemIds[15], "node0/menu/field_keys/b8");
    EXPECT_EQ(bindings.fieldKeyDetailLabels[15], "Reset");

    EXPECT_FALSE(core.IsPlaying());
    core.SetMenuItemValue("node0/menu/field_keys/b7", 1.0f);
    EXPECT_TRUE(core.IsPlaying());

    core.SetMenuItemValue("node0/menu/field_keys/a5", 1.0f);
    EXPECT_EQ(core.GetActivePage(), daisyhost::DaisySubharmoniqPage::kRhythm);
    EXPECT_EQ(core.GetRhythmTarget(0), daisyhost::DaisySubharmoniqRhythmTarget::kSeq2);

    core.SetMenuItemValue("node0/menu/field_keys/b5", 1.0f);
    EXPECT_EQ(core.GetQuantizeMode(), daisyhost::DaisySubharmoniqQuantizeMode::kEightEqual);

    core.SetMenuItemValue("node0/menu/field_keys/b6", 1.0f);
    EXPECT_EQ(core.GetSeqOctaveRange(), 5);

    core.HandleMidiEvent(0xF8, 0, 0);
    EXPECT_GT(core.GetTriggerCount(), 0u);
    core.SetMenuItemValue("node0/menu/field_keys/b8", 1.0f);
    EXPECT_EQ(core.GetSequencerStepIndex(0), 0);
    EXPECT_EQ(core.GetSequencerStepIndex(1), 0);
}

TEST(SubharmoniqCoreTest, FieldB7StartsAudioAndBothSequencersAfterA7RhythmEdit)
{
    daisyhost::apps::SubharmoniqCore core("node0");
    core.Prepare(48000.0, 48);
    core.ResetToDefaultState(0);

    core.SetMenuItemValue("node0/menu/field_keys/a7", 1.0f);
    EXPECT_EQ(core.GetActivePage(), daisyhost::DaisySubharmoniqPage::kRhythm);

    core.SetMenuItemValue("node0/menu/field_keys/b7", 1.0f);
    EXPECT_TRUE(core.IsPlaying());

    std::array<float, 48000> left{};
    std::array<float, 48000> right{};
    core.ProcessAudio(left.data(), right.data(), left.size());

    float energy = 0.0f;
    for(std::size_t i = 0; i < left.size(); ++i)
    {
        ASSERT_TRUE(std::isfinite(left[i]));
        ASSERT_TRUE(std::isfinite(right[i]));
        energy += std::abs(left[i]) + std::abs(right[i]);
    }

    EXPECT_GT(core.GetSequencerStepIndex(0), 0);
    EXPECT_GT(core.GetSequencerStepIndex(1), 0);
    EXPECT_GT(core.GetTriggerCount(), 0u);
    EXPECT_GT(energy, 1.0f);
}
} // namespace
