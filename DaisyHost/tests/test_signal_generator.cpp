#include <gtest/gtest.h>

#include "daisyhost/SignalGenerator.h"

namespace
{
TEST(SignalGeneratorTest, ManualCvVoltageMapsToNormalizedFiveVoltRange)
{
    daisyhost::CvInputGeneratorState state;
    state.mode         = daisyhost::CvInputSourceMode::kManual;
    state.manualVolts  = 2.5f;

    const float volts = daisyhost::StepCvInputGenerator(&state, 0.1);

    EXPECT_NEAR(volts, 2.5f, 0.0001f);
    EXPECT_NEAR(daisyhost::CvVoltsToNormalized(volts), 0.5f, 0.0001f);
}

TEST(SignalGeneratorTest, GeneratorCvUsesBiasAndDepthWithinZeroToFiveVolts)
{
    daisyhost::CvInputGeneratorState state;
    state.mode           = daisyhost::CvInputSourceMode::kGenerator;
    state.waveform       = daisyhost::BasicWaveform::kSine;
    state.frequencyHz    = 1.0f;
    state.amplitudeVolts = 2.5f;
    state.biasVolts      = 2.5f;
    state.phase          = 0.0f;

    const float firstVolts = daisyhost::StepCvInputGenerator(&state, 0.25);
    const float secondVolts = daisyhost::StepCvInputGenerator(&state, 0.25);

    EXPECT_NEAR(firstVolts, 2.5f, 0.0001f);
    EXPECT_NEAR(secondVolts, 5.0f, 0.0001f);
    EXPECT_NEAR(state.phase, 0.5f, 0.0001f);
}

TEST(SignalGeneratorTest, GeneratorAmplitudeIsClampedToHalfRange)
{
    daisyhost::CvInputGeneratorState state;
    state.mode           = daisyhost::CvInputSourceMode::kGenerator;
    state.waveform       = daisyhost::BasicWaveform::kSine;
    state.frequencyHz    = 1.0f;
    state.amplitudeVolts = 5.0f;
    state.biasVolts      = 2.5f;
    state.phase          = 0.25f;

    const float volts = daisyhost::StepCvInputGenerator(&state, 0.0);

    EXPECT_NEAR(volts, 5.0f, 0.0001f);
}
} // namespace
