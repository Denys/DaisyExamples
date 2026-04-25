#include <gtest/gtest.h>

#include "daisyhost/AppRegistry.h"
#include "daisyhost/BoardControlMapping.h"

namespace
{
std::unique_ptr<daisyhost::HostedAppCore> MakeApp(const std::string& appId,
                                                  const std::string& nodeId)
{
    auto app = daisyhost::CreateHostedAppCore(appId, nodeId);
    EXPECT_NE(app, nullptr);
    return app;
}

daisyhost::ParameterDescriptor MakeParameter(const std::string& id,
                                             int                importanceRank,
                                             bool               automatable = true)
{
    daisyhost::ParameterDescriptor parameter;
    parameter.id             = id;
    parameter.label          = id;
    parameter.importanceRank = importanceRank;
    parameter.automatable    = automatable;
    return parameter;
}

void ExpectKnobTarget(const daisyhost::DaisyFieldControlMapping& mapping,
                      std::size_t                               zeroBasedIndex,
                      const std::string&                        parameterSuffix)
{
    ASSERT_LT(zeroBasedIndex, mapping.knobs.size());
    EXPECT_TRUE(mapping.knobs[zeroBasedIndex].available);
    EXPECT_EQ(mapping.knobs[zeroBasedIndex].targetKind,
              daisyhost::BoardSurfaceTargetKind::kParameter);
    EXPECT_EQ(mapping.knobs[zeroBasedIndex].targetId,
              "node0/param/" + parameterSuffix);
}

void ExpectKnobDisabled(const daisyhost::DaisyFieldControlMapping& mapping,
                        std::size_t                               zeroBasedIndex)
{
    ASSERT_LT(zeroBasedIndex, mapping.knobs.size());
    EXPECT_FALSE(mapping.knobs[zeroBasedIndex].available);
    EXPECT_EQ(mapping.knobs[zeroBasedIndex].targetKind,
              daisyhost::BoardSurfaceTargetKind::kUnavailable);
    EXPECT_TRUE(mapping.knobs[zeroBasedIndex].targetId.empty());
}

TEST(BoardControlMappingTest, FieldKnobsMirrorPatchPageThenRankedParameters)
{
    const auto app = MakeApp("vasynth", "node0");
    ASSERT_NE(app, nullptr);

    const auto mapping = daisyhost::BuildDaisyFieldControlMapping(
        app->GetPatchBindings(), app->GetParameters(), 4, "node0");

    ASSERT_EQ(mapping.knobs.size(), daisyhost::kDaisyFieldKnobCount);
    EXPECT_EQ(mapping.knobs[0].controlId, "node0/control/field_knob_1");
    EXPECT_EQ(mapping.knobs[0].targetKind,
              daisyhost::BoardSurfaceTargetKind::kControl);
    EXPECT_TRUE(mapping.knobs[0].available);
    EXPECT_EQ(mapping.knobs[0].targetId, "node0/control/osc_mix");
    EXPECT_EQ(mapping.knobs[1].targetId, "node0/control/detune");
    EXPECT_EQ(mapping.knobs[2].targetId, "node0/control/osc1_wave");
    EXPECT_EQ(mapping.knobs[3].targetId, "node0/control/osc2_wave");

    EXPECT_EQ(mapping.knobs[4].controlId, "node0/control/field_knob_5");
    EXPECT_EQ(mapping.knobs[4].targetKind,
              daisyhost::BoardSurfaceTargetKind::kParameter);
    EXPECT_EQ(mapping.knobs[4].targetId, "node0/param/filter_cutoff");
    EXPECT_EQ(mapping.knobs[5].targetId, "node0/param/resonance");
    EXPECT_EQ(mapping.knobs[6].targetId, "node0/param/filter_env_amount");
    EXPECT_EQ(mapping.knobs[7].targetId, "node0/param/level");
}

TEST(BoardControlMappingTest, FieldExtraKnobsExcludeExplicitPatchParameterIds)
{
    daisyhost::HostedAppPatchBindings patchBindings;
    patchBindings.knobControlIds[0] = "node0/control/custom_a";
    patchBindings.knobControlIds[1] = "node0/control/custom_b";
    patchBindings.knobControlIds[2] = "node0/control/custom_c";
    patchBindings.knobControlIds[3] = "node0/control/custom_d";
    patchBindings.knobParameterIds[0] = "node0/param/alpha";
    patchBindings.knobParameterIds[1] = "node0/param/bravo";
    patchBindings.knobParameterIds[2] = "node0/param/charlie";
    patchBindings.knobParameterIds[3] = "node0/param/delta";

    const std::vector<daisyhost::ParameterDescriptor> parameters = {
        MakeParameter("node0/param/alpha", 0),
        MakeParameter("node0/param/bravo", 1),
        MakeParameter("node0/param/charlie", 2),
        MakeParameter("node0/param/delta", 3),
        MakeParameter("node0/param/echo", 4),
    };

    const auto mapping = daisyhost::BuildDaisyFieldControlMapping(
        patchBindings, parameters, 4, "node0");

    ExpectKnobTarget(mapping, 4, "echo");
    ExpectKnobDisabled(mapping, 5);
    ExpectKnobDisabled(mapping, 6);
    ExpectKnobDisabled(mapping, 7);
}

TEST(BoardControlMappingTest, FieldExtraKnobsUseLockedMappingsForSupportedApps)
{
    const auto multidelay = MakeApp("multidelay", "node0");
    const auto torus = MakeApp("torus", "node0");
    const auto cloudseed = MakeApp("cloudseed", "node0");
    const auto braids = MakeApp("braids", "node0");
    const auto harmoniqs = MakeApp("harmoniqs", "node0");
    const auto vasynth = MakeApp("vasynth", "node0");

    const auto multidelayMapping = daisyhost::BuildDaisyFieldControlMapping(
        multidelay->GetPatchBindings(), multidelay->GetParameters(), 4, "node0");
    ExpectKnobTarget(multidelayMapping, 4, "delay_tertiary");
    ExpectKnobDisabled(multidelayMapping, 5);
    ExpectKnobDisabled(multidelayMapping, 6);
    ExpectKnobDisabled(multidelayMapping, 7);

    const auto torusMapping = daisyhost::BuildDaisyFieldControlMapping(
        torus->GetPatchBindings(), torus->GetParameters(), 4, "node0");
    ExpectKnobTarget(torusMapping, 4, "position");
    ExpectKnobTarget(torusMapping, 5, "polyphony");
    ExpectKnobTarget(torusMapping, 6, "model");
    ExpectKnobTarget(torusMapping, 7, "easter_fx");

    const auto cloudseedMapping = daisyhost::BuildDaisyFieldControlMapping(
        cloudseed->GetPatchBindings(), cloudseed->GetParameters(), 4, "node0");
    ExpectKnobTarget(cloudseedMapping, 4, "pre_delay");
    ExpectKnobTarget(cloudseedMapping, 5, "damping");
    ExpectKnobTarget(cloudseedMapping, 6, "mod_amount");
    ExpectKnobTarget(cloudseedMapping, 7, "mod_rate");

    const auto braidsMapping = daisyhost::BuildDaisyFieldControlMapping(
        braids->GetPatchBindings(), braids->GetParameters(), 4, "node0");
    ExpectKnobTarget(braidsMapping, 4, "signature");
    ExpectKnobDisabled(braidsMapping, 5);
    ExpectKnobDisabled(braidsMapping, 6);
    ExpectKnobDisabled(braidsMapping, 7);

    const auto harmoniqsMapping = daisyhost::BuildDaisyFieldControlMapping(
        harmoniqs->GetPatchBindings(), harmoniqs->GetParameters(), 4, "node0");
    ExpectKnobTarget(harmoniqsMapping, 4, "attack");
    ExpectKnobTarget(harmoniqsMapping, 5, "release");
    ExpectKnobTarget(harmoniqsMapping, 6, "detune");
    ExpectKnobTarget(harmoniqsMapping, 7, "level");

    const auto vasynthMapping = daisyhost::BuildDaisyFieldControlMapping(
        vasynth->GetPatchBindings(), vasynth->GetParameters(), 4, "node0");
    ExpectKnobTarget(vasynthMapping, 4, "filter_cutoff");
    ExpectKnobTarget(vasynthMapping, 5, "resonance");
    ExpectKnobTarget(vasynthMapping, 6, "filter_env_amount");
    ExpectKnobTarget(vasynthMapping, 7, "level");
}

TEST(BoardControlMappingTest, MissingExtraParametersDisableFieldKnobs)
{
    daisyhost::HostedAppPatchBindings patchBindings;
    patchBindings.knobControlIds[0] = "nodeX/control/alpha";
    patchBindings.knobControlIds[1] = "nodeX/control/bravo";
    patchBindings.knobControlIds[2] = "nodeX/control/charlie";
    patchBindings.knobControlIds[3] = "nodeX/control/delta";

    const std::vector<daisyhost::ParameterDescriptor> parameters = {
        MakeParameter("nodeX/param/alpha", 0),
        MakeParameter("nodeX/param/bravo", 1),
        MakeParameter("nodeX/param/internal", 2, false),
    };

    const auto mapping = daisyhost::BuildDaisyFieldControlMapping(
        patchBindings, parameters, 4, "nodeX");

    for(std::size_t index = 0; index < 4; ++index)
    {
        EXPECT_TRUE(mapping.knobs[index].available);
        EXPECT_EQ(mapping.knobs[index].targetKind,
                  daisyhost::BoardSurfaceTargetKind::kControl);
    }
    for(std::size_t index = 4; index < mapping.knobs.size(); ++index)
    {
        EXPECT_FALSE(mapping.knobs[index].available);
        EXPECT_EQ(mapping.knobs[index].targetKind,
                  daisyhost::BoardSurfaceTargetKind::kUnavailable);
        EXPECT_TRUE(mapping.knobs[index].targetId.empty());
    }
}

TEST(BoardControlMappingTest, FieldCvInputsAndGateUsePatchVirtualPorts)
{
    const auto app = MakeApp("multidelay", "nodeA");
    ASSERT_NE(app, nullptr);

    const auto mapping = daisyhost::BuildDaisyFieldControlMapping(
        app->GetPatchBindings(), app->GetParameters(), 4, "nodeA");

    ASSERT_EQ(mapping.cvInputs.size(), daisyhost::kDaisyFieldCvInputCount);
    EXPECT_TRUE(mapping.cvInputs[0].available);
    EXPECT_EQ(mapping.cvInputs[0].targetKind,
              daisyhost::BoardSurfaceTargetKind::kCvInput);
    EXPECT_EQ(mapping.cvInputs[0].targetId, "nodeA/port/cv_in_1");
    EXPECT_EQ(mapping.cvInputs[3].targetId, "nodeA/port/cv_in_4");

    EXPECT_TRUE(mapping.gateInput.available);
    EXPECT_EQ(mapping.gateInput.targetKind,
              daisyhost::BoardSurfaceTargetKind::kGateInput);
    EXPECT_EQ(mapping.gateInput.targetId, "nodeA/port/gate_in_1");
}

TEST(BoardControlMappingTest, FieldCvOutputsMirrorK5AndK6MappedParameters)
{
    const auto app = MakeApp("vasynth", "node0");
    ASSERT_NE(app, nullptr);

    const auto mapping = daisyhost::BuildDaisyFieldControlMapping(
        app->GetPatchBindings(), app->GetParameters(), app->GetMenuModel(), 4, "node0");

    ASSERT_EQ(mapping.cvOutputs.size(), daisyhost::kDaisyFieldCvOutputCount);
    EXPECT_TRUE(mapping.cvOutputs[0].available);
    EXPECT_EQ(mapping.cvOutputs[0].controlId, "node0/control/field_cv_out_1");
    EXPECT_EQ(mapping.cvOutputs[0].targetKind,
              daisyhost::BoardSurfaceTargetKind::kParameter);
    EXPECT_EQ(mapping.cvOutputs[0].targetId, "node0/param/filter_cutoff");
    EXPECT_EQ(mapping.cvOutputs[0].label, "CV OUT 1");
    EXPECT_TRUE(mapping.cvOutputs[1].available);
    EXPECT_EQ(mapping.cvOutputs[1].targetId, "node0/param/resonance");
}

TEST(BoardControlMappingTest, FieldCvOutputPortIdsFollowNodeId)
{
    EXPECT_EQ(daisyhost::MakeDaisyFieldCvOutputPortId("node1", 0),
              "node1/port/field_cv_out_1");
    EXPECT_EQ(daisyhost::MakeDaisyFieldCvOutputPortId("node1", 1),
              "node1/port/field_cv_out_2");
    EXPECT_THROW(daisyhost::MakeDaisyFieldCvOutputPortId("node1", 2),
                 std::out_of_range);
}

TEST(BoardControlMappingTest, FieldCvOutputsDisableWhenExtraKnobsAreUnavailable)
{
    daisyhost::HostedAppPatchBindings patchBindings;
    patchBindings.knobControlIds[0] = "nodeX/control/alpha";
    patchBindings.knobControlIds[1] = "nodeX/control/bravo";
    patchBindings.knobControlIds[2] = "nodeX/control/charlie";
    patchBindings.knobControlIds[3] = "nodeX/control/delta";

    const std::vector<daisyhost::ParameterDescriptor> parameters = {
        MakeParameter("nodeX/param/alpha", 0),
        MakeParameter("nodeX/param/bravo", 1),
    };
    const daisyhost::MenuModel menu;

    const auto mapping = daisyhost::BuildDaisyFieldControlMapping(
        patchBindings, parameters, menu, 4, "nodeX");

    for(const auto& cvOutput : mapping.cvOutputs)
    {
        EXPECT_FALSE(cvOutput.available);
        EXPECT_EQ(cvOutput.targetKind, daisyhost::BoardSurfaceTargetKind::kUnavailable);
        EXPECT_TRUE(cvOutput.targetId.empty());
    }
}

TEST(BoardControlMappingTest, FieldSwitchesMapToFirstTwoMomentaryUtilities)
{
    const auto app = MakeApp("vasynth", "node0");
    ASSERT_NE(app, nullptr);

    const auto mapping = daisyhost::BuildDaisyFieldControlMapping(
        app->GetPatchBindings(), app->GetParameters(), app->GetMenuModel(), 4, "node0");

    ASSERT_EQ(mapping.switches.size(), daisyhost::kDaisyFieldSwitchCount);
    EXPECT_TRUE(mapping.switches[0].available);
    EXPECT_EQ(mapping.switches[0].controlId, "node0/control/field_sw_1");
    EXPECT_EQ(mapping.switches[0].targetKind,
              daisyhost::BoardSurfaceTargetKind::kMenuItem);
    EXPECT_EQ(mapping.switches[0].targetId, "node0/menu/utilities/audition");
    EXPECT_EQ(mapping.switches[0].label, "SW1");
    EXPECT_EQ(mapping.switches[0].detailLabel, "Audition");

    EXPECT_TRUE(mapping.switches[1].available);
    EXPECT_EQ(mapping.switches[1].targetId, "node0/menu/utilities/init_patch");
    EXPECT_EQ(mapping.switches[1].detailLabel, "Init Patch");
}

TEST(BoardControlMappingTest, FieldSwitchesDisableWhenMomentaryUtilitiesAreMissing)
{
    const daisyhost::HostedAppPatchBindings patchBindings;
    const std::vector<daisyhost::ParameterDescriptor> parameters;
    daisyhost::MenuModel menu;
    daisyhost::MenuSection section;
    section.id = "node0/menu/info";
    section.title = "Info";
    section.items.push_back({"node0/menu/info/page",
                             "Page",
                             false,
                             daisyhost::MenuItemActionKind::kReadonly,
                             0.0f,
                             "",
                             ""});
    menu.sections.push_back(section);

    const auto mapping = daisyhost::BuildDaisyFieldControlMapping(
        patchBindings, parameters, menu, 4, "node0");

    for(const auto& fieldSwitch : mapping.switches)
    {
        EXPECT_FALSE(fieldSwitch.available);
        EXPECT_EQ(fieldSwitch.targetKind,
                  daisyhost::BoardSurfaceTargetKind::kUnavailable);
        EXPECT_TRUE(fieldSwitch.targetId.empty());
    }
}

TEST(BoardControlMappingTest, FieldLedBindingsExposeKeysSwitchesAndGateIndicators)
{
    const daisyhost::HostedAppPatchBindings patchBindings;
    const std::vector<daisyhost::ParameterDescriptor> parameters;
    const daisyhost::MenuModel menu;

    const auto mapping = daisyhost::BuildDaisyFieldControlMapping(
        patchBindings, parameters, menu, 4, "node0");

    ASSERT_EQ(mapping.leds.size(), daisyhost::kDaisyFieldLedCount);
    EXPECT_EQ(mapping.leds[0].controlId, "node0/led/field_key_a_1");
    EXPECT_EQ(mapping.leds[0].label, "A1 LED");
    EXPECT_EQ(mapping.leds[0].targetKind, daisyhost::BoardSurfaceTargetKind::kLed);
    EXPECT_EQ(mapping.leds[0].targetId, "node0/control/field_key_a_1");
    EXPECT_TRUE(mapping.leds[0].available);

    EXPECT_EQ(mapping.leds[15].controlId, "node0/led/field_key_b_8");
    EXPECT_EQ(mapping.leds[16].controlId, "node0/led/field_sw_1");
    EXPECT_EQ(mapping.leds[16].targetId, "node0/control/field_sw_1");
    EXPECT_EQ(mapping.leds[18].controlId, "node0/led/field_gate_in");
    EXPECT_EQ(mapping.leds[18].targetId, "node0/control/field_gate_in");
    EXPECT_EQ(mapping.leds[19].controlId, "node0/led/field_gate_out");
    EXPECT_EQ(mapping.leds[19].targetId, "node0/port/gate_out_1");
}

TEST(BoardControlMappingTest, FieldKeysMapChromaticallyFromKeyboardOctave)
{
    const daisyhost::HostedAppPatchBindings            patchBindings;
    const std::vector<daisyhost::ParameterDescriptor> parameters;

    const auto mapping = daisyhost::BuildDaisyFieldControlMapping(
        patchBindings, parameters, 4, "node0");

    ASSERT_EQ(mapping.keys.size(), daisyhost::kDaisyFieldKeyCount);
    EXPECT_EQ(mapping.keys[0].controlId, "node0/control/field_key_a_1");
    EXPECT_EQ(mapping.keys[0].label, "A1");
    EXPECT_EQ(mapping.keys[0].targetKind,
              daisyhost::BoardSurfaceTargetKind::kMidiNote);
    EXPECT_TRUE(mapping.keys[0].available);
    EXPECT_EQ(mapping.keys[0].midiNote, 60);

    EXPECT_EQ(mapping.keys[15].controlId, "node0/control/field_key_b_8");
    EXPECT_EQ(mapping.keys[15].label, "B8");
    EXPECT_EQ(mapping.keys[15].midiNote, 75);
}
} // namespace
