#include "daisyhost/SignalGenerator.h"

#include <algorithm>
#include <cmath>

namespace daisyhost
{
namespace
{
constexpr float kTwoPi = 6.28318530717958647692f;

float WrapPhase(float phase)
{
    if(phase >= 1.0f || phase < 0.0f)
    {
        phase -= std::floor(phase);
    }
    return phase;
}
} // namespace

int ClampBasicWaveform(int waveform)
{
    return std::clamp(waveform,
                      static_cast<int>(BasicWaveform::kSine),
                      static_cast<int>(BasicWaveform::kSaw));
}

const char* GetBasicWaveformName(int waveform)
{
    switch(static_cast<BasicWaveform>(ClampBasicWaveform(waveform)))
    {
        case BasicWaveform::kSine: return "Sine";
        case BasicWaveform::kTriangle: return "Triangle";
        case BasicWaveform::kSquare: return "Square";
        case BasicWaveform::kSaw: return "Saw";
    }

    return "Sine";
}

float GenerateBasicWaveformSample(BasicWaveform waveform, float phase)
{
    const float wrapped = WrapPhase(phase);
    switch(waveform)
    {
        case BasicWaveform::kSine:
            return std::sin(wrapped * kTwoPi);
        case BasicWaveform::kTriangle:
            return 1.0f - 4.0f * std::fabs(wrapped - 0.5f);
        case BasicWaveform::kSquare: return wrapped < 0.5f ? -1.0f : 1.0f;
        case BasicWaveform::kSaw:
        default: return (wrapped * 2.0f) - 1.0f;
    }
}

float ClampCvVoltage(float volts)
{
    return std::clamp(volts, 0.0f, 5.0f);
}

float CvVoltsToNormalized(float volts)
{
    return ClampCvVoltage(volts) / 5.0f;
}

float NormalizedToCvVolts(float normalizedValue)
{
    return std::clamp(normalizedValue, 0.0f, 1.0f) * 5.0f;
}

float StepCvInputGenerator(CvInputGeneratorState* state, double elapsedSeconds)
{
    if(state == nullptr)
    {
        return 0.0f;
    }

    float outputVolts = ClampCvVoltage(state->manualVolts);
    if(state->mode == CvInputSourceMode::kGenerator)
    {
        outputVolts = ClampCvVoltage(
            state->biasVolts
            + state->amplitudeVolts
                  * GenerateBasicWaveformSample(state->waveform, state->phase));

        if(elapsedSeconds > 0.0)
        {
            state->phase = WrapPhase(
                state->phase
                + static_cast<float>(state->frequencyHz * elapsedSeconds));
        }
    }
    else
    {
        state->phase = WrapPhase(state->phase);
    }

    return outputVolts;
}
} // namespace daisyhost
