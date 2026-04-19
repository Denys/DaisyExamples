#include <array>
#include <cmath>
#include <vector>

#include <gtest/gtest.h>

#include "daisyhost/apps/MultiDelayCore.h"

namespace
{
float MapExpectedDelay(float normalized)
{
    const float minDelay = 48000.0f * 0.05f;
    const float maxDelay = static_cast<float>(
        daisyhost::apps::MultiDelayCore::kMaxDelaySamples);
    return std::exp(
        std::log(minDelay)
        + (normalized * (std::log(maxDelay) - std::log(minDelay))));
}

std::size_t FindParameterIndex(
    const std::vector<daisyhost::ParameterDescriptor>& parameters,
    const std::string&                                 id)
{
    for(std::size_t i = 0; i < parameters.size(); ++i)
    {
        if(parameters[i].id == id)
        {
            return i;
        }
    }
    return parameters.size();
}

const daisyhost::MenuSection* FindSection(const daisyhost::MenuModel& menu,
                                          const std::string&          id)
{
    for(const auto& section : menu.sections)
    {
        if(section.id == id)
        {
            return &section;
        }
    }
    return nullptr;
}

void ClickEncoder(daisyhost::apps::MultiDelayCore& core)
{
    core.SetEncoderPress(true);
    core.SetEncoderPress(false);
}

TEST(MultiDelayCoreTest, ExposesParameterDescriptorsInPriorityOrder)
{
    daisyhost::apps::MultiDelayCore core("node0");
    core.Prepare(48000.0, 64);

    const auto& parameters = core.GetParameters();
    ASSERT_EQ(parameters.size(), 5u);

    EXPECT_EQ(parameters[0].id, "node0/param/dry_wet");
    EXPECT_EQ(parameters[0].role, daisyhost::ParameterRole::kMix);
    EXPECT_EQ(parameters[0].importanceRank, 0);
    EXPECT_TRUE(parameters[0].menuEditable);
    EXPECT_FLOAT_EQ(parameters[0].defaultNormalizedValue, 0.50f);
    EXPECT_FLOAT_EQ(parameters[0].effectiveNormalizedValue, 0.50f);
    EXPECT_EQ(parameters[0].stepCount, 0);
    EXPECT_EQ(parameters[0].unitLabel, "%");
    EXPECT_TRUE(parameters[0].automatable);
    EXPECT_TRUE(parameters[0].stateful);

    EXPECT_EQ(parameters[1].id, "node0/param/delay_primary");
    EXPECT_EQ(parameters[1].role, daisyhost::ParameterRole::kPrimaryDelay);
    EXPECT_EQ(parameters[1].importanceRank, 1);
    EXPECT_FLOAT_EQ(parameters[1].defaultNormalizedValue, 0.0f);
    EXPECT_FLOAT_EQ(parameters[1].effectiveNormalizedValue, 0.0f);
    EXPECT_EQ(parameters[1].unitLabel, "samples");
    EXPECT_TRUE(parameters[1].stateful);

    EXPECT_EQ(parameters[2].id, "node0/param/delay_secondary");
    EXPECT_EQ(parameters[2].role, daisyhost::ParameterRole::kSecondaryDelay);
    EXPECT_EQ(parameters[2].importanceRank, 2);
    EXPECT_FLOAT_EQ(parameters[2].defaultNormalizedValue, 0.0f);
    EXPECT_FLOAT_EQ(parameters[2].effectiveNormalizedValue, 0.0f);
    EXPECT_EQ(parameters[2].unitLabel, "samples");
    EXPECT_TRUE(parameters[2].stateful);

    EXPECT_EQ(parameters[3].id, "node0/param/feedback");
    EXPECT_EQ(parameters[3].role, daisyhost::ParameterRole::kFeedback);
    EXPECT_EQ(parameters[3].importanceRank, 3);
    EXPECT_FLOAT_EQ(parameters[3].defaultNormalizedValue, 0.0f);
    EXPECT_FLOAT_EQ(parameters[3].effectiveNormalizedValue, 0.0f);
    EXPECT_EQ(parameters[3].unitLabel, "%");
    EXPECT_TRUE(parameters[3].stateful);

    EXPECT_EQ(parameters[4].id, "node0/param/delay_tertiary");
    EXPECT_EQ(parameters[4].role, daisyhost::ParameterRole::kTertiaryDelay);
    EXPECT_EQ(parameters[4].importanceRank, 4);
    EXPECT_FLOAT_EQ(parameters[4].defaultNormalizedValue, 0.0f);
    EXPECT_FLOAT_EQ(parameters[4].effectiveNormalizedValue, 0.0f);
    EXPECT_EQ(parameters[4].unitLabel, "samples");
    EXPECT_TRUE(parameters[4].stateful);
}

TEST(MultiDelayCoreTest, ExposesTopLevelMenuSectionsAndClosedStatusByDefault)
{
    daisyhost::apps::MultiDelayCore core("node0");
    core.Prepare(48000.0, 64);

    const auto& menu = core.GetMenuModel();
    EXPECT_FALSE(menu.isOpen);
    EXPECT_FALSE(menu.isEditing);
    EXPECT_EQ(menu.currentSectionId, "root");

    const auto* root = FindSection(menu, "root");
    ASSERT_NE(root, nullptr);
    ASSERT_EQ(root->items.size(), 5u);
    EXPECT_EQ(root->items[0].label, "Params");
    EXPECT_EQ(root->items[1].label, "Input");
    EXPECT_EQ(root->items[2].label, "MIDI");
    EXPECT_EQ(root->items[3].label, "Tracker");
    EXPECT_EQ(root->items[4].label, "About");
}

TEST(MultiDelayCoreTest, EncoderNavigatesMenuAndEditsParametersLive)
{
    daisyhost::apps::MultiDelayCore core("node0");
    core.Prepare(48000.0, 64);

    ClickEncoder(core);
    EXPECT_TRUE(core.GetMenuModel().isOpen);
    EXPECT_EQ(core.GetMenuModel().currentSectionId, "root");

    ClickEncoder(core);
    EXPECT_EQ(core.GetMenuModel().currentSectionId, "params");
    EXPECT_FALSE(core.GetMenuModel().isEditing);

    core.SetEncoderDelta(1);
    EXPECT_EQ(core.GetMenuModel().currentSelection, 1);
    ClickEncoder(core);
    EXPECT_TRUE(core.GetMenuModel().isEditing);

    core.SetEncoderDelta(3);
    EXPECT_EQ(core.GetDryWetPercent(), 65);
    EXPECT_EQ(core.GetParameters()[0].normalizedValue, 0.65f);

    ClickEncoder(core);
    EXPECT_FALSE(core.GetMenuModel().isEditing);

    const auto& display = core.GetDisplayModel();
    EXPECT_EQ(display.mode, daisyhost::DisplayMode::kMenu);
    ASSERT_FALSE(display.texts.empty());
    EXPECT_EQ(display.texts.front().text, "Params");
}

TEST(MultiDelayCoreTest, KnobAndCvControlsMapToFinalHierarchy)
{
    daisyhost::apps::MultiDelayCore core("node0");
    core.Prepare(48000.0, 32);

    core.SetControl(
        daisyhost::apps::MultiDelayCore::MakeKnobControlId("node0", 1), 0.25f);
    core.SetControl(
        daisyhost::apps::MultiDelayCore::MakeKnobControlId("node0", 2), 0.0f);
    core.SetControl(
        daisyhost::apps::MultiDelayCore::MakeKnobControlId("node0", 3), 1.0f);
    core.SetControl(
        daisyhost::apps::MultiDelayCore::MakeKnobControlId("node0", 4), 0.5f);

    daisyhost::PortValue cv1;
    cv1.type   = daisyhost::VirtualPortType::kCv;
    cv1.scalar = 0.75f;
    core.SetPortInput(
        daisyhost::apps::MultiDelayCore::MakeCvInputPortId("node0", 1), cv1);

    EXPECT_NEAR(core.GetDelayTargetSamples(0), 2400.0f, 0.5f);
    EXPECT_NEAR(core.GetDelayTargetSamples(1), 48000.0f, 1.0f);
    EXPECT_NEAR(core.GetDelayTargetSamples(2), MapExpectedDelay(0.75f), 1.0f);
    EXPECT_FLOAT_EQ(core.GetFeedback(), 0.5f);
    EXPECT_EQ(core.GetDryWetPercent(), 25);
}

TEST(MultiDelayCoreTest, PatchBindingsRoundTripThroughControlReadback)
{
    daisyhost::apps::MultiDelayCore core("node0");
    core.Prepare(48000.0, 32);

    const auto bindings = core.GetPatchBindings();
    const std::array<float, 4> testValues = {{0.17f, 0.38f, 0.59f, 0.77f}};

    for(std::size_t i = 0; i < bindings.knobControlIds.size(); ++i)
    {
        ASSERT_FALSE(bindings.knobControlIds[i].empty());
        core.SetControl(bindings.knobControlIds[i], testValues[i]);
        const auto reflected = core.GetControlValue(bindings.knobControlIds[i]);
        ASSERT_TRUE(reflected.hasValue) << "missing reflected value for "
                                        << bindings.knobControlIds[i];
        EXPECT_NEAR(reflected.value, testValues[i], 0.0001f)
            << "binding mismatch for " << bindings.knobControlIds[i];
    }
}

TEST(MultiDelayCoreTest, LastTouchWinsUntilKnobOrCvActuallyMovesAgain)
{
    daisyhost::apps::MultiDelayCore core("node0");
    core.Prepare(48000.0, 64);

    const auto& parameters = core.GetParameters();
    const auto  primaryIndex
        = FindParameterIndex(parameters, "node0/param/delay_primary");
    const auto tertiaryIndex
        = FindParameterIndex(parameters, "node0/param/delay_tertiary");
    ASSERT_LT(primaryIndex, parameters.size());
    ASSERT_LT(tertiaryIndex, parameters.size());

    core.SetControl(
        daisyhost::apps::MultiDelayCore::MakeKnobControlId("node0", 2), 0.20f);
    EXPECT_NEAR(core.GetParameters()[primaryIndex].normalizedValue, 0.20f, 0.0001f);

    core.SetMenuItemValue("node0/menu/params/delay_primary", 0.80f);
    EXPECT_NEAR(core.GetParameters()[primaryIndex].normalizedValue, 0.80f, 0.0001f);

    core.SetControl(
        daisyhost::apps::MultiDelayCore::MakeKnobControlId("node0", 2), 0.20f);
    EXPECT_NEAR(core.GetParameters()[primaryIndex].normalizedValue, 0.80f, 0.0001f);

    core.SetControl(
        daisyhost::apps::MultiDelayCore::MakeKnobControlId("node0", 2), 0.35f);
    EXPECT_NEAR(core.GetParameters()[primaryIndex].normalizedValue, 0.35f, 0.0001f);

    daisyhost::PortValue cv1;
    cv1.type   = daisyhost::VirtualPortType::kCv;
    cv1.scalar = 0.10f;
    core.SetPortInput(
        daisyhost::apps::MultiDelayCore::MakeCvInputPortId("node0", 1), cv1);
    EXPECT_NEAR(core.GetParameters()[tertiaryIndex].normalizedValue, 0.10f, 0.0001f);

    core.SetMenuItemValue("node0/menu/params/delay_tertiary", 0.90f);
    EXPECT_NEAR(core.GetParameters()[tertiaryIndex].normalizedValue, 0.90f, 0.0001f);

    core.SetPortInput(
        daisyhost::apps::MultiDelayCore::MakeCvInputPortId("node0", 1), cv1);
    EXPECT_NEAR(core.GetParameters()[tertiaryIndex].normalizedValue, 0.90f, 0.0001f);

    cv1.scalar = 0.12f;
    core.SetPortInput(
        daisyhost::apps::MultiDelayCore::MakeCvInputPortId("node0", 1), cv1);
    EXPECT_NEAR(core.GetParameters()[tertiaryIndex].normalizedValue, 0.12f, 0.0001f);
}

TEST(MultiDelayCoreTest, SupportsDirectParameterAccessAndRejectsUnknownIds)
{
    daisyhost::apps::MultiDelayCore core("node0");
    core.Prepare(48000.0, 64);

    ASSERT_TRUE(core.SetParameterValue("node0/param/delay_primary", 0.8f));
    const auto requested = core.GetParameterValue("node0/param/delay_primary");
    ASSERT_TRUE(requested.hasValue);
    EXPECT_NEAR(requested.value, 0.8f, 0.0001f);

    const auto effective
        = core.GetEffectiveParameterValue("node0/param/delay_primary");
    ASSERT_TRUE(effective.hasValue);
    EXPECT_NEAR(effective.value, 0.8f, 0.0001f);

    EXPECT_FALSE(core.SetParameterValue("node0/param/unknown", 0.5f));
    EXPECT_FALSE(core.GetParameterValue("node0/param/unknown").hasValue);
    EXPECT_FALSE(core.GetEffectiveParameterValue("node0/param/unknown").hasValue);

    const auto requestedAfterUnknown
        = core.GetParameterValue("node0/param/delay_primary");
    ASSERT_TRUE(requestedAfterUnknown.hasValue);
    EXPECT_NEAR(requestedAfterUnknown.value, 0.8f, 0.0001f);
}

TEST(MultiDelayCoreTest, CanResetCaptureAndRestoreCanonicalParameterState)
{
    daisyhost::apps::MultiDelayCore source("node0");
    source.Prepare(48000.0, 64);

    ASSERT_TRUE(source.SetParameterValue("node0/param/dry_wet", 0.23f));
    ASSERT_TRUE(source.SetParameterValue("node0/param/delay_primary", 0.71f));
    ASSERT_TRUE(source.SetParameterValue("node0/param/delay_secondary", 0.52f));
    ASSERT_TRUE(source.SetParameterValue("node0/param/feedback", 0.61f));
    ASSERT_TRUE(source.SetParameterValue("node0/param/delay_tertiary", 0.87f));

    const auto captured = source.CaptureStatefulParameterValues();
    ASSERT_EQ(captured.size(), 5u);
    EXPECT_NEAR(captured.at("node0/param/dry_wet"), 0.23f, 0.0001f);
    EXPECT_NEAR(captured.at("node0/param/delay_primary"), 0.71f, 0.0001f);
    EXPECT_NEAR(captured.at("node0/param/delay_secondary"), 0.52f, 0.0001f);
    EXPECT_NEAR(captured.at("node0/param/feedback"), 0.61f, 0.0001f);
    EXPECT_NEAR(captured.at("node0/param/delay_tertiary"), 0.87f, 0.0001f);

    source.ResetToDefaultState(1234u);
    EXPECT_EQ(source.GetDryWetPercent(), 50);

    const auto resetPrimary = source.GetParameterValue("node0/param/delay_primary");
    const auto resetSecondary
        = source.GetParameterValue("node0/param/delay_secondary");
    const auto resetFeedback = source.GetParameterValue("node0/param/feedback");
    const auto resetTertiary
        = source.GetParameterValue("node0/param/delay_tertiary");
    ASSERT_TRUE(resetPrimary.hasValue);
    ASSERT_TRUE(resetSecondary.hasValue);
    ASSERT_TRUE(resetFeedback.hasValue);
    ASSERT_TRUE(resetTertiary.hasValue);
    EXPECT_FLOAT_EQ(resetPrimary.value, 0.0f);
    EXPECT_FLOAT_EQ(resetSecondary.value, 0.0f);
    EXPECT_FLOAT_EQ(resetFeedback.value, 0.0f);
    EXPECT_FLOAT_EQ(resetTertiary.value, 0.0f);

    daisyhost::apps::MultiDelayCore restored("node0");
    restored.Prepare(48000.0, 64);
    restored.ResetToDefaultState(1234u);
    restored.RestoreStatefulParameterValues(captured);

    const auto restoredDryWet = restored.GetParameterValue("node0/param/dry_wet");
    const auto restoredPrimary
        = restored.GetParameterValue("node0/param/delay_primary");
    const auto restoredSecondary
        = restored.GetParameterValue("node0/param/delay_secondary");
    const auto restoredFeedback
        = restored.GetParameterValue("node0/param/feedback");
    const auto restoredTertiary
        = restored.GetParameterValue("node0/param/delay_tertiary");

    ASSERT_TRUE(restoredDryWet.hasValue);
    ASSERT_TRUE(restoredPrimary.hasValue);
    ASSERT_TRUE(restoredSecondary.hasValue);
    ASSERT_TRUE(restoredFeedback.hasValue);
    ASSERT_TRUE(restoredTertiary.hasValue);
    EXPECT_NEAR(restoredDryWet.value, 0.23f, 0.0001f);
    EXPECT_NEAR(restoredPrimary.value, 0.71f, 0.0001f);
    EXPECT_NEAR(restoredSecondary.value, 0.52f, 0.0001f);
    EXPECT_NEAR(restoredFeedback.value, 0.61f, 0.0001f);
    EXPECT_NEAR(restoredTertiary.value, 0.87f, 0.0001f);
}

TEST(MultiDelayCoreTest, DisplaySwitchesBetweenStatusAndMenuModels)
{
    daisyhost::apps::MultiDelayCore core("node0");
    core.Prepare(48000.0, 64);

    const auto statusDisplay = core.GetDisplayModel();
    EXPECT_EQ(statusDisplay.mode, daisyhost::DisplayMode::kStatus);
    ASSERT_GE(statusDisplay.texts.size(), 2u);
    EXPECT_EQ(statusDisplay.texts[0].text, "Multi Delay");

    ClickEncoder(core);
    const auto rootDisplay = core.GetDisplayModel();
    EXPECT_EQ(rootDisplay.mode, daisyhost::DisplayMode::kMenu);
    ASSERT_FALSE(rootDisplay.texts.empty());
    EXPECT_EQ(rootDisplay.texts[0].text, "Menu");
}

TEST(MultiDelayCoreTest, FixedInputProducesDeterministicOutputAndPortLevels)
{
    daisyhost::apps::MultiDelayCore core("node0");
    core.Prepare(48000.0, 64);
    core.SetControl(
        daisyhost::apps::MultiDelayCore::MakeKnobControlId("node0", 1), 0.5f);
    core.SetControl(
        daisyhost::apps::MultiDelayCore::MakeKnobControlId("node0", 2), 0.0f);
    core.SetControl(
        daisyhost::apps::MultiDelayCore::MakeKnobControlId("node0", 3), 0.0f);
    core.SetControl(
        daisyhost::apps::MultiDelayCore::MakeKnobControlId("node0", 4), 0.0f);

    daisyhost::PortValue cv1;
    cv1.type   = daisyhost::VirtualPortType::kCv;
    cv1.scalar = 0.0f;
    core.SetPortInput(
        daisyhost::apps::MultiDelayCore::MakeCvInputPortId("node0", 1), cv1);

    std::vector<float> inputSignal(64, 1.0f);
    std::array<std::vector<float>, 4> outputs;
    for(auto& channel : outputs)
    {
        channel.assign(64, -1.0f);
    }

    std::array<const float*, 1> inputPtrs = {{inputSignal.data()}};
    std::array<float*, 4> outputPtrs = {
        {outputs[0].data(), outputs[1].data(), outputs[2].data(), outputs[3].data()}};

    core.Process({inputPtrs.data(), inputPtrs.size()},
                 {outputPtrs.data(), outputPtrs.size()},
                 64);

    EXPECT_NEAR(outputs[0][0], 0.0f, 0.0001f);
    EXPECT_NEAR(outputs[1][0], 0.0f, 0.0001f);
    EXPECT_NEAR(outputs[2][0], 0.0f, 0.0001f);
    EXPECT_NEAR(outputs[3][0], 0.5f, 0.0001f);
    EXPECT_NEAR(outputs[0][1], 0.95990407f, 0.0001f);
    EXPECT_NEAR(outputs[3][1], 0.93195683f, 0.0001f);

    float checksum = 0.0f;
    for(std::size_t frame = 0; frame < 64; ++frame)
    {
        checksum += outputs[3][frame];
    }

    EXPECT_NEAR(checksum, 60.331989f, 0.0001f);

    const auto mixPort = core.GetPortOutput(
        daisyhost::apps::MultiDelayCore::MakeAudioOutputPortId("node0", 4));
    EXPECT_NEAR(mixPort.scalar, 0.95000005f, 0.0001f);
}
} // namespace
