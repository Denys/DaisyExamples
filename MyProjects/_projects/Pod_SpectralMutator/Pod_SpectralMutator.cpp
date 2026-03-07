#include "daisy_pod.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

DaisyPod hw;
Svf      focus_filter_l;
Svf      focus_filter_r;
Svf      mode_filter_l;
Svf      mode_filter_r;
Overdrive drive_l;
Overdrive drive_r;
DelayLine<float, 2048> comb_l;
DelayLine<float, 2048> comb_r;
Oscillator              motion_lfo;

enum ParamPage
{
    PAGE_FOCUS = 0,
    PAGE_MOTION,
    PAGE_COLOR,
    PAGE_COUNT
};

ParamPage current_page = PAGE_FOCUS;
int       current_mode = 0;
bool      hold_motion  = false;
float     sample_rate_hz = 48000.0f;
float     held_motion_value = 0.0f;

float focus_freq   = 0.35f;
float focus_res    = 0.25f;
float motion_depth = 0.30f;
float motion_rate  = 0.20f;
float color_amount = 0.25f;
float wet_mix      = 0.35f;

float smooth_focus_freq   = 0.35f;
float smooth_focus_res    = 0.25f;
float smooth_motion_depth = 0.30f;
float smooth_motion_rate  = 0.20f;
float smooth_color_amount = 0.25f;
float smooth_wet_mix      = 0.35f;

void ProcessControls();
void UpdateLedState();
void ProcessBaseChain(float in_l, float in_r, float& out_l, float& out_r);
void ProcessSpectralMode(float in_l, float in_r, float& out_l, float& out_r);
float GetMotionValue();

static inline float Clamp01(float x)
{
    return fclamp(x, 0.0f, 1.0f);
}

static inline float FocusFreqHz(float raw)
{
    return 80.0f * powf(250.0f, Clamp01(raw));
}

static inline float MotionRateHz(float raw)
{
    return 0.05f * powf(80.0f, Clamp01(raw));
}

static inline size_t CombDelaySamples(float raw)
{
    const float delay_ms = 3.0f + 18.0f * Clamp01(raw);
    const float samples  = (delay_ms * 0.001f) * sample_rate_hz;
    return static_cast<size_t>(fclamp(samples, 1.0f, 2047.0f));
}

static inline float SmoothToward(float current, float target, float coeff)
{
    return current + coeff * (target - current);
}

void ProcessControls()
{
    const int inc = hw.encoder.Increment();
    if(inc != 0)
    {
        int next = static_cast<int>(current_page) + inc;
        next     = (next % PAGE_COUNT + PAGE_COUNT) % PAGE_COUNT;
        current_page = static_cast<ParamPage>(next);
    }

    if(hw.button1.RisingEdge())
        current_mode = (current_mode + 1) & 0x3;

    if(hw.button2.RisingEdge())
        hold_motion = !hold_motion;

    const float k1 = hw.knob1.Process();
    const float k2 = hw.knob2.Process();

    switch(current_page)
    {
        case PAGE_FOCUS:
            focus_freq = Clamp01(k1);
            focus_res  = Clamp01(k2);
            break;

        case PAGE_MOTION:
            motion_depth = Clamp01(k1);
            motion_rate  = Clamp01(k2);
            break;

        case PAGE_COLOR:
            color_amount = Clamp01(k1);
            wet_mix      = Clamp01(k2);
            break;

        default: break;
    }
}

void UpdateLedState()
{
    switch(current_page)
    {
        case PAGE_FOCUS:
            hw.led1.Set(0.0f, 0.6f, 0.9f);
            break;

        case PAGE_MOTION:
            hw.led1.Set(0.9f, 0.4f, 0.0f);
            break;

        case PAGE_COLOR:
            hw.led1.Set(0.8f, 0.0f, 0.8f);
            break;

        default:
            hw.led1.Set(0.0f, 0.0f, 0.0f);
            break;
    }

    if(hold_motion)
    {
        hw.led2.Set(0.0f, 0.0f, 1.0f);
    }
    else
    {
        const float mode_level = 0.2f + 0.2f * static_cast<float>(current_mode);
        hw.led2.Set(mode_level, mode_level, 0.0f);
    }
}

