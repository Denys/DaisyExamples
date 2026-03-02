#include "daisy_pod.h"
#include "daisysp.h"
#include "modal_voice.h"

using namespace daisy;
using namespace daisysp;

DaisyPod hw;

// DSP Modules
Svf        tone_filter;
ReverbSc   reverb;
ModalSynth synth;

// Encoder mode (0-3)
int mode = 0;

// Fuzz parameters (Mode 0)
float drive_amount   = 1.0f;
float tone_cutoff    = 2000.0f;
bool  hard_clip_mode = true;

// Reverb parameters (Modes 1 & 2)
float rev_feedback = 0.85f;
float rev_lpfreq   = 10000.0f;
float rev_mix      = 0.5f;

// Synth state
bool synth_enabled = true;

// Clipping constants
static const float HARD_CLIP_THRESH = 0.8f;
static const float SOFT_CLIP_DRIVE  = 3.0f;
static const float SOFT_CLIP_NORM   = 1.0f / tanhf(3.0f);

// Number of encoder modes
static const int NUM_MODES = 4;

void UpdateEncoder()
{
    mode += hw.encoder.Increment();
    mode = (mode % NUM_MODES + NUM_MODES) % NUM_MODES;

    // Encoder push: RESERVED for future sequencer implementation
    // if(hw.encoder.RisingEdge()) { }
}

void UpdateKnobs()
{
    float k1 = hw.knob1.Process();
    float k2 = hw.knob2.Process();

    switch(mode)
    {
        case 0: // Fuzz: Drive + Tone
            drive_amount = 1.0f + k1 * 19.0f;
            tone_cutoff  = 200.0f * powf(60.0f, k2);
            tone_filter.SetFreq(tone_cutoff);
            break;

        case 1: // Reverb: Feedback + LP Freq
            rev_feedback = 0.6f + k1 * 0.399f;
            rev_lpfreq   = 500.0f * powf(36.0f, k2); // 500 Hz - 18 kHz (log)
            reverb.SetFeedback(rev_feedback);
            reverb.SetLpFreq(rev_lpfreq);
            break;

        case 2: // Mix: Reverb Mix + Voice Damping
            rev_mix = k1;
            synth.SetParam(2, k2); // Damping: 0-1
            break;

        case 3: // Synth: Structure + Brightness
            synth.SetParam(0, k1); // Structure: 0-1
            synth.SetParam(1, k2); // Brightness: 0-1
            break;
    }
}

void UpdateButtons()
{
    if(hw.button1.RisingEdge())
    {
        hard_clip_mode = !hard_clip_mode;
    }

    if(hw.button2.RisingEdge())
    {
        synth_enabled = !synth_enabled;
    }
}

void UpdateLeds()
{
    switch(mode)
    {
        case 0: // Fuzz mode
            hw.led1.Set((drive_amount - 1.0f) / 19.0f, 0.0f, 0.0f);
            if(hard_clip_mode)
                hw.led2.Set(1.0f, 0.0f, 0.0f);
            else
                hw.led2.Set(0.0f, 1.0f, 0.0f);
            break;

        case 1: // Reverb
            hw.led1.Set(1.0f, 0.6f, 0.0f); // Yellow
            hw.led2.Set(0.0f, 0.0f, 0.0f);
            break;

        case 2: // Mix
            hw.led1.Set(0.0f, 0.0f, 0.0f);
            hw.led2.Set(0.0f, 1.0f, 1.0f); // Cyan
            break;

        case 3: // Synth
        {
            hw.led1.Set(0.5f, 0.0f, 1.0f); // Purple
            float activity = synth.GetActiveCount() / 4.0f;
            if(synth_enabled)
                hw.led2.Set(0.0f, activity, activity); // Cyan = voices
            else
                hw.led2.Set(0.2f, 0.0f, 0.0f); // Dim red = disabled
            break;
        }
    }
}

void HandleMidiMessage(MidiEvent m)
{
    switch(m.type)
    {
        case NoteOn:
        {
            NoteOnEvent p = m.AsNoteOn();
            synth.NoteOn(p.note, p.velocity);
        }
        break;
        case NoteOff:
        {
            synth.NoteOff(m.data[0]);
        }
        break;
        default: break;
    }
}

static void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                           AudioHandle::InterleavingOutputBuffer out,
                           size_t                                size)
{
    hw.ProcessAllControls();

    UpdateEncoder();
    UpdateKnobs();
    UpdateButtons();
    UpdateLeds();

    for(size_t i = 0; i < size; i += 2)
    {
        // 1. Synth voice (always process to let resonators decay)
        float voice_sig = synth.Process();
        if(!synth_enabled)
            voice_sig = 0.0f;

        // 2. Audio input (mono sum)
        float input_sig = (in[i] + in[i + 1]) * 0.5f;

        // 3. Mix synth + input
        float sig = input_sig + voice_sig;

        // 4. Drive stage
        sig *= drive_amount;

        // 5. Clipping stage
        if(hard_clip_mode)
        {
            sig = fminf(fmaxf(sig, -HARD_CLIP_THRESH), HARD_CLIP_THRESH);
        }
        else
        {
            sig = tanhf(sig * SOFT_CLIP_DRIVE) * SOFT_CLIP_NORM;
        }

        // 6. Tone filter (SVF lowpass)
        tone_filter.Process(sig);
        sig = tone_filter.Low();

        // 7. ReverbSc (stereo output with dry/wet mix)
        float wet_l, wet_r;
        reverb.Process(sig, sig, &wet_l, &wet_r);
        float dry = sig;
        out[i]     = dry * (1.0f - rev_mix) + wet_l * rev_mix;
        out[i + 1] = dry * (1.0f - rev_mix) + wet_r * rev_mix;
    }
}

int main(void)
{
    hw.Init();
    hw.SetAudioBlockSize(4);
    float sr = hw.AudioSampleRate();

    // Initialize tone filter
    tone_filter.Init(sr);
    tone_filter.SetFreq(2000.0f);
    tone_filter.SetRes(0.1f);
    tone_filter.SetDrive(0.0f);

    // Initialize ReverbSc
    reverb.Init(sr);
    reverb.SetFeedback(0.85f);
    reverb.SetLpFreq(10000.0f);

    // Initialize synth voice (Modal - swap here for different voices)
    synth.Init(sr);

    // Start audio processing
    hw.StartAdc();
    hw.StartAudio(AudioCallback);

    // Start MIDI (TRS jack)
    hw.midi.StartReceive();

    for(;;)
    {
        hw.midi.Listen();
        while(hw.midi.HasEvents())
        {
            HandleMidiMessage(hw.midi.PopEvent());
        }
        hw.UpdateLeds();
    }
}
