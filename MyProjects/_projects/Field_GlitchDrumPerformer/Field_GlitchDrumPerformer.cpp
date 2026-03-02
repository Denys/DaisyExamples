#include "daisy_pod.h"
#include "daisysp.h"
#include <cmath>

using namespace daisy;
using namespace daisysp;

DaisyPod hw;

AnalogBassDrum     kick;
AnalogSnareDrum    snare;
Particle           particle;
GrainletOscillator grainlet;
Dust               dust;
AdEnv              grainEnv;
Overdrive          drive;
Metro              stepClock;

constexpr int kNumSteps = 8;

bool kickPattern[kNumSteps];
bool snarePattern[kNumSteps];

volatile bool  fillHeld      = false;
volatile int   currentStep   = -1;
volatile float stepPulse     = 0.0f;
volatile float tempoBpm      = 128.0f;
volatile int   selectedStep  = 0;
volatile int   parameterPage = 0;

volatile float kickTone    = 0.60f;
volatile float kickDecay   = 0.60f;
volatile float snareTone   = 0.55f;
volatile float snareSnappy = 0.70f;
volatile float texture     = 0.45f;
volatile float grainColor  = 0.50f;
volatile float masterDrive = 0.45f;

volatile float particleMix = 0.20f;
volatile float grainMix    = 0.12f;
volatile float driveBlend  = 0.55f;

float    prevKnob1       = 0.0f;
float    prevKnob2       = 0.0f;
bool     knobLogInit     = false;
uint32_t lastStatusLogMs = 0;

int WrapStep(int in)
{
    while(in < 0)
        in += kNumSteps;
    return in % kNumSteps;
}

void InitPattern()
{
    for(int i = 0; i < kNumSteps; i++)
    {
        kickPattern[i]  = false;
        snarePattern[i] = false;
    }

    kickPattern[0] = true;
    kickPattern[3] = true;
    kickPattern[4] = true;
    kickPattern[6] = true;

    snarePattern[2] = true;
    snarePattern[6] = true;
}

void LogStatus(bool force)
{
    const uint32_t now = System::GetNow();
    if(!force && (now - lastStatusLogMs) < 1000)
        return;

    lastStatusLogMs = now;
    hw.seed.PrintLine("[STATUS] step=%d edit=%d page=%d bpm=%d fill=%d",
                      currentStep + 1,
                      selectedStep + 1,
                      parameterPage + 1,
                      static_cast<int>(tempoBpm),
                      fillHeld ? 1 : 0);
}

void LogKnobs(float k1, float k2)
{
    if(!knobLogInit)
    {
        prevKnob1   = k1;
        prevKnob2   = k2;
        knobLogInit = true;
        return;
    }

    if(fabsf(k1 - prevKnob1) > 0.02f || fabsf(k2 - prevKnob2) > 0.02f)
    {
        prevKnob1 = k1;
        prevKnob2 = k2;
        hw.seed.PrintLine("[KNOB] page=%d k1=%d%% k2=%d%%",
                          parameterPage + 1,
                          static_cast<int>(k1 * 100.0f),
                          static_cast<int>(k2 * 100.0f));
    }
}

void UpdateEngineParameters(float k1, float k2)
{
    switch(parameterPage)
    {
        case 0:
            kickTone  = k1;
            kickDecay = 0.08f + (k2 * 0.90f);
            break;
        case 1:
            snareTone   = k1;
            snareSnappy = k2;
            break;
        case 2:
            texture    = k1;
            grainColor = k2;
            break;
        case 3:
            tempoBpm    = 70.0f + (k1 * 190.0f);
            masterDrive = k2;
            break;
    }

    kick.SetTone(kickTone);
    kick.SetDecay(kickDecay);
    kick.SetAttackFmAmount(0.25f + (kickDecay * 0.60f));

    snare.SetTone(snareTone);
    snare.SetSnappy(snareSnappy);
    snare.SetDecay(0.10f + (snareSnappy * 0.90f));

    particle.SetDensity(0.08f + (texture * 0.80f) + (fillHeld ? 0.55f : 0.0f));
    particle.SetFreq(180.0f + (grainColor * 4200.0f));
    particle.SetResonance(0.20f + (texture * 0.75f));
    particle.SetSpread(0.35f + (grainColor * 2.60f));
    particle.SetGain(0.35f + (texture * 0.65f));

    grainlet.SetFreq(110.0f + (grainColor * 660.0f));
    grainlet.SetFormantFreq(220.0f + (grainColor * 4600.0f));
    grainlet.SetShape(0.20f + (texture * 1.80f));
    grainlet.SetBleed(0.25f + (texture * 0.75f));

    particleMix = 0.04f + (texture * 0.32f);
    grainMix    = fillHeld ? (0.08f + (texture * 0.50f)) : (0.01f + (texture * 0.14f));
    driveBlend  = 0.20f + (masterDrive * 0.70f);

    stepClock.SetFreq((tempoBpm / 60.0f) * 4.0f);
    drive.SetDrive(masterDrive);
}