float GetMotionValue()
{
    motion_lfo.SetFreq(MotionRateHz(smooth_motion_rate));

    if(hold_motion)
        return held_motion_value;

    held_motion_value = motion_lfo.Process();
    return held_motion_value;
}

void ProcessBaseChain(float in_l, float in_r, float& out_l, float& out_r)
{
    const float input_trim = fclamp(0.70f + 0.20f * (1.0f - smooth_color_amount), 0.55f, 0.90f);
    const float cutoff     = FocusFreqHz(smooth_focus_freq);
    const float resonance  = fclamp(0.10f + 0.75f * smooth_focus_res, 0.10f, 0.90f);
    const float drive_amt  = fclamp(0.08f + 0.72f * smooth_color_amount, 0.05f, 0.90f);
    const float mix        = Clamp01(smooth_wet_mix);

    focus_filter_l.SetFreq(cutoff);
    focus_filter_r.SetFreq(cutoff);
    focus_filter_l.SetRes(resonance);
    focus_filter_r.SetRes(resonance);
    drive_l.SetDrive(drive_amt);
    drive_r.SetDrive(drive_amt);

    const float dry_l = in_l;
    const float dry_r = in_r;
    const float trim_l = dry_l * input_trim;
    const float trim_r = dry_r * input_trim;

    focus_filter_l.Process(trim_l);
    focus_filter_r.Process(trim_r);

    const float focused_l = (0.70f * focus_filter_l.Low()) + (0.30f * focus_filter_l.Band());
    const float focused_r = (0.70f * focus_filter_r.Low()) + (0.30f * focus_filter_r.Band());
    float       mode_l    = 0.0f;
    float       mode_r    = 0.0f;
    ProcessSpectralMode(focused_l, focused_r, mode_l, mode_r);

    const float colored_l = drive_l.Process(mode_l);
    const float colored_r = drive_r.Process(mode_r);

    out_l = fclamp((dry_l * (1.0f - mix)) + (colored_l * mix), -1.0f, 1.0f);
    out_r = fclamp((dry_r * (1.0f - mix)) + (colored_r * mix), -1.0f, 1.0f);
}

void ProcessSpectralMode(float in_l, float in_r, float& out_l, float& out_r)
{
    const float motion    = GetMotionValue();
    const float mode_freq = fclamp(FocusFreqHz(smooth_focus_freq)
                                       * (1.10f + smooth_color_amount
                                          + smooth_motion_depth * 0.35f * motion),
                                   80.0f,
                                   sample_rate_hz / 3.1f);
    const float mode_res  = fclamp(0.15f + 0.70f * smooth_focus_res, 0.15f, 0.92f);

    mode_filter_l.SetFreq(mode_freq);
    mode_filter_r.SetFreq(mode_freq);
    mode_filter_l.SetRes(mode_res);
    mode_filter_r.SetRes(mode_res);

    mode_filter_l.Process(in_l);
    mode_filter_r.Process(in_r);

    switch(current_mode & 0x3)
    {
        case 0: // Notch
            out_l = 0.55f * in_l + 0.45f * mode_filter_l.Notch();
            out_r = 0.55f * in_r + 0.45f * mode_filter_r.Notch();
            break;

        case 1: // Bandpass
            out_l = mode_filter_l.Band();
            out_r = mode_filter_r.Band();
            break;

        case 2: // Peak / formant-ish emphasis
            out_l = 0.50f * in_l + 0.50f * mode_filter_l.Peak();
            out_r = 0.50f * in_r + 0.50f * mode_filter_r.Peak();
            break;

        case 3: // Comb
        default:
        {
            const float feedback = fclamp(0.20f + 0.55f * smooth_color_amount, 0.20f, 0.78f);
            comb_l.SetDelay(CombDelaySamples(smooth_focus_freq + smooth_motion_depth * 0.15f * motion));
            comb_r.SetDelay(CombDelaySamples(smooth_focus_freq * 0.85f + 0.10f
                                             + smooth_motion_depth * 0.12f * motion));
            const float delayed_l = comb_l.Read();
            const float delayed_r = comb_r.Read();
            comb_l.Write(in_l + delayed_l * feedback);
            comb_r.Write(in_r + delayed_r * feedback);
            out_l = 0.65f * in_l + 0.35f * delayed_l;
            out_r = 0.65f * in_r + 0.35f * delayed_r;
            break;
        }
    }
}

