/**
 * Field_WavetableDroneLab.cpp
 * 
 * A self-evolving drone generator using wavetable morphing and multi-stage modulation.
 * Uses LFOs to slowly sweep through spectral content without user input.
 * 
 * Complexity: 7/10
 * 
 * DSP Chain:
 *   OscillatorBank (8 harmonics) ─┐
 *                                 ├─► CrossFade (LFO modulated) ─► SVF LP ─► SVF HP ─► Output
 *   VariableShapeOsc ─────────────┘
 * 
 * Controls:
 *   K1: Bank Mix       K2: VarShape Morph    K3: LFO1 Rate     K4: LFO2 Rate
 *   K5: LP Cutoff      K6: HP Cutoff         K7: Filter Res    K8: Master Volume
 *   A1-A8: Drone Presets    B1-B8: LFO Shapes    SW1: Freeze    SW2: HP Enable
 */

#include "daisy_field.h"
#include "daisysp.h"
#include "../../foundation_examples/field_defaults.h"
#include <cmath>
#include <cstdio>

using namespace daisy;
using namespace daisysp;
using namespace FieldDefaults;

// Hardware and UI
DaisyField        hw;
FieldKeyboardLEDs keyLeds;
FieldOLEDDisplay  display;

// ============================================================================
// DSP MODULES
// ============================================================================

// Oscillator sources
OscillatorBank          oscBank; // 8 harmonically-related oscillators
VariableShapeOscillator varOsc;  // Morphable waveform

// LFOs
Oscillator lfo1; // Morph modulation
Oscillator lfo2; // Level modulation

// Filters
Svf svfLP; // Lowpass
Svf svfHP; // Highpass

// ============================================================================
// STATE
// ============================================================================

// Current preset (0-7 for A1-A8)
int current_preset = 0;

// Current LFO shape (0-7 for B1-B8)
int current_lfo_shape = 0;

// Switch states
bool freeze_enabled = false;
bool audio_running  = true; // SW2: Start/Stop toggle

// Frozen LFO position (when freeze is on)
float frozen_morph_pos = 0.0f;

// LFO waveform types
const uint8_t kLfoWaveforms[8] = {
    Oscillator::WAVE_SIN,    // B1: Sine
    Oscillator::WAVE_TRI,    // B2: Triangle
    Oscillator::WAVE_SAW,    // B3: Saw Up
    Oscillator::WAVE_RAMP,   // B4: Saw Down (Ramp)
    Oscillator::WAVE_SQUARE, // B5: Square
    Oscillator::WAVE_SIN,    // B6: "Random" - use sine, apply S&H in code
    Oscillator::WAVE_SIN,    // B7: Sync Slow - both same phase
    Oscillator::WAVE_SIN     // B8: Sync Fast - both same phase, faster
};

// ============================================================================
// PARAMETERS
// ============================================================================

struct Params
{
    float bank_mix  = 0.5f;   // K1: 0=VarOsc, 1=OscBank
    float var_morph = 0.3f;   // K2: Variable shape morph
    float lfo1_rate = 0.1f;   // K3: 0.01 - 2.0 Hz
    float lfo2_rate = 0.2f;   // K4: 0.01 - 5.0 Hz
    float lp_cutoff = 2000.f; // K5: 200 - 8000 Hz
    float hp_cutoff = 100.f;  // K6: 20 - 2000 Hz
    float resonance = 0.3f;   // K7: 0.0 - 0.9
    float lfo_depth = 0.5f;   // K8: 0.0 - 1.0 (modulation intensity)
} params;

// Smoothed values for click-free parameter changes
struct SmoothedParams
{
    float bank_mix  = 0.5f;
    float lp_cutoff = 2000.f;
    float hp_cutoff = 100.f;
} smoothed;

// Smoothing coefficient (lower = smoother)
constexpr float kSmoothCoeff = 0.01f;

// ============================================================================
// PRESETS
// ============================================================================

struct DronePreset
{
    float       base_freq; // Fundamental frequency
    float       bank_mix;  // Default bank mix
    float       lp_cutoff; // Default LP cutoff
    float       hp_cutoff; // Default HP cutoff
    float       resonance; // Default resonance
    const char* name;
};

