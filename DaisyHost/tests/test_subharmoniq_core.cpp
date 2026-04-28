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

bool DisplayContainsText(const daisyhost::DisplayModel& display,
                         const std::string&            needle)
{
    for(const auto& text : display.texts)
    {
        if(text.text.find(needle) != std::string::npos)
        {
            return true;
        }
    }
    return false;
}

float StereoEnergy(const std::array<float, 48000>& left,
                   const std::array<float, 48000>& right)
{
    float energy = 0.0f;
    for(std::size_t i = 0; i < left.size(); ++i)
    {
        EXPECT_TRUE(std::isfinite(left[i]));
        EXPECT_TRUE(std::isfinite(right[i]));
        energy += std::abs(left[i]) + std::abs(right[i]);
    }
    return energy;
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
    EXPECT_EQ(binding.page, daisyhost::DaisySubharmoniqPage::kSeqRhythm);
    EXPECT_EQ(binding.pageLabel, "Seq/Rhy");
    EXPECT_EQ(binding.parameterLabels[0], "S1 Step1");
    EXPECT_EQ(binding.parameterLabels[1], "S1 Step2");
    EXPECT_EQ(binding.parameterLabels[2], "S1 Step3");
    EXPECT_EQ(binding.parameterLabels[3], "S1 Step4");
}

TEST(SubharmoniqCoreTest, FieldPerformancePagesMatchApprovedHybridMap)
{
    daisyhost::DaisySubharmoniqCore core;
    core.Prepare(48000.0, 48);
    core.ResetToDefaultState(0);

    auto binding = core.GetActivePageBinding();
    EXPECT_EQ(binding.page, daisyhost::DaisySubharmoniqPage::kSeqRhythm);
    EXPECT_EQ(binding.pageLabel, "Seq/Rhy");
    EXPECT_EQ(binding.parameterIds[0], "seq1_step1");
    EXPECT_EQ(binding.parameterIds[3], "seq1_step4");
    EXPECT_EQ(binding.parameterIds[4], "seq2_step1");
    EXPECT_EQ(binding.parameterIds[7], "seq2_step4");

    ASSERT_TRUE(core.SetActivePage(daisyhost::DaisySubharmoniqPage::kVco));
    binding = core.GetActivePageBinding();
    EXPECT_EQ(binding.pageLabel, "VCO");
    EXPECT_EQ(binding.parameterIds[0], "vco1_pitch");
    EXPECT_EQ(binding.parameterIds[1], "vco2_pitch");
    EXPECT_EQ(binding.parameterIds[2], "vco1_sub1_div");
    EXPECT_EQ(binding.parameterIds[3], "vco1_sub2_div");
    EXPECT_EQ(binding.parameterIds[4], "vco2_sub1_div");
    EXPECT_EQ(binding.parameterIds[5], "vco2_sub2_div");
    EXPECT_EQ(binding.parameterIds[6], "quantize_mode");
    EXPECT_EQ(binding.parameterIds[7], "seq_oct_range");
    EXPECT_EQ(binding.parameterLabels[6], "Quantize");
    EXPECT_EQ(binding.parameterLabels[7], "Seq Oct");

    ASSERT_TRUE(core.SetActivePage(daisyhost::DaisySubharmoniqPage::kVcf));
    binding = core.GetActivePageBinding();
    EXPECT_EQ(binding.pageLabel, "VCF");
    EXPECT_EQ(binding.parameterIds[0], "cutoff");
    EXPECT_EQ(binding.parameterIds[1], "resonance");
    EXPECT_EQ(binding.parameterIds[2], "vcf_env_amt");
    EXPECT_EQ(binding.parameterIds[3], "vcf_attack");
    EXPECT_EQ(binding.parameterIds[4], "vcf_decay");
    EXPECT_EQ(binding.parameterIds[5], "tempo");
    EXPECT_EQ(binding.parameterIds[6], "drive");
    EXPECT_EQ(binding.parameterIds[7], "output");

    ASSERT_TRUE(core.SetActivePage(daisyhost::DaisySubharmoniqPage::kVcaMix));
    binding = core.GetActivePageBinding();
    EXPECT_EQ(binding.pageLabel, "VCA/Mix");
    EXPECT_EQ(binding.parameterIds[0], "vco1_level");
    EXPECT_EQ(binding.parameterIds[1], "vco1_sub1_level");
    EXPECT_EQ(binding.parameterIds[2], "vco1_sub2_level");
    EXPECT_EQ(binding.parameterIds[3], "vco2_level");
    EXPECT_EQ(binding.parameterIds[4], "vco2_sub1_level");
    EXPECT_EQ(binding.parameterIds[5], "vco2_sub2_level");
    EXPECT_EQ(binding.parameterIds[6], "vca_decay");
    EXPECT_EQ(binding.parameterIds[7], "output");
}

