#include <gtest/gtest.h>

#include <algorithm>

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

void ExpectKnobControlTarget(const daisyhost::DaisyFieldControlMapping& mapping,
                             std::size_t                               zeroBasedIndex,
                             const std::string&                        controlSuffix)
{
    ASSERT_LT(zeroBasedIndex, mapping.knobs.size());
    EXPECT_TRUE(mapping.knobs[zeroBasedIndex].available);
    EXPECT_EQ(mapping.knobs[zeroBasedIndex].targetKind,
              daisyhost::BoardSurfaceTargetKind::kControl);
    EXPECT_EQ(mapping.knobs[zeroBasedIndex].targetId,
              "node0/control/" + controlSuffix);
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
    const auto polyosc = MakeApp("polyosc", "node0");

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
    ExpectKnobControlTarget(cloudseedMapping, 4, "pre_delay");
    ExpectKnobControlTarget(cloudseedMapping, 5, "damping");
    ExpectKnobControlTarget(cloudseedMapping, 6, "mod_amount");
    ExpectKnobControlTarget(cloudseedMapping, 7, "mod_rate");

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

    const auto polyoscMapping = daisyhost::BuildDaisyFieldControlMapping(
        polyosc->GetPatchBindings(), polyosc->GetParameters(), 4, "node0");
    ExpectKnobTarget(polyoscMapping, 4, "waveform");
    ExpectKnobDisabled(polyoscMapping, 5);
    ExpectKnobDisabled(polyoscMapping, 6);
    ExpectKnobDisabled(polyoscMapping, 7);
}

TEST(BoardControlMappingTest, FieldExtraKnobsExposePolyOscWaveformOnly)
{
    const auto polyosc = MakeApp("polyosc", "node0");

    const auto mapping = daisyhost::BuildDaisyFieldControlMapping(
        polyosc->GetPatchBindings(), polyosc->GetParameters(), 4, "node0");

    ExpectKnobTarget(mapping, 4, "waveform");
    ExpectKnobDisabled(mapping, 5);
    ExpectKnobDisabled(mapping, 6);
    ExpectKnobDisabled(mapping, 7);
}

TEST(BoardControlMappingTest, FieldAlternativeLayoutUsesRemainingPublicParameters)
{
    const auto multidelay = MakeApp("multidelay", "node0");
    ASSERT_NE(multidelay, nullptr);

    const auto multidelayMapping = daisyhost::BuildDaisyFieldControlMapping(
        multidelay->GetPatchBindings(),
        multidelay->GetParameters(),
        multidelay->GetMenuModel(),
        4,
        "node0",
        daisyhost::DaisyFieldKnobLayoutMode::kControllableParameters);

    ExpectKnobTarget(multidelayMapping, 0, "delay_tertiary");
    EXPECT_EQ(multidelayMapping.knobs[0].detailLabel, "Delay 3");
    for(std::size_t i = 1; i < daisyhost::kDaisyFieldKnobCount; ++i)
    {
        ExpectKnobDisabled(multidelayMapping, i);
    }

    const auto cloudseed = MakeApp("cloudseed", "node0");
    ASSERT_NE(cloudseed, nullptr);

    const auto cloudseedMapping = daisyhost::BuildDaisyFieldControlMapping(
        cloudseed->GetPatchBindings(),
        cloudseed->GetParameters(),
        cloudseed->GetMenuModel(),
        4,
        "node0",
        daisyhost::DaisyFieldKnobLayoutMode::kControllableParameters);

    ExpectKnobTarget(cloudseedMapping, 0, "eq_low_freq");
    ExpectKnobTarget(cloudseedMapping, 1, "eq_high_freq");
    ExpectKnobTarget(cloudseedMapping, 2, "eq_cutoff");
    ExpectKnobTarget(cloudseedMapping, 3, "eq_low_gain");
    ExpectKnobTarget(cloudseedMapping, 4, "eq_high_gain");
    ExpectKnobTarget(cloudseedMapping, 5, "eq_cross_seed");
    ExpectKnobTarget(cloudseedMapping, 6, "seed_diffusion");
    ExpectKnobTarget(cloudseedMapping, 7, "seed_delay");
}

