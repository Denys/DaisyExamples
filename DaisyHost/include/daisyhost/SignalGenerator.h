#pragma once

#include <cstdint>

namespace daisyhost
{
enum class BasicWaveform
{
    kSine = 0,
    kTriangle,
    kSquare,
    kSaw,
};

int         ClampBasicWaveform(int waveform);
const char* GetBasicWaveformName(int waveform);
float       GenerateBasicWaveformSample(BasicWaveform waveform, float phase);

enum class CvInputSourceMode
{
    kManual = 0,
    kGenerator,
};

struct CvInputGeneratorState
{
    CvInputSourceMode mode           = CvInputSourceMode::kManual;
    BasicWaveform     waveform       = BasicWaveform::kSine;
    float             frequencyHz    = 1.0f;
    float             amplitudeVolts = 2.5f;
    float             biasVolts      = 2.5f;
    float             manualVolts    = 2.5f;
    float             phase          = 0.0f;
};

float ClampCvVoltage(float volts);
float ClampCvAmplitudeVolts(float volts);
float CvVoltsToNormalized(float volts);
float NormalizedToCvVolts(float normalizedValue);
float StepCvInputGenerator(CvInputGeneratorState* state, double elapsedSeconds);
} // namespace daisyhost
