#include "daisy_field.h"
#include "daisysp.h"
#include "voice.h"
#include "ui_handler.h"
#include "midi_handler.h"
#include "wavetables.h"

// Wavetable Synth with Morphing for Daisy Field
// Based on implementation plan in wavetable_synth_morph_plan.md

using namespace daisy;
using namespace daisysp;
using namespace synth;

DaisyField       hw;
Voice            voice;
UiHandler        ui;
SynthMidiHandler midi;

// Audio callback for real-time processing (keep lightweight!)
void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    // Generate audio only - no I/O processing here
    for(size_t i = 0; i < size; i++)
    {
        float sample = voice.Process();
        out[0][i]    = sample; // Left channel
        out[1][i]    = sample; // Right channel (mono for now)
    }
}

int main(void)
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
    voice.Init(48000.0f, default_bank, NUM_WAVETABLES_PER_BANK, WAVETABLE_SIZE);

    // Initialize UI and MIDI handlers
    ui.Init(&hw);
    ui.SetVoice(&voice);

    midi.Init(&hw);
    midi.SetVoice(&voice);

    // Start audio processing
    hw.StartAdc();
    hw.StartAudio(AudioCallback);

    // Start MIDI receive
    hw.midi.StartReceive();

    // Main loop - process controls and MIDI here (not in audio callback)
    while(1)
    {
        // Process hardware controls (knobs, keys, switches)
        ui.ProcessControls();

        // Process incoming MIDI messages
        midi.ProcessMidi();

        // Update OLED display
        ui.UpdateDisplay();

        // Small delay to prevent busy-looping (controls update at ~100Hz)
        System::Delay(10);
    }
}