TEST(BoardControlMappingTest, FieldAlternativeLayoutDoesNotOverlapDefaultKnobs)
{
    const auto cloudseed = MakeApp("cloudseed", "node0");
    ASSERT_NE(cloudseed, nullptr);

    const auto defaultMapping = daisyhost::BuildDaisyFieldControlMapping(
        cloudseed->GetPatchBindings(),
        cloudseed->GetParameters(),
        cloudseed->GetMenuModel(),
        4,
        "node0",
        daisyhost::DaisyFieldKnobLayoutMode::kPatchPagePlusExtras);
    const auto alternativeMapping = daisyhost::BuildDaisyFieldControlMapping(
        cloudseed->GetPatchBindings(),
        cloudseed->GetParameters(),
        cloudseed->GetMenuModel(),
        4,
        "node0",
        daisyhost::DaisyFieldKnobLayoutMode::kControllableParameters);

    std::vector<std::string> defaultTargets;
    for(const auto& knob : defaultMapping.knobs)
    {
        if(knob.available)
        {
            defaultTargets.push_back(knob.targetId);
        }
    }

    for(const auto& knob : alternativeMapping.knobs)
    {
        if(!knob.available)
        {
            continue;
        }
        EXPECT_EQ(std::find(defaultTargets.begin(),
                            defaultTargets.end(),
                            knob.targetId),
                  defaultTargets.end())
            << knob.targetId;
    }
}

TEST(BoardControlMappingTest, FieldPublicParameterListUsesControllableMetadata)
{
    const auto cloudseed = MakeApp("cloudseed", "node0");
    ASSERT_NE(cloudseed, nullptr);

    const auto publicParameters = daisyhost::BuildDaisyFieldPublicParameterList(
        cloudseed->GetPatchBindings(), cloudseed->GetParameters());

    ASSERT_EQ(publicParameters.size(), 16u);
    EXPECT_EQ(publicParameters[0].targetId, "node0/param/mix");
    EXPECT_EQ(publicParameters[0].detailLabel, "Mix");
    EXPECT_EQ(publicParameters[4].targetId, "node0/param/pre_delay");
    EXPECT_EQ(publicParameters[7].targetId, "node0/param/mod_rate");
    EXPECT_EQ(publicParameters[8].targetId, "node0/param/eq_low_freq");
    EXPECT_EQ(publicParameters[15].targetId, "node0/param/seed_delay");
}

TEST(BoardControlMappingTest, FieldDrawerPagesNavigateCircularly)
{
    EXPECT_EQ(daisyhost::StepDaisyFieldDrawerPage(
                  daisyhost::DaisyFieldDrawerPage::kKeyboardMidiCv, 1),
              daisyhost::DaisyFieldDrawerPage::kPublicParameters);
    EXPECT_EQ(daisyhost::StepDaisyFieldDrawerPage(
                  daisyhost::DaisyFieldDrawerPage::kPublicParameters, 1),
              daisyhost::DaisyFieldDrawerPage::kRackAudio);
    EXPECT_EQ(daisyhost::StepDaisyFieldDrawerPage(
                  daisyhost::DaisyFieldDrawerPage::kRackAudio, 1),
              daisyhost::DaisyFieldDrawerPage::kKeyboardMidiCv);
    EXPECT_EQ(daisyhost::StepDaisyFieldDrawerPage(
                  daisyhost::DaisyFieldDrawerPage::kKeyboardMidiCv, -1),
              daisyhost::DaisyFieldDrawerPage::kRackAudio);
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

TEST(BoardControlMappingTest, FieldSwitchesMapToHostedAppMenuNavigation)
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
    EXPECT_EQ(mapping.switches[0].targetId, "node0/menu/navigation/back");
    EXPECT_EQ(mapping.switches[0].label, "SW1");
    EXPECT_EQ(mapping.switches[0].detailLabel, "Back");

    EXPECT_TRUE(mapping.switches[1].available);
    EXPECT_EQ(mapping.switches[1].targetId, "node0/menu/navigation/forward");
    EXPECT_EQ(mapping.switches[1].detailLabel, "Forward");
}

TEST(BoardControlMappingTest, FieldSwitchesStayAvailableForHostedAppMenuNavigation)
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

    EXPECT_TRUE(mapping.switches[0].available);
    EXPECT_EQ(mapping.switches[0].targetId, "node0/menu/navigation/back");
    EXPECT_TRUE(mapping.switches[1].available);
    EXPECT_EQ(mapping.switches[1].targetId, "node0/menu/navigation/forward");
}

