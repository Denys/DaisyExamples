/**
 * x0x_drum_machine.cpp
 * Daisy Field — x0x Style 16-Step Drum Sequencer
 *
 * Project 6 from NotebookLM_Field_10_examples.md
 * Three synthesized drum voices (kick, snare, hi-hat) driven by a Metro clock.
 * BTN1 cycles the active edit lane; A/B keys toggle step on/off.
 *
 * Controls
 * ─────────────────────────────────────────────────────────────────────────────
 *  K1  BPM           60–180 BPM
 *  K2  Kick Freq     50–250 Hz
 *  K3  Kick Decay    0.05–0.95
 *  K4  Snare Snappy  0–1
 *  K5  Snare Decay   0.05–0.95
 *  K6  HiHat Decay   0.05–1.0
 *  K7  Kick Accent   0–1
 *  K8  Master Volume 0–1
 *  BTN1  Cycle edit lane: KICK → SNARE → HIHAT → KICK
 *  BTN2  Clear current lane's 16-step pattern
 *  A1-A8  Toggle steps 1-8  for active drum
 *  B1-B8  Toggle steps 9-16 for active drum
 *
 * Build:  make clean && make
 * Flash:  make program
 */

#include "daisy_field.h"
#include "daisysp.h"
#include <cstdio>
#include "../../foundation_examples/field_defaults.h"

using namespace daisy;
using namespace daisysp;
using namespace FieldDefaults;

// ─────────────────────────────────────────────────────────────────────────────
// Hardware
// ─────────────────────────────────────────────────────────────────────────────
DaisyField hw;

// ─────────────────────────────────────────────────────────────────────────────
// DSP Objects
// ─────────────────────────────────────────────────────────────────────────────
Metro           step_clock;
AnalogBassDrum  bass_drum;
AnalogSnareDrum snare_drum;
WhiteNoise      hh_noise;  // hi-hat: noise source
OnePole         hh_hp;     // hi-hat: 1-pole high-pass filter (metallic character)
AdEnv           hh_env;    // hi-hat: amplitude envelope

// ─────────────────────────────────────────────────────────────────────────────
// Sequencer State
// current_step: written by audio callback, read by main loop → volatile
// pattern arrays: written by main loop, read by audio callback
//   (plain bool[] — single-core ARM, no data hazard for bool writes)
// ─────────────────────────────────────────────────────────────────────────────
static constexpr int kNumSteps = 16;

bool kick_steps[kNumSteps];
bool snare_steps[kNumSteps];
bool hihat_steps[kNumSteps];

volatile int current_step = -1; // -1 → first metro tick advances to step 0

// ─────────────────────────────────────────────────────────────────────────────
// Parameters (volatile: written by main loop, read by audio callback)
// ─────────────────────────────────────────────────────────────────────────────
volatile float p_bpm          = 120.f;
volatile float p_kick_freq    = 0.05f; // 0–1; mapped to 50–250 Hz in callback
volatile float p_kick_decay   = 0.55f;
volatile float p_kick_accent  = 0.75f;
volatile float p_snare_snappy = 0.60f;
volatile float p_snare_decay  = 0.45f;
volatile float p_hh_decay     = 0.40f;
volatile float p_master_vol   = 0.80f;

// ─────────────────────────────────────────────────────────────────────────────
// UI State (main loop only — no audio callback access)
// ─────────────────────────────────────────────────────────────────────────────
enum DrumLane
{
    LANE_KICK  = 0,
    LANE_SNARE = 1,
    LANE_HIHAT = 2,
    LANE_COUNT = 3
};

DrumLane edit_lane = LANE_KICK;

static const char* kDrumNames[LANE_COUNT] = {"KICK", "SNARE", "HIHAT"};

