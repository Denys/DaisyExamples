/**
 * Field_AmbientGarden.cpp
 *
 * A self-playing generative ambient synthesizer using probabilistic logic
 * and Modal synthesis. A Turing Machine shift register generates pseudo-random
 * values, quantized to musical scales, driving 4 Modal voices through reverb.
 *
 * Complexity: 9/10
 *
 * DSP Chain:
 *   RandomClock -> TuringMachine -> ScaleQuantizer -> 4x ModalVoice
 *   -> Mixer -> SVF LP -> ReverbSc (stereo) -> Soft Clip -> Output
 *
 * Controls:
 *   K1: Density     K2: Scale Root   K3: Reverb Size   K4: Harmony Spread
 *   K5: Brightness  K6: Damping      K7: Structure      K8: Wet/Dry
 *   A1-A8: Scale Select    B1-B8: Voice Presets
 *   SW1: Freeze            SW2: Stereo Width
 */

#include "daisy_field.h"
#include "daisysp.h"
#include "../../foundation_examples/field_defaults.h"
#include "turing_machine.h"
#include "scale_quantizer.h"
#include "random_clock.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>

using namespace daisy;
using namespace daisysp;
using namespace FieldDefaults;

// ============================================================================
// HARDWARE & UI
// ============================================================================

DaisyField        hw;
FieldKeyboardLEDs keyLeds;
FieldOLEDDisplay  display;

// ============================================================================
// DSP MODULES
// ============================================================================

static constexpr int kNumVoices = 4;

ModalVoice   modal[kNumVoices];
ReverbSc     reverb;
Svf          preLpfL;
Svf          preLpfR;
TuringMachine tm;
ScaleQuantizer quantizer;
RandomClock   rclock;

// ============================================================================
// STATE
// ============================================================================

int  current_scale  = 0;  // A-row: scale index (0-7)
int  current_preset = 0;  // B-row: voice preset index (0-7)
bool freeze_enabled = false;
bool stereo_wide    = false;

// Last triggered note names for display
float last_voice_freq[kNumVoices] = {0, 0, 0, 0};
bool  voice_active[kNumVoices]    = {false, false, false, false};

// Trigger pulse counters for LED animation (in main loop ticks)
int voice_pulse[kNumVoices] = {0, 0, 0, 0};

// Knob values read once per loop (avoids multiple Process() calls)
float knob_values[8] = {0};

// ============================================================================
// PARAMETERS
// ============================================================================

struct Params
{
    float density    = 0.4f;  // K1
    float root_knob  = 0.0f;  // K2 raw
    float reverb_size = 0.6f; // K3
    float spread     = 0.3f;  // K4
    float brightness = 0.5f;  // K5
    float damping    = 0.4f;  // K6
    float structure  = 0.5f;  // K7
    float wet_dry    = 0.6f;  // K8
} params;

// Smoothed values for click-free audio (reverb + LPF params only)
// Note: brightness/damping/structure are applied at 60Hz via ProcessKnobs() — sufficient
struct SmoothedParams
{
    float lpf_cutoff = 4000.f;
    float rev_decay  = 0.85f;
    float rev_lpfreq = 8000.f;
    float wet_dry    = 0.6f;
} smoothed;

constexpr float kSmoothCoeff = 0.01f;

// ============================================================================
// VOICE PRESETS
// ============================================================================

struct VoicePreset
{
    float       brightness;
    float       structure;
    float       damping;
    float       accent;
    const char* name;
};

const VoicePreset kPresets[8] = {
    {0.9f, 0.3f, 0.3f, 0.8f, "Glass"},      // B1
    {0.4f, 0.7f, 0.5f, 0.6f, "Marimba"},     // B2
    {0.7f, 0.2f, 0.2f, 0.9f, "Metal"},       // B3
    {0.3f, 0.5f, 0.7f, 0.3f, "Soft Pad"},    // B4
    {0.6f, 0.4f, 0.4f, 0.7f, "Gamelan"},     // B5
    {0.8f, 0.1f, 0.15f, 0.5f, "WindChime"},  // B6
    {0.5f, 0.6f, 0.6f, 0.85f, "Steel Dr"},   // B7
    {0.2f, 0.8f, 0.1f, 0.4f, "Temple"},      // B8
};

