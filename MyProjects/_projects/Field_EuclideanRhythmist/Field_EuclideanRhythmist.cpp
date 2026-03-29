#include "daisy_field.h"
#include "daisysp.h"
#include <cmath>
#include <cstdio>

using namespace daisy;
using namespace daisysp;

// Hardware
DaisyField hw;

// Constants
const int kMaxSteps  = 16;
const int kNumVoices = 8;

// Keyboard indices: A-row = 0..7 (A1=0..A8=7), B-row = 8..15 (B1=8..B8=15)
const int kKeyAIndices[8] = {0, 1, 2, 3, 4, 5, 6, 7};
const int kKeyBIndices[8] = {8, 9, 10, 11, 12, 13, 14, 15};

// Voice names
const char* kVoiceNames[8]
    = {"Kick", "Snare", "Clap", "CHat", "OHat", "Tom", "Rim", "Cow"};

// Simple Euclidean pattern using Bresenham
static void BuildPattern(int k, int n, bool* out)
{
    for(int i = 0; i < kMaxSteps; i++)
        out[i] = false;
    if(n <= 0 || k <= 0)
        return;
    if(k >= n)
    {
        for(int i = 0; i < n; i++)
            out[i] = true;
        return;
    }
    for(int i = 0; i < k; i++)
    {
        out[(i * n) / k] = true;
    }
}

// SEPARATE drum modules (not bundled in one giant struct!)
AnalogBassDrum     kick;
SyntheticSnareDrum snare;
HiHat<>            closed_hat;
HiHat<>            open_hat;
// Simple noise-based for clap, tom, rim, cowbell
WhiteNoise noise_src;
Oscillator tom_osc, rim_osc, cow_osc;
Svf        clap_filt, cow_filt;
AdEnv      clap_env, tom_env, tom_pitch, rim_env, cow_env;

// Per-voice data (minimal - just pattern params and state)
struct VoiceData
{
    int   k = 4, n = 16, rot = 0;
    float decay = 0.5f, tone = 0.5f;
    bool  muted = false;
    bool  pattern[kMaxSteps];

    void UpdatePattern()
    {
        bool base[kMaxSteps];
        BuildPattern(k, n, base);
        for(int i = 0; i < kMaxSteps; i++)
        {
            int src    = (i - rot + n) % n;
            pattern[i] = (i < n) ? base[src] : false;
        }
    }

    bool Hit(int step) const
    {
        if(n <= 0)
            return false;
        return pattern[step % n];
    }
};

VoiceData voices[kNumVoices];
int       selected_voice = 0;

// Trigger specific drum
void TrigVoice(int v)
{
    if(voices[v].muted)
        return;
    float d = voices[v].decay;
    float t = voices[v].tone;
    switch(v)
    {
        case 0: kick.Trig(); break;
        case 1: snare.Trig(); break;
        case 2: clap_env.Trigger(); break;
        case 3: closed_hat.Trig(); break;
        case 4: open_hat.Trig(); break;
        case 5:
            tom_env.Trigger();
            tom_pitch.Trigger();
            break;
        case 6: rim_env.Trigger(); break;
        case 7: cow_env.Trigger(); break;
    }
}

// Process specific drum
float ProcessVoice(int v)
{
    switch(v)
    {
        case 0: return kick.Process();
        case 1: return snare.Process();
        case 2:
            clap_filt.Process(noise_src.Process());
            return clap_filt.Band() * clap_env.Process();
        case 3: return closed_hat.Process();
        case 4: return open_hat.Process();
        case 5:
        {
            float p = tom_pitch.Process();
            tom_osc.SetFreq(80.0f + p * 60.0f);
            return tom_osc.Process() * tom_env.Process();
        }
        case 6: return rim_osc.Process() * rim_env.Process();
        case 7:
            cow_filt.Process(cow_osc.Process());
            return cow_filt.Band() * cow_env.Process();
    }
    return 0.0f;
}

// ── OLED zoom state ────────────────────────────────────────────────────────
float    kz_prev[8]  = {};
int      kz_idx      = -1;
uint32_t kz_time     = 0;
constexpr uint32_t kZoomMs    = 1400;
constexpr float    kZoomDelta = 0.015f;

