/**
 * Pod_MarkovAIDrummer
 *
 * Daisy Pod generative drum sketch using a lightweight Markov trigger model.
 *
 * Signal:
 *   Metro-style clock -> Markov state logic -> AnalogBassDrum + HiHat
 *
 * Controls:
 * - Encoder Press: Cycle pages (Time -> AI -> Tone)
 * - Button 1: Play/Stop
 * - Button 2: Reseed Markov generator / reset chain history
 * - Knob 1/2:
 *   Page 1 (Time): Tempo / Swing
 *   Page 2 (AI):   Kick Density / Hat Complexity
 *   Page 3 (Tone): Kick Decay / Hat Pitch
 */

#include "daisy_pod.h"
#include "daisysp.h"
#include <cstdint>

using namespace daisy;
using namespace daisysp;

DaisyPod hw;

AnalogBassDrum kick;
HiHat<>        hat;
Metro          step_clock;

enum ParamPage
{
    PAGE_TIME = 0,
    PAGE_AI   = 1,
    PAGE_TONE = 2,
    PAGE_COUNT
};

ParamPage active_page = PAGE_TIME;
bool      running     = true;

// Time/groove.
float tempo_bpm = 122.0f;
float swing_amt = 0.0f; // 0..0.75
bool  next_odd  = false;
int   step_idx  = -1;

// AI controls.
float kick_density   = 0.62f; // 0..1
float hat_complexity = 0.58f; // 0..1

// Tone controls.
float kick_decay = 0.42f;
float hat_pitch  = 5200.0f;

// Visual pulses.
volatile float beat_pulse = 0.0f;
volatile float kick_pulse = 0.0f;
volatile float hat_pulse  = 0.0f;

// Markov history: two-bit shift register (last two trigger results).
uint8_t kick_state = 0;
uint8_t hat_state  = 0;

// Small, fast RNG (xorshift32).
uint32_t rng_state = 0x4D595DF4u;

static inline float Clamp01(float x)
{
    return fclamp(x, 0.0f, 1.0f);
}

static inline float Rand01()
{
    rng_state ^= (rng_state << 13);
    rng_state ^= (rng_state >> 17);
    rng_state ^= (rng_state << 5);
    return static_cast<float>(rng_state & 0x00FFFFFFu) / 16777216.0f;
}

void ReseedGenerator()
{
    const uint32_t t = System::GetNow();
    rng_state         = 0xA341316Cu ^ (t * 1664525u + 1013904223u);
    kick_state        = 0;
    hat_state         = 0;
}

void UpdateClockForNextInterval()
{
    const float base_hz = (tempo_bpm / 60.0f) * 4.0f; // 16th notes
    const float factor  = next_odd ? (1.0f + 0.5f * swing_amt) : (1.0f - 0.5f * swing_amt);
    const float safe    = fmaxf(factor, 0.15f);
    step_clock.SetFreq(base_hz / safe);
}

float ComputeKickProbability()
{
    // Index = 2-bit Markov state from previous kick triggers.
    static const float k_base[4] = {0.84f, 0.23f, 0.67f, 0.39f};
    const float        markov     = k_base[kick_state & 0x3];
    const float        density    = 0.08f + (0.86f * kick_density);
    const float        accent     = ((step_idx & 0x3) == 0) ? 0.14f : 0.0f;
    return Clamp01((0.56f * markov) + (0.44f * density) + accent);
}

float ComputeHatProbability(bool kick_trig)
{
    static const float h_base[4] = {0.12f, 0.72f, 0.34f, 0.86f};
    const float        markov    = h_base[hat_state & 0x3];
    const float        density   = 0.06f + (0.88f * hat_complexity);
    const float        offbeat   = (step_idx & 1) ? (0.14f * hat_complexity) : 0.0f;
    float              p         = (0.60f * markov) + (0.40f * density) + offbeat;

    // Leave room for kick when complexity is low; allow overlap when high.
    if(kick_trig)
    {
        p *= (0.55f + (0.45f * hat_complexity));
    }
    return Clamp01(p);
}

