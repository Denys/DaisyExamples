#pragma once

#include <cstdint>

namespace daisyhost
{
enum class TestInputSignalMode
{
    kHostInput = 0,
    kSineInput,
    kSawInput,
    kNoiseInput,
    kImpulseInput,
    kTriangleInput,
    kSquareInput,
};

int         ClampTestInputSignalMode(int mode);
const char* GetTestInputSignalModeName(int mode);
float       TestInputSignalModeToNormalized(int mode);
int         NormalizedToTestInputSignalMode(float normalizedValue);

void GenerateSyntheticTestInput(int            mode,
                                float          level,
                                float          frequencyHz,
                                double         sampleRate,
                                float*         phase,
                                std::uint32_t* noiseState,
                                bool*          impulseRequested,
                                float*         destination,
                                int            numSamples);
} // namespace daisyhost
