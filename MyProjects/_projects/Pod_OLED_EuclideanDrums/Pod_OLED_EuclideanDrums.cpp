// ============================================================
// Pod_OLED_EuclideanDrums.cpp
//
// 2-voice euclidean drum machine for Daisy Pod
// with SSD1306 OLED display + external encoder + CON/BAK buttons.
//
// Upgraded from DaisyExamples/pod/EuclideanDrums with:
//   - OLED sequencer visualization (dot-grid, live step cursor)
//   - Hierarchical menu for all parameters
//   - Pot zoom overlay on parameter changes
//   - Tap tempo (Button 2, rolling avg of 4 taps)
//   - Drum select toggle (Button 1, LED2 green/blue)
//   - Beat-synced LED1 flash
//   - All controls in main loop (not audio ISR)
//   - Non-interleaved audio callback
//
// Hardware:
//   Daisy Pod (old DIP-socket rev) + Daisy Seed Rev7
//   SSD1306 128x64 I2C OLED (SCL=D11, SDA=D12)
//   External EC11 encoder (D7/D8/D9) + CON (D10) + BAK (D22)
//
// Build:  make
// Flash:  make program  (ST-Link)
// ============================================================

#include "daisy_pod.h"
#include "daisysp.h"
#include "DrumSeqUI.h"
#include "DrumControls.h"

using namespace daisy;
using namespace daisysp;

// ---- Global objects -----------------------------------------
static DaisyPod     pod;
static DrumSeqUI    ui;
static DrumControls ctl;

// ---- DSP modules --------------------------------------------
static Oscillator osc;
static WhiteNoise noise;
static AdEnv      kickVolEnv, kickPitchEnv, snareEnv;
static Metro      tick;

// ---- Shared volatile state ----------------------------------
// Written by main loop (patterns, params), read by audio ISR.
// Step counters written by audio ISR, read by main (display).
static volatile bool    kickSeq[32];
static volatile bool    snareSeq[32];
static volatile uint8_t kickStep  = 0;
static volatile uint8_t snareStep = 0;
static volatile bool    tick_flag = false;
static AudioParams      ap;

// ---- DSP init -----------------------------------------------
void SetupDrums(float samplerate)
{
    osc.Init(samplerate);
    osc.SetWaveform(Oscillator::WAVE_TRI);
    osc.SetAmp(1);

    noise.Init();

    snareEnv.Init(samplerate);
    snareEnv.SetTime(ADENV_SEG_ATTACK, .01f);
    snareEnv.SetTime(ADENV_SEG_DECAY, .2f);
    snareEnv.SetMax(1);
    snareEnv.SetMin(0);

    kickPitchEnv.Init(samplerate);
    kickPitchEnv.SetTime(ADENV_SEG_ATTACK, .01f);
    kickPitchEnv.SetTime(ADENV_SEG_DECAY, .05f);
    kickPitchEnv.SetMax(400);
    kickPitchEnv.SetMin(50);

    kickVolEnv.Init(samplerate);
    kickVolEnv.SetTime(ADENV_SEG_ATTACK, .01f);
    kickVolEnv.SetTime(ADENV_SEG_DECAY, 1.f);
    kickVolEnv.SetMax(1);
    kickVolEnv.SetMin(0);
}

// ---- Audio callback -----------------------------------------
// Non-interleaved. Reads volatile AudioParams + sequences.
// RULES: no I2C, no GPIO reads, no dynamic allocation, no printf.

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    // Snapshot volatile params (atomic single-word reads on CM7)
    float tempo_hz      = ap.tempo_hz;
    float kick_decay    = ap.kick_decay;
    float kick_pitch_hi = ap.kick_pitch_max;
    float snare_decay   = ap.snare_decay;
    float volume        = ap.volume;
    float mix           = ap.mix;
    uint8_t kl          = ap.kick_length;
    uint8_t sl          = ap.snare_length;
    if(kl < 1) kl = 1;
    if(sl < 1) sl = 1;

    // Per-block parameter updates
    tick.SetFreq(tempo_hz);
    kickVolEnv.SetTime(ADENV_SEG_DECAY, kick_decay);
    kickPitchEnv.SetMax(kick_pitch_hi);
    snareEnv.SetTime(ADENV_SEG_DECAY, snare_decay);

    for(size_t i = 0; i < size; i++)
    {
        // Metro tick → advance step counters, trigger envelopes
        if(tick.Process())
        {
            kickStep++;
            snareStep++;
            if(kickStep >= kl)  kickStep  = 0;
            if(snareStep >= sl) snareStep = 0;

            if(kickSeq[kickStep]) {
                kickVolEnv.Trigger();
                kickPitchEnv.Trigger();
            }
            if(snareSeq[snareStep]) {
                snareEnv.Trigger();
            }
            tick_flag = true;
        }

        // DSP: kick = triangle osc with pitch sweep + volume envelope
        float kck_env = kickVolEnv.Process();
        osc.SetFreq(kickPitchEnv.Process());
        osc.SetAmp(kck_env);
        float kick_out = osc.Process();

        // DSP: snare = white noise with volume envelope
        float snr_env   = snareEnv.Process();
        float snare_out = noise.Process() * snr_env;

        // Mix: 0=all kick, 1=all snare, 0.5=equal
        float sig = (1.f - mix) * kick_out + mix * snare_out;
        sig *= volume;

        out[0][i] = sig;
        out[1][i] = sig;
    }
}

// ---- Main ---------------------------------------------------
int main()
{
    // Hardware init
    pod.Init();
    pod.SetAudioBlockSize(4);
    pod.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
    float samplerate  = pod.AudioSampleRate();
    float callbackrate = pod.AudioCallbackRate();

    // DSP init
    SetupDrums(samplerate);
    tick.Init(8.f, callbackrate);  // 120 BPM default → 8 Hz metro

    // Clear sequences
    for(int i = 0; i < 32; i++) {
        kickSeq[i]  = false;
        snareSeq[i] = false;
    }

    // Init AudioParams with defaults matching DrumSeqUI defaults
    ap.tempo_hz      = 120.f * 4.f / 60.f;  // 8.0 Hz
    ap.kick_decay    = 0.5f;                  // V_KICK_DECAY=50 → 500ms
    ap.kick_pitch_max = 400.f;
    ap.snare_decay   = 0.2f;                  // V_SNARE_DECAY=20 → 200ms
    ap.volume        = 0.8f;                  // V_VOLUME=80 → 80%
    ap.mix           = 0.5f;                  // V_MIX=50 → equal
    ap.kick_length   = 16;
    ap.snare_length  = 16;

    // Init UI and controls (order: ui first, then ctl)
    ui.Init();
    ctl.Init(pod, ui, kickSeq, snareSeq, kickStep, snareStep, tick_flag, ap);

    // Start audio before ADC
    pod.StartAdc();
    pod.StartAudio(AudioCallback);

    uint32_t last_draw = 0;

    while(true)
    {
        // Poll all controls, navigate menu, drive LEDs, rebuild patterns
        ctl.Poll();

        // Draw OLED at ~30 fps
        uint32_t now = System::GetNow();
        if(now - last_draw >= 33) {
            last_draw = now;
            ui.Draw(kickSeq, snareSeq, kickStep, snareStep);
        }
    }
}