TEST(SubharmoniqCoreTest, RestoresLegacyAndSchemaPageState)
{
    daisyhost::DaisySubharmoniqCore core;
    core.Prepare(48000.0, 48);

    core.RestoreStatefulParameterValues({{"state/page", 1.0f}});
    EXPECT_EQ(core.GetActivePage(), daisyhost::DaisySubharmoniqPage::kVco);

    core.RestoreStatefulParameterValues({{"state/page", 2.0f}});
    EXPECT_EQ(core.GetActivePage(), daisyhost::DaisySubharmoniqPage::kVcaMix);

    core.RestoreStatefulParameterValues({{"state/page", 5.0f}});
    EXPECT_EQ(core.GetActivePage(), daisyhost::DaisySubharmoniqPage::kVcf);

    core.RestoreStatefulParameterValues({{"state/page", 4.0f}});
    EXPECT_EQ(core.GetActivePage(), daisyhost::DaisySubharmoniqPage::kSeqRhythm);

    core.RestoreStatefulParameterValues({{"state/page_schema", 1.0f},
                                         {"state/page", 2.0f}});
    EXPECT_EQ(core.GetActivePage(), daisyhost::DaisySubharmoniqPage::kVcf);
}

TEST(SubharmoniqCoreTest, CapturedPageAndSeqOctaveStateRoundTrip)
{
    daisyhost::DaisySubharmoniqCore source;
    source.Prepare(48000.0, 48);
    ASSERT_TRUE(source.SetActivePage(daisyhost::DaisySubharmoniqPage::kFilter));
    source.SetSeqOctaveRange(5);

    daisyhost::DaisySubharmoniqCore restored;
    restored.Prepare(48000.0, 48);
    restored.RestoreStatefulParameterValues(source.CaptureStatefulParameterValues());

    EXPECT_EQ(restored.GetActivePage(), daisyhost::DaisySubharmoniqPage::kFilter);
    EXPECT_EQ(restored.GetSeqOctaveRange(), 5);
}

TEST(SubharmoniqCoreTest, LegacyFirmwarePageNamesRemainSupported)
{
    daisyhost::DaisySubharmoniqCore core;
    core.Prepare(48000.0, 48);

    ASSERT_TRUE(core.SetActivePage(daisyhost::DaisySubharmoniqPage::kSeq));
    EXPECT_EQ(core.GetActivePage(), daisyhost::DaisySubharmoniqPage::kSeq);
    auto binding = core.GetActivePageBinding();
    EXPECT_EQ(binding.pageLabel, "Seq");
    EXPECT_EQ(binding.parameterIds[0], "seq1_step1");

    ASSERT_TRUE(core.SetActivePage(daisyhost::DaisySubharmoniqPage::kRhythm));
    binding = core.GetActivePageBinding();
    EXPECT_EQ(binding.pageLabel, "Rhythm");
    EXPECT_EQ(binding.parameterIds[0], "rhythm1_div");

    ASSERT_TRUE(core.SetActivePage(daisyhost::DaisySubharmoniqPage::kFilter));
    binding = core.GetActivePageBinding();
    EXPECT_EQ(binding.pageLabel, "Filter");
    EXPECT_EQ(binding.parameterIds[0], "cutoff");
}

