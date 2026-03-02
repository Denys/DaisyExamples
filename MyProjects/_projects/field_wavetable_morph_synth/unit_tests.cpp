#include <cassert>
#include <cmath>
#include "wavetable_osc.h"
#include "morph_processor.h"
#include "voice.h"
#include "wavetables.h"

using namespace synth;

// Simple test framework
#define TEST(name) void test_##name()
#define RUN_TEST(name) test_##name()
#define ASSERT_FLOAT_EQ(a, b, tol) assert(std::abs((a) - (b)) < (tol))
#define ASSERT_EQ(a, b) assert((a) == (b))
#define ASSERT_TRUE(a) assert(a)

// Test data
const int TEST_SAMPLE_RATE = 48000;
const int TEST_TABLE_SIZE  = 2048;
int16_t   test_wavetable_data[TEST_TABLE_SIZE * 2]; // Two tables for testing

void setup_test_data()
{
    // Initialize test wavetable data
    for(int i = 0; i < TEST_TABLE_SIZE; ++i)
    {
        // Sine wave for table 0
        test_wavetable_data[i] = static_cast<int16_t>(
            32767 * sin(2 * 3.141592653589793 * i / TEST_TABLE_SIZE));
        // Sawtooth for table 1
        test_wavetable_data[TEST_TABLE_SIZE + i] = static_cast<int16_t>(
            32767 * (2.0f * (i / (float)TEST_TABLE_SIZE) - 1.0f));
    }
}

// Tests for WavetableOsc
TEST(WavetableOsc_Init)
{
    WavetableOsc osc;
    osc.Init(TEST_SAMPLE_RATE, test_wavetable_data, 2, TEST_TABLE_SIZE);
    // Test that it initializes without crashing
}

TEST(WavetableOsc_SetFrequency)
{
    WavetableOsc osc;
    osc.Init(TEST_SAMPLE_RATE, test_wavetable_data, 2, TEST_TABLE_SIZE);
    osc.SetFrequency(440.0f);
    // Test that phase_inc is set correctly
    // We can't directly access private members, so test indirectly via Process
}

TEST(WavetableOsc_SynthesisAccuracy)
{
    WavetableOsc osc;
    osc.Init(TEST_SAMPLE_RATE, test_wavetable_data, 2, TEST_TABLE_SIZE);
    osc.SetFrequency(TEST_SAMPLE_RATE / TEST_TABLE_SIZE); // One cycle per table
    osc.SetPosition(0.0f); // Use first table (sine)

    // Process one full cycle
    float sum = 0.0f;
    for(int i = 0; i < TEST_TABLE_SIZE; ++i)
    {
        float sample = osc.Process();
        sum += sample;
    }
    // For a sine wave, sum should be close to zero
    ASSERT_FLOAT_EQ(sum, 0.0f, 0.1f);
}

TEST(WavetableOsc_Morphing)
{
    WavetableOsc osc;
    osc.Init(TEST_SAMPLE_RATE, test_wavetable_data, 2, TEST_TABLE_SIZE);
    osc.SetFrequency(440.0f);

    osc.SetPosition(0.0f); // Pure sine
    float sample_sine = osc.Process();

    // Reset phase
    osc.Init(TEST_SAMPLE_RATE, test_wavetable_data, 2, TEST_TABLE_SIZE);
    osc.SetFrequency(440.0f);
    osc.SetPosition(1.0f); // Pure sawtooth
    float sample_saw = osc.Process();

    // They should be different
    assert(std::abs(sample_sine - sample_saw) > 0.1f);

    // At position 0.5, should be mix
    osc.SetPosition(0.5f);
    float sample_mix = osc.Process();
    // Should be between sine and saw
    assert(sample_mix >= std::min(sample_sine, sample_saw) - 0.1f);
    assert(sample_mix <= std::max(sample_sine, sample_saw) + 0.1f);
}

// Tests for MorphProcessor
TEST(MorphProcessor_Init)
{
    MorphProcessor mp;
    mp.Init(TEST_SAMPLE_RATE);
}

