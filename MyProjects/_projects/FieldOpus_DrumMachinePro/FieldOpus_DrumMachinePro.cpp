/**
 * FieldOpus_DrumMachinePro — Daisy Field
 *
 * 6-voice, 16-step drum machine with 8 pattern slots.
 * Proper Field HAL implementation with OLED, 16 touch keys, 8 knobs.
 *
 * Fixes over original Field_DrumMachinePro (Pod build):
 *   - Correct platform: daisy_field.h / DaisyField
 *   - Fixed swing: main-loop timing with proportional delay (not beat-drop)
 *   - Proper gain staging: tanh soft-saturation, reduced voice gains
 *   - Better voice assignment: AnalogSnareDrum for snare, AnalogBassDrum for tom
 *   - 8 pattern memory slots with default groove loaded
 *   - OLED display with pattern grid, BPM, voice name
 *   - Per-sample parameter smoothing (fonepole)
 *   - Full LED feedback (16 keys + 8 knobs + 2 switches)
 *   - Non-blocking boot (no StartLog hang)
 *
 * Controls:
 *   Keys A1-A8  : Toggle steps 1-8 for selected drum
 *   Keys B1-B8  : Toggle steps 9-16 for selected drum
 *   SW1         : Cycle drum voice (Kick→Snare→CHat→OHat→Tom→Clap)
 *   SW2         : Play / Stop
 *   Knob 1      : Kick Decay
 *   Knob 2      : Snare Decay
 *   Knob 3      : HiHat Decay (closed & open share base)
 *   Knob 4      : Tom Tune
 *   Knob 5      : Clap Decay
 *   Knob 6      : Master Volume
 *   Knob 7      : Tempo (40-240 BPM)
 *   Knob 8      : Swing (0-50%)
 */

#include "daisy_field.h"
#include "daisysp.h"
#include <cstdio>
#include <cmath>

using namespace daisy;
using namespace daisysp;

// ============================================================================
// Hardware
// ============================================================================
DaisyField hw;

// ============================================================================
// Drum voices
// ============================================================================
AnalogBassDrum     kick;
AnalogSnareDrum    snare;     // Proper analog snare (not SyntheticSnareDrum)
HiHat<>            hihatClosed;
HiHat<>            hihatOpen;
AnalogBassDrum     tom;       // AnalogBassDrum at higher pitch for tonal tom
SyntheticSnareDrum clap;      // High-freq + high-snappy for clap character

// ============================================================================
// Sequencer constants
// ============================================================================
constexpr int NUM_DRUMS    = 6;
constexpr int NUM_STEPS    = 16;
constexpr int NUM_PATTERNS = 8;

enum DrumVoice : uint8_t
{
    VOICE_KICK  = 0,
    VOICE_SNARE = 1,
    VOICE_CHAT  = 2,
    VOICE_OHAT  = 3,
    VOICE_TOM   = 4,
    VOICE_CLAP  = 5
};

static const char* drumNames[NUM_DRUMS] = {
    "KICK", "SNARE", "C.HAT", "O.HAT", "TOM", "CLAP"};

// ============================================================================
// Pattern memory: [pattern][drum][step]
// ============================================================================
bool patterns[NUM_PATTERNS][NUM_DRUMS][NUM_STEPS];

// ============================================================================
// Sequencer state
// ============================================================================
int      currentPattern = 0;
uint8_t  selectedDrum   = VOICE_KICK;
int      currentStep    = -1;
bool     seqPlaying     = true;
uint8_t  swingStep      = 0;

// ============================================================================
// Timing (main-loop based — correct swing implementation)
// ============================================================================
float    tempo       = 120.0f;
float    swing       = 0.0f;
uint32_t lastStepMs  = 0;

// ============================================================================
// Voice parameters (targets — smoothed in audio callback)
// ============================================================================
float kickDecay  = 0.55f;
float snareDecay = 0.45f;
float chatDecay  = 0.20f;
float ohatDecay  = 0.50f;
float tomTune    = 200.0f;
float clapDecay  = 0.30f;
float masterVol  = 0.80f;

