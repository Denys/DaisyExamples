#include "daisy_field.h"
#include "daisysp.h"
#include "noisetoaster_modes.h"
#include <cmath>
#include <cstdio>

using namespace daisy;
using namespace daisysp;
using namespace noisetoaster;

DaisyField hw;

Oscillator vco;
Oscillator lfo;
WhiteNoise noise;
Svf        vcf;
AdEnv      ar_env;

namespace
{
constexpr float kChromaticNotes[8] = {48.0f, 50.0f, 52.0f, 53.0f, 55.0f, 57.0f, 59.0f, 60.0f};
constexpr const char* kNoteLabels[8] = {"C3", "D3", "E3", "F3", "G3", "A3", "B3", "C4"};
constexpr const char* kKnobLongNames[8] = {"VCO Freq",
                                            "VCO LFO Depth",
                                            "VCO AREG Depth",
                                            "VCF Cutoff",
                                            "VCF Resonance",
                                            "VCF Mod Depth",
                                            "AREG Attack",
                                            "AREG Release"};

constexpr float    kFixedLfoRateHz      = 2.2f;
constexpr float    kFixedOutputLevel    = 0.72f;
constexpr float    kFixedNoiseBlend     = 0.18f;
constexpr float    kKnobMoveThreshold   = 0.015f;
constexpr uint32_t kFocusTimeoutMs      = 1400;
constexpr uint32_t kBlinkPeriodMs       = 250;
constexpr float    kPitchLfoRangeOct    = 0.35f;
constexpr float    kPitchAregRangeOct   = 0.75f;
constexpr float    kFilterModRangeHz    = 6000.0f;
constexpr float    kDefaultKnobValues[8] = {0.5f, 0.25f, 0.35f, 0.50f, 0.10f, 0.40f, 0.05f, 0.28f};

volatile VcoWave      vco_wave       = VcoWave::Saw;
volatile LfoWave      lfo_wave       = LfoWave::Sine;
volatile VcfModSource vcf_mod_source = VcfModSource::Lfo;

volatile bool repeat_mode        = false;
volatile bool vca_bypass         = false;
volatile bool note_armed         = false;
volatile bool ar_trigger_pending = false;
volatile bool vco_reset_pending  = false;
volatile int  active_key         = -1;

volatile float note_hz        = 110.0f;
volatile float knob_values[8] = {};
float          prev_knobs[8]  = {};
int            focus_knob     = -1;
uint32_t       focus_start    = 0;

VcoWave applied_vco_wave = VcoWave::Saw;
LfoWave applied_lfo_wave = LfoWave::Sine;

float Clamp01(float v)
{
    return fclamp(v, 0.0f, 1.0f);
}

bool BlinkPhase()
{
    return ((System::GetNow() / kBlinkPeriodMs) % 2) == 0;
}

float AttackTimeFromKnob(float value)
{
    const float v = Clamp01(value);
    return 0.002f + (v * v * 1.20f);
}

float ReleaseTimeFromKnob(float value)
{
    const float v = Clamp01(value);
    return 0.010f + (v * v * 1.80f);
}

float BaseCutoffHz(float value)
{
    const float v = Clamp01(value);
    return 50.0f + (v * v * 8000.0f);
}

void ApplyVcoWave(VcoWave mode)
{
    switch(mode)
    {
        case VcoWave::Saw: vco.SetWaveform(Oscillator::WAVE_POLYBLEP_SAW); break;
        case VcoWave::Square: vco.SetWaveform(Oscillator::WAVE_POLYBLEP_SQUARE); break;
        case VcoWave::Triangle:
        default: vco.SetWaveform(Oscillator::WAVE_POLYBLEP_TRI); break;
    }
}

void ApplyLfoWave(LfoWave mode)
{
    switch(mode)
    {
        case LfoWave::Sine: lfo.SetWaveform(Oscillator::WAVE_SIN); break;
        case LfoWave::Square: lfo.SetWaveform(Oscillator::WAVE_SQUARE); break;
        case LfoWave::Triangle:
        default: lfo.SetWaveform(Oscillator::WAVE_TRI); break;
    }
}

void QueueAregTrigger()
{
    if(!note_armed)
        return;

    ar_trigger_pending = true;
    vco_reset_pending  = true;
}

void ArmAndTriggerKey(int key_index)
{
    active_key = key_index;
    note_hz    = mtof(kChromaticNotes[key_index]);
    note_armed = true;
    QueueAregTrigger();
}

void Panic()
{
    note_armed         = false;
    active_key         = -1;
    ar_trigger_pending = false;
    vco_reset_pending  = false;
}

void PrimeKnobDefaults()
{
    for(int i = 0; i < 8; i++)
    {
        knob_values[i] = kDefaultKnobValues[i];
        prev_knobs[i]  = knob_values[i];
    }
}

void SnapshotKnobs()
{
    for(int i = 0; i < 8; i++)
    {
        knob_values[i] = hw.knob[i].Value();
        if(fabsf(knob_values[i] - prev_knobs[i]) > kKnobMoveThreshold)
        {
            focus_knob   = i;
            focus_start  = System::GetNow();
            prev_knobs[i] = knob_values[i];
        }
    }
}

void FormatFocusValue(int index, char* buffer, size_t size)
{
    const float value = Clamp01(knob_values[index]);

    switch(index)
    {
        case 0:
        {
            const float semitones = (value - 0.5f) * 24.0f;
            snprintf(buffer, size, "%+.1f st", semitones);
        }
        break;

        case 1:
        case 2:
        case 4:
        case 5: snprintf(buffer, size, "%d%%", (int)(value * 100.0f + 0.5f)); break;

        case 3: snprintf(buffer, size, "%.0f Hz", BaseCutoffHz(value)); break;

        case 6: snprintf(buffer, size, "%.0f ms", AttackTimeFromKnob(value) * 1000.0f); break;

        case 7: snprintf(buffer, size, "%.0f ms", ReleaseTimeFromKnob(value) * 1000.0f); break;

        default: snprintf(buffer, size, "%d%%", (int)(value * 100.0f + 0.5f)); break;
    }
}

void DrawOverview()
{
    char line[32];
    char note[8];

    snprintf(note, sizeof(note), "%s", (active_key >= 0) ? kNoteLabels[active_key] : "---");

    hw.display.Fill(false);

    hw.display.SetCursor(0, 0);
    hw.display.WriteString("MFOS NOISE TOASTER", Font_6x8, true);

    snprintf(line,
             sizeof(line),
             "V:%s L:%s F:%s",
             ShortName(vco_wave),
             ShortName(lfo_wave),
             ShortName(vcf_mod_source));
    hw.display.SetCursor(0, 8);
    hw.display.WriteString(line, Font_6x8, true);

    snprintf(line,
             sizeof(line),
             "N:%s REP:%d BYP:%d",
             note,
             repeat_mode ? 1 : 0,
             vca_bypass ? 1 : 0);
    hw.display.SetCursor(0, 16);
    hw.display.WriteString(line, Font_6x8, true);

    snprintf(line,
             sizeof(line),
             "%s:%02d %s:%02d",
             KnobLabel(0),
             (int)(Clamp01(knob_values[0]) * 99.0f + 0.5f),
             KnobLabel(1),
             (int)(Clamp01(knob_values[1]) * 99.0f + 0.5f));
    hw.display.SetCursor(0, 24);
    hw.display.WriteString(line, Font_6x8, true);

    snprintf(line,
             sizeof(line),
             "%s:%02d %s:%02d",
             KnobLabel(2),
             (int)(Clamp01(knob_values[2]) * 99.0f + 0.5f),
             KnobLabel(3),
             (int)(Clamp01(knob_values[3]) * 99.0f + 0.5f));
    hw.display.SetCursor(0, 32);
    hw.display.WriteString(line, Font_6x8, true);

    snprintf(line,
             sizeof(line),
             "%s:%02d %s:%02d",
             KnobLabel(4),
             (int)(Clamp01(knob_values[4]) * 99.0f + 0.5f),
             KnobLabel(5),
             (int)(Clamp01(knob_values[5]) * 99.0f + 0.5f));
    hw.display.SetCursor(0, 40);
    hw.display.WriteString(line, Font_6x8, true);

    snprintf(line,
             sizeof(line),
             "%s:%02d %s:%02d",
             KnobLabel(6),
             (int)(Clamp01(knob_values[6]) * 99.0f + 0.5f),
             KnobLabel(7),
             (int)(Clamp01(knob_values[7]) * 99.0f + 0.5f));
    hw.display.SetCursor(0, 48);
    hw.display.WriteString(line, Font_6x8, true);

    snprintf(line, sizeof(line), "LFO:%.1fHz OUT:%d", kFixedLfoRateHz, (int)(kFixedOutputLevel * 100.0f + 0.5f));
    hw.display.SetCursor(0, 56);
    hw.display.WriteString(line, Font_6x8, true);

    hw.display.Update();
}

void DrawFocusedParameter()
{
    char title[32];
    char value[32];
    char line[32];
    const float raw = Clamp01(knob_values[focus_knob]);

    snprintf(title, sizeof(title), "%s", kKnobLongNames[focus_knob]);
    FormatFocusValue(focus_knob, value, sizeof(value));

    hw.display.Fill(false);
    hw.display.SetCursor(0, 0);
    hw.display.WriteString(title, Font_7x10, true);
    hw.display.SetCursor(0, 18);
    hw.display.WriteString(value, Font_11x18, true);

    snprintf(line,
             sizeof(line),
             "V:%s L:%s F:%s",
             ShortName(vco_wave),
             ShortName(lfo_wave),
             ShortName(vcf_mod_source));
    hw.display.SetCursor(0, 44);
    hw.display.WriteString(line, Font_6x8, true);

    const int bar_width = (int)(raw * 127.0f);
    hw.display.DrawRect(0, 54, 127, 62, true, false);
    if(bar_width > 0)
        hw.display.DrawRect(0, 54, bar_width, 62, true, true);

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

void UpdateLeds()
{
    const bool blink = BlinkPhase();

    for(int i = 0; i < 8; i++)
    {
        hw.led_driver.SetLed(KeyALedId(i), (note_armed && active_key == i) ? 1.0f : 0.0f);
        hw.led_driver.SetLed(DaisyField::LED_KNOB_1 + i, Clamp01(knob_values[i]));
    }

    hw.led_driver.SetLed(DaisyField::LED_KEY_B1 + 0,
                         LedBrightnessForThreeState(ThreeStateForVcoWave(vco_wave), blink));
    hw.led_driver.SetLed(DaisyField::LED_KEY_B1 + 1,
                         LedBrightnessForThreeState(ThreeStateForLfoWave(lfo_wave), blink));
    hw.led_driver.SetLed(DaisyField::LED_KEY_B1 + 2,
                         LedBrightnessForThreeState(ThreeStateForVcfModSource(vcf_mod_source), blink));
    hw.led_driver.SetLed(DaisyField::LED_KEY_B1 + 3, repeat_mode ? 1.0f : 0.0f);
    hw.led_driver.SetLed(DaisyField::LED_KEY_B1 + 4, vca_bypass ? 1.0f : 0.0f);

    for(int i = 5; i < 8; i++)
        hw.led_driver.SetLed(DaisyField::LED_KEY_B1 + i, 0.0f);

    hw.led_driver.SetLed(DaisyField::LED_SW_1, note_armed ? 0.35f : 0.0f);
    hw.led_driver.SetLed(DaisyField::LED_SW_2, 0.0f);
    hw.led_driver.SwapBuffersAndTransmit();
}

void UpdateControls()
{
    hw.ProcessAllControls();
    SnapshotKnobs();

    for(int i = 0; i < 8; i++)
    {
        if(hw.KeyboardRisingEdge(i))
            ArmAndTriggerKey(i);
    }

    if(hw.KeyboardRisingEdge(8))
        vco_wave = AdvanceVcoWave(vco_wave);

    if(hw.KeyboardRisingEdge(9))
        lfo_wave = AdvanceLfoWave(lfo_wave);

    if(hw.KeyboardRisingEdge(10))
    {
        vcf_mod_source = AdvanceVcfModSource(vcf_mod_source);
    }

    if(hw.KeyboardRisingEdge(11))
        repeat_mode = !repeat_mode;

    if(hw.KeyboardRisingEdge(12))
    {
        vca_bypass = !vca_bypass;
    }

    if(hw.sw[0].RisingEdge())
    {
        QueueAregTrigger();
    }

    if(hw.sw[1].RisingEdge())
    {
        Panic();
    }
}
} // namespace

static void AudioCallback(AudioHandle::InputBuffer  in,
                          AudioHandle::OutputBuffer out,
                          size_t                    size)
{
    (void)in;

    const VcoWave      desired_vco_wave = vco_wave;
    const LfoWave      desired_lfo_wave = lfo_wave;
    const VcfModSource filter_mod_src   = vcf_mod_source;
    const bool         armed            = note_armed;
    const bool         repeat           = repeat_mode;
    const bool         bypass           = vca_bypass;
    const float        armed_note_hz    = note_hz;

    const float coarse_tune   = Clamp01(knob_values[0]);
    const float vco_lfo_depth = Clamp01(knob_values[1]);
    const float vco_ar_depth  = Clamp01(knob_values[2]);
    const float cutoff_knob   = Clamp01(knob_values[3]);
    const float resonance     = Clamp01(knob_values[4]);
    const float vcf_mod_depth = Clamp01(knob_values[5]);
    const float attack_time   = AttackTimeFromKnob(knob_values[6]);
    const float release_time  = ReleaseTimeFromKnob(knob_values[7]);

    if(desired_vco_wave != applied_vco_wave)
    {
        ApplyVcoWave(desired_vco_wave);
        applied_vco_wave = desired_vco_wave;
    }

    if(desired_lfo_wave != applied_lfo_wave)
    {
        ApplyLfoWave(desired_lfo_wave);
        applied_lfo_wave = desired_lfo_wave;
    }

    ar_env.SetTime(ADENV_SEG_ATTACK, attack_time);
    ar_env.SetTime(ADENV_SEG_DECAY, release_time);
    lfo.SetFreq(kFixedLfoRateHz);

    for(size_t i = 0; i < size; i++)
    {
        if(ShouldQueueRepeatTrigger(armed, repeat, ar_env.IsRunning(), ar_trigger_pending))
            QueueAregTrigger();

        if(ar_trigger_pending)
        {
            if(vco_reset_pending)
            {
                vco.Reset();
                vco_reset_pending = false;
            }

            ar_env.Trigger();
            ar_trigger_pending = false;
        }

        const float lfo_sig = lfo.Process();
        const float ar      = ar_env.Process();

        float tuned_note = armed_note_hz;
        tuned_note *= std::pow(2.0f, (coarse_tune - 0.5f) * 2.0f);
        tuned_note *= std::pow(2.0f,
                               (lfo_sig * vco_lfo_depth * kPitchLfoRangeOct)
                                   + (ar * vco_ar_depth * kPitchAregRangeOct));
        tuned_note = fclamp(tuned_note, 20.0f, 8000.0f);
        vco.SetFreq(tuned_note);

        const float osc_sample   = vco.Process();
        const float noise_sample = noise.Process();
        const float source       = ((1.0f - kFixedNoiseBlend) * osc_sample)
                             + (kFixedNoiseBlend * noise_sample);

        float cutoff_mod = 0.0f;
        switch(filter_mod_src)
        {
            case VcfModSource::Lfo:
                cutoff_mod = (0.5f * (lfo_sig + 1.0f)) * vcf_mod_depth * kFilterModRangeHz;
                break;
            case VcfModSource::Areg: cutoff_mod = ar * vcf_mod_depth * kFilterModRangeHz; break;
            case VcfModSource::Off:
            default: cutoff_mod = 0.0f; break;
        }

        vcf.SetFreq(fclamp(BaseCutoffHz(cutoff_knob) + cutoff_mod, 40.0f, 12000.0f));
        vcf.SetRes(0.05f + 0.92f * resonance);
        vcf.Process(source);

        const float amp = armed ? (bypass ? 1.0f : ar) : 0.0f;
        float       sig = vcf.Low() * amp * kFixedOutputLevel;
        sig             = fclamp(sig, -0.95f, 0.95f);

        out[0][i] = sig;
        out[1][i] = sig;
    }
}

int main(void)
{
    hw.Init();
    hw.SetAudioBlockSize(48);
    PrimeKnobDefaults();

    const float sr = hw.AudioSampleRate();

    vco.Init(sr);
    vco.SetAmp(0.82f);
    ApplyVcoWave(vco_wave);

    lfo.Init(sr);
    lfo.SetAmp(1.0f);
    ApplyLfoWave(lfo_wave);
    lfo.SetFreq(kFixedLfoRateHz);

    noise.Init();

    vcf.Init(sr);
    vcf.SetDrive(0.35f);
    vcf.SetRes(0.18f);

    ar_env.Init(sr);
    ar_env.SetMin(0.0f);
    ar_env.SetMax(1.0f);
    ar_env.SetCurve(-18.0f);
    ar_env.SetTime(ADENV_SEG_ATTACK, AttackTimeFromKnob(knob_values[6]));
    ar_env.SetTime(ADENV_SEG_DECAY, ReleaseTimeFromKnob(knob_values[7]));

    hw.seed.StartLog(true);
    hw.seed.PrintLine("Field_MFOS_NoiseToaster ready");
    hw.seed.PrintLine("A1-A8 note select+trigger | B1 VCO | B2 LFO | B3 VCF src");
    hw.seed.PrintLine("B4 repeat | B5 bypass | SW1 manual gate | SW2 panic");
    hw.seed.PrintLine("Fixed internal: LFO %.1f Hz | OUT %.0f%% | Noise %.0f%%",
                      kFixedLfoRateHz,
                      kFixedOutputLevel * 100.0f,
                      kFixedNoiseBlend * 100.0f);

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