// ─────────────────────────────────────────────────────────────────────────────
// Helper: pointer to active lane's step pattern
// ─────────────────────────────────────────────────────────────────────────────
bool* GetActivePattern()
{
    switch(edit_lane)
    {
        case LANE_SNARE: return snare_steps;
        case LANE_HIHAT: return hihat_steps;
        default:         return kick_steps;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Default preset initialisation
// ─────────────────────────────────────────────────────────────────────────────
void InitPatterns()
{
    for(int i = 0; i < kNumSteps; i++)
    {
        kick_steps[i]  = false;
        snare_steps[i] = false;
        hihat_steps[i] = false;
    }

    // 4-on-the-floor kick
    kick_steps[0] = kick_steps[4] = kick_steps[8] = kick_steps[12] = true;

    // Backbeat snare (beats 2 & 4)
    snare_steps[4] = snare_steps[12] = true;

    // 8th-note hi-hat
    hihat_steps[0]  = hihat_steps[2]  = hihat_steps[4]  = hihat_steps[6]  = true;
    hihat_steps[8]  = hihat_steps[10] = hihat_steps[12] = hihat_steps[14] = true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Audio Callback
// ─────────────────────────────────────────────────────────────────────────────
void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    // Apply parameters once per block (efficient, avoids per-sample overhead)
    step_clock.SetFreq((p_bpm / 60.f) * 4.f); // 16th-note rate

    bass_drum.SetFreq(50.f + p_kick_freq * 200.f); // 50–250 Hz
    bass_drum.SetDecay(p_kick_decay);
    bass_drum.SetAccent(p_kick_accent);

    snare_drum.SetDecay(p_snare_decay);
    snare_drum.SetSnappy(p_snare_snappy);

    hh_env.SetTime(ADENV_SEG_DECAY, p_hh_decay);

    for(size_t i = 0; i < size; i++)
    {
        bool trig_kick  = false;
        bool trig_snare = false;
        bool trig_hihat = false;

        if(step_clock.Process())
        {
            int next = current_step + 1;
            if(next >= kNumSteps)
                next = 0;

            current_step = next;
            trig_kick    = kick_steps[next];
            trig_snare   = snare_steps[next];
            trig_hihat   = hihat_steps[next];
        }

        float k = bass_drum.Process(trig_kick);
        float s = snare_drum.Process(trig_snare);
        if(trig_hihat) hh_env.Trigger();
        float h = hh_hp.Process(hh_noise.Process()) * hh_env.Process();

        float mix = (k * 0.60f + s * 0.40f + h * 0.25f) * p_master_vol;
        mix = fclamp(mix, -0.95f, 0.95f);

        out[0][i] = mix;
        out[1][i] = mix;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Knob Processing
// ─────────────────────────────────────────────────────────────────────────────
void ProcessControls()
{
    const float k1 = hw.knob[0].Process();
    const float k2 = hw.knob[1].Process();
    const float k3 = hw.knob[2].Process();
    const float k4 = hw.knob[3].Process();
    const float k5 = hw.knob[4].Process();
    const float k6 = hw.knob[5].Process();
    const float k7 = hw.knob[6].Process();
    const float k8 = hw.knob[7].Process();

    p_bpm          = 60.f + k1 * 120.f;   // 60–180 BPM
    p_kick_freq    = k2;                   // 0–1 (mapped to Hz in callback)
    p_kick_decay   = 0.05f + k3 * 0.90f;  // minimum decay prevents silence glitch
    p_snare_snappy = k4;
    p_snare_decay  = 0.05f + k5 * 0.90f;
    p_hh_decay     = 0.05f + k6 * 0.95f;
    p_kick_accent  = k7;
    p_master_vol   = k8;
}

// ─────────────────────────────────────────────────────────────────────────────
// Button + Key Events
// ─────────────────────────────────────────────────────────────────────────────
void HandleUiEvents()
{
    // BTN1: cycle active edit lane Kick → Snare → HiHat → Kick
    if(hw.sw[0].RisingEdge())
        edit_lane = static_cast<DrumLane>((edit_lane + 1) % LANE_COUNT);

    // BTN2: clear active lane's pattern
    if(hw.sw[1].RisingEdge())
    {
        bool* pat = GetActivePattern();
        for(int j = 0; j < kNumSteps; j++)
            pat[j] = false;
    }

    // A1-A8 (kKeyAIndices 0–7): toggle steps 0-7 for active drum
    // B1-B8 (kKeyBIndices 8–15): toggle steps 8-15 for active drum
    bool* pat = GetActivePattern();
    for(int i = 0; i < 8; i++)
    {
        if(hw.KeyboardRisingEdge(kKeyAIndices[i]))
            pat[i] = !pat[i];

        if(hw.KeyboardRisingEdge(kKeyBIndices[i]))
            pat[i + 8] = !pat[i + 8];
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// LED Update
// Active drum's step pattern + playhead indicator + knob LEDs
// Uses kLedKeysA/kLedKeysB from field_defaults.h (correct reversed A-row mapping)
// ─────────────────────────────────────────────────────────────────────────────
void UpdateLeds()
{
    const bool* pat  = GetActivePattern();
    const int   step = current_step;

    for(int i = 0; i < 8; i++)
    {
        // A-row: steps 0-7  (dim = off, mid = on, bright = playhead)
        float a_br = pat[i] ? 0.45f : 0.02f;
        if(step == i)
            a_br = 1.0f;
        hw.led_driver.SetLed(kLedKeysA[i], a_br);

        // B-row: steps 8-15
        float b_br = pat[i + 8] ? 0.45f : 0.02f;
        if(step == i + 8)
            b_br = 1.0f;
        hw.led_driver.SetLed(kLedKeysB[i], b_br);
    }

    // Knob LEDs mirror knob positions
    for(int i = 0; i < 8; i++)
        hw.led_driver.SetLed(kLedKnobs[i], hw.knob[i].Value());

    // Switch LED 1: brightness indicates active drum (bright=Kick, mid=Snare, dim=HiHat)
    const float sw_brightness[LANE_COUNT] = {1.0f, 0.5f, 0.15f};
    hw.led_driver.SetLed(kLedSwitches[0], sw_brightness[edit_lane]);
    hw.led_driver.SetLed(kLedSwitches[1], 1.0f); // always on: running indicator

    hw.led_driver.SwapBuffersAndTransmit();
}

// ─────────────────────────────────────────────────────────────────────────────
// OLED Display
// Line 0: "x0x [DRUM]  BPM"
// Line 1: "Step:XX  Vol:XX%"
// Line 2: Active lane params
// Line 3: 16-block step pattern with playhead outline
// ─────────────────────────────────────────────────────────────────────────────
void UpdateDisplay()
{
    hw.display.Fill(false);
    char line[32];

    // Title: drum name + BPM
    hw.display.SetCursor(0, 0);
    snprintf(line, sizeof(line), "x0x[%s] %3dBPM", kDrumNames[edit_lane], (int)p_bpm);
    hw.display.WriteString(line, Font_6x8, true);

    // Step counter + master vol
    hw.display.SetCursor(0, 10);
    const int step_disp = (current_step >= 0) ? current_step + 1 : 0;
    snprintf(line, sizeof(line), "Step:%02d  Vol:%2d%%", step_disp, (int)(p_master_vol * 100.f));
    hw.display.WriteString(line, Font_6x8, true);

    // Active lane param summary
    hw.display.SetCursor(0, 20);
    if(edit_lane == LANE_KICK)
        snprintf(line, sizeof(line), "F:%.0fHz D:%.2f A:%.2f",
                 50.f + p_kick_freq * 200.f, p_kick_decay, p_kick_accent);
    else if(edit_lane == LANE_SNARE)
        snprintf(line, sizeof(line), "Snp:%.2f Dcy:%.2f", p_snare_snappy, p_snare_decay);
    else
        snprintf(line, sizeof(line), "HHDcy:%.2f", p_hh_decay);
    hw.display.WriteString(line, Font_6x8, true);

    // Step pattern label
    hw.display.SetCursor(0, 32);
    hw.display.WriteString("Ptn:", Font_6x8, true);

    // 16 tiny blocks (5px wide, 7px tall) starting at x=28
    const bool* pat  = GetActivePattern();
    const int   step = current_step;
    for(int i = 0; i < kNumSteps; i++)
    {
        const int x  = 28 + (i * 5);
        const int y0 = 32;
        const int y1 = 39;
        hw.display.DrawRect(x, y0, x + 3, y1, pat[i], true);
        // Playhead: outline rect around current step
        if(step == i)
            hw.display.DrawRect(x - 1, y0 - 1, x + 4, y1 + 1, true, false);
    }

    hw.display.Update();
}

// ─────────────────────────────────────────────────────────────────────────────
// Main
// ─────────────────────────────────────────────────────────────────────────────
int main(void)
{
    hw.Init();
    hw.SetAudioBlockSize(kRecommendedBlockSize); // 48 samples
    const float sr = hw.AudioSampleRate();       // 48 kHz

    // ── Init DSP (all before StartAudio) ──────────────────────────────────
    step_clock.Init((p_bpm / 60.f) * 4.f, sr); // 120 BPM → 8 Hz 16th-note clock

    bass_drum.Init(sr);
    bass_drum.SetFreq(60.f);
    bass_drum.SetAccent(0.75f);
    bass_drum.SetTone(0.5f);
    bass_drum.SetSelfFmAmount(0.3f);

    snare_drum.Init(sr);
    snare_drum.SetFreq(185.f);
    snare_drum.SetAccent(0.70f);
    snare_drum.SetTone(0.5f);

    hh_noise.Init();
    hh_noise.SetAmp(1.0f);

    hh_hp.Init();
    hh_hp.SetFrequency(5000.f / sr);  // normalized freq: 5kHz / 48kHz ≈ 0.104 → metallic HP
    hh_hp.SetFilterMode(OnePole::FILTER_MODE_HIGH_PASS);

    hh_env.Init(sr);
    hh_env.SetTime(ADENV_SEG_ATTACK, 0.001f);
    hh_env.SetTime(ADENV_SEG_DECAY, 0.4f);
    hh_env.SetMin(0.0f);
    hh_env.SetMax(1.0f);

    // ── Default step patterns ─────────────────────────────────────────────
    InitPatterns();

    // ── Clear display before audio starts (prevents white-noise on boot) ──
    hw.display.Fill(false);
    hw.display.SetCursor(0, 0);
    hw.display.WriteString("x0x INIT...", Font_6x8, true);
    hw.display.Update();

    // ── Start hardware ────────────────────────────────────────────────────
    hw.StartAdc();          // CRITICAL: must come before StartAudio
    hw.StartAudio(AudioCallback);

    // ── Main loop ─────────────────────────────────────────────────────────
    while(true)
    {
        hw.ProcessAllControls();
        ProcessControls();
        HandleUiEvents();
        UpdateDisplay();   // SPI — blocking, fully independent of LED I2C bus
        UpdateLeds();      // I2C DMA — starts async transfer, completes during Delay
        System::Delay(2); // matches Field_AnalogDrumCore reference timing
    }
}