const DronePreset kPresets[8] = {
    {55.0f, 0.9f, 400.f, 30.f, 0.2f, "Deep Bass"},   // A1
    {110.f, 0.6f, 1500.f, 80.f, 0.3f, "Warm Pad"},   // A2
    {220.f, 0.4f, 6000.f, 500.f, 0.5f, "Shimmer"},   // A3
    {82.4f, 0.8f, 600.f, 50.f, 0.4f, "Dark Amb"},    // A4
    {165.f, 0.5f, 5000.f, 200.f, 0.4f, "Bright Ev"}, // A5
    {130.8f, 0.7f, 3000.f, 100.f, 0.2f, "Sparse"},   // A6
    {98.0f, 0.3f, 2500.f, 60.f, 0.6f, "Dense"},      // A7
    {196.f, 0.2f, 8000.f, 800.f, 0.3f, "Noise Wash"} // A8
};

// ============================================================================
// FUNCTIONS
// ============================================================================

inline float Lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

inline float Smooth(float current, float target, float coeff)
{
    return current + (target - current) * coeff;
}

void LoadPreset(int idx)
{
    if(idx < 0 || idx >= 8)
        return;

    const DronePreset& p = kPresets[idx];

    // Set oscillator bank fundamental
    oscBank.SetFreq(p.base_freq);

    // Set variable oscillator frequency (slightly detuned)
    varOsc.SetFreq(p.base_freq * 1.003f);

    current_preset = idx;
}

void SetLfoShape(int idx)
{
    if(idx < 0 || idx >= 8)
        return;

    lfo1.SetWaveform(kLfoWaveforms[idx]);
    lfo2.SetWaveform(kLfoWaveforms[idx]);

    // Special handling for sync modes
    if(idx == 6) // B7: Sync Slow
    {
        lfo1.SetFreq(0.05f);
        lfo2.SetFreq(0.05f);
    }
    else if(idx == 7) // B8: Sync Fast
    {
        lfo1.SetFreq(0.5f);
        lfo2.SetFreq(0.5f);
    }

    current_lfo_shape = idx;
}

void ProcessKnobs()
{
    // Read raw knob values
    float k1 = hw.knob[0].Process(); // Bank Mix
    float k2 = hw.knob[1].Process(); // VarShape Morph
    float k3 = hw.knob[2].Process(); // LFO1 Rate
    float k4 = hw.knob[3].Process(); // LFO2 Rate
    float k5 = hw.knob[4].Process(); // LP Cutoff
    float k6 = hw.knob[5].Process(); // HP Cutoff
    float k7 = hw.knob[6].Process(); // Resonance
    float k8 = hw.knob[7].Process(); // LFO Depth

    // Map to parameter ranges
    params.bank_mix  = k1;
    params.var_morph = k2;
    params.lfo1_rate = 0.01f + k3 * 1.99f;  // 0.01 - 2.0 Hz
    params.lfo2_rate = 0.01f + k4 * 4.99f;  // 0.01 - 5.0 Hz
    params.lp_cutoff = 200.f + k5 * 7800.f; // 200 - 8000 Hz
    params.hp_cutoff = 20.f + k6 * 1980.f;  // 20 - 2000 Hz
    params.resonance = k7 * 0.9f;           // 0.0 - 0.9
    params.lfo_depth = k8;                  // 0.0 - 1.0

    // Update LFO rates (unless in sync mode)
    if(current_lfo_shape < 6)
    {
        lfo1.SetFreq(params.lfo1_rate);
        lfo2.SetFreq(params.lfo2_rate);
    }

    // Update variable oscillator shape
    varOsc.SetPW(params.var_morph);
    varOsc.SetWaveshape(params.var_morph);

    // Update filter resonance
    svfLP.SetRes(params.resonance);
    svfHP.SetRes(params.resonance * 0.5f); // HP gets less resonance
}