// ============================================================================
// NOTE NAMES (for display)
// ============================================================================

const char* kNoteNames[12] = {
    "C", "C#", "D", "D#", "E", "F",
    "F#", "G", "G#", "A", "A#", "B"};

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

inline float Smooth(float current, float target, float coeff)
{
    return current + (target - current) * coeff;
}

// xorshift32 PRNG — lock-free, callback-safe (rand() is NOT real-time safe)
static uint32_t xr_state = 12345u;

inline float RandomFloat()
{
    xr_state ^= xr_state << 13;
    xr_state ^= xr_state >> 17;
    xr_state ^= xr_state << 5;
    return static_cast<float>(xr_state) * 2.3283064e-10f; // / 2^32 → [0, 1)
}

int FreqToMidiNote(float freq)
{
    if(freq <= 0.f)
        return 0;
    return static_cast<int>(12.f * log2f(freq / 440.f) + 69.f + 0.5f);
}

// ============================================================================
// VOICE DISTRIBUTION - called on each clock trigger
// ============================================================================

void OnClockTrigger()
{
    uint8_t tm_output = tm.Process();
    float   base_freq = quantizer.Process(tm_output);
    float   spread    = params.spread;

    const VoicePreset& preset = kPresets[current_preset];

    // Voice 1: melody (always fires)
    modal[0].SetFreq(base_freq);
    modal[0].SetAccent(preset.accent);
    modal[0].Trig();
    last_voice_freq[0] = base_freq;
    voice_active[0]    = true;
    voice_pulse[0]     = 6; // ~100ms at 60Hz

    // Voice 2: harmony low (60% chance)
    if(RandomFloat() < 0.6f)
    {
        float semitones_down = spread * 12.0f;
        float freq2 = base_freq * powf(2.0f, -semitones_down / 12.0f);
        if(freq2 < 30.0f)
            freq2 = 30.0f;
        modal[1].SetFreq(freq2);
        modal[1].SetAccent(preset.accent * 0.8f);
        modal[1].Trig();
        last_voice_freq[1] = freq2;
        voice_active[1]    = true;
        voice_pulse[1]     = 6;
    }

    // Voice 3: harmony high (40% chance)
    if(RandomFloat() < 0.4f)
    {
        float semitones_up = spread * 7.0f;
        float freq3 = base_freq * powf(2.0f, semitones_up / 12.0f);
        if(freq3 > 8000.0f)
            freq3 = 8000.0f;
        modal[2].SetFreq(freq3);
        modal[2].SetAccent(preset.accent * 0.7f);
        modal[2].Trig();
        last_voice_freq[2] = freq3;
        voice_active[2]    = true;
        voice_pulse[2]     = 6;
    }

    // Voice 4: accent chime (20% chance)
    if(RandomFloat() < 0.2f)
    {
        float freq4 = base_freq * 2.0f; // Octave up
        if(freq4 > 8000.0f)
            freq4 = 8000.0f;
        modal[3].SetFreq(freq4);
        modal[3].SetAccent(preset.accent);
        modal[3].SetBrightness(fminf(preset.brightness + 0.2f, 1.0f));
        modal[3].Trig();
        last_voice_freq[3] = freq4;
        voice_active[3]    = true;
        voice_pulse[3]     = 6;
    }
}

// ============================================================================
// KNOB PROCESSING (called from main loop ONLY)
// ============================================================================