TEST(SubharmoniqCoreTest, QuantizeAndSeqOctaveArePageEditableParameters)
{
    daisyhost::DaisySubharmoniqCore core;
    core.Prepare(48000.0, 48);

    ASSERT_TRUE(core.SetParameterValue("quantize_mode", 0.5f));
    EXPECT_EQ(core.GetQuantizeMode(),
              daisyhost::DaisySubharmoniqQuantizeMode::kEightEqual);
    float value = 0.0f;
    ASSERT_TRUE(core.GetParameterValue("quantize_mode", &value));
    EXPECT_FLOAT_EQ(value, 0.5f);

    ASSERT_TRUE(core.SetParameterValue("seq_oct_range", 1.0f));
    EXPECT_EQ(core.GetSeqOctaveRange(), 5);
    ASSERT_TRUE(core.GetParameterValue("seq_oct_range", &value));
    EXPECT_FLOAT_EQ(value, 1.0f);
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

TEST(SubharmoniqCoreTest, MultipleMidiNotesAndFieldClockPulsesStayAudible)
{
    daisyhost::apps::SubharmoniqCore core("node0");
    core.Prepare(48000.0, 48);
    core.ResetToDefaultState(0);
    ASSERT_TRUE(core.SetParameterValue("output", 1.0f));
    ASSERT_TRUE(core.SetParameterValue("cutoff", 0.65f));

    std::array<float, 48000> left{};
    std::array<float, 48000> right{};
    float midiEnergy = 0.0f;

    for(int note = 0; note < 8; ++note)
    {
        left.fill(0.0f);
        right.fill(0.0f);
        core.HandleMidiEvent(0x90, static_cast<std::uint8_t>(48 + note), 100);
        core.ProcessAudio(left.data(), right.data(), left.size());
        const float noteEnergy = StereoEnergy(left, right);
        EXPECT_GT(noteEnergy, 0.01f);
        midiEnergy += noteEnergy;
        core.HandleMidiEvent(0x80, static_cast<std::uint8_t>(48 + note), 0);
    }

    core.SetMenuItemValue("node0/menu/field_keys/b7", 1.0f);
    float clockEnergy = 0.0f;
    for(int block = 0; block < 4; ++block)
    {
        left.fill(0.0f);
        right.fill(0.0f);
        core.HandleMidiEvent(0xF8, 0, 0);
        core.ProcessAudio(left.data(), right.data(), left.size());
        clockEnergy += StereoEnergy(left, right);
    }

    EXPECT_TRUE(core.IsPlaying());
    EXPECT_GT(core.GetTriggerCount(), 0u);
    EXPECT_GT(midiEnergy, 10.0f);
    EXPECT_GT(clockEnergy, 0.01f);
}

TEST(SubharmoniqCoreTest, OledCompactStatusUsesClearQuantizeLabel)
{
    daisyhost::apps::SubharmoniqCore core("node0");
    core.Prepare(48000.0, 48);
    core.ResetToDefaultState(0);
    core.SetQuantizeMode(daisyhost::DaisySubharmoniqQuantizeMode::kTwelveJust);
    core.TickUi(0.0);

    const auto& display = core.GetDisplayModel();
    EXPECT_TRUE(DisplayContainsText(display, "Quant 12-JI"));
    EXPECT_FALSE(DisplayContainsText(display, "Q 12-JI"));
}

TEST(SubharmoniqCoreTest, OledShowsKnobTouchZoomForTwoSecondsAfterLastChange)
{
    daisyhost::apps::SubharmoniqCore core("node0");
    core.Prepare(48000.0, 48);
    core.ResetToDefaultState(0);
    ASSERT_TRUE(core.SetActivePage(daisyhost::DaisySubharmoniqPage::kVcf));

    core.SetControl("node0/control/cutoff", 0.50f);
    auto display = core.GetDisplayModel();
    EXPECT_TRUE(DisplayContainsText(display, "Cutoff"));
    EXPECT_TRUE(DisplayContainsText(display, "K1"));
    EXPECT_FALSE(display.bars.empty());

    core.TickUi(1900.0);
    display = core.GetDisplayModel();
    EXPECT_TRUE(DisplayContainsText(display, "Cutoff"));

    core.TickUi(200.0);
    display = core.GetDisplayModel();
    EXPECT_FALSE(DisplayContainsText(display, "K1 Cutoff"));
    EXPECT_TRUE(DisplayContainsText(display, "Subharmoniq VCF"));
}

TEST(SubharmoniqCoreTest, OledTouchZoomRefreshesWhenParameterMovesAgain)
{
    daisyhost::apps::SubharmoniqCore core("node0");
    core.Prepare(48000.0, 48);
    core.ResetToDefaultState(0);
    ASSERT_TRUE(core.SetActivePage(daisyhost::DaisySubharmoniqPage::kVcf));

    core.SetControl("node0/control/cutoff", 0.35f);
    core.TickUi(1500.0);
    core.SetControl("node0/control/cutoff", 0.70f);
    core.TickUi(1000.0);

    auto display = core.GetDisplayModel();
    EXPECT_TRUE(DisplayContainsText(display, "K1 Cutoff"));

    core.TickUi(1100.0);
    display = core.GetDisplayModel();
    EXPECT_FALSE(DisplayContainsText(display, "K1 Cutoff"));
}

TEST(SubharmoniqCoreTest, OledDoesNotRefreshWhenIdleControlValueIsReapplied)
{
    daisyhost::apps::SubharmoniqCore core("node0");
    core.Prepare(48000.0, 48);
    core.ResetToDefaultState(0);
    ASSERT_TRUE(core.SetActivePage(daisyhost::DaisySubharmoniqPage::kVcf));

    core.SetControl("node0/control/cutoff", 0.62f);
    core.TickUi(1900.0);
    core.SetControl("node0/control/cutoff", 0.62f);
    core.TickUi(200.0);

    auto display = core.GetDisplayModel();
    EXPECT_FALSE(DisplayContainsText(display, "K1 Cutoff"));
    EXPECT_TRUE(DisplayContainsText(display, "Subharmoniq VCF"));
}

TEST(SubharmoniqCoreTest, OledShowsCvMappedParameterZoom)
{
    daisyhost::apps::SubharmoniqCore core("node0");
    core.Prepare(48000.0, 48);
    core.ResetToDefaultState(0);

    core.SetCvInput(2, 0.50f);
    auto display = core.GetDisplayModel();
    EXPECT_TRUE(DisplayContainsText(display, "CV2"));
    EXPECT_TRUE(DisplayContainsText(display, "Cutoff"));
    EXPECT_FALSE(display.bars.empty());

    core.TickUi(2100.0);
    display = core.GetDisplayModel();
    EXPECT_FALSE(DisplayContainsText(display, "CV2 Cutoff"));
}

TEST(SubharmoniqCoreTest, OledShowsMenuParameterEditZoom)
{
    daisyhost::apps::SubharmoniqCore core("node0");
    core.Prepare(48000.0, 48);
    core.ResetToDefaultState(0);
    ASSERT_TRUE(core.SetActivePage(daisyhost::DaisySubharmoniqPage::kVcf));

    core.SetMenuItemValue("node0/menu/filter/param/cutoff", 0.44f);
    auto display = core.GetDisplayModel();
    EXPECT_TRUE(DisplayContainsText(display, "K1 Cutoff"));
    EXPECT_FALSE(display.bars.empty());
}

TEST(SubharmoniqCoreTest, OledShowsB7PlayConfirmation)
{
    daisyhost::apps::SubharmoniqCore core("node0");
    core.Prepare(48000.0, 48);
    core.ResetToDefaultState(0);

    core.SetMenuItemValue("node0/menu/field_keys/b7", 1.0f);
    auto display = core.GetDisplayModel();
    EXPECT_TRUE(DisplayContainsText(display, "B7 Play"));
    EXPECT_TRUE(DisplayContainsText(display, "Running"));
    EXPECT_TRUE(DisplayContainsText(display, "12-note Equal"));

    core.TickUi(2100.0);
    display = core.GetDisplayModel();
    EXPECT_FALSE(DisplayContainsText(display, "B7 Play"));
    EXPECT_TRUE(DisplayContainsText(display, "Play"));
}

TEST(SubharmoniqCoreTest, OledShowsB5QuantizeConfirmation)
{
    daisyhost::apps::SubharmoniqCore core("node0");
    core.Prepare(48000.0, 48);
    core.ResetToDefaultState(0);

    core.SetMenuItemValue("node0/menu/field_keys/b5", 1.0f);
    auto display = core.GetDisplayModel();
    EXPECT_TRUE(DisplayContainsText(display, "B5 Quantize"));
    EXPECT_TRUE(DisplayContainsText(display, "8-note Equal"));
}

TEST(SubharmoniqCoreTest, OledShowsRhythmTargetConfirmation)
{
    daisyhost::apps::SubharmoniqCore core("node0");
    core.Prepare(48000.0, 48);
    core.ResetToDefaultState(0);

    core.SetMenuItemValue("node0/menu/field_keys/a6", 1.0f);
    auto display = core.GetDisplayModel();
    EXPECT_TRUE(DisplayContainsText(display, "A6 Rhythm 2"));
    EXPECT_TRUE(DisplayContainsText(display, "Both"));
    EXPECT_TRUE(DisplayContainsText(display, "R2 advances both"));
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
    EXPECT_EQ(bindings.knobDetailLabels[0], "S1 Step1");
    EXPECT_EQ(bindings.knobDetailLabels[1], "S1 Step2");
    EXPECT_EQ(bindings.knobDetailLabels[2], "S1 Step3");
    EXPECT_EQ(bindings.knobDetailLabels[3], "S1 Step4");
    EXPECT_EQ(bindings.fieldKnobDetailLabels[4], "S2 Step1");
    EXPECT_EQ(bindings.fieldKnobDetailLabels[7], "S2 Step4");
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
    EXPECT_EQ(voiceBindings.knobDetailLabels[3], "VCO1 Sub2");
    EXPECT_EQ(voiceBindings.fieldKnobDetailLabels[4], "VCO2 Sub1");
    EXPECT_EQ(voiceBindings.fieldKnobDetailLabels[5], "VCO2 Sub2");
    EXPECT_EQ(voiceBindings.fieldKnobDetailLabels[6], "Quantize");
    EXPECT_EQ(voiceBindings.fieldKnobParameterIds[6], "node0/param/quantize_mode");

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

    EXPECT_EQ(core.GetActivePage(), daisyhost::DaisySubharmoniqPage::kSeqRhythm);
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
    EXPECT_EQ(core.GetActivePage(), daisyhost::DaisySubharmoniqPage::kSeqRhythm);
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

TEST(SubharmoniqCoreTest, HostedWrapperReportsStatefulFieldKeyLedValues)
{
    daisyhost::apps::SubharmoniqCore core("node0");
    core.Prepare(48000.0, 48);
    core.ResetToDefaultState(0);

    auto leds = core.GetFieldKeyLedValues();
    EXPECT_FLOAT_EQ(leds[4], 0.5f);
    EXPECT_FLOAT_EQ(leds[12], 0.5f);
    EXPECT_FLOAT_EQ(leds[13], 0.5f);
    EXPECT_FLOAT_EQ(leds[14], 0.0f);

    core.SetRhythmTarget(0, daisyhost::DaisySubharmoniqRhythmTarget::kSeq1);
    leds = core.GetFieldKeyLedValues();
    EXPECT_FLOAT_EQ(leds[4], 0.5f);

    core.SetRhythmTarget(0, daisyhost::DaisySubharmoniqRhythmTarget::kBoth);
    core.SetQuantizeMode(daisyhost::DaisySubharmoniqQuantizeMode::kTwelveJust);
    core.SetSeqOctaveRange(5);
    core.SetMenuItemValue("node0/menu/field_keys/b7", 1.0f);
    leds = core.GetFieldKeyLedValues();
    EXPECT_FLOAT_EQ(leds[4], 1.0f);
    EXPECT_FLOAT_EQ(leds[12], 1.0f);
    EXPECT_FLOAT_EQ(leds[13], 1.0f);
    EXPECT_FLOAT_EQ(leds[14], 1.0f);
    EXPECT_FLOAT_EQ(leds[15], 0.0f);
}

TEST(SubharmoniqCoreTest, FieldB7StartsAudioAndBothSequencersAfterA7RhythmEdit)
{
    daisyhost::apps::SubharmoniqCore core("node0");
    core.Prepare(48000.0, 48);
    core.ResetToDefaultState(0);

    core.SetMenuItemValue("node0/menu/field_keys/a7", 1.0f);
    EXPECT_EQ(core.GetActivePage(), daisyhost::DaisySubharmoniqPage::kSeqRhythm);

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