void ProcessControls()
{
    hw.ProcessDigitalControls();
    hw.ProcessAnalogControls();

    const int inc = hw.encoder.Increment();
    if(inc != 0)
    {
        selectedStep = WrapStep(selectedStep + inc);
        hw.seed.PrintLine("[ENC] step=%d", selectedStep + 1);
    }

    if(hw.encoder.RisingEdge())
    {
        parameterPage = (parameterPage + 1) % 4;
        hw.seed.PrintLine("[ENC] page=%d", parameterPage + 1);
    }

    const bool bothPressed = hw.button1.Pressed() && hw.button2.Pressed();
    fillHeld               = bothPressed;

    if(!bothPressed)
    {
        if(hw.button1.RisingEdge())
        {
            kickPattern[selectedStep] = !kickPattern[selectedStep];
            hw.seed.PrintLine("[BTN1] kick step=%d state=%d",
                              selectedStep + 1,
                              kickPattern[selectedStep] ? 1 : 0);
        }

        if(hw.button2.RisingEdge())
        {
            snarePattern[selectedStep] = !snarePattern[selectedStep];
            hw.seed.PrintLine("[BTN2] snare step=%d state=%d",
                              selectedStep + 1,
                              snarePattern[selectedStep] ? 1 : 0);
        }
    }

    const float k1 = hw.knob1.Process();
    const float k2 = hw.knob2.Process();
    UpdateEngineParameters(k1, k2);
    LogKnobs(k1, k2);
}

void UpdateLeds()
{
    if(stepPulse > 0.01f)
        stepPulse *= 0.91f;
    else
        stepPulse = 0.0f;

    static const float pageColor[4][3] = {
        {0.80f, 0.10f, 0.10f},
        {0.10f, 0.75f, 0.10f},
        {0.10f, 0.40f, 0.80f},
        {0.75f, 0.20f, 0.75f},
    };

    const float pulse = 0.04f + (stepPulse * 0.90f);
    hw.led1.Set(pageColor[parameterPage][0] * pulse,
                pageColor[parameterPage][1] * pulse,
                pageColor[parameterPage][2] * pulse);

    if(fillHeld)
    {
        hw.led2.Set(0.80f, 0.15f, 0.70f);
    }
    else
    {
        const float kickOn  = kickPattern[selectedStep] ? 0.80f : 0.06f;
        const float snareOn = snarePattern[selectedStep] ? 0.80f : 0.06f;
        hw.led2.Set(kickOn, 0.02f, snareOn);
    }

    hw.UpdateLeds();
}

static void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                          AudioHandle::InterleavingOutputBuffer out,
                          size_t                                size)
{
    for(size_t i = 0; i < size; i += 2)
    {
        bool trigKick  = false;
        bool trigSnare = false;

        if(stepClock.Process())
        {
            const int nextStep = (currentStep + 1) % kNumSteps;
            currentStep        = nextStep;
            stepPulse          = 1.0f;
            trigKick           = kickPattern[nextStep];
            trigSnare          = snarePattern[nextStep];
        }

        // Fill mode adds stochastic glitch events from Dust.
        if(fillHeld)
        {
            const float dustImpulse = dust.Process();
            if(dustImpulse > 0.55f)
                grainEnv.Trigger();
            if(dustImpulse > 0.90f)
                trigSnare = true;
            if(dustImpulse > 0.97f)
                trigKick = true;
        }

        const float kickSig     = kick.Process(trigKick);
        const float snareSig    = snare.Process(trigSnare);
        const float particleSig = particle.Process() * particleMix;
        const float grainSig    = grainlet.Process() * grainEnv.Process() * grainMix;

        const float dry    = (kickSig * 0.90f) + (snareSig * 0.75f);
        const float glitch = particleSig + grainSig;
        const float pre    = dry + glitch;

        const float driven = drive.Process(pre);
        float       sig    = ((1.0f - driveBlend) * pre) + (driveBlend * driven);

        sig = fclamp(sig, -0.95f, 0.95f);
        out[i]     = sig;
        out[i + 1] = sig;
    }
}

int main(void)
{
    hw.Init();
    hw.SetAudioBlockSize(48);
    const float sampleRate = hw.AudioSampleRate();

    // Block until serial terminal is connected.
    hw.seed.StartLog(true);
    hw.seed.PrintLine("=== GlitchDrumPerformer Pod Boot ===");
    hw.seed.PrintLine("[BOOT] block=%d samplerate=%d", 48, static_cast<int>(sampleRate));

    kick.Init(sampleRate);
    kick.SetFreq(53.0f);
    kick.SetAccent(0.75f);

    snare.Init(sampleRate);
    snare.SetFreq(190.0f);
    snare.SetAccent(0.70f);

    particle.Init(sampleRate);
    particle.SetSpread(1.5f);

    grainlet.Init(sampleRate);
    grainlet.SetBleed(0.60f);

    dust.Init();

    grainEnv.Init(sampleRate);
    grainEnv.SetTime(ADENV_SEG_ATTACK, 0.001f);
    grainEnv.SetTime(ADENV_SEG_DECAY, 0.08f);

    drive.Init();
    stepClock.Init((tempoBpm / 60.0f) * 4.0f, sampleRate);

    InitPattern();
    UpdateEngineParameters(0.60f, 0.60f);

    hw.StartAdc();
    hw.StartAudio(AudioCallback);
    LogStatus(true);

    while(1)
    {
        ProcessControls();
        UpdateLeds();
        LogStatus(false);
        System::Delay(1);
    }
}
