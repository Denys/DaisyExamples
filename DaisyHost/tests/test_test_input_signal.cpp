#include <array>
#include <cstdint>

#include <gtest/gtest.h>

#include "daisyhost/TestInputSignal.h"

namespace
{
TEST(TestInputSignalTest, ClampsModesAndNamesSawInput)
{
    EXPECT_EQ(daisyhost::ClampTestInputSignalMode(-1),
              static_cast<int>(daisyhost::TestInputSignalMode::kHostInput));
    EXPECT_EQ(daisyhost::ClampTestInputSignalMode(99),
              static_cast<int>(daisyhost::TestInputSignalMode::kSquareInput));
    EXPECT_STREQ(
        daisyhost::GetTestInputSignalModeName(
            static_cast<int>(daisyhost::TestInputSignalMode::kTriangleInput)),
        "Triangle");
    EXPECT_STREQ(
        daisyhost::GetTestInputSignalModeName(
            static_cast<int>(daisyhost::TestInputSignalMode::kSquareInput)),
        "Square");
    EXPECT_STREQ(
        daisyhost::GetTestInputSignalModeName(
            static_cast<int>(daisyhost::TestInputSignalMode::kSawInput)),
        "Saw");
}

TEST(TestInputSignalTest, GeneratesDeterministicSawWave)
{
    float         phase        = 0.0f;
    std::uint32_t noiseState   = 1u;
    bool          impulseState = false;
    std::array<float, 4> output = {{0.0f, 0.0f, 0.0f, 0.0f}};

    daisyhost::GenerateSyntheticTestInput(
        static_cast<int>(daisyhost::TestInputSignalMode::kSawInput),
        1.0f,
        880.0f,
        3520.0,
        &phase,
        &noiseState,
        &impulseState,
        output.data(),
        static_cast<int>(output.size()));

    EXPECT_NEAR(output[0], -1.0f, 0.0001f);
    EXPECT_NEAR(output[1], -0.5f, 0.0001f);
    EXPECT_NEAR(output[2], 0.0f, 0.0001f);
    EXPECT_NEAR(output[3], 0.5f, 0.0001f);
    EXPECT_NEAR(phase, 0.0f, 0.0001f);
}

TEST(TestInputSignalTest, GeneratesDeterministicTriangleWave)
{
    float          phase        = 0.0f;
    std::uint32_t  noiseState   = 1u;
    bool           impulseState = false;
    std::array<float, 4> output = {{0.0f, 0.0f, 0.0f, 0.0f}};

    daisyhost::GenerateSyntheticTestInput(
        static_cast<int>(daisyhost::TestInputSignalMode::kTriangleInput),
        1.0f,
        880.0f,
        3520.0,
        &phase,
        &noiseState,
        &impulseState,
        output.data(),
        static_cast<int>(output.size()));

    EXPECT_NEAR(output[0], -1.0f, 0.0001f);
    EXPECT_NEAR(output[1], 0.0f, 0.0001f);
    EXPECT_NEAR(output[2], 1.0f, 0.0001f);
    EXPECT_NEAR(output[3], 0.0f, 0.0001f);
}
} // namespace