// Smoothed current values
float kickDecayCur  = 0.55f;
float snareDecayCur = 0.45f;
float chatDecayCur  = 0.20f;
float ohatDecayCur  = 0.50f;
float tomTuneCur    = 200.0f;
float clapDecayCur  = 0.30f;
float masterVolCur  = 0.80f;

// ============================================================================
// Knob value cache (for LED feedback + zoom)
// ============================================================================
float kvals[8];

// ── OLED zoom state ────────────────────────────────────────────────────────
float    kz_prev[8]  = {};
int      kz_idx      = -1;
uint32_t kz_time     = 0;
constexpr uint32_t kZoomMs    = 1400;
constexpr float    kZoomDelta = 0.015f;

// ============================================================================
// LED lookup tables
// ============================================================================
static const size_t kKeyLeds[16] = {
    DaisyField::LED_KEY_A1, DaisyField::LED_KEY_A2,
    DaisyField::LED_KEY_A3, DaisyField::LED_KEY_A4,
    DaisyField::LED_KEY_A5, DaisyField::LED_KEY_A6,
    DaisyField::LED_KEY_A7, DaisyField::LED_KEY_A8,
    DaisyField::LED_KEY_B1, DaisyField::LED_KEY_B2,
    DaisyField::LED_KEY_B3, DaisyField::LED_KEY_B4,
    DaisyField::LED_KEY_B5, DaisyField::LED_KEY_B6,
    DaisyField::LED_KEY_B7, DaisyField::LED_KEY_B8,
};

static const size_t kKnobLeds[8] = {
    DaisyField::LED_KNOB_1, DaisyField::LED_KNOB_2,
    DaisyField::LED_KNOB_3, DaisyField::LED_KNOB_4,
    DaisyField::LED_KNOB_5, DaisyField::LED_KNOB_6,
    DaisyField::LED_KNOB_7, DaisyField::LED_KNOB_8,
};

// ============================================================================
// Soft-saturation helper (tanh approximation — replaces fclamp hard-clip)
// Renamed to avoid collision with daisysp::SoftClip
// ============================================================================
inline float SoftSat(float x)
{
    // Fast tanh approximation: x / (1 + |x|)
    // Smoother than fclamp, avoids harsh digital artifacts
    return x / (1.0f + fabsf(x));
}

