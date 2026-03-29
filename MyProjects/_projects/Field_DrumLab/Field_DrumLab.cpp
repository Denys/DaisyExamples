#include "daisy_field.h"
#include "daisysp.h"
#include "drumlab_voices.h"
#include <cstdio>
#include <cmath>

using namespace daisy;
using namespace daisysp;
using namespace drumlab;

// ── Hardware ─────────────────────────────────────────────────────────────────
DaisyField hw;

// ── DSP Instances ─────────────────────────────────────────────────────────────
AnalogBassDrum                      kick;
SyntheticSnareDrum                  snare;
HiHat<SquareNoise, LinearVCA, true> hihat;
AnalogSnareDrum                     clap;
SyntheticBassDrum                   perc;

// ── Anonymous namespace — all state ──────────────────────────────────────────
namespace
{

// ── Focus state ───────────────────────────────────────────────────────────────
volatile VoiceId focus_voice  = VoiceId::Kick;
volatile PageId  current_page = PageId::Synth;

// ── Trigger pending flags (main → audio) ─────────────────────────────────────
volatile bool kick_trig  = false;
volatile bool snare_trig = false;
volatile bool hihat_trig = false;
volatile bool hihat_open = false; // true = open hat decay; false = closed
volatile bool clap_trig  = false;
volatile bool perc_trig  = false;

// ── Per-voice parameters (written by main loop, read in audio callback) ───────
// Synth page: 7 focus-mapped parameters per voice
volatile float
    v_pitch[kNumVoices]; // Hz (kick/snare/clap/perc) or 0-1 (hihat tone freq)
volatile float v_decay[kNumVoices];
volatile float v_tone[kNumVoices];
volatile float v_timbre[kNumVoices]; // snappy / dirtiness / noisiness
volatile float v_accent[kNumVoices];
volatile float v_pan[kNumVoices]; // 0=full L, 0.5=centre, 1=full R
volatile float v_level[kNumVoices];

// Mix page: individual levels + global controls
volatile float mix_level[kNumVoices]; // K1-K5 on mix page
volatile float master_decay  = 1.0f;  // K6 on mix page (multiplier)
volatile float master_accent = 1.0f;  // K7 on mix page (multiplier)
volatile float master_vol    = 0.80f; // K8

// ── Trigger flash tracking (for A-row LEDs) ───────────────────────────────────
uint32_t trig_flash_start[kNumVoices] = {};

// ── Knob smoothing ────────────────────────────────────────────────────────────
float    knob_values[8] = {};
float    prev_knobs[8]  = {};
int      focus_knob     = -1;
uint32_t focus_start    = 0;

// ── OLED helpers ──────────────────────────────────────────────────────────────
constexpr const char* kProjectName = "DRUM LAB";

// ── Utility functions ─────────────────────────────────────────────────────────

float Clamp01(float v)
{
    return fclamp(v, 0.0f, 1.0f);
}


// Map K1 (0-1) to a voice's pitch in Hz.
// Each voice uses its natural frequency range.
float PitchHz(VoiceId v, float knob)
{
    const float k = Clamp01(knob);
    switch(v)
    {
        case VoiceId::Kick: return 30.0f + k * k * 170.0f;      // 30-200 Hz
        case VoiceId::Snare: return 100.0f + k * 300.0f;        // 100-400 Hz
        case VoiceId::HiHat: return 1000.0f + k * k * 11000.0f; // 1-12 kHz
        case VoiceId::Clap: return 150.0f + k * 350.0f;         // 150-500 Hz
        case VoiceId::Perc: return 100.0f + k * k * 700.0f;     // 100-800 Hz
        default: return 440.0f;
    }
}

// Format focused parameter value for OLED zoom
void FormatFocusValue(int knob_idx, VoiceId v, char* buf, size_t sz)
{
    const float val = Clamp01(knob_values[knob_idx]);

    if(current_page == PageId::Synth)
    {
        switch(knob_idx)
        {
            case 0: // Pitch
            {
                const float hz = PitchHz(v, val);
                snprintf(buf, sz, "%.0f Hz", hz);
                break;
            }
            case 1: // Decay
            case 2: // Tone
            case 3: // Timbre
            case 4: // Accent
            case 6: // Level
                snprintf(buf, sz, "%d%%", (int)(val * 100.0f + 0.5f));
                break;
            case 5: // Pan
            {
                const float pan_pct = (val - 0.5f) * 200.0f;
                if(val < 0.48f)
                    snprintf(buf, sz, "L%-3d", (int)(-pan_pct + 0.5f));
                else if(val > 0.52f)
                    snprintf(buf, sz, "R%-3d", (int)(pan_pct + 0.5f));
                else
                    snprintf(buf, sz, " CTR");
                break;
            }
            default: snprintf(buf, sz, "%d%%", (int)(val * 100.0f + 0.5f));
        }
    }
    else // Mix page
    {
        snprintf(buf, sz, "%d%%", (int)(val * 100.0f + 0.5f));
    }
}

// ── OLED draw ─────────────────────────────────────────────────────────────────
void DrawOverview()
{
    char line[32];

    hw.display.Fill(false);

    // Row 0: project name + focus voice
    snprintf(line,
             sizeof(line),
             "%-9s [%s]",
             kProjectName,
             VoiceShortName(focus_voice));
    hw.display.SetCursor(0, 0);
    hw.display.WriteString(line, Font_6x8, true);

    // Row 1: page indicator and master volume
    snprintf(line,
             sizeof(line),
             "%s  Vol:%d%%",
             current_page == PageId::Synth ? "SYN" : "MIX",
             (int)(master_vol * 100.0f + 0.5f));
    hw.display.SetCursor(0, 8);
    hw.display.WriteString(line, Font_6x8, true);

    // Rows 2-4: focus voice parameters on Synth page
    const int vi = static_cast<int>(focus_voice);
    snprintf(line,
             sizeof(line),
             "Pit:%.0fHz Dec:%d%%",
             (double)v_pitch[vi],
             (int)(v_decay[vi] * 100.0f + 0.5f));
    hw.display.SetCursor(0, 16);
    hw.display.WriteString(line, Font_6x8, true);

    snprintf(line,
             sizeof(line),
             "Ton:%d%% Tmb:%d%%",
             (int)(v_tone[vi] * 100.0f + 0.5f),
             (int)(v_timbre[vi] * 100.0f + 0.5f));
    hw.display.SetCursor(0, 24);
    hw.display.WriteString(line, Font_6x8, true);

    snprintf(line,
             sizeof(line),
             "Acc:%d%% Lv:%d%%",
             (int)(v_accent[vi] * 100.0f + 0.5f),
             (int)(v_level[vi] * 100.0f + 0.5f));
    hw.display.SetCursor(0, 32);
    hw.display.WriteString(line, Font_6x8, true);

    // Row 5: K8 master volume label
    snprintf(line, sizeof(line), "K8:Vol B1-5:Focus");
    hw.display.SetCursor(0, 40);
    hw.display.WriteString(line, Font_6x8, true);

    // Row 6: A-row and SW labels
    snprintf(line, sizeof(line), "A1-6:Pad A7:ALL");
    hw.display.SetCursor(0, 48);
    hw.display.WriteString(line, Font_6x8, true);

    // Row 7: page state hint
    snprintf(line, sizeof(line), "SW1:Page SW2:Panic");
    hw.display.SetCursor(0, 56);
    hw.display.WriteString(line, Font_6x8, true);

    hw.display.Update();
}

void DrawFocusedParameter()
{
    char title[32];
    char value[32];
    char line[32];

    const bool  synth_page = (current_page == PageId::Synth);
    const char* param_name = synth_page ? kSynthParamNames[focus_knob]
                                        : kMixParamNames[focus_knob];

    snprintf(title,
             sizeof(title),
             "%s > %s",
             VoiceFullName(focus_voice),
             param_name);
    FormatFocusValue(focus_knob, focus_voice, value, sizeof(value));

    hw.display.Fill(false);
    hw.display.SetCursor(0, 0);
    hw.display.WriteString(title, Font_7x10, true);
    hw.display.SetCursor(0, 18);
    hw.display.WriteString(value, Font_11x18, true);

    snprintf(line,
             sizeof(line),
             "%s | %s",
             VoiceShortName(focus_voice),
             synth_page ? "SYNTH" : "MIX");
    hw.display.SetCursor(0, 44);
    hw.display.WriteString(line, Font_6x8, true);

    // Progress bar
    const float raw   = Clamp01(knob_values[focus_knob]);
    const int   bar_w = (int)(raw * 127.0f);
    hw.display.DrawRect(0, 54, 127, 62, true, false);
    if(bar_w > 0)
        hw.display.DrawRect(0, 54, bar_w, 62, true, true);

    hw.display.Update();
}

void UpdateOled()
{
    if(focus_knob >= 0 && (System::GetNow() - focus_start) < kFocusTimeoutMs)
        DrawFocusedParameter();
    else
    {
        focus_knob = -1;
        DrawOverview();
    }
}

// ── LED update ────────────────────────────────────────────────────────────────
void UpdateLeds()
{
    const uint32_t now = System::GetNow();

    // A-row: flash on trigger, else off
    // A1=voice0(Kick), A2=voice1(Snare), A3=A4=voice2(HiHat), A5=voice3(Clap), A6=voice4(Perc)
    for(int i = 0; i < kNumVoices; i++)
    {
        const float brightness
            = ((now - trig_flash_start[i]) < kTriggerFlashMs) ? 1.0f : 0.0f;
        hw.led_driver.SetLed(KeyALedId(i), brightness);
    }
    // A7 = crash-all key (index 6)
    hw.led_driver.SetLed(KeyALedId(6), 0.0f);
    // A8 = reserved (index 7)
    hw.led_driver.SetLed(KeyALedId(7), 0.0f);

    // B-row: focus indicator (B1-B5 = voices 0-4)
    for(int i = 0; i < kNumVoices; i++)
    {
        const float br = LedForFocus(focus_voice, static_cast<VoiceId>(i));
        hw.led_driver.SetLed(FocusLedIndex(static_cast<VoiceId>(i)), br);
    }
    // B6-B8: off
    hw.led_driver.SetLed(KeyBLedId(5), 0.0f);
    hw.led_driver.SetLed(KeyBLedId(6), 0.0f);
    hw.led_driver.SetLed(KeyBLedId(7), 0.0f);

    // Knob LEDs: reflect current knob values
    for(int i = 0; i < 8; i++)
        hw.led_driver.SetLed(DaisyField::LED_KNOB_1 + i,
                             Clamp01(knob_values[i]));

    // SW1 LED: page indicator (on = Mix page)
    hw.led_driver.SetLed(DaisyField::LED_SW_1,
                         current_page == PageId::Mix ? 1.0f : 0.25f);
    hw.led_driver.SetLed(DaisyField::LED_SW_2, 0.0f);

    hw.led_driver.SwapBuffersAndTransmit();
}

// ── Knob capture and focus-routing ────────────────────────────────────────────
void SnapshotKnobs()
{
    for(int i = 0; i < 8; i++)
    {
        const float val = hw.knob[i].Value();
        if(fabsf(val - prev_knobs[i]) > kKnobMoveThresh)
        {
            focus_knob    = i;
            focus_start   = System::GetNow();
            prev_knobs[i] = val;
        }
        knob_values[i] = val;
    }
}

void RouteFocusKnobs()
{
    const int vi = static_cast<int>(focus_voice);

    if(current_page == PageId::Synth)
    {
        v_pitch[vi]  = PitchHz(focus_voice, knob_values[0]);
        v_decay[vi]  = Clamp01(knob_values[1]);
        v_tone[vi]   = Clamp01(knob_values[2]);
        v_timbre[vi] = Clamp01(knob_values[3]);
        v_accent[vi] = Clamp01(knob_values[4]);
        v_pan[vi]    = Clamp01(knob_values[5]);
        v_level[vi]  = Clamp01(knob_values[6]);
    }
    else // Mix page
    {
        for(int i = 0; i < kNumVoices; i++)
            mix_level[i] = Clamp01(knob_values[i]);
        master_decay  = 0.2f + Clamp01(knob_values[5]) * 1.6f;
        master_accent = Clamp01(knob_values[6]);
    }
    master_vol = Clamp01(knob_values[7]);
}

// ── Fire a voice trigger and illuminate its LED ───────────────────────────────
void TrigVoice(int vi)
{
    trig_flash_start[vi] = System::GetNow();
    switch(static_cast<VoiceId>(vi))
    {
        case VoiceId::Kick: kick_trig = true; break;
        case VoiceId::Snare: snare_trig = true; break;
        case VoiceId::HiHat: hihat_trig = true; break;
        case VoiceId::Clap: clap_trig = true; break;
        case VoiceId::Perc: perc_trig = true; break;
        default: break;
    }
}

// ── Panic ──────────────────────────────────────────────────────────────────────
void Panic()
{
    kick_trig = snare_trig = hihat_trig = clap_trig = perc_trig = false;
}

// ── Default voice parameters ───────────────────────────────────────────────────
void PrimeVoiceDefaults()
{
    for(int i = 0; i < kNumVoices; i++)
    {
        v_pitch[i]   = kDefaultFreq[i];
        v_decay[i]   = kDefaultDecay[i];
        v_tone[i]    = kDefaultTone[i];
        v_timbre[i]  = kDefaultTimbre[i];
        v_accent[i]  = kDefaultAccent[i];
        v_pan[i]     = kDefaultPan[i];
        v_level[i]   = kDefaultLevel[i];
        mix_level[i] = 0.80f;
    }
    for(int i = 0; i < 8; i++)
    {
        knob_values[i] = kDefaultKnobValues[i];
        prev_knobs[i]  = kDefaultKnobValues[i];
    }
}

// ── Control update (main loop only) ──────────────────────────────────────────
void UpdateControls()
{
    hw.ProcessAllControls();
    SnapshotKnobs();
    RouteFocusKnobs();

    // A-row pad indices (hw.KeyboardRisingEdge 0..7 = A1..A8)
    // A1=Kick  A2=Snare  A3=ClosedHat  A4=OpenHat  A5=Clap  A6=Perc  A7=All  A8=reserved
    if(hw.KeyboardRisingEdge(0)) // A1: Kick
        TrigVoice(static_cast<int>(VoiceId::Kick));

    if(hw.KeyboardRisingEdge(1)) // A2: Snare
        TrigVoice(static_cast<int>(VoiceId::Snare));

    if(hw.KeyboardRisingEdge(2)) // A3: Closed HiHat (chokes open)
    {
        hihat_open = false;
        TrigVoice(static_cast<int>(VoiceId::HiHat));
    }

    if(hw.KeyboardRisingEdge(3)) // A4: Open HiHat
    {
        hihat_open = true;
        TrigVoice(static_cast<int>(VoiceId::HiHat));
    }

    if(hw.KeyboardRisingEdge(4)) // A5: Clap
        TrigVoice(static_cast<int>(VoiceId::Clap));

    if(hw.KeyboardRisingEdge(5)) // A6: Perc
        TrigVoice(static_cast<int>(VoiceId::Perc));

    if(hw.KeyboardRisingEdge(6)) // A7: Crash — all voices + open hat
    {
        TrigVoice(static_cast<int>(VoiceId::Kick));
        TrigVoice(static_cast<int>(VoiceId::Snare));
        hihat_open = true;
        TrigVoice(static_cast<int>(VoiceId::HiHat));
        TrigVoice(static_cast<int>(VoiceId::Clap));
        TrigVoice(static_cast<int>(VoiceId::Perc));
    }
    // A8 (index 7): reserved

    // B1-B5 (indices 8-12): focus selection
    for(int i = 0; i < kNumVoices; i++)
    {
        if(hw.KeyboardRisingEdge(FocusKeyIndex(static_cast<VoiceId>(i))))
            focus_voice = static_cast<VoiceId>(i);
    }

    // SW1: page toggle
    if(hw.sw[0].RisingEdge())
        current_page
            = (current_page == PageId::Synth) ? PageId::Mix : PageId::Synth;

    // SW2: panic
    if(hw.sw[1].RisingEdge())
        Panic();
}

} // namespace