TEST(BoardControlMappingTest, FieldCvTargetOptionsPreserveLatchedKnobTargets)
{
    const auto app = MakeApp("cloudseed", "node0");
    ASSERT_NE(app, nullptr);

    const auto defaultMapping = daisyhost::BuildDaisyFieldControlMapping(
        app->GetPatchBindings(), app->GetParameters(), app->GetMenuModel(), 4, "node0");
    const auto alternativeMapping = daisyhost::BuildDaisyFieldControlMapping(
        app->GetPatchBindings(),
        app->GetParameters(),
        app->GetMenuModel(),
        4,
        "node0",
        daisyhost::DaisyFieldKnobLayoutMode::kControllableParameters);
    const auto defaultOptions = daisyhost::BuildDaisyFieldCvTargetOptions(
        defaultMapping, alternativeMapping, "");
    ASSERT_GT(defaultOptions.size(), 1u);
    EXPECT_EQ(defaultOptions[0].targetKind,
              daisyhost::BoardSurfaceTargetKind::kUnavailable);
    EXPECT_EQ(defaultOptions[1].targetId, "node0/control/size");
    EXPECT_EQ(defaultOptions[1].detailLabel, "K2.1 Size");

    const std::string latchedTarget = "node0/control/size";
    app->SetMenuItemValue("node0/menu/pages/page", 1.0f);
    const auto advancedMapping = daisyhost::BuildDaisyFieldControlMapping(
        app->GetPatchBindings(), app->GetParameters(), app->GetMenuModel(), 4, "node0");
    const auto advancedAlternativeMapping = daisyhost::BuildDaisyFieldControlMapping(
        app->GetPatchBindings(),
        app->GetParameters(),
        app->GetMenuModel(),
        4,
        "node0",
        daisyhost::DaisyFieldKnobLayoutMode::kControllableParameters);
    const auto advancedOptions = daisyhost::BuildDaisyFieldCvTargetOptions(
        advancedMapping, advancedAlternativeMapping, latchedTarget);

    ASSERT_FALSE(advancedOptions.empty());
    const auto found = std::find_if(
        advancedOptions.begin(),
        advancedOptions.end(),
        [&latchedTarget](const daisyhost::BoardSurfaceBinding& binding) {
            return binding.targetId == latchedTarget;
        });
    ASSERT_NE(found, advancedOptions.end());
    EXPECT_EQ(found->detailLabel, "Latched K target");
}

TEST(BoardControlMappingTest, FieldCvTargetOptionsExcludeAudioCriticalTargets)
{
    const auto app = MakeApp("cloudseed", "node0");
    ASSERT_NE(app, nullptr);

    const auto defaultMapping = daisyhost::BuildDaisyFieldControlMapping(
        app->GetPatchBindings(), app->GetParameters(), app->GetMenuModel(), 4, "node0");
    const auto alternativeMapping = daisyhost::BuildDaisyFieldControlMapping(
        app->GetPatchBindings(),
        app->GetParameters(),
        app->GetMenuModel(),
        4,
        "node0",
        daisyhost::DaisyFieldKnobLayoutMode::kControllableParameters);
    const auto defaultOptions = daisyhost::BuildDaisyFieldCvTargetOptions(
        defaultMapping, alternativeMapping, "");

    auto hasTarget = [&defaultOptions](const std::string& targetId) {
        return std::any_of(
            defaultOptions.begin(),
            defaultOptions.end(),
            [&targetId](const daisyhost::BoardSurfaceBinding& binding) {
                return binding.targetId == targetId;
            });
    };

    EXPECT_FALSE(hasTarget("node0/control/mix"));
    EXPECT_FALSE(hasTarget("node0/param/global_input_mix"));
    EXPECT_FALSE(hasTarget("node0/param/global_dry_out"));
    EXPECT_TRUE(hasTarget("node0/control/size"));
    EXPECT_TRUE(hasTarget("node0/control/decay"));
    EXPECT_TRUE(hasTarget("node0/control/diffusion"));
    EXPECT_TRUE(hasTarget("node0/param/eq_low_freq"));
    EXPECT_TRUE(hasTarget("node0/param/eq_cutoff"));
}

TEST(BoardControlMappingTest, FieldCvTargetOptionsLabelDefaultAndAlternativeSets)
{
    const auto app = MakeApp("cloudseed", "node0");
    ASSERT_NE(app, nullptr);

    const auto defaultMapping = daisyhost::BuildDaisyFieldControlMapping(
        app->GetPatchBindings(),
        app->GetParameters(),
        app->GetMenuModel(),
        4,
        "node0",
        daisyhost::DaisyFieldKnobLayoutMode::kPatchPagePlusExtras);
    const auto alternativeMapping = daisyhost::BuildDaisyFieldControlMapping(
        app->GetPatchBindings(),
        app->GetParameters(),
        app->GetMenuModel(),
        4,
        "node0",
        daisyhost::DaisyFieldKnobLayoutMode::kControllableParameters);

    const auto options = daisyhost::BuildDaisyFieldCvTargetOptions(
        defaultMapping, alternativeMapping, "");

    auto hasLabel = [&options](const std::string& label) {
        return std::any_of(
            options.begin(),
            options.end(),
            [&label](const daisyhost::BoardSurfaceBinding& binding) {
                return binding.detailLabel == label;
            });
    };

    EXPECT_TRUE(hasLabel("K2.1 Size"));
    EXPECT_TRUE(hasLabel("K8.1 Mod Rate"));
    EXPECT_TRUE(hasLabel("K1.2 Low Freq"));
    EXPECT_TRUE(hasLabel("K3.2 Cutoff"));
    EXPECT_TRUE(hasLabel("K8.2 Delay Seed"));
    EXPECT_FALSE(hasLabel("K1.1 Mix"));
}