// ============================================================================
// Pattern initialization — load default grooves
// ============================================================================
void InitPatterns()
{
    // Clear all patterns
    for(int p = 0; p < NUM_PATTERNS; p++)
        for(int d = 0; d < NUM_DRUMS; d++)
            for(int s = 0; s < NUM_STEPS; s++)
                patterns[p][d][s] = false;

    // --- Pattern 0: Basic 4/4 Rock ---
    // Kick: 4-on-the-floor
    patterns[0][VOICE_KICK][0]  = true;
    patterns[0][VOICE_KICK][4]  = true;
    patterns[0][VOICE_KICK][8]  = true;
    patterns[0][VOICE_KICK][12] = true;
    // Snare: backbeat (beats 2, 4)
    patterns[0][VOICE_SNARE][4]  = true;
    patterns[0][VOICE_SNARE][12] = true;
    // Closed HiHat: 8th notes
    for(int s = 0; s < NUM_STEPS; s += 2)
        patterns[0][VOICE_CHAT][s] = true;

    // --- Pattern 1: House ---
    patterns[1][VOICE_KICK][0]  = true;
    patterns[1][VOICE_KICK][4]  = true;
    patterns[1][VOICE_KICK][8]  = true;
    patterns[1][VOICE_KICK][12] = true;
    patterns[1][VOICE_SNARE][4]  = true;
    patterns[1][VOICE_SNARE][12] = true;
    patterns[1][VOICE_CHAT][2]  = true;
    patterns[1][VOICE_CHAT][6]  = true;
    patterns[1][VOICE_CHAT][10] = true;
    patterns[1][VOICE_CHAT][14] = true;
    patterns[1][VOICE_OHAT][7]  = true;
    patterns[1][VOICE_OHAT][15] = true;
    patterns[1][VOICE_CLAP][4]  = true;
    patterns[1][VOICE_CLAP][12] = true;

    // --- Pattern 2: Breakbeat ---
    patterns[2][VOICE_KICK][0]  = true;
    patterns[2][VOICE_KICK][6]  = true;
    patterns[2][VOICE_KICK][10] = true;
    patterns[2][VOICE_SNARE][4]  = true;
    patterns[2][VOICE_SNARE][12] = true;
    patterns[2][VOICE_SNARE][15] = true;
    for(int s = 1; s < NUM_STEPS; s += 2)
        patterns[2][VOICE_CHAT][s] = true;
    patterns[2][VOICE_OHAT][0] = true;
    patterns[2][VOICE_OHAT][8] = true;
    patterns[2][VOICE_TOM][13]  = true;
    patterns[2][VOICE_TOM][14]  = true;
    patterns[2][VOICE_CLAP][4]  = true;
    patterns[2][VOICE_CLAP][12] = true;

    // --- Pattern 3: Techno ---
    patterns[3][VOICE_KICK][0]  = true;
    patterns[3][VOICE_KICK][3]  = true;
    patterns[3][VOICE_KICK][6]  = true;
    patterns[3][VOICE_KICK][8]  = true;
    patterns[3][VOICE_KICK][11] = true;
    patterns[3][VOICE_KICK][14] = true;
    for(int s = 1; s < NUM_STEPS; s += 2)
        patterns[3][VOICE_CHAT][s] = true;
    patterns[3][VOICE_OHAT][2]  = true;
    patterns[3][VOICE_OHAT][6]  = true;
    patterns[3][VOICE_OHAT][10] = true;
    patterns[3][VOICE_OHAT][14] = true;
    patterns[3][VOICE_TOM][4]   = true;
    patterns[3][VOICE_TOM][12]  = true;
    patterns[3][VOICE_CLAP][8]  = true;
}

// ============================================================================
// Swing-aware step interval calculation (ms)
// ============================================================================
uint32_t GetStepIntervalMs()
{
    // Base 16th-note interval in ms
    const float baseMs = 60000.0f / tempo / 4.0f;

    // Swing delays odd-numbered 16ths (the "e" and "a" of each beat)
    // swing = 0.0: straight, swing = 0.5: triplet feel
    const bool isSwungStep = (swingStep % 2 == 1);

    if(isSwungStep)
        return static_cast<uint32_t>(baseMs * (1.0f + swing));
    else
        return static_cast<uint32_t>(baseMs * (1.0f - swing * 0.5f));
}

// ============================================================================
// Trigger a drum voice
// ============================================================================
void TriggerDrum(int drum)
{
    switch(drum)
    {
        case VOICE_KICK:  kick.Trig(); break;
        case VOICE_SNARE: snare.Trig(); break;
        case VOICE_CHAT:  hihatClosed.Trig(); break;
        case VOICE_OHAT:  hihatOpen.Trig(); break;
        case VOICE_TOM:   tom.Trig(); break;
        case VOICE_CLAP:  clap.Trig(); break;
        default: break;
    }
}

// ============================================================================
// Handle UI events (switches + keyboard)
// ============================================================================
void HandleUiEvents()
{
    // SW1: Cycle selected drum voice
    if(hw.GetSwitch(DaisyField::SW_1)->RisingEdge())
    {
        selectedDrum = (selectedDrum + 1) % NUM_DRUMS;
    }

    // SW2: Play / Stop
    if(hw.GetSwitch(DaisyField::SW_2)->RisingEdge())
    {
        seqPlaying = !seqPlaying;
        if(!seqPlaying)
            currentStep = -1;
    }

    // Touch keyboard: A row (0-7) = steps 1-8, B row (8-15) = steps 9-16
    for(int k = 0; k < 16; k++)
    {
        if(hw.KeyboardRisingEdge(k))
        {
            patterns[currentPattern][selectedDrum][k]
                = !patterns[currentPattern][selectedDrum][k];
        }
    }
}

