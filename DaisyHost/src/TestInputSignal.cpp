#include "daisyhost/TestInputSignal.h"

#include <algorithm>
#include <cmath>

#include "daisyhost/SignalGenerator.h"

namespace daisyhost
{
namespace
{
float Clamp01(float value)
{
    return std::clamp(value, 0.0f, 1.0f);
}

std::uint32_t NextNoise(std::uint32_t* state)
{
    if(state == nullptr)
    {
        static std::uint32_t fallback = 1u;
        state                         = &fallback;
    }

    *state = (*state * 1664525u) + 1013904223u;
    return *state;
}
} // namespace

int ClampTestInputSignalMode(int mode)
{
    return std::clamp(mode,
                      static_cast<int>(TestInputSignalMode::kHostInput),
                      static_cast<int>(TestInputSignalMode::kSquareInput));
}

const char* GetTestInputSignalModeName(int mode)
{
    switch(static_cast<TestInputSignalMode>(ClampTestInputSignalMode(mode)))
    {
        case TestInputSignalMode::kHostInput: return "Host In";
        case TestInputSignalMode::kSineInput: return "Sine";
        case TestInputSignalMode::kTriangleInput: return "Triangle";
        case TestInputSignalMode::kSquareInput: return "Square";
        case TestInputSignalMode::kSawInput: return "Saw";
        case TestInputSignalMode::kNoiseInput: return "Noise";
        case TestInputSignalMode::kImpulseInput: return "Impulse";
    }

    return "Host In";
}

float TestInputSignalModeToNormalized(int mode)
{
    const int clamped = ClampTestInputSignalMode(mode);
    const int maxMode = static_cast<int>(TestInputSignalMode::kSquareInput);
    if(maxMode <= 0)
    {
        return 0.0f;
    }
    return static_cast<float>(clamped) / static_cast<float>(maxMode);
}

int NormalizedToTestInputSignalMode(float normalizedValue)
{
    const int maxMode = static_cast<int>(TestInputSignalMode::kSquareInput);
    return ClampTestInputSignalMode(static_cast<int>(
        std::round(Clamp01(normalizedValue) * static_cast<float>(maxMode))));
}

void GenerateSyntheticTestInput(int            mode,
                                float          level,
                                float          frequencyHz,
                                double         sampleRate,
                                float*         phase,
                                std::uint32_t* noiseState,
                                bool*          impulseRequested,
                                float*         destination,
                                int            numSamples)
{
    if(destination == nullptr || numSamples <= 0)
    {
        return;
    }

    std::fill(destination, destination + numSamples, 0.0f);

    const float  clampedLevel = Clamp01(level);
    const double safeRate     = sampleRate > 1.0 ? sampleRate : 48000.0;
    const double increment    = std::max(0.0f, frequencyHz) / safeRate;
    float        localPhase   = phase != nullptr ? *phase : 0.0f;

    switch(static_cast<TestInputSignalMode>(ClampTestInputSignalMode(mode)))
    {
        case TestInputSignalMode::kHostInput: break;

        case TestInputSignalMode::kSineInput:
        case TestInputSignalMode::kTriangleInput:
        case TestInputSignalMode::kSquareInput:
        case TestInputSignalMode::kSawInput:
            for(int sample = 0; sample < numSamples; ++sample)
            {
                BasicWaveform waveform = BasicWaveform::kSaw;
                switch(static_cast<TestInputSignalMode>(
                    ClampTestInputSignalMode(mode)))
                {
                    case TestInputSignalMode::kSineInput:
                        waveform = BasicWaveform::kSine;
                        break;
                    case TestInputSignalMode::kTriangleInput:
                        waveform = BasicWaveform::kTriangle;
                        break;
                    case TestInputSignalMode::kSquareInput:
                        waveform = BasicWaveform::kSquare;
                        break;
                    case TestInputSignalMode::kSawInput:
                    default: waveform = BasicWaveform::kSaw; break;
                }

                destination[sample]
                    = GenerateBasicWaveformSample(waveform, localPhase)
                      * clampedLevel;
                localPhase += static_cast<float>(increment);
                if(localPhase >= 1.0f)
                {
                    localPhase -= std::floor(localPhase);
                }
            }
            break;

        case TestInputSignalMode::kNoiseInput:
            for(int sample = 0; sample < numSamples; ++sample)
            {
                const float normalized = static_cast<float>(
                    (NextNoise(noiseState) >> 8) / static_cast<double>(0x00FFFFFFu));
                destination[sample]
                    = ((normalized * 2.0f) - 1.0f) * clampedLevel;
            }
            break;

        case TestInputSignalMode::kImpulseInput:
            if(impulseRequested != nullptr && *impulseRequested)
            {
                destination[0]      = clampedLevel;
                *impulseRequested   = false;
            }
            break;
    }

    if(phase != nullptr)
    {
        *phase = localPhase;
    }
}
} // namespace daisyhost