void AudioCallback(AudioHandle::InputBuffer in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    hw.ProcessAllControls();
    ProcessControls();
    smooth_focus_freq   = SmoothToward(smooth_focus_freq, focus_freq, 0.08f);
    smooth_focus_res    = SmoothToward(smooth_focus_res, focus_res, 0.08f);
    smooth_motion_depth = SmoothToward(smooth_motion_depth, motion_depth, 0.08f);
    smooth_motion_rate  = SmoothToward(smooth_motion_rate, motion_rate, 0.08f);
    smooth_color_amount = SmoothToward(smooth_color_amount, color_amount, 0.08f);
    smooth_wet_mix      = SmoothToward(smooth_wet_mix, wet_mix, 0.08f);

    for(size_t i = 0; i < size; ++i)
    {
        float wet_l = 0.0f;
        float wet_r = 0.0f;
        ProcessBaseChain(in[0][i], in[1][i], wet_l, wet_r);
        out[0][i] = wet_l;
        out[1][i] = wet_r;
    }
}

int main(void)
{
    hw.Init();
    hw.SetAudioBlockSize(48);

    sample_rate_hz = hw.AudioSampleRate();
    const float sample_rate = sample_rate_hz;
    focus_filter_l.Init(sample_rate);
    focus_filter_r.Init(sample_rate);
    mode_filter_l.Init(sample_rate);
    mode_filter_r.Init(sample_rate);
    focus_filter_l.SetFreq(FocusFreqHz(focus_freq));
    focus_filter_r.SetFreq(FocusFreqHz(focus_freq));
    mode_filter_l.SetFreq(FocusFreqHz(focus_freq));
    mode_filter_r.SetFreq(FocusFreqHz(focus_freq));
    focus_filter_l.SetRes(0.10f + 0.80f * focus_res);
    focus_filter_r.SetRes(0.10f + 0.80f * focus_res);
    mode_filter_l.SetRes(0.15f + 0.75f * focus_res);
    mode_filter_r.SetRes(0.15f + 0.75f * focus_res);
    focus_filter_l.SetDrive(0.0f);
    focus_filter_r.SetDrive(0.0f);
    mode_filter_l.SetDrive(0.0f);
    mode_filter_r.SetDrive(0.0f);
    drive_l.Init();
    drive_r.Init();
    comb_l.Init();
    comb_r.Init();
    motion_lfo.Init(sample_rate);
    motion_lfo.SetWaveform(Oscillator::WAVE_SIN);
    motion_lfo.SetAmp(1.0f);
    motion_lfo.SetFreq(MotionRateHz(smooth_motion_rate));
    comb_l.SetDelay(CombDelaySamples(focus_freq));
    comb_r.SetDelay(CombDelaySamples(focus_freq));
    drive_l.SetDrive(0.08f + 0.72f * color_amount);
    drive_r.SetDrive(0.08f + 0.72f * color_amount);

    hw.StartAdc();
    hw.StartAudio(AudioCallback);

    for(;;)
    {
        UpdateLedState();
        hw.UpdateLeds();
        System::Delay(1);
    }
}
