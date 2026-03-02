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

        hw.led_driver.SetLed(DaisyField::LED_KEY_A1 + i, aBrightness);
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

void UpdateDisplay()
{
    hw.display.Fill(false);

    char line[32];

    hw.display.SetCursor(0, 0);
    hw.display.WriteString((char*)"ANALOG DRUM CORE", Font_6x8, true);

    hw.display.SetCursor(0, 10);
    snprintf(line,
             sizeof(line),
             "Lane:%s  Run:%d",
             (editLane == EDIT_KICK) ? "Kick " : "Snare",
             seqPlaying ? 1 : 0);
    hw.display.WriteString(line, Font_6x8, true);

    const int bpmInt  = (int)(tempoBpm + 0.5f);
    const int stepInt = currentStep + 1;
    hw.display.SetCursor(0, 20);
    snprintf(line, sizeof(line), "BPM:%3d Step:%02d", bpmInt, stepInt);
    hw.display.WriteString(line, Font_6x8, true);

    const int kd = (int)(kickDecay * 100.0f + 0.5f);
    const int sd = (int)(snareDecay * 100.0f + 0.5f);
    hw.display.SetCursor(0, 30);
    snprintf(line, sizeof(line), "KD:%02d SD:%02d Drv:%02d", kd, sd, (int)(masterDrive * 99.0f));
    hw.display.WriteString(line, Font_6x8, true);

    // Show active edit lane pattern as 16 tiny blocks.
    hw.display.SetCursor(0, 42);
    hw.display.WriteString((char*)"Pattern:", Font_6x8, true);
    for(int i = 0; i < 16; i++)
    {
        const bool active = (editLane == EDIT_KICK) ? kickPattern[i] : snarePattern[i];
        const int  x      = 40 + (i * 5);
        const int  y0     = 44;
        const int  y1     = 50;

        hw.display.DrawRect(x, y0, x + 3, y1, active, true);
        if(currentStep == i)
            hw.display.DrawRect(x - 1, y0 - 1, x + 4, y1 + 1, true, false);
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