// Sequencer state
float    seq_sr               = 48000.0f;
float    seq_tempo            = 120.0f;
float    seq_swing            = 0.0f;
bool     seq_playing          = true;
uint32_t seq_step             = 0;
uint32_t seq_samples_per_step = 6000;
uint32_t seq_sample_count     = 0;

// Audio callback - minimal, fast
void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    for(size_t i = 0; i < size; i++)
    {
        float mix_l = 0.0f, mix_r = 0.0f;

        if(seq_playing)
        {
            seq_sample_count++;
            float    mult     = ((seq_step % 2) == 0) ? (1.0f - seq_swing)
                                                      : (1.0f + seq_swing);
            uint32_t step_len = (uint32_t)(seq_samples_per_step * mult);
            if(step_len < 1)
                step_len = 1;

            if(seq_sample_count >= step_len)
            {
                seq_sample_count = 0;
                for(int v = 0; v < kNumVoices; v++)
                {
                    if(voices[v].Hit(seq_step))
                        TrigVoice(v);
                }
                seq_step = (seq_step + 1) % kMaxSteps;
            }
        }

        // Mix voices: Kick/Snare/Tom center-left, hats/clap/rim/cow center-right
        float sig0 = ProcessVoice(0);
        mix_l += sig0;
        mix_r += sig0 * 0.3f;
        float sig1 = ProcessVoice(1);
        mix_l += sig1;
        mix_r += sig1 * 0.3f;
        float sig2 = ProcessVoice(2);
        mix_r += sig2;
        mix_l += sig2 * 0.3f;
        float sig3 = ProcessVoice(3);
        mix_r += sig3;
        mix_l += sig3 * 0.3f;
        float sig4 = ProcessVoice(4);
        mix_r += sig4;
        mix_l += sig4 * 0.3f;
        float sig5 = ProcessVoice(5);
        mix_l += sig5;
        mix_r += sig5 * 0.3f;
        float sig6 = ProcessVoice(6);
        mix_r += sig6;
        mix_l += sig6 * 0.3f;
        float sig7 = ProcessVoice(7);
        mix_r += sig7;
        mix_l += sig7 * 0.3f;

        out[0][i] = mix_l * 0.4f;
        out[1][i] = mix_r * 0.4f;
    }
}

static const char* kEucKnobNames[8] = {
    "Pulses", "Length", "Rotation",
    "Decay", "Tone", "---",
    "Tempo", "Swing"
};

// Knob cache for zoom detection (updated after each hw.knob[i].Process() call)
float kvals_eu[8] = {};

void DrawZoom()
{
    char val[32];
    float v = kvals_eu[kz_idx];
    switch(kz_idx)
    {
        case 0: snprintf(val, 32, "%d pulses", (int)(v * 16.0f + 0.5f)); break;
        case 1: snprintf(val, 32, "%d steps", 1 + (int)(v * 15.0f)); break;
        case 6: snprintf(val, 32, "%.0f BPM", 40.0f + v * 200.0f); break;
        case 7: snprintf(val, 32, "%d%%", (int)(v * 50.0f + 0.5f)); break;
        default: snprintf(val, 32, "%d%%", (int)(v * 100.0f + 0.5f)); break;
    }
    hw.display.Fill(false);
    hw.display.SetCursor(0, 0);
    hw.display.WriteString(kEucKnobNames[kz_idx], Font_7x10, true);
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
        float v = kvals_eu[i];
        if(fabsf(v - kz_prev[i]) > kZoomDelta)
        {
            kz_idx     = i;
            kz_time    = System::GetNow();
            kz_prev[i] = v;
        }
    }
}

