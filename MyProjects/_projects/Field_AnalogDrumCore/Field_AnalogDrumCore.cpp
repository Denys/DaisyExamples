#include "daisy_field.h"
#include "daisysp.h"
#include <cstdio>

using namespace daisy;
using namespace daisysp;

DaisyField hw;

AnalogBassDrum  kick;
AnalogSnareDrum snare;
Overdrive       drive;
Metro           stepClock;

constexpr int kNumSteps = 16;

bool kickPattern[kNumSteps];
bool snarePattern[kNumSteps];

enum EditLane
{
    EDIT_KICK = 0,
    EDIT_SNARE
};

volatile bool  seqPlaying  = true;
volatile int   currentStep = -1;
volatile float tempoBpm    = 122.0f;
volatile float stepHz      = (122.0f / 60.0f) * 4.0f;

volatile float kickTone     = 0.65f;
volatile float kickDecay    = 0.55f;
volatile float kickAttackFm = 0.35f;
volatile float snareTone    = 0.55f;
volatile float snareDecay   = 0.45f;
volatile float snareSnappy  = 0.60f;
volatile float masterDrive  = 0.35f;

EditLane editLane = EDIT_KICK;

// ── OLED zoom state ────────────────────────────────────────────────────────
float    kz_prev[8]  = {};
int      kz_idx      = -1;
uint32_t kz_time     = 0;
constexpr uint32_t kZoomMs    = 1400;
constexpr float    kZoomDelta = 0.015f;

void InitPattern()
{
    for(int i = 0; i < kNumSteps; i++)
    {
        kickPattern[i]  = false;
        snarePattern[i] = false;
    }

    // Classic 4-on-floor starter groove.
    kickPattern[0]  = true;
    kickPattern[4]  = true;
    kickPattern[8]  = true;
    kickPattern[12] = true;

    snarePattern[4]  = true;
    snarePattern[12] = true;
}

void ToggleStep(int step)
{
    if(step < 0 || step >= kNumSteps)
        return;

    if(editLane == EDIT_KICK)
        kickPattern[step] = !kickPattern[step];
    else
        snarePattern[step] = !snarePattern[step];
}

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

    kickTone     = k1;
    kickDecay    = 0.05f + (k2 * 0.90f);
    kickAttackFm = k3;
    snareTone    = k4;
    snareDecay   = 0.05f + (k5 * 0.95f);
    snareSnappy  = k6;
    tempoBpm     = 60.0f + (k7 * 180.0f);
    stepHz       = (tempoBpm / 60.0f) * 4.0f;
    masterDrive  = k8;
}

void HandleUiEvents()
{
    if(hw.sw[0].RisingEdge())
    {
        editLane = (editLane == EDIT_KICK) ? EDIT_SNARE : EDIT_KICK;
    }

    if(hw.sw[1].RisingEdge())
    {
        seqPlaying = !seqPlaying;
        if(!seqPlaying)
            currentStep = -1;
    }

    // Field keyboard indices: A row = 0..7, B row = 8..15.
    for(int i = 0; i < 8; i++)
    {
        if(hw.KeyboardRisingEdge(i))
            ToggleStep(i);

        if(hw.KeyboardRisingEdge(i + 8))
            ToggleStep(i + 8);
    }
}

void UpdateLeds()
{
    for(int i = 0; i < 8; i++)
    {
        const bool onA = (editLane == EDIT_KICK) ? kickPattern[i] : snarePattern[i];
        const bool onB = (editLane == EDIT_KICK) ? kickPattern[i + 8] : snarePattern[i + 8];

        float aBrightness = onA ? 0.45f : 0.02f;
        float bBrightness = onB ? 0.45f : 0.02f;

        if(currentStep == i)
            aBrightness = 1.0f;
        if(currentStep == i + 8)
            bBrightness = 1.0f;

        hw.led_driver.SetLed(DaisyField::LED_KEY_A1 - i, aBrightness); // A-row reversed: A1=15..A8=8
        hw.led_driver.SetLed(DaisyField::LED_KEY_B1 + i, bBrightness);
    }

    for(int i = 0; i < 8; i++)
    {
        hw.led_driver.SetLed(DaisyField::LED_KNOB_1 + i, hw.knob[i].Value());
    }

    hw.led_driver.SetLed(DaisyField::LED_SW_1, editLane == EDIT_KICK ? 0.20f : 1.0f);
    hw.led_driver.SetLed(DaisyField::LED_SW_2, seqPlaying ? 1.0f : 0.20f);

    hw.led_driver.SwapBuffersAndTransmit();
}

static const char* kAdcKnobNames[8] = {
    "Kick Tone", "Kick Decay", "Attack FM",
    "Snare Tone", "Snr Decay", "Snappy",
    "Tempo", "Drive"
};

