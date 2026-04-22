#include <array>

#include <gtest/gtest.h>

#include "daisyhost/EffectiveHostStateSnapshot.h"
#include "daisyhost/TestInputSignal.h"
#include "daisyhost/apps/MultiDelayCore.h"
#include "daisyhost/apps/TorusCore.h"

namespace
{
TEST(EffectiveHostStateSnapshotTest,
     CapturesCanonicalAndEffectiveValuesAlongsideCvGateAndAudioInputState)
{
    daisyhost::apps::MultiDelayCore core("node0");
    core.Prepare(48000.0, 64);
    core.ResetToDefaultState(99u);

    ASSERT_TRUE(core.SetParameterValue("node0/param/delay_tertiary", 0.35f));

    daisyhost::PortValue cvInput;
    cvInput.type   = daisyhost::VirtualPortType::kCv;
    cvInput.scalar = 0.90f;
    core.SetPortInput(
        daisyhost::apps::MultiDelayCore::MakeCvInputPortId("node0", 1), cvInput);

    const auto parameters = core.GetParameters();
    const auto automationSlots
        = daisyhost::BuildHostAutomationSlotBindings(parameters);

    std::array<daisyhost::HostCvInputState, 4> cvInputs{};
    cvInputs[0].normalizedValue = 0.90f;
    cvInputs[0].volts           = 4.50f;
    cvInputs[0].sourceMode      = 1;
    cvInputs[0].waveform        = 2;
    cvInputs[0].frequencyHz     = 1.50f;
    cvInputs[0].amplitudeVolts  = 1.25f;
    cvInputs[0].biasVolts       = 3.50f;
    cvInputs[0].manualVolts     = 4.00f;

    std::array<daisyhost::HostGateInputState, 2> gateInputs{};
    gateInputs[0].value = true;

    daisyhost::HostAudioInputState audioInput;
    audioInput.mode        = static_cast<int>(daisyhost::TestInputSignalMode::kSawInput);
    audioInput.level       = 3.5f;
    audioInput.frequencyHz = 330.0f;

    const auto snapshot = daisyhost::BuildEffectiveHostStateSnapshot(
        "multidelay",
        "Multi Delay",
        core.GetPatchBindings(),
        parameters,
        automationSlots,
        cvInputs,
        gateInputs,
        audioInput);

    EXPECT_EQ(snapshot.appId, "multidelay");
    EXPECT_EQ(snapshot.appDisplayName, "Multi Delay");
    ASSERT_EQ(snapshot.automationSlots.size(), daisyhost::kHostAutomationSlotCount);
    EXPECT_EQ(snapshot.automationSlots[4].parameterId, "node0/param/delay_tertiary");
    EXPECT_FLOAT_EQ(snapshot.automationSlots[4].normalizedValue, 0.90f);
    EXPECT_FLOAT_EQ(snapshot.automationSlots[4].effectiveNormalizedValue, 0.90f);
    EXPECT_EQ(snapshot.cvInputs[0].portId, "node0/port/cv_in_1");
    EXPECT_FLOAT_EQ(snapshot.cvInputs[0].normalizedValue, 0.90f);
    EXPECT_FLOAT_EQ(snapshot.cvInputs[0].volts, 4.50f);
    EXPECT_EQ(snapshot.gateInputs[0].portId, "node0/port/gate_in_1");
    EXPECT_TRUE(snapshot.gateInputs[0].value);
    EXPECT_EQ(snapshot.audioInput.mode,
              static_cast<int>(daisyhost::TestInputSignalMode::kSawInput));
    EXPECT_EQ(snapshot.audioInput.modeName, "Saw");
    EXPECT_FLOAT_EQ(snapshot.audioInput.level, 3.5f);
    EXPECT_FLOAT_EQ(snapshot.audioInput.frequencyHz, 330.0f);
}

TEST(EffectiveHostStateSnapshotTest,
     BuildsCoherentTorusSnapshotAfterCanonicalStateRestore)
{
    daisyhost::apps::TorusCore source("node0");
    daisyhost::apps::TorusCore restored("node0");

    source.Prepare(48000.0, daisyhost::apps::TorusCore::kPreferredBlockSize);
    restored.Prepare(48000.0, daisyhost::apps::TorusCore::kPreferredBlockSize);
    source.ResetToDefaultState(1234u);
    restored.ResetToDefaultState(1234u);

    ASSERT_TRUE(source.SetParameterValue("node0/param/frequency", 0.42f));
    ASSERT_TRUE(source.SetParameterValue("node0/param/brightness", 0.71f));
    ASSERT_TRUE(source.SetParameterValue("node0/param/position", 0.63f));

    restored.RestoreStatefulParameterValues(source.CaptureStatefulParameterValues());

    const auto parameters = restored.GetParameters();
    const auto automationSlots
        = daisyhost::BuildHostAutomationSlotBindings(parameters);

    const std::array<daisyhost::HostCvInputState, 4> cvInputs{};
    const std::array<daisyhost::HostGateInputState, 2> gateInputs{};
    const daisyhost::HostAudioInputState audioInput{};

    const auto snapshot = daisyhost::BuildEffectiveHostStateSnapshot(
        "torus",
        "Torus",
        restored.GetPatchBindings(),
        parameters,
        automationSlots,
        cvInputs,
        gateInputs,
        audioInput);

    EXPECT_EQ(snapshot.appId, "torus");
    EXPECT_EQ(snapshot.appDisplayName, "Torus");
    EXPECT_EQ(snapshot.automationSlots[0].parameterId, "node0/param/frequency");
    EXPECT_FLOAT_EQ(snapshot.automationSlots[0].normalizedValue, 0.42f);
    EXPECT_FLOAT_EQ(snapshot.automationSlots[0].effectiveNormalizedValue, 0.42f);
    EXPECT_EQ(snapshot.automationSlots[2].parameterId, "node0/param/brightness");
    EXPECT_FLOAT_EQ(snapshot.automationSlots[2].normalizedValue, 0.71f);
    EXPECT_EQ(snapshot.automationSlots[4].parameterId, "node0/param/position");
    EXPECT_FLOAT_EQ(snapshot.automationSlots[4].normalizedValue, 0.63f);
}
} // namespace
