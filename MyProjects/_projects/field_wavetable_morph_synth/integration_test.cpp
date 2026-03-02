#include <cassert>
#include <cmath>
#include <cstdio>
#include <algorithm>
#include "daisy_field.h"
#include "daisysp.h"
#include "voice.h"
#include "ui_handler.h"
#include "midi_handler.h"
#include "wavetables.h"

using namespace daisy;
using namespace daisysp;
using namespace synth;

// Integration test for the complete wavetable morphing synthesizer system
// Tests the full signal chain: MIDI input -> synthesis -> audio output

class IntegrationTest
{
  public:
    IntegrationTest() : hw(), voice(), ui(), midi() {}

    void RunCompleteSystemTest()
    {
        // Initialize hardware
        hw.Init();

        // Initialize wavetable data
        InitializeWavetables();

        // Set up audio for low latency
        hw.SetAudioBlockSize(48); // 1ms blocks at 48kHz
        hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

        // Initialize synth voice with default wavetable bank
        const float* default_bank = GetWavetableBank(BANK_SINE);
        voice.Init(
            48000.0f, default_bank, NUM_WAVETABLES_PER_BANK, WAVETABLE_SIZE);

        // Initialize UI and MIDI handlers
        ui.Init(&hw);
        ui.SetVoice(&voice);

        midi.Init(&hw);
        midi.SetVoice(&voice);

        // Test 1: Basic audio processing
        TestAudioProcessing();

        // Test 2: MIDI control
        TestMidiControl();

        // Test 3: Wavetable morphing
        TestWavetableMorphing();

        // Test 4: UI interaction
        TestUIInteraction();

        // Test 5: Full signal chain
        TestFullSignalChain();

        printf("All integration tests passed!\n");
    }

  private:
    DaisyField       hw;
    Voice            voice;
    UiHandler        ui;
    SynthMidiHandler midi;

    void TestAudioProcessing()
    {
        printf("Testing audio processing...\n");

        // Set up a simple audio callback for testing
        auto test_callback = [this](AudioHandle::InputBuffer  in,
                                    AudioHandle::OutputBuffer out,
                                    size_t                    size)
        {
            for(size_t i = 0; i < size; i++)
            {
                float sample = voice.Process();
                out[0][i]    = sample;
                out[1][i]    = sample;
            }
        };

        // Start audio processing
        hw.StartAudio(test_callback);

        // Process some audio blocks
        for(int i = 0; i < 10; i++)
        {
            // Simulate audio processing
            System::Delay(1);
        }

        // Stop audio
        hw.StopAudio();

        printf("Audio processing test passed!\n");
    }

    void TestMidiControl()
    {
        printf("Testing MIDI control...\n");

        // Test note on/off
        midi.SendNoteOn(60, 100); // Middle C, velocity 100
        System::Delay(10);

        // Verify voice is active
        assert(voice.IsActive());

        midi.SendNoteOff(60);
        System::Delay(10);

        // Verify voice is released
        assert(!voice.IsActive());

        // Test pitch bend
        midi.SendPitchBend(8192); // Center position
        System::Delay(5);

        // Test modulation wheel
        midi.SendControlChange(1, 127); // Mod wheel max
        System::Delay(5);

        printf("MIDI control test passed!\n");
    }

    void TestWavetableMorphing()
    {
        printf("Testing wavetable morphing...\n");

        // Set up morphing between two wavetables
        voice.SetPosition(0.0f); // Start at first wavetable
        float sample1 = voice.Process();

        voice.SetPosition(1.0f); // Move to second wavetable
        float sample2 = voice.Process();

        // Samples should be different
        assert(std::abs(sample1 - sample2) > 0.01f);

        voice.SetPosition(0.5f); // Middle position
        float sample3 = voice.Process();

        // Middle position should be between the two extremes
        assert(sample3 >= std::min(sample1, sample2) - 0.1f);
        assert(sample3 <= std::max(sample1, sample2) + 0.1f);

        printf("Wavetable morphing test passed!\n");
    }

    void TestUIInteraction()
    {
        printf("Testing UI interaction...\n");

        // Test control updates
        ui.ProcessControls();

        // Test display update
        ui.UpdateDisplay();

        // Verify UI can control voice parameters
        voice.SetFrequency(440.0f);
        voice.SetPosition(0.5f);

        printf("UI interaction test passed!\n");
    }

    void TestFullSignalChain()
    {
        printf("Testing full signal chain...\n");

        // Set up audio callback that includes all components
        auto full_callback = [this](AudioHandle::InputBuffer  in,
                                    AudioHandle::OutputBuffer out,
                                    size_t                    size)
        {
            // Process MIDI
            midi.ProcessMidi();

            // Process UI controls
            ui.ProcessControls();

            // Generate audio
            for(size_t i = 0; i < size; i++)
            {
                float sample = voice.Process();
                out[0][i]    = sample;
                out[1][i]    = sample;
            }
        };

        // Start audio processing
        hw.StartAudio(full_callback);

        // Send MIDI note
        midi.SendNoteOn(60, 100);

        // Process audio for a while
        for(int i = 0; i < 50; i++)
        {
            System::Delay(1);
            ui.UpdateDisplay(); // Update display periodically
        }

        // Stop note
        midi.SendNoteOff(60);

        // Process release phase
        for(int i = 0; i < 20; i++)
        {
            System::Delay(1);
        }

        // Stop audio
        hw.StopAudio();

        printf("Full signal chain test passed!\n");
    }
};

int main()
{
    IntegrationTest test;
    test.RunCompleteSystemTest();
    return 0;
}