void UpdateDisplay(const VoiceData& v, char* buf)
{
    CheckKnobs();
    if(kz_idx >= 0 && (System::GetNow() - kz_time) < kZoomMs)
    {
        DrawZoom();
        return;
    }
    kz_idx = -1;

    hw.display.Fill(false);

    snprintf(buf, 32, "%s %d %s",
             kVoiceNames[selected_voice],
             selected_voice + 1,
             seq_playing ? ">" : "||");
    hw.display.SetCursor(0, 0);
    hw.display.WriteString(buf, Font_7x10, true);

    snprintf(buf, 32, "K:%d N:%d R:%d", v.k, v.n, v.rot);
    hw.display.SetCursor(0, 12);
    hw.display.WriteString(buf, Font_6x8, true);

    snprintf(buf, 32, "BPM:%.0f Sw:%.0f%%", seq_tempo, seq_swing * 100.0f);
    hw.display.SetCursor(0, 22);
    hw.display.WriteString(buf, Font_6x8, true);

    const int step_disp = (v.n > 0) ? ((int)(seq_step % v.n) + 1) : 1;
    snprintf(buf, 32, "Step:%d/%d  Dec:%d%% T:%d%%",
             step_disp, v.n,
             (int)(v.decay * 100.0f + 0.5f),
             (int)(v.tone * 100.0f + 0.5f));
    hw.display.SetCursor(0, 32);
    hw.display.WriteString(buf, Font_6x8, true);

    // Euclidean ring diagram
    const int cx = 100, cy = 40, rad = 18;
    if(v.n > 0)
    {
        for(int i = 0; i < v.n; i++)
        {
            float angle = (float)i / v.n * 6.28318f - 1.5708f;
            int   px    = cx + (int)(cosf(angle) * rad);
            int   py    = cy + (int)(sinf(angle) * rad);
            int   cur   = (int)(seq_step % v.n);
            if(i == cur)
                hw.display.DrawCircle(px, py, 3, true);
            else if(v.pattern[i])
                hw.display.DrawCircle(px, py, 2, true);
            else
                hw.display.DrawPixel(px, py, true);
        }
    }

    hw.display.Update();
}