// ============================================================================
// AUDIO CALLBACK
// ============================================================================

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    // Smooth parameters to avoid clicks
    smoothed.bank_mix
        = Smooth(smoothed.bank_mix, params.bank_mix, kSmoothCoeff);
    smoothed.lp_cutoff
        = Smooth(smoothed.lp_cutoff, params.lp_cutoff, kSmoothCoeff);
    smoothed.hp_cutoff
        = Smooth(smoothed.hp_cutoff, params.hp_cutoff, kSmoothCoeff);

    // Update filter cutoffs with smoothed values
    svfLP.SetFreq(smoothed.lp_cutoff);
    svfHP.SetFreq(smoothed.hp_cutoff);

    for(size_t i = 0; i < size; i++)
    {
        // Process LFOs
        float lfo1_out = lfo1.Process();
        float lfo2_out = lfo2.Process();

        // Use frozen position if freeze enabled
        float morph_mod = freeze_enabled ? frozen_morph_pos : lfo1_out;
        float level_mod = freeze_enabled ? 1.0f : (0.7f + 0.3f * lfo2_out);

        // Process oscillator bank
        oscBank.SetAmplitudes(nullptr); // Use default even amplitudes
        float bank_out = oscBank.Process();

        // Process variable shape oscillator
        float var_out = varOsc.Process();

        // Crossfade between sources with LFO modulation (depth controlled by K8)
        float mix_pos = smoothed.bank_mix + morph_mod * params.lfo_depth * 0.4f;
        mix_pos       = fclamp(mix_pos, 0.0f, 1.0f);
        float mixed   = Lerp(var_out, bank_out, mix_pos);

        // Apply level modulation (depth controlled by K8)
        mixed *= level_mod * (0.5f + 0.5f * params.lfo_depth);

        // Process through lowpass filter
        svfLP.Process(mixed);
        float lp_out = svfLP.Low();

        // Process through highpass filter (always active - bypass by setting K6 to min)
        svfHP.Process(lp_out);
        float hp_out = svfHP.High();

        // Mute output if stopped
        float final_out = audio_running ? tanhf(hp_out * 2.0f) * 0.8f : 0.0f;

        // Stereo output (identical for now)
        out[0][i] = final_out;
        out[1][i] = final_out;
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

    // Initialize UI helpers
    keyLeds.Init(&hw);
    display.Init(&hw);
    display.SetTitle("Drone Lab");

    // Set parameter labels
    display.SetLabel(0, "BankMix");
    display.SetLabel(1, "Morph");
    display.SetLabel(2, "LFO1 Rt");
    display.SetLabel(3, "LFO2 Rt");
    display.SetLabel(4, "LP Cut");
    display.SetLabel(5, "HP Cut");
    display.SetLabel(6, "Reson");
    display.SetLabel(7, "LFO Dep");

    // Initialize Oscillator Bank
    oscBank.Init(sr);
    float amplitudes[8] = {1.0f, 0.7f, 0.5f, 0.35f, 0.25f, 0.18f, 0.12f, 0.08f};
    oscBank.SetAmplitudes(amplitudes);
    oscBank.SetFreq(kPresets[0].base_freq);

    // Initialize Variable Shape Oscillator
    varOsc.Init(sr);
    varOsc.SetFreq(kPresets[0].base_freq * 1.003f);
    varOsc.SetSync(false);
    varOsc.SetSyncFreq(1.0f);
    varOsc.SetPW(0.5f);
    varOsc.SetWaveshape(0.5f);

    // Initialize LFOs
    lfo1.Init(sr);
    lfo1.SetFreq(0.1f);
    lfo1.SetWaveform(Oscillator::WAVE_SIN);
    lfo1.SetAmp(1.0f);

    lfo2.Init(sr);
    lfo2.SetFreq(0.2f);
    lfo2.SetWaveform(Oscillator::WAVE_SIN);
    lfo2.SetAmp(1.0f);

    // Initialize Filters with preset values to avoid corner cases
    const DronePreset& initPreset = kPresets[0];

    svfLP.Init(sr);
    svfLP.SetFreq(initPreset.lp_cutoff);
    svfLP.SetRes(initPreset.resonance);
    svfLP.SetDrive(0.0f);

    svfHP.Init(sr);
    svfHP.SetFreq(initPreset.hp_cutoff);
    svfHP.SetRes(initPreset.resonance * 0.5f);
    svfHP.SetDrive(0.0f);

    // Initialize smoothed params to preset values (prevents startup transients)
    smoothed.bank_mix  = initPreset.bank_mix;
    smoothed.lp_cutoff = initPreset.lp_cutoff;
    smoothed.hp_cutoff = initPreset.hp_cutoff;

    // Also sync the params struct
    params.bank_mix  = initPreset.bank_mix;
    params.lp_cutoff = initPreset.lp_cutoff;
    params.hp_cutoff = initPreset.hp_cutoff;
    params.resonance = initPreset.resonance;

    // Load initial preset (sets oscillator frequencies)
    LoadPreset(0);
    keyLeds.SetA(0, true); // Highlight A1
    keyLeds.SetB(0, true); // Highlight B1 (sine LFO)

    hw.StartAdc();
    hw.StartAudio(AudioCallback);

    while(1)
    {
        // Process controls in main loop
        hw.ProcessAllControls();
        ProcessKnobs();

        // Row A: Drone preset selection
        for(int i = 0; i < 8; i++)
        {
            if(hw.KeyboardRisingEdge(kKeyAIndices[i]))
            {
                // Clear all A LEDs
                for(int j = 0; j < 8; j++)
                    keyLeds.SetA(j, false);

                // Load new preset
                LoadPreset(i);
                keyLeds.SetA(i, true);
            }
        }

        // Row B: LFO shape selection
        for(int i = 0; i < 8; i++)
        {
            if(hw.KeyboardRisingEdge(kKeyBIndices[i]))
            {
                // Clear all B LEDs
                for(int j = 0; j < 8; j++)
                    keyLeds.SetB(j, false);

                // Set new LFO shape
                SetLfoShape(i);
                keyLeds.SetB(i, true);
            }
        }

        // SW1: Freeze toggle
        if(hw.sw[0].RisingEdge())
        {
            freeze_enabled = !freeze_enabled;
            if(freeze_enabled)
            {
                // Capture current LFO position
                frozen_morph_pos = lfo1.Process();
            }
        }

        // SW2: Start/Stop toggle
        if(hw.sw[1].RisingEdge())
        {
            audio_running = !audio_running;
        }

        // Update keyboard LEDs
        keyLeds.Update();

        // Update display
        for(int i = 0; i < 8; i++)
            display.SetValue(i, hw.knob[i].Process());

        // Custom display content
        hw.display.Fill(false);

        // Title with preset name
        char title_buf[32];
        snprintf(title_buf,
                 sizeof(title_buf),
                 "Drone: %s",
                 kPresets[current_preset].name);
        hw.display.SetCursor(0, 0);
        hw.display.WriteString(title_buf, Font_6x8, true);

        // Show active parameter
        int active_param = display.GetActiveParam();
        if(active_param >= 0)
        {
            const char* labels[8] = {"Bank Mix",
                                     "Morph",
                                     "LFO1 Rate",
                                     "LFO2 Rate",
                                     "LP Cutoff",
                                     "HP Cutoff",
                                     "Resonance",
                                     "LFO Depth"};

            char buf[32];
            hw.display.SetCursor(0, 10);
            snprintf(buf, sizeof(buf), "> %s", labels[active_param]);
            hw.display.WriteString(buf, Font_6x8, true);

            hw.display.SetCursor(0, 20);
            snprintf(buf, sizeof(buf), "%.2f", hw.knob[active_param].Process());
            hw.display.WriteString(buf, Font_11x18, true);
        }

        // Status line
        char status[32];
        snprintf(status,
                 sizeof(status),
                 "LFO:%d %s %s",
                 current_lfo_shape + 1,
                 freeze_enabled ? "FRZ" : "RUN",
                 audio_running ? "PLAY" : "STOP");
        hw.display.SetCursor(0, 54);
        hw.display.WriteString(status, Font_6x8, true);

        hw.display.Update();

        // Update switch LEDs
        hw.led_driver.SetLed(kLedSwitches[0], freeze_enabled ? 1.0f : 0.0f);
        hw.led_driver.SetLed(kLedSwitches[1], audio_running ? 1.0f : 0.0f);

        // Update knob LEDs
        for(int i = 0; i < 8; i++)
            hw.led_driver.SetLed(kLedKnobs[i], hw.knob[i].Value());

        // NOTE: SwapBuffersAndTransmit is called by keyLeds.Update()

        System::Delay(16);
    }
}