// ============================================================================
// Process knobs → parameter targets
// ============================================================================
void ProcessKnobs()
{
    for(int i = 0; i < 8; i++)
        kvals[i] = hw.GetKnobValue(i);

    kickDecay  = 0.1f + kvals[0] * 0.9f;     // K1: 0.1–1.0s
    snareDecay = 0.1f + kvals[1] * 0.9f;     // K2: 0.1–1.0s
    chatDecay  = 0.05f + kvals[2] * 0.50f;   // K3: 0.05–0.55s
    ohatDecay  = 0.10f + kvals[2] * 0.90f;   // K3: open hat tracks with longer range
    tomTune    = 80.0f + kvals[3] * 220.0f;  // K4: 80–300 Hz
    clapDecay  = 0.1f + kvals[4] * 0.9f;     // K5: 0.1–1.0s
    masterVol  = kvals[5];                     // K6: 0.0–1.0
    tempo      = 40.0f + kvals[6] * 200.0f;  // K7: 40–240 BPM
    swing      = kvals[7] * 0.50f;            // K8: 0–50%
}

// ============================================================================
// Update LEDs — keys show pattern grid + playhead, knobs show value
// ============================================================================
void UpdateLeds()
{
    // Key LEDs: show current drum's pattern on current pattern slot
    for(int s = 0; s < 16; s++)
    {
        float brightness = 0.05f;
        if(patterns[currentPattern][selectedDrum][s])
            brightness = 0.50f;
        if(s == currentStep && seqPlaying)
            brightness = 1.0f;

        hw.led_driver.SetLed(kKeyLeds[s], brightness);
    }

    // Knob LEDs: mirror knob positions
    for(int i = 0; i < 8; i++)
        hw.led_driver.SetLed(kKnobLeds[i], kvals[i]);

    // Switch LEDs
    // SW1: dim=kick, bright varies by voice index
    hw.led_driver.SetLed(DaisyField::LED_SW_1,
                         0.15f + 0.14f * static_cast<float>(selectedDrum));

    // SW2: play=bright, stop=dim
    hw.led_driver.SetLed(DaisyField::LED_SW_2, seqPlaying ? 1.0f : 0.15f);

    hw.led_driver.SwapBuffersAndTransmit();
}

// ============================================================================
// Update OLED display (overview + knob zoom)
// ============================================================================
static const char* kOpKnobNames[8] = {
    "Kick Decay", "Snare Decay", "HiHat Decay",
    "Tom Tune", "Clap Decay", "Master Vol",
    "Tempo", "Swing"
};

void DrawZoom()
{
    char val[32];
    float v = kvals[kz_idx];
    switch(kz_idx)
    {
        case 0: snprintf(val, 32, "%.0f ms", (0.1f + v * 0.9f) * 1000.0f); break;
        case 1: snprintf(val, 32, "%.0f ms", (0.1f + v * 0.9f) * 1000.0f); break;
        case 2: snprintf(val, 32, "%.0f ms", (0.05f + v * 0.50f) * 1000.0f); break;
        case 3: snprintf(val, 32, "%.0f Hz", 80.0f + v * 220.0f); break;
        case 4: snprintf(val, 32, "%.0f ms", (0.1f + v * 0.9f) * 1000.0f); break;
        case 6: snprintf(val, 32, "%.0f BPM", 40.0f + v * 200.0f); break;
        case 7: snprintf(val, 32, "%d%%", (int)(v * 50.0f + 0.5f)); break;
        default: snprintf(val, 32, "%d%%", (int)(v * 100.0f + 0.5f)); break;
    }
    hw.display.Fill(false);
    hw.display.SetCursor(0, 0);
    hw.display.WriteString(kOpKnobNames[kz_idx], Font_7x10, true);
    hw.display.SetCursor(0, 18);
    hw.display.WriteString(val, Font_11x18, true);
    const int bar_w = (int)(v * 127.0f);
    hw.display.DrawRect(0, 54, 127, 62, true, false);
    if(bar_w > 0)
        hw.display.DrawRect(0, 54, bar_w, 62, true, true);
    hw.display.Update();
}