TEST(BoardControlMappingTest, FieldCvLatchedTargetOwnsItsCvLane)
{
    EXPECT_TRUE(daisyhost::ShouldForwardDaisyFieldCvInput(""));
    EXPECT_FALSE(
        daisyhost::ShouldForwardDaisyFieldCvInput("node0/control/size"));
    EXPECT_FALSE(
        daisyhost::ShouldForwardDaisyFieldCvInput("node0/param/eq_cutoff"));
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

TEST(BoardControlMappingTest, SubharmoniqFieldKeysMapToDedicatedPerformanceActions)
{
    const auto app = MakeApp("subharmoniq", "node0");
    ASSERT_NE(app, nullptr);

    const auto mapping = daisyhost::BuildDaisyFieldControlMapping(
        app->GetPatchBindings(), app->GetParameters(), app->GetMenuModel(), 4, "node0");

    ASSERT_EQ(mapping.keys.size(), daisyhost::kDaisyFieldKeyCount);
    EXPECT_EQ(mapping.keys[0].controlId, "node0/control/field_key_a_1");
    EXPECT_EQ(mapping.keys[0].label, "A1");
    EXPECT_EQ(mapping.keys[0].detailLabel, "Seq1 Step1");
    EXPECT_EQ(mapping.keys[0].targetKind,
              daisyhost::BoardSurfaceTargetKind::kMenuItem);
    EXPECT_EQ(mapping.keys[0].targetId, "node0/menu/field_keys/a1");
    EXPECT_EQ(mapping.keys[0].midiNote, -1);

    EXPECT_EQ(mapping.keys[6].label, "A7");
    EXPECT_EQ(mapping.keys[6].detailLabel, "Rhythm 3");
    EXPECT_EQ(mapping.keys[6].targetId, "node0/menu/field_keys/a7");

    EXPECT_EQ(mapping.keys[14].label, "B7");
    EXPECT_EQ(mapping.keys[14].detailLabel, "Play/Stop");
    EXPECT_EQ(mapping.keys[14].targetKind,
              daisyhost::BoardSurfaceTargetKind::kMenuItem);
    EXPECT_EQ(mapping.keys[14].targetId, "node0/menu/field_keys/b7");
    EXPECT_EQ(mapping.keys[14].midiNote, -1);

    EXPECT_EQ(mapping.keys[15].label, "B8");
    EXPECT_EQ(mapping.keys[15].detailLabel, "Reset");
    EXPECT_EQ(mapping.keys[15].targetId, "node0/menu/field_keys/b8");
}

TEST(BoardControlMappingTest, FieldKeyInteractivityAllowsMidiAndMenuActions)
{
    const daisyhost::HostedAppPatchBindings           defaultBindings;
    const std::vector<daisyhost::ParameterDescriptor> parameters;
    const auto midiMapping = daisyhost::BuildDaisyFieldControlMapping(
        defaultBindings, parameters, 4, "node0");
    EXPECT_TRUE(daisyhost::IsDaisyFieldKeyInteractive(midiMapping.keys[0]));

    const auto app = MakeApp("subharmoniq", "node0");
    ASSERT_NE(app, nullptr);
    const auto menuMapping = daisyhost::BuildDaisyFieldControlMapping(
        app->GetPatchBindings(), app->GetParameters(), app->GetMenuModel(), 4, "node0");
    ASSERT_EQ(menuMapping.keys[14].targetKind,
              daisyhost::BoardSurfaceTargetKind::kMenuItem);
    ASSERT_EQ(menuMapping.keys[14].midiNote, -1);
    EXPECT_TRUE(daisyhost::IsDaisyFieldKeyInteractive(menuMapping.keys[14]));

    auto unavailable = menuMapping.keys[14];
    unavailable.available = false;
    EXPECT_FALSE(daisyhost::IsDaisyFieldKeyInteractive(unavailable));

    auto missingTarget = menuMapping.keys[14];
    missingTarget.targetId.clear();
    EXPECT_FALSE(daisyhost::IsDaisyFieldKeyInteractive(missingTarget));
}
} // namespace