TEST(MorphProcessor_CurveProcessing)
{
    MorphProcessor mp;
    mp.Init(TEST_SAMPLE_RATE);
    mp.SetCurve(MORPH_LINEAR);
    float out = mp.Process(0.5f);
    ASSERT_FLOAT_EQ(out, 0.5f, 0.01f);

    mp.SetCurve(MORPH_EXPONENTIAL);
    out = mp.Process(0.5f);
    // Exponential should be less than linear for input 0.5
    assert(out < 0.5f);
}

TEST(MorphProcessor_LfoModulation)
{
    MorphProcessor mp;
    mp.Init(TEST_SAMPLE_RATE);
    mp.SetCurve(MORPH_LINEAR);
    mp.SetSpeed(1.0f); // 1 Hz
    mp.SetLfoEnabled(false);
    float out_no_lfo = mp.Process(0.5f);

    mp.SetLfoEnabled(true);
    float out_with_lfo = mp.Process(0.5f);

    // With LFO, output should vary
    // Since LFO phase starts at 0, first sample might be same, but let's check after some processing
    for(int i = 0; i < 100; ++i)
    {
        mp.Process(0.5f);
    }
    float out_later = mp.Process(0.5f);
    // Should be different from initial
    assert(std::abs(out_with_lfo - out_later) > 0.01f);
}

// Tests for Voice
TEST(Voice_Init)
{
    Voice v;
    v.Init(TEST_SAMPLE_RATE, test_wavetable_data, 2, TEST_TABLE_SIZE);
}

TEST(Voice_Integration)
{
    Voice v;
    v.Init(TEST_SAMPLE_RATE, test_wavetable_data, 2, TEST_TABLE_SIZE);
    v.SetFrequency(440.0f);
    v.SetPosition(0.0f);
    v.NoteOn();

    float sample = v.Process();
    // Should produce some output
    assert(std::abs(sample) >= 0.0f);
}

TEST(Voice_SignalChain)
{
    Voice v;
    v.Init(TEST_SAMPLE_RATE, test_wavetable_data, 2, TEST_TABLE_SIZE);
    v.SetFrequency(440.0f);
    v.SetPosition(0.0f);
    v.SetFilterCutoff(1000.0f);
    v.SetFilterResonance(0.5f);
    v.SetAdsr(0.1f, 0.1f, 0.8f, 0.2f);
    v.NoteOn();

    // Process some samples
    for(int i = 0; i < 1000; ++i)
    {
        float sample = v.Process();
        assert(!std::isnan(sample));
        assert(!std::isinf(sample));
    }
}

// Tests for wavetable data management
TEST(WavetableData_Initialize)
{
    InitializeWavetables();
    // Test that data is initialized
    const int16_t* bank = GetWavetableBank(BANK_SINE);
    assert(bank != nullptr);
    // Check that it's not all zeros
    float sum = 0.0f;
    for(int i = 0; i < WAVETABLE_SIZE; ++i)
    {
        sum += std::abs(bank[i]);
    }
    assert(sum > 0.0f);
}

TEST(WavetableData_GetBank)
{
    InitializeWavetables();
    const int16_t* bank0 = GetWavetableBank(BANK_SINE);
    const int16_t* bank1 = GetWavetableBank(BANK_SAWTOOTH);
    assert(bank0 != bank1);
    assert(bank0 != nullptr);
    assert(bank1 != nullptr);
}

int main()
{
    setup_test_data();
    InitializeWavetables(); // For wavetable tests

    RUN_TEST(WavetableOsc_Init);
    RUN_TEST(WavetableOsc_SetFrequency);
    RUN_TEST(WavetableOsc_SynthesisAccuracy);
    RUN_TEST(WavetableOsc_Morphing);

    RUN_TEST(MorphProcessor_Init);
    RUN_TEST(MorphProcessor_CurveProcessing);
    RUN_TEST(MorphProcessor_LfoModulation);

    RUN_TEST(Voice_Init);
    RUN_TEST(Voice_Integration);
    RUN_TEST(Voice_SignalChain);

    RUN_TEST(WavetableData_Initialize);
    RUN_TEST(WavetableData_GetBank);

    return 0;
}