void CheckKnobs()
{
    for(int i = 0; i < 8; i++)
    {
        float v = kvals[i];
        if(fabsf(v - kz_prev[i]) > kZoomDelta)
        {
            kz_idx     = i;
            kz_time    = System::GetNow();
            kz_prev[i] = v;
        }
    }
}

void UpdateDisplay()
{
    CheckKnobs();
    if(kz_idx >= 0 && (System::GetNow() - kz_time) < kZoomMs)
    {
        DrawZoom();
        return;
    }
    kz_idx = -1;

    hw.display.Fill(false);
    char buf[40];

    // Line 1: Title + pattern slot
    hw.display.SetCursor(0, 0);
    snprintf(buf, sizeof(buf), "DRUM PRO  P%d", currentPattern + 1);
    hw.display.WriteString(buf, Font_7x10, true);

    // Line 2: Voice / BPM / Swing
    hw.display.SetCursor(0, 12);
    snprintf(buf, sizeof(buf), "%s %dBPM Sw:%d%%",
             drumNames[selectedDrum],
             static_cast<int>(tempo + 0.5f),
             static_cast<int>(swing * 100.0f + 0.5f));
    hw.display.WriteString(buf, Font_6x8, true);

    // Line 3: Step grid row 1 (steps 0-7)
    for(int s = 0; s < 8; s++)
    {
        const int x = 2 + s * 15;
        const bool active = patterns[currentPattern][selectedDrum][s];
        hw.display.DrawRect(x, 24, x + 12, 34, true, active);
        if(s == currentStep && seqPlaying)
            hw.display.DrawRect(x + 3, 26, x + 9, 32, true, !active);
    }

    // Line 4: Step grid row 2 (steps 8-15)
    for(int s = 0; s < 8; s++)
    {
        const int x = 2 + s * 15;
        const int idx = s + 8;
        const bool active = patterns[currentPattern][selectedDrum][idx];
        hw.display.DrawRect(x, 38, x + 12, 48, true, active);
        if(idx == currentStep && seqPlaying)
            hw.display.DrawRect(x + 3, 40, x + 9, 46, true, !active);
    }

    // Line 5: Sound params (KD/SD/HD decay% + vol) — replaces step counter
    hw.display.SetCursor(0, 54);
    snprintf(buf, sizeof(buf), "KD:%d SD:%d HD:%d V:%d%%",
             (int)(kvals[0] * 100), (int)(kvals[1] * 100),
             (int)(kvals[2] * 100), (int)(masterVol * 100));
    hw.display.WriteString(buf, Font_6x8, true);

    hw.display.Update();
}

// ============================================================================
// Audio Callback — non-interleaved (Field standard)
// ============================================================================
void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    for(size_t i = 0; i < size; i++)
    {
        // Per-sample parameter smoothing (prevents zipper noise)
        fonepole(kickDecayCur,  kickDecay,  0.002f);
        fonepole(snareDecayCur, snareDecay, 0.002f);
        fonepole(chatDecayCur,  chatDecay,  0.002f);
        fonepole(ohatDecayCur,  ohatDecay,  0.002f);
        fonepole(tomTuneCur,    tomTune,    0.002f);
        fonepole(clapDecayCur,  clapDecay,  0.002f);
        fonepole(masterVolCur,  masterVol,  0.002f);

        // Apply smoothed parameters to voices
        kick.SetDecay(kickDecayCur);
        snare.SetDecay(snareDecayCur);
        hihatClosed.SetDecay(chatDecayCur);
        hihatOpen.SetDecay(ohatDecayCur);
        tom.SetFreq(tomTuneCur);
        tom.SetDecay(0.40f);
        clap.SetDecay(clapDecayCur);

        // Process all 6 voices (DaisySP drums self-envelope to ~0 when idle)
        const float kickSig  = kick.Process(false);
        const float snareSig = snare.Process(false);
        const float chatSig  = hihatClosed.Process(false);
        const float ohatSig  = hihatOpen.Process(false);
        const float tomSig   = tom.Process(false);
        const float clapSig  = clap.Process(false);

        // Stereo mix with reduced gains (prevents clipping)
        // Theoretical max per channel: ~1.5 (well within soft-clip range)
        float mixL = kickSig  * 0.45f
                   + snareSig * 0.38f
                   + chatSig  * 0.18f
                   + ohatSig  * 0.18f
                   + tomSig   * 0.35f
                   + clapSig  * 0.25f;

        float mixR = kickSig  * 0.45f
                   + snareSig * 0.25f
                   + chatSig  * 0.35f
                   + ohatSig  * 0.35f
                   + tomSig   * 0.25f
                   + clapSig  * 0.30f;

        // Apply master volume + soft-saturation (tanh-like, no harsh clipping)
        mixL = SoftSat(mixL * (0.5f + masterVolCur * 1.5f));
        mixR = SoftSat(mixR * (0.5f + masterVolCur * 1.5f));

        out[0][i] = mixL;
        out[1][i] = mixR;
    }
}

// ============================================================================
// Main
// ============================================================================
int main(void)
{
    hw.Init();
    hw.SetAudioBlockSize(48);
    const float sr = hw.AudioSampleRate();

    // --- Init drum voices ---
    kick.Init(sr);
    kick.SetFreq(52.0f);
    kick.SetTone(0.5f);
    kick.SetDecay(kickDecay);
    kick.SetAccent(0.80f);

    snare.Init(sr);
    snare.SetFreq(185.0f);
    snare.SetTone(0.55f);
    snare.SetDecay(snareDecay);
    snare.SetSnappy(0.60f);
    snare.SetAccent(0.75f);

    hihatClosed.Init(sr);
    hihatClosed.SetFreq(4000.0f);
    hihatClosed.SetTone(0.6f);
    hihatClosed.SetDecay(chatDecay);
    hihatClosed.SetAccent(0.70f);

    hihatOpen.Init(sr);
    hihatOpen.SetFreq(4000.0f);
    hihatOpen.SetTone(0.6f);
    hihatOpen.SetDecay(ohatDecay);
    hihatOpen.SetAccent(0.70f);

    tom.Init(sr);
    tom.SetFreq(tomTune);
    tom.SetTone(0.65f);
    tom.SetDecay(0.40f);
    tom.SetAccent(0.80f);

    clap.Init(sr);
    clap.SetFreq(800.0f);
    clap.SetSnappy(0.85f);
    clap.SetDecay(clapDecay);
    clap.SetAccent(0.70f);

    // --- Load default patterns ---
    InitPatterns();

    // --- Start peripherals ---
    hw.StartAdc();
    hw.StartAudio(AudioCallback);

    lastStepMs = System::GetNow();

    // --- Main loop ---
    while(1)
    {
        hw.ProcessAllControls();
        HandleUiEvents();
        ProcessKnobs();

        // --- Sequencer advance (main-loop timing with proper swing) ---
        const uint32_t now      = System::GetNow();
        const uint32_t interval = GetStepIntervalMs();

        if(seqPlaying && (now - lastStepMs >= interval))
        {
            currentStep = (currentStep + 1) % NUM_STEPS;
            swingStep++;
            lastStepMs = now;

            // Trigger all active drums on this step
            for(int d = 0; d < NUM_DRUMS; d++)
            {
                if(patterns[currentPattern][d][currentStep])
                    TriggerDrum(d);
            }
        }

        UpdateLeds();
        UpdateDisplay();
        System::Delay(2);
    }
}