void DrawZoom()
{
    char val[32];
    float v = hw.knob[kz_idx].Value();
    switch(kz_idx)
    {
        case 1: snprintf(val, 32, "%.0f ms", (0.05f + v * 0.90f) * 1000.0f); break;
        case 4: snprintf(val, 32, "%.0f ms", (0.05f + v * 0.95f) * 1000.0f); break;
        case 6: snprintf(val, 32, "%.0f BPM", 60.0f + v * 180.0f); break;
        default: snprintf(val, 32, "%d%%", (int)(v * 100.0f + 0.5f)); break;
    }
    hw.display.Fill(false);
    hw.display.SetCursor(0, 0);
    hw.display.WriteString(kAdcKnobNames[kz_idx], Font_7x10, true);
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
        float v = hw.knob[i].Value();
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
    char line[32];

    // Row 0: title + BPM
    snprintf(line, sizeof(line), "DRUMCORE  BPM:%d", (int)(tempoBpm + 0.5f));
    hw.display.SetCursor(0, 0);
    hw.display.WriteString(line, Font_6x8, true);

    // Row 1: lane + play state
    snprintf(line, sizeof(line), "Lane:%s  Run:%d",
             (editLane == EDIT_KICK) ? "Kick " : "Snare", seqPlaying ? 1 : 0);
    hw.display.SetCursor(0, 8);
    hw.display.WriteString(line, Font_6x8, true);

    // Row 2: kick params
    snprintf(line, sizeof(line), "KT:%d AF:%d KD:%d%%",
             (int)(kickTone * 100.0f + 0.5f),
             (int)(kickAttackFm * 100.0f + 0.5f),
             (int)(kickDecay * 100.0f + 0.5f));
    hw.display.SetCursor(0, 16);
    hw.display.WriteString(line, Font_6x8, true);

    // Row 3: snare params
    snprintf(line, sizeof(line), "ST:%d SS:%d  SD:%d%%",
             (int)(snareTone * 100.0f + 0.5f),
             (int)(snareSnappy * 100.0f + 0.5f),
             (int)(snareDecay * 100.0f + 0.5f));
    hw.display.SetCursor(0, 24);
    hw.display.WriteString(line, Font_6x8, true);

    // Row 4: drive + step counter
    snprintf(line, sizeof(line), "Drive:%d%%  Step:%02d",
             (int)(masterDrive * 100.0f + 0.5f), currentStep + 1);
    hw.display.SetCursor(0, 32);
    hw.display.WriteString(line, Font_6x8, true);

    // Pattern grid: 16 steps × 8px = 128px wide at y=42..50
    const bool* pat = (editLane == EDIT_KICK) ? kickPattern : snarePattern;
    for(int i = 0; i < kNumSteps; i++)
    {
        const int x  = i * 8;
        hw.display.DrawRect(x, 42, x + 6, 50, pat[i], true);
        if(currentStep == i)
            hw.display.DrawRect(x - 1, 41, x + 7, 51, true, false);
    }

    hw.display.Update();
}

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    // Callback owns DSP object state updates.
    kick.SetTone(kickTone);
    kick.SetDecay(kickDecay);
    kick.SetAttackFmAmount(kickAttackFm);

    snare.SetTone(snareTone);
    snare.SetDecay(snareDecay);
    snare.SetSnappy(snareSnappy);

    stepClock.SetFreq(stepHz);
    drive.SetDrive(masterDrive);

    for(size_t i = 0; i < size; i++)
    {
        bool trigKick  = false;
        bool trigSnare = false;

        if(seqPlaying && stepClock.Process())
        {
            int nextStep = currentStep + 1;
            if(nextStep >= kNumSteps)
                nextStep = 0;

            currentStep = nextStep;
            trigKick    = kickPattern[nextStep];
            trigSnare   = snarePattern[nextStep];
        }

        const float kickSig  = kick.Process(trigKick);
        const float snareSig = snare.Process(trigSnare);

        const float dry = (kickSig * 0.90f) + (snareSig * 0.75f);
        const float wet = drive.Process(dry);
        float       sig = (0.65f * dry) + (0.55f * wet);

        sig = fclamp(sig, -0.95f, 0.95f);

        out[0][i] = sig;
        out[1][i] = sig;
    }
}

int main(void)
{
    hw.Init();
    hw.SetAudioBlockSize(48);
    const float sampleRate = hw.AudioSampleRate();

    kick.Init(sampleRate);
    kick.SetFreq(52.0f);
    kick.SetAccent(0.75f);

    snare.Init(sampleRate);
    snare.SetFreq(185.0f);
    snare.SetAccent(0.70f);

    drive.Init();
    stepClock.Init(stepHz, sampleRate);

    InitPattern();

    hw.StartAdc();
    hw.StartAudio(AudioCallback);

    while(1)
    {
        hw.ProcessAllControls();
        HandleUiEvents();
        ProcessControls();
        UpdateLeds();
        UpdateDisplay();
        System::Delay(2);
    }
}