void ProcessKnobs()
{
    // Read each knob exactly once per loop iteration
    for(int i = 0; i < 8; i++)
        knob_values[i] = hw.knob[i].Process();

    float k1 = knob_values[0];
    float k2 = knob_values[1];
    float k3 = knob_values[2];
    float k4 = knob_values[3];
    float k5 = knob_values[4];
    float k6 = knob_values[5];
    float k7 = knob_values[6];
    float k8 = knob_values[7];

    params.density    = k1;
    params.root_knob  = k2;
    params.reverb_size = k3;
    params.spread     = k4;
    params.brightness = k5;
    params.damping    = k6;
    params.structure  = k7;
    params.wet_dry    = k8;

    // Update clock density
    rclock.SetDensity(params.density);

    // Update scale root: quantize K2 to 12 semitones
    int root_offset = static_cast<int>(k2 * 11.99f);
    quantizer.SetRoot(48 + root_offset); // C3 + offset

    // Update TM probability from spread knob
    // Center (0.5) = locked patterns, extremes = more chaos
    float prob = fabsf(k4 * 2.0f - 1.0f) * 0.5f;
    tm.SetProbability(prob);

    // Update voice parameters from knobs
    for(int v = 0; v < kNumVoices; v++)
    {
        modal[v].SetBrightness(params.brightness);
        modal[v].SetDamping(params.damping);
        modal[v].SetStructure(params.structure);
    }
}

// ============================================================================
// AUDIO CALLBACK
// ============================================================================

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    // Map reverb knob to reverb params
    float rev_target_decay  = 0.7f + params.reverb_size * 0.29f;  // 0.70 - 0.99
    float rev_target_lpfreq = 2000.f + params.reverb_size * 14000.f; // 2k-16k
    smoothed.rev_decay  = Smooth(smoothed.rev_decay, rev_target_decay, kSmoothCoeff);
    smoothed.rev_lpfreq = Smooth(smoothed.rev_lpfreq, rev_target_lpfreq, kSmoothCoeff);
    smoothed.wet_dry    = Smooth(smoothed.wet_dry, params.wet_dry, kSmoothCoeff);

    // Map brightness to LPF cutoff
    float lpf_target = 800.0f + params.brightness * 11200.f; // 800 - 12000 Hz
    smoothed.lpf_cutoff = Smooth(smoothed.lpf_cutoff, lpf_target, kSmoothCoeff);

    // Update filters
    preLpfL.SetFreq(smoothed.lpf_cutoff);
    preLpfR.SetFreq(smoothed.lpf_cutoff);

    // Update reverb
    reverb.SetFeedback(smoothed.rev_decay);
    reverb.SetLpFreq(smoothed.rev_lpfreq);

    for(size_t i = 0; i < size; i++)
    {
        // Check clock trigger
        if(rclock.Process())
        {
            OnClockTrigger();
        }

        // Sum voices
        float sum_l = 0.0f;
        float sum_r = 0.0f;

        for(int v = 0; v < kNumVoices; v++)
        {
            float sig = modal[v].Process();

            if(stereo_wide)
            {
                // Wide: V0,V2 left; V1,V3 right
                if(v == 0 || v == 2)
                {
                    sum_l += sig * 0.35f;
                    sum_r += sig * 0.15f;
                }
                else
                {
                    sum_l += sig * 0.15f;
                    sum_r += sig * 0.35f;
                }
            }
            else
            {
                // Mono center
                sum_l += sig * 0.25f;
                sum_r += sig * 0.25f;
            }
        }

        // Pre-reverb lowpass filter
        preLpfL.Process(sum_l);
        float filt_l = preLpfL.Low();
        preLpfR.Process(sum_r);
        float filt_r = preLpfR.Low();

        // Reverb (stereo)
        float rev_l = 0.0f;
        float rev_r = 0.0f;
        reverb.Process(filt_l, filt_r, &rev_l, &rev_r);

        // Wet/dry mix
        float mix = smoothed.wet_dry;
        float out_l = filt_l * (1.0f - mix) + rev_l * mix;
        float out_r = filt_r * (1.0f - mix) + rev_r * mix;

        // Soft clip
        out[0][i] = tanhf(out_l * 1.5f) * 0.8f;
        out[1][i] = tanhf(out_r * 1.5f) * 0.8f;
    }
}

// ============================================================================
// MAIN
// ============================================================================

