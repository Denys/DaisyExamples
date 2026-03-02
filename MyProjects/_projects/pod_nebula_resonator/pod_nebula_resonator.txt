#include "daisy_pod.h"
#include "daisysp.h"
#include "freeze_buffer.h"
#include "texture_voice.h"

using namespace daisy;
using namespace daisysp;

DaisyPod hw;

// ===== DSP Modules =====
BlOsc       osc;
WhiteNoise  noise;
ReverbSc    reverb;
Metro       seq_clock;

// Custom DSP
DSY_SDRAM_BSS FreezeBuffer freezeBuf;
TextureVoice               cloud;

// ===== FSM State =====
static const int NUM_PAGES = 3;
int              page      = 0;

// ===== Source Parameters (Page 0) =====
float timbre_morph  = 0.0f;  // 0-1: 0-0.5 osc shape, 0.5-1.0 noise blend
float source_decay  = 0.5f;  // Envelope decay time (normalized 0-1)
float current_freq  = 110.0f;
float target_freq   = 110.0f;

// ===== Granular Parameters (Page 1) =====
float texture_amount = 0.3f; // Jitter intensity 0-1
float scan_position  = 0.5f; // Manual scan 0-1

// ===== Motion Parameters (Page 2) =====
float seq_depth = 0.3f;  // Sequencer modulation amount 0-1
float rev_mix   = 0.4f;  // Reverb dry/wet 0-1

// ===== Freeze State =====
bool freeze_active = false;

// ===== Sequencer =====
float seq_steps[8]   = {0.1f, 0.2f, 0.5f, 0.1f, 0.8f, 0.9f, 0.4f, 0.2f};
int   seq_idx        = 0;
bool  seq_running     = false;
float seq_current_val = 0.0f;

// ===== Soft Takeover =====
// stored_val[page][knob], locked[page][knob]
float stored_val[NUM_PAGES][2];
bool  knob_locked[NUM_PAGES][2];
static const float TAKEOVER_THRESH = 0.05f;

// ===== Smoothed Parameters =====
float smooth_texture = 0.3f;
float smooth_scan    = 0.5f;
float smooth_rev_mix = 0.4f;

// ===== LED Beat Pulse =====
uint32_t beat_led_timer  = 0;
bool     beat_led_active = false;

// ---------------------------------------------------------
// Soft Takeover Logic
// ---------------------------------------------------------
float ApplyTakeover(int pg, int knob_idx, float raw_knob, float current_param)
{
    if(knob_locked[pg][knob_idx])
    {
        // Check if knob has crossed the stored value
        if(fabsf(raw_knob - stored_val[pg][knob_idx]) < TAKEOVER_THRESH)
        {
            knob_locked[pg][knob_idx] = false; // Unlock
        }
        return current_param; // Don't update yet
    }
    stored_val[pg][knob_idx] = raw_knob;
    return raw_knob;
}

// Lock all knobs when changing pages
void LockKnobs()
{
    for(int p = 0; p < NUM_PAGES; p++)
    {
        for(int k = 0; k < 2; k++)
        {
            knob_locked[p][k] = true;
        }
    }
}

// ---------------------------------------------------------
// Control Handling
// ---------------------------------------------------------
void UpdateEncoder()
{
    int inc = hw.encoder.Increment();
    if(inc != 0)
    {
        LockKnobs();
        page = (page + inc);
        page = ((page % NUM_PAGES) + NUM_PAGES) % NUM_PAGES;
    }
}

void UpdateKnobs()
{
    float k1 = hw.knob1.Process();
    float k2 = hw.knob2.Process();

    switch(page)
    {
        case 0: // SOURCE: Timbre Morph + Decay
        {
            timbre_morph = ApplyTakeover(0, 0, k1, timbre_morph);
            source_decay = ApplyTakeover(0, 1, k2, source_decay);

            // Timbre Morph macro:
            // 0.0-0.5: Sweep osc waveform (Saw -> Square)
            // 0.5-1.0: Crossfade in noise
            if(timbre_morph <= 0.5f)
            {
                float morph = timbre_morph * 2.0f; // 0-1
                // BlOsc: 0=Saw, 1=Square (use PW for shape)
                osc.SetPw(morph);
            }
            break;
        }

        case 1: // GRANULAR: Texture + Scan
        {
            texture_amount = ApplyTakeover(1, 0, k1, texture_amount);
            scan_position  = ApplyTakeover(1, 1, k2, scan_position);
            break;
        }

        case 2: // MOTION: Seq Depth + Reverb Mix
        {
            seq_depth = ApplyTakeover(2, 0, k1, seq_depth);
            rev_mix   = ApplyTakeover(2, 1, k2, rev_mix);
            break;
        }
    }
}

void UpdateButtons()
{
    // Button 1: Freeze toggle
    if(hw.button1.RisingEdge())
    {
        freeze_active = !freeze_active;
        freezeBuf.SetFreeze(freeze_active);
    }

    // Button 2: Sequencer start/stop
    if(hw.button2.RisingEdge())
    {
        seq_running = !seq_running;
        if(seq_running)
            seq_idx = 0;
    }
}