// ── Audio Callback ────────────────────────────────────────────────────────────
static void AudioCallback(AudioHandle::InputBuffer  in,
                          AudioHandle::OutputBuffer out,
                          size_t                    size)
{
    (void)in;

    // Snapshot volatile state once per block
    const bool do_kick = kick_trig;
    if(do_kick)
        kick_trig = false;
    const bool do_snare = snare_trig;
    if(do_snare)
        snare_trig = false;
    const bool do_hihat = hihat_trig;
    if(do_hihat)
        hihat_trig = false;
    const bool do_clap = clap_trig;
    if(do_clap)
        clap_trig = false;
    const bool do_perc = perc_trig;
    if(do_perc)
        perc_trig = false;
    const bool open_hat = hihat_open;

    // Apply voice parameters once per block (control-rate update)
    // Kick
    kick.SetFreq(v_pitch[0]);
    kick.SetDecay(v_decay[0] * master_decay);
    kick.SetTone(v_tone[0]);
    kick.SetAccent(v_accent[0] * master_accent);

    // Snare
    snare.SetFreq(v_pitch[1]);
    snare.SetDecay(v_decay[1] * master_decay);
    snare.SetFmAmount(v_tone[1]); // tone = FM sweep character
    snare.SetSnappy(v_timbre[1]);
    snare.SetAccent(v_accent[1] * master_accent);

    // HiHat
    hihat.SetFreq(v_pitch[2]);
    hihat.SetDecay(open_hat ? 0.7f : v_decay[2]);
    hihat.SetTone(v_tone[2]);
    hihat.SetNoisiness(v_timbre[2]);
    hihat.SetAccent(v_accent[2] * master_accent);

    // Clap (AnalogSnareDrum)
    clap.SetFreq(v_pitch[3]);
    clap.SetDecay(v_decay[3] * master_decay);
    clap.SetTone(v_tone[3]);
    clap.SetSnappy(v_timbre[3]);
    clap.SetAccent(v_accent[3] * master_accent);

    // Perc (SyntheticBassDrum)
    perc.SetFreq(v_pitch[4]);
    perc.SetDecay(v_decay[4] * master_decay);
    perc.SetTone(v_tone[4]);
    perc.SetDirtiness(v_timbre[4]);
    perc.SetAccent(v_accent[4] * master_accent);

    for(size_t i = 0; i < size; i++)
    {
        // Process each voice (trigger on first sample only)
        const bool  trig_now = (i == 0);
        const float s_kick   = kick.Process(trig_now && do_kick);
        const float s_snare  = snare.Process(trig_now && do_snare);
        const float s_hihat  = hihat.Process(trig_now && do_hihat);
        const float s_clap   = clap.Process(trig_now && do_clap);
        const float s_perc   = perc.Process(trig_now && do_perc);

        // Per-voice level (mix page overrides synth page level when mix_level != default)
        const float lk = v_level[0] * mix_level[0];
        const float ls = v_level[1] * mix_level[1];
        const float lh = v_level[2] * mix_level[2];
        const float lc = v_level[3] * mix_level[3];
        const float lp = v_level[4] * mix_level[4];

        // Stereo pan: linear pan law (L = 1-pan, R = pan), pan in [0,1]
        float sigL = s_kick * lk * (1.0f - v_pan[0])
                     + s_snare * ls * (1.0f - v_pan[1])
                     + s_hihat * lh * (1.0f - v_pan[2])
                     + s_clap * lc * (1.0f - v_pan[3])
                     + s_perc * lp * (1.0f - v_pan[4]);

        float sigR = s_kick * lk * v_pan[0] + s_snare * ls * v_pan[1]
                     + s_hihat * lh * v_pan[2] + s_clap * lc * v_pan[3]
                     + s_perc * lp * v_pan[4];

        sigL *= master_vol;
        sigR *= master_vol;

        out[0][i] = fclamp(sigL, -kOutputCeiling, kOutputCeiling);
        out[1][i] = fclamp(sigR, -kOutputCeiling, kOutputCeiling);
    }
}