int main(void)
{
    hw.Init();
    hw.SetAudioBlockSize(48);
    seq_sr               = hw.AudioSampleRate();
    seq_samples_per_step = (uint32_t)(seq_sr * 60.0f / (120.0f * 4.0f));

    // Init drums
    kick.Init(seq_sr);
    snare.Init(seq_sr);
    closed_hat.Init(seq_sr);
    closed_hat.SetDecay(0.05f);
    open_hat.Init(seq_sr);
    open_hat.SetDecay(0.3f);
    noise_src.Init();
    tom_osc.Init(seq_sr);
    tom_osc.SetWaveform(Oscillator::WAVE_SIN);
    rim_osc.Init(seq_sr);
    rim_osc.SetWaveform(Oscillator::WAVE_SIN);
    rim_osc.SetFreq(900.0f);
    cow_osc.Init(seq_sr);
    cow_osc.SetWaveform(Oscillator::WAVE_SQUARE);
    cow_osc.SetFreq(560.0f);
    clap_filt.Init(seq_sr);
    clap_filt.SetFreq(1500.0f);
    clap_filt.SetRes(0.5f);
    cow_filt.Init(seq_sr);
    cow_filt.SetFreq(1200.0f);
    cow_filt.SetRes(0.2f);
    clap_env.Init(seq_sr);
    clap_env.SetTime(ADENV_SEG_ATTACK, 0.001f);
    clap_env.SetTime(ADENV_SEG_DECAY, 0.1f);
    tom_env.Init(seq_sr);
    tom_env.SetTime(ADENV_SEG_ATTACK, 0.001f);
    tom_env.SetTime(ADENV_SEG_DECAY, 0.3f);
    tom_pitch.Init(seq_sr);
    tom_pitch.SetTime(ADENV_SEG_ATTACK, 0.001f);
    tom_pitch.SetTime(ADENV_SEG_DECAY, 0.1f);
    rim_env.Init(seq_sr);
    rim_env.SetTime(ADENV_SEG_ATTACK, 0.001f);
    rim_env.SetTime(ADENV_SEG_DECAY, 0.03f);
    cow_env.Init(seq_sr);
    cow_env.SetTime(ADENV_SEG_ATTACK, 0.001f);
    cow_env.SetTime(ADENV_SEG_DECAY, 0.3f);

    // Init voice patterns
    for(int i = 0; i < kNumVoices; i++)
        voices[i].UpdatePattern();

    hw.StartAdc();
    hw.StartAudio(AudioCallback);

    char buf[32] = {};

    while(1)
    {
        hw.ProcessAllControls();

        // Read knobs — also populate kvals_eu for zoom detection
        float k0 = hw.knob[0].Process(); kvals_eu[0] = k0; // Pulses
        float k1 = hw.knob[1].Process(); kvals_eu[1] = k1; // Length
        float k2 = hw.knob[2].Process(); kvals_eu[2] = k2; // Rotation
        float k3 = hw.knob[3].Process(); kvals_eu[3] = k3; // Decay
        float k4 = hw.knob[4].Process(); kvals_eu[4] = k4; // Tone
        float k5 = hw.knob[5].Process(); kvals_eu[5] = k5; // (unused)
        float k6 = hw.knob[6].Process(); kvals_eu[6] = k6; // Tempo
        float k7 = hw.knob[7].Process(); kvals_eu[7] = k7; // Swing

        // Update tempo
        seq_tempo            = 40.0f + k6 * 200.0f;
        seq_samples_per_step = (uint32_t)(seq_sr * 60.0f / (seq_tempo * 4.0f));
        if(seq_samples_per_step < 1)
            seq_samples_per_step = 1;
        seq_swing = k7 * 0.5f;

        // Transport
        if(hw.sw[0].RisingEdge())
        {
            seq_playing = !seq_playing;
            if(!seq_playing)
            {
                seq_step         = 0;
                seq_sample_count = 0;
            }
        }

        // Voice selection
        for(int i = 0; i < 8; i++)
        {
            if(hw.KeyboardRisingEdge(kKeyAIndices[i]))
                selected_voice = i;
        }
        // Mute toggle
        for(int i = 0; i < 8; i++)
        {
            if(hw.KeyboardRisingEdge(kKeyBIndices[i]))
                voices[i].muted = !voices[i].muted;
        }

        // Update selected voice
        VoiceData& v     = voices[selected_voice];
        int        new_n = 1 + (int)(k1 * 15);
        if(new_n < 1)
            new_n = 1;
        if(new_n > 16)
            new_n = 16;
        int new_k = (int)(k0 * new_n);
        if(new_k < 0)
            new_k = 0;
        if(new_k > new_n)
            new_k = new_n;
        int new_rot = (new_n > 1) ? (int)(k2 * (new_n - 1)) : 0;

        v.k     = new_k;
        v.n     = new_n;
        v.rot   = new_rot;
        v.decay = k3;
        v.tone  = k4;
        v.UpdatePattern();

        // Update drum params based on selected voice's decay/tone
        kick.SetDecay(v.decay);
        snare.SetDecay(v.decay);
        snare.SetSnappy(v.tone);
        clap_env.SetTime(ADENV_SEG_DECAY, 0.05f + v.decay * 0.2f);
        clap_filt.SetFreq(800.0f + v.tone * 2000.0f);
        closed_hat.SetDecay(0.01f + v.decay * 0.1f);
        open_hat.SetDecay(0.1f + v.decay * 0.5f);
        tom_env.SetTime(ADENV_SEG_DECAY, 0.1f + v.decay * 0.4f);

        // LEDs
        for(int i = 0; i < 8; i++)
        {
            hw.led_driver.SetLed(DaisyField::LED_KEY_A1 + i,
                                 (i == selected_voice) ? 1.0f : 0.1f);
            hw.led_driver.SetLed(DaisyField::LED_KEY_B1 + i,
                                 voices[i].muted ? 0.0f : 0.3f);
        }
        for(int i = 0; i < 8; i++)
        {
            hw.led_driver.SetLed(DaisyField::LED_KNOB_1 + i,
                                 hw.knob[i].Value());
        }
        hw.led_driver.SetLed(DaisyField::LED_SW_1, seq_playing ? 1.0f : 0.0f);
        hw.led_driver.SwapBuffersAndTransmit();

        // OLED — zoom on knob move, else overview
        UpdateDisplay(v, buf);
        System::Delay(10);
    }
}