void UpdateLeds()
{
    // LED 1: Page color + freeze state overlay
    float freeze_r = freeze_active ? 1.0f : 0.0f;
    float freeze_g = freeze_active ? 0.0f : 0.4f;

    switch(page)
    {
        case 0: // SOURCE: Cyan (blend with freeze indicator)
            hw.led1.Set(freeze_r, freeze_active ? 0.0f : 1.0f,
                        freeze_active ? 0.0f : 1.0f);
            break;
        case 1: // GRANULAR: Magenta
            hw.led1.Set(freeze_active ? 1.0f : 1.0f,
                        freeze_active ? 0.0f : 0.0f,
                        freeze_active ? 0.0f : 1.0f);
            break;
        case 2: // MOTION: Yellow
            hw.led1.Set(1.0f, freeze_active ? 0.0f : 0.6f, 0.0f);
            break;
    }

    // LED 2: Sequencer pulse
    if(seq_running && beat_led_active)
        hw.led2.Set(0.0f, 0.8f, 0.8f); // Cyan pulse on beat
    else if(seq_running)
        hw.led2.Set(0.0f, 0.1f, 0.1f); // Dim between beats
    else
        hw.led2.Set(freeze_r * 0.3f, freeze_g * 0.3f, 0.0f); // Freeze echo
}

// ---------------------------------------------------------
// MIDI Handler
// ---------------------------------------------------------
void HandleMidiMessage(MidiEvent m)
{
    switch(m.type)
    {
        case NoteOn:
        {
            NoteOnEvent p = m.AsNoteOn();
            if(p.velocity > 0)
            {
                // Set oscillator frequency from MIDI note
                target_freq = mtof(p.note);
            }
            break;
        }
        case SystemRealTime:
        {
            // MIDI Clock sync (0xF8 = timing clock, 24 PPQN)
            if(m.srt_type == TimingClock && seq_running)
            {
                static int midi_tick = 0;
                midi_tick++;
                if(midi_tick >= 6) // 6 ticks = 16th note at 24 PPQN
                {
                    midi_tick = 0;
                    seq_idx   = (seq_idx + 1) % 8;
                    beat_led_active = true;
                    beat_led_timer  = 0;
                }
            }
            break;
        }
        default: break;
    }
}

// ---------------------------------------------------------
// Audio Callback
// ---------------------------------------------------------
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
        // === 1. Internal Source ===
        // Smooth frequency changes
        fonepole(current_freq, target_freq, 0.002f);
        osc.SetFreq(current_freq);

        float osc_sig   = osc.Process();
        float noise_sig = noise.Process();

        // Timbre morph: noise crossfade for morph > 0.5
        float noise_amt = 0.0f;
        if(timbre_morph > 0.5f)
            noise_amt = (timbre_morph - 0.5f) * 2.0f; // 0-1

        float source_sig
            = osc_sig * (1.0f - noise_amt) + noise_sig * noise_amt * 0.5f;

        // === 2. Write to Freeze Buffer ===
        freezeBuf.Write(source_sig);

        // === 3. Sequencer ===
        if(seq_running && seq_clock.Process())
        {
            seq_idx = (seq_idx + 1) & 7;
            beat_led_active = true;
            beat_led_timer  = 0;
        }

        // Smooth sequencer value
        float seq_target = seq_steps[seq_idx];
        fonepole(seq_current_val, seq_target, 0.001f);

        // Beat LED decay
        beat_led_timer++;
        if(beat_led_timer > 2000) // ~42ms at 48kHz
            beat_led_active = false;

        // === 4. Calculate Scan Position ===
        float final_scan = scan_position;
        if(seq_running)
        {
            final_scan += seq_current_val * seq_depth;
            // Wrap 0-1 (both addends are [0,1], so sum <= 2 — one pass suffices)
            if(final_scan > 1.0f)
                final_scan -= 1.0f;
        }

        // Smooth texture and scan
        fonepole(smooth_texture, texture_amount, 0.001f);
        fonepole(smooth_scan, final_scan, 0.001f);

        // === 5. Granular Engine ===
        float wetL, wetR;
        cloud.Process(freezeBuf, smooth_scan, smooth_texture, &wetL, &wetR);

        // === 6. Reverb ===
        fonepole(smooth_rev_mix, rev_mix, 0.001f);
        float rev_l, rev_r;
        reverb.Process(wetL, wetR, &rev_l, &rev_r);

        float dry_amt = 1.0f - smooth_rev_mix;
        out[i]     = wetL * dry_amt + rev_l * smooth_rev_mix;
        out[i + 1] = wetR * dry_amt + rev_r * smooth_rev_mix;
    }
}

// ---------------------------------------------------------
// Main
// ---------------------------------------------------------
int main(void)
{
    hw.Init();
    hw.SetAudioBlockSize(4);
    float sr = hw.AudioSampleRate();

    // Initialize internal source
    osc.Init(sr);
    osc.SetWaveform(BlOsc::WAVE_SAW);
    osc.SetFreq(110.0f);
    osc.SetAmp(0.5f);
    osc.SetPw(0.0f);
    noise.Init();

    // Initialize freeze buffer (pre-fills with sawtooth)
    freezeBuf.Init();

    // Initialize granular engine
    cloud.Init(sr);

    // Initialize sequencer clock
    seq_clock.Init(4.0f, sr); // 4 Hz internal clock

    // Initialize reverb
    reverb.Init(sr);
    reverb.SetFeedback(0.85f);
    reverb.SetLpFreq(10000.0f);

    // Initialize soft takeover stored values
    for(int p = 0; p < NUM_PAGES; p++)
    {
        for(int k = 0; k < 2; k++)
        {
            stored_val[p][k]  = 0.5f;
            knob_locked[p][k] = true;
        }
    }

    // Start audio
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