// ── Main ──────────────────────────────────────────────────────────────────────
int main(void)
{
    hw.Init();
    hw.SetAudioBlockSize(48);

    PrimeVoiceDefaults();

    const float sr = hw.AudioSampleRate();

    // Initialize all DSP modules before StartAudio()
    kick.Init(sr);
    kick.SetSelfFmAmount(kKickSelfFm);
    kick.SetAttackFmAmount(kKickAttackFm);

    snare.Init(sr);
    snare.SetFmAmount(kSnareFmAmount);

    hihat.Init(sr);

    clap.Init(sr);
    clap.SetFreq(kClapBaseFreqHz);

    perc.Init(sr);
    perc.SetFmEnvelopeAmount(kPercFmEnvAmount);
    perc.SetFmEnvelopeDecay(kPercFmEnvDecay);

    // Apply startup defaults to DSP
    kick.SetFreq(kDefaultFreq[0]);
    kick.SetDecay(kDefaultDecay[0]);
    kick.SetTone(kDefaultTone[0]);
    kick.SetAccent(kDefaultAccent[0]);

    snare.SetFreq(kDefaultFreq[1]);
    snare.SetDecay(kDefaultDecay[1]);
    snare.SetFmAmount(
        kSnareFmAmount); // fixed FM + default tone overlap: use kDefaultTone[1]
    snare.SetSnappy(kDefaultTimbre[1]);
    snare.SetAccent(kDefaultAccent[1]);

    hihat.SetFreq(kDefaultFreq[2]);
    hihat.SetDecay(kDefaultDecay[2]);
    hihat.SetTone(kDefaultTone[2]);
    hihat.SetNoisiness(kDefaultTimbre[2]);
    hihat.SetAccent(kDefaultAccent[2]);

    clap.SetDecay(kDefaultDecay[3]);
    clap.SetTone(kDefaultTone[3]);
    clap.SetSnappy(kDefaultTimbre[3]);
    clap.SetAccent(kDefaultAccent[3]);

    perc.SetFreq(kDefaultFreq[4]);
    perc.SetDecay(kDefaultDecay[4]);
    perc.SetTone(kDefaultTone[4]);
    perc.SetDirtiness(kDefaultTimbre[4]);
    perc.SetAccent(kDefaultAccent[4]);

    hw.seed.StartLog(false);
    hw.seed.PrintLine("Field_DrumLab ready");
    hw.seed.PrintLine(
        "A1=Kick A2=Snare A3=CHat A4=OHat A5=Clap A6=Perc A7=ALL");
    hw.seed.PrintLine("B1-B5=Focus  SW1=Page(Syn/Mix)  SW2=Panic");
    hw.seed.PrintLine(
        "MIDI ch10: 36=Kick 38=Snr 42=CHat 46=OHat 39=Clap 37=Perc");

    hw.StartAdc();
    System::Delay(20);
    hw.ProcessAllControls();
    SnapshotKnobs();
    UpdateLeds();
    UpdateOled();

    hw.StartAudio(AudioCallback);

    while(1)
    {
        UpdateControls();
        UpdateLeds();
        UpdateOled();
        System::Delay(4);
    }
}