int main(void)
{
    // Initialize hardware
    hw.Init();
    hw.SetAudioBlockSize(kRecommendedBlockSize);
    float sr = hw.AudioSampleRate();

    // Seed xorshift32 PRNG from hardware timer (never allow 0)
    xr_state = System::GetNow() | 1u;

    // Initialize UI helpers
    keyLeds.Init(&hw);
    display.Init(&hw);
    display.SetTitle("Ambient Garden");

    display.SetLabel(0, "Density");
    display.SetLabel(1, "Root");
    display.SetLabel(2, "Reverb");
    display.SetLabel(3, "Spread");
    display.SetLabel(4, "Bright");
    display.SetLabel(5, "Damp");
    display.SetLabel(6, "Struct");
    display.SetLabel(7, "Wet/Dry");

    // Initialize custom generative modules
    tm.Init();
    quantizer.Init();
    rclock.Init(sr);

    // Initialize Modal Voices
    const VoicePreset& initPreset = kPresets[0];
    for(int v = 0; v < kNumVoices; v++)
    {
        modal[v].Init(sr);
        modal[v].SetFreq(220.0f);
        modal[v].SetBrightness(initPreset.brightness);
        modal[v].SetStructure(initPreset.structure);
        modal[v].SetDamping(initPreset.damping);
        modal[v].SetAccent(initPreset.accent);
    }

    // Initialize Reverb
    reverb.Init(sr);
    reverb.SetFeedback(0.85f);
    reverb.SetLpFreq(8000.f);

    // Initialize pre-reverb filters
    preLpfL.Init(sr);
    preLpfL.SetFreq(4000.f);
    preLpfL.SetRes(0.2f);
    preLpfL.SetDrive(0.0f);

    preLpfR.Init(sr);
    preLpfR.SetFreq(4000.f);
    preLpfR.SetRes(0.2f);
    preLpfR.SetDrive(0.0f);

    // Set initial scale and preset LEDs
    keyLeds.SetA(0, true); // A1: Pentatonic Major
    keyLeds.SetB(0, true); // B1: Glass Bells

    hw.StartAdc();

    // Pre-warm display: process controls once so last_values[] holds real ADC
    // readings. Without this, FieldOLEDDisplay's -999 init causes all 8 knobs
    // to register as "changed" on the first SetValue() loop, leaving K8 (Wet/Dry)
    // as active_param_ at boot — display shows "> Wet/Dry" for 2 seconds.
    hw.ProcessAllControls();
    ProcessKnobs();
    for(int i = 0; i < 8; i++)
        display.SetValue(i, knob_values[i]);
    display.SetActiveParam(-1); // start in voice view, no active param

    hw.StartAudio(AudioCallback);

    // ========================================================================
    // MAIN LOOP
    // ========================================================================
    while(1)
    {
        hw.ProcessAllControls();
        ProcessKnobs();

        // --------------------------------------------------------------------
        // A-Row: Scale selection (exclusive)
        // --------------------------------------------------------------------
        for(int i = 0; i < 8; i++)
        {
            if(hw.KeyboardRisingEdge(kKeyAIndices[i]))
            {
                for(int j = 0; j < 8; j++)
                    keyLeds.SetA(j, false);

                current_scale = i;
                quantizer.SetScale(i);
                keyLeds.SetA(i, true);
            }
        }

        // --------------------------------------------------------------------
        // B-Row: Voice preset selection (exclusive)
        // --------------------------------------------------------------------
        for(int i = 0; i < 8; i++)
        {
            if(hw.KeyboardRisingEdge(kKeyBIndices[i]))
            {
                for(int j = 0; j < 8; j++)
                    keyLeds.SetB(j, false);

                current_preset = i;
                const VoicePreset& p = kPresets[i];
                for(int v = 0; v < kNumVoices; v++)
                {
                    modal[v].SetBrightness(p.brightness);
                    modal[v].SetStructure(p.structure);
                    modal[v].SetDamping(p.damping);
                }
                keyLeds.SetB(i, true);
            }
        }

        // --------------------------------------------------------------------
        // SW1: Freeze toggle
        // --------------------------------------------------------------------
        if(hw.sw[0].RisingEdge())
        {
            freeze_enabled = !freeze_enabled;
            rclock.SetFreeze(freeze_enabled);
        }

        // --------------------------------------------------------------------
        // SW2: Stereo width toggle
        // --------------------------------------------------------------------
        if(hw.sw[1].RisingEdge())
        {
            stereo_wide = !stereo_wide;
        }

        // --------------------------------------------------------------------
        // Voice pulse LED animation (decrement counters)
        // --------------------------------------------------------------------
        for(int v = 0; v < kNumVoices; v++)
        {
            if(voice_pulse[v] > 0)
                voice_pulse[v]--;
        }

        // --------------------------------------------------------------------
        // OLED Display
        // --------------------------------------------------------------------
        // Feed cached knob values to display helper (for change detection)
        for(int i = 0; i < 8; i++)
            display.SetValue(i, knob_values[i]);

        hw.display.Fill(false);

        // Line 0: Title with scale + root
        int root_offset = static_cast<int>(params.root_knob * 11.99f);
        char title_buf[32];
        snprintf(title_buf, sizeof(title_buf),
                 "Garden: %s %s",
                 quantizer.GetScaleName(),
                 kNoteNames[root_offset]);
        hw.display.SetCursor(0, 0);
        hw.display.WriteString(title_buf, Font_6x8, true);

        // Active parameter display with timeout (revert to voice view after 2s)
        int active_param = display.GetActiveParam();
        if(active_param >= 0)
        {
            uint32_t elapsed = System::GetNow() - display.GetLastChangeTime();
            if(elapsed > 2000)
            {
                display.SetActiveParam(-1);
                active_param = -1;
            }
        }

        if(active_param >= 0)
        {
            const char* labels[8] = {
                "Density", "Scale Root", "Reverb Size", "Spread",
                "Brightness", "Damping", "Structure", "Wet/Dry"};

            char buf[32];
            hw.display.SetCursor(0, 10);
            snprintf(buf, sizeof(buf), "> %s", labels[active_param]);
            hw.display.WriteString(buf, Font_6x8, true);

            hw.display.SetCursor(0, 20);
            snprintf(buf, sizeof(buf), "%.2f", knob_values[active_param]);
            hw.display.WriteString(buf, Font_11x18, true);
        }
        else
        {
            // Show voice activity when no knob is being adjusted
            char voice_buf[32];
            for(int v = 0; v < kNumVoices; v++)
            {
                if(last_voice_freq[v] > 0)
                {
                    int midi = FreqToMidiNote(last_voice_freq[v]);
                    int note = midi % 12;
                    int oct  = (midi / 12) - 1;
                    snprintf(voice_buf, sizeof(voice_buf),
                             "V%d:%s%d", v + 1, kNoteNames[note], oct);
                }
                else
                {
                    snprintf(voice_buf, sizeof(voice_buf), "V%d:--", v + 1);
                }
                hw.display.SetCursor((v < 2) ? 0 : 64, 12 + (v % 2) * 10);
                hw.display.WriteString(voice_buf, Font_6x8, true);
            }
        }

        // Status line
        char status[32];
        snprintf(status, sizeof(status), "%s %s %s",
                 freeze_enabled ? "FRZ" : "RUN",
                 stereo_wide ? "Wide" : "Mono",
                 kPresets[current_preset].name);
        hw.display.SetCursor(0, 54);
        hw.display.WriteString(status, Font_6x8, true);

        hw.display.Update();

        // --------------------------------------------------------------------
        // LED Updates
        // --------------------------------------------------------------------

        // Switch LEDs
        hw.led_driver.SetLed(kLedSwitches[0], freeze_enabled ? 1.0f : 0.0f);
        hw.led_driver.SetLed(kLedSwitches[1], stereo_wide ? 1.0f : 0.0f);

        // Knob LEDs: proportional brightness
        for(int i = 0; i < 8; i++)
            hw.led_driver.SetLed(kLedKnobs[i], hw.knob[i].Value());

        // Voice trigger pulse on B-row LEDs
        // Flash B1-B4 briefly on voice trigger, but never touch the current preset LED
        for(int v = 0; v < kNumVoices; v++)
        {
            if(v == current_preset)
                continue; // Preserve preset selection LED
            keyLeds.SetB(v, voice_pulse[v] > 0);
        }

        keyLeds.Update();

        System::Delay(16);
    }
}
