#include <algorithm>

#include <gtest/gtest.h>

#include "daisyhost/AppRegistry.h"
#include "daisyhost/HostModulation.h"

namespace
{
TEST(HostModulationTest, SumsFourActiveLanesAndClampsInNativeUnits)
{
    daisyhost::ParameterDescriptor parameter;
    parameter.id              = "node0/param/mix";
    parameter.label           = "Mix";
    parameter.normalizedValue = 0.50f;
    parameter.nativeMinimum   = 0.0f;
    parameter.nativeMaximum   = 100.0f;
    parameter.nativeDefault   = 50.0f;
    parameter.nativePrecision = 0;
    parameter.unitLabel       = "%";

    std::array<daisyhost::HostModulationLane, daisyhost::kHostModulationLaneCount> lanes{};
    lanes[0].enabled          = true;
    lanes[0].source           = daisyhost::HostModulationSource::kCv1;
    lanes[0].cvTargetMinimum  = 0.0f;
    lanes[0].cvTargetMaximum  = 20.0f;
    lanes[1].enabled          = true;
    lanes[1].source           = daisyhost::HostModulationSource::kLfo2;
    lanes[1].bipolarDepth     = 12.0f;
    lanes[2].enabled          = true;
    lanes[2].source           = daisyhost::HostModulationSource::kLfo3;
    lanes[2].bipolarDepth     = 8.0f;
    lanes[3].enabled          = true;
    lanes[3].source           = daisyhost::HostModulationSource::kCv4;
    lanes[3].cvTargetMinimum  = -40.0f;
    lanes[3].cvTargetMaximum  = 40.0f;

    daisyhost::HostModulationSourceValues sources;
    sources.cvNormalized[0]  = 1.0f;
    sources.cvNormalized[3]  = 1.0f;
    sources.lfoBipolar[1]    = -0.50f;
    sources.lfoBipolar[2]    = 0.25f;

    const auto result = daisyhost::EvaluateHostModulation(parameter, lanes, sources);

    EXPECT_FLOAT_EQ(result.baseNativeValue, 50.0f);
    EXPECT_FLOAT_EQ(result.lanes[0].nativeContribution, 20.0f);
    EXPECT_FLOAT_EQ(result.lanes[1].nativeContribution, -6.0f);
    EXPECT_FLOAT_EQ(result.lanes[2].nativeContribution, 2.0f);
    EXPECT_FLOAT_EQ(result.lanes[3].nativeContribution, 40.0f);
    EXPECT_FLOAT_EQ(result.resultNativeValue, 100.0f);
    EXPECT_FLOAT_EQ(result.resultNormalizedValue, 1.0f);
    EXPECT_TRUE(result.clamped);
}

TEST(HostModulationTest, IgnoresBypassedEmptyAndDiscreteLanes)
{
    daisyhost::ParameterDescriptor continuous;
    continuous.id              = "node0/param/pre_delay";
    continuous.normalizedValue = 0.25f;
    continuous.nativeMinimum   = 0.0f;
    continuous.nativeMaximum   = 1000.0f;
    continuous.nativeDefault   = 0.0f;
    continuous.nativePrecision = 0;
    continuous.unitLabel       = "ms";

    std::array<daisyhost::HostModulationLane, daisyhost::kHostModulationLaneCount> lanes{};
    lanes[0].enabled      = false;
    lanes[0].source       = daisyhost::HostModulationSource::kLfo1;
    lanes[0].bipolarDepth = 500.0f;
    lanes[1].enabled      = true;
    lanes[1].source       = daisyhost::HostModulationSource::kNone;
    lanes[1].bipolarDepth = 500.0f;

    daisyhost::HostModulationSourceValues sources;
    sources.lfoBipolar[0] = 1.0f;

    const auto result = daisyhost::EvaluateHostModulation(continuous, lanes, sources);
    EXPECT_FLOAT_EQ(result.resultNativeValue, 250.0f);
    EXPECT_FLOAT_EQ(result.resultNormalizedValue, 0.25f);
    EXPECT_FALSE(result.clamped);

    daisyhost::ParameterDescriptor discrete = continuous;
    discrete.stepCount = 4;
    EXPECT_FALSE(daisyhost::IsHostModulationTargetEligible(discrete));
    EXPECT_TRUE(daisyhost::IsHostModulationTargetEligible(continuous));
}

TEST(HostModulationTest, HostedAppsKeepBaseValuesWhenEffectiveValueChanges)
{
    for(const auto& registration : daisyhost::GetHostedAppRegistrations())
    {
        auto app = registration.create("node0");
        app->Prepare(48000.0, 64);
        app->ResetToDefaultState(0);

        const auto& parameters = app->GetParameters();
        const auto parameterIt = std::find_if(
            parameters.begin(),
            parameters.end(),
            [](const daisyhost::ParameterDescriptor& parameter) {
                return daisyhost::IsHostModulationTargetEligible(parameter);
            });
        ASSERT_NE(parameterIt, parameters.end())
            << "no eligible modulation target in " << registration.appId;
        const std::string parameterId = parameterIt->id;

        ASSERT_TRUE(app->SetParameterValue(parameterId, 0.25f))
            << registration.appId << " rejected base set for "
            << parameterId;
        ASSERT_TRUE(app->SetEffectiveParameterValue(parameterId, 0.75f))
            << registration.appId << " rejected effective set for "
            << parameterId;

        const auto base = app->GetParameterValue(parameterId);
        const auto effective = app->GetEffectiveParameterValue(parameterId);
        ASSERT_TRUE(base.hasValue) << registration.appId;
        ASSERT_TRUE(effective.hasValue) << registration.appId;
        EXPECT_FLOAT_EQ(base.value, 0.25f) << registration.appId;
        EXPECT_FLOAT_EQ(effective.value, 0.75f) << registration.appId;

        app->ClearEffectiveParameterOverrides();
        const auto restoredEffective
            = app->GetEffectiveParameterValue(parameterId);
        ASSERT_TRUE(restoredEffective.hasValue) << registration.appId;
        EXPECT_FLOAT_EQ(restoredEffective.value, 0.25f) << registration.appId;
    }
}
} // namespace