void TriggerStep()
{
    step_idx = (step_idx + 1) & 0x0F;

    const float p_kick    = ComputeKickProbability();
    const bool  kick_trig = (Rand01() < p_kick);

    const float p_hat    = ComputeHatProbability(kick_trig);
    const bool  hat_trig = (Rand01() < p_hat);

    if(kick_trig)
    {
        kick.SetDecay(kick_decay);
        kick.Trig();
        kick_pulse = 1.0f;
    }

    if(hat_trig)
    {
        // Small random variation keeps hats from sounding static.
        const float var = 0.85f + 0.30f * Rand01();
        hat.SetFreq(hat_pitch * var);
        hat.SetDecay(0.03f + 0.22f * hat_complexity);
        hat.Trig();
        hat_pulse = 1.0f;
    }

    kick_state = static_cast<uint8_t>(((kick_state << 1) | (kick_trig ? 1 : 0)) & 0x3);
    hat_state  = static_cast<uint8_t>(((hat_state << 1) | (hat_trig ? 1 : 0)) & 0x3);
    beat_pulse = 1.0f;
}

void ProcessControls()
{
    hw.ProcessDigitalControls();
    hw.ProcessAnalogControls();

    if(hw.encoder.RisingEdge())
    {
        active_page = static_cast<ParamPage>((active_page + 1) % PAGE_COUNT);
        hw.seed.PrintLine("[PAGE] %d", static_cast<int>(active_page) + 1);
    }

    if(hw.button1.RisingEdge())
    {
        running = !running;
        hw.seed.PrintLine("[RUN] %s", running ? "ON" : "OFF");
    }

    if(hw.button2.RisingEdge())
    {
        ReseedGenerator();
        hw.seed.PrintLine("[AI] Reseed + history reset");
    }

    const float k1 = hw.knob1.Process();
    const float k2 = hw.knob2.Process();

    switch(active_page)
    {
        case PAGE_TIME:
            tempo_bpm = 70.0f + (140.0f * k1);
            swing_amt = 0.75f * k2;
            UpdateClockForNextInterval();
            break;

        case PAGE_AI:
            kick_density   = k1;
            hat_complexity = k2;
            break;

        case PAGE_TONE:
            kick_decay = 0.08f + (0.84f * k1);
            hat_pitch  = 1500.0f + (8500.0f * k2);
            break;

        default: break;
    }
}

void UpdateLeds()
{
    beat_pulse *= 0.90f;
    kick_pulse *= 0.88f;
    hat_pulse *= 0.88f;

    static const float page_color[PAGE_COUNT][3] = {
        {0.08f, 0.80f, 0.20f}, // Time: green
        {0.75f, 0.08f, 0.75f}, // AI: magenta
        {0.80f, 0.10f, 0.10f}, // Tone: red
    };

    const float pulse = 0.18f + (0.82f * beat_pulse);
    hw.led1.Set(page_color[active_page][0] * pulse,
                page_color[active_page][1] * pulse,
                page_color[active_page][2] * pulse);

    if(!running)
    {
        hw.led2.Set(0.15f, 0.02f, 0.02f);
    }
    else
    {
        hw.led2.Set(0.85f * kick_pulse, 0.02f, 0.90f * hat_pulse);
    }

    hw.UpdateLeds();
}

static void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                          AudioHandle::InterleavingOutputBuffer out,
                          size_t                                size)
{
    (void)in;

    for(size_t i = 0; i < size; i += 2)
    {
        if(running && step_clock.Process())
        {
            TriggerStep();
            next_odd = !next_odd;
            UpdateClockForNextInterval();
        }

        const float kick_sig = kick.Process();
        const float hat_sig  = hat.Process();

        float mix = (0.92f * kick_sig) + (0.54f * hat_sig);
        mix       = fclamp(mix, -0.95f, 0.95f);

        out[i]     = mix;
        out[i + 1] = mix;
    }
}

int main(void)
{
    hw.Init();
    hw.SetAudioBlockSize(48);
    hw.seed.StartLog(true);
    hw.seed.PrintLine("=== Pod_MarkovAIDrummer Boot ===");

    const float sr = hw.AudioSampleRate();

    kick.Init(sr);
    kick.SetFreq(52.0f);
    kick.SetTone(0.42f);
    kick.SetDecay(kick_decay);

    hat.Init(sr);
    hat.SetFreq(hat_pitch);
    hat.SetDecay(0.09f);

    step_clock.Init((tempo_bpm / 60.0f) * 4.0f, sr);
    UpdateClockForNextInterval();
    ReseedGenerator();

    hw.StartAdc();
    hw.StartAudio(AudioCallback);

    while(1)
    {
        ProcessControls();
        UpdateLeds();
        System::Delay(1);
    }
}
