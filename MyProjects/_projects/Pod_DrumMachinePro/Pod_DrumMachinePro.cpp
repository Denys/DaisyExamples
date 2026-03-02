/**
 * Pod_DrumMachinePro
 *
 * 6-voice drum machine with 16-step patterns.
 *
 * Platform: Daisy Pod
 *
 * Controls:
 * - Encoder turn: Select edit step (1-16)
 * - Encoder press: Cycle parameter pages
 * - Button 1: Toggle selected step for current drum
 * - Button 2: Cycle drum voice
 * - Knob 1/2: Edit parameters on active page
 *
 * Pages:
 * 1) Kick Decay / Snare Decay
 * 2) CHat Decay / OHat Decay
 * 3) Tom Tune / Clap Decay
 * 4) Tempo / Swing
 */

#include "daisy_pod.h"
#include "daisysp.h"
#include <cmath>

using namespace daisy;
using namespace daisysp;

DaisyPod hw;

// Drum voices
AnalogBassDrum     kick;
SyntheticSnareDrum snare;
HiHat<>            hihatClosed;
HiHat<>            hihatOpen;
SyntheticSnareDrum tom;
SyntheticSnareDrum clap;

// Sequencer state
constexpr int NUM_DRUMS    = 6;
constexpr int NUM_STEPS    = 16;
constexpr int NUM_PATTERNS = 1;

enum DrumVoice
{
    VOICE_KICK  = 0,
    VOICE_SNARE = 1,
    VOICE_CHAT  = 2,
    VOICE_OHAT  = 3,
    VOICE_TOM   = 4,
    VOICE_CLAP  = 5
};

const char* drumNames[NUM_DRUMS] = {
    "Kick", "Snare", "CHat", "OHat", "Tom", "Clap"
};

// Pattern memory: [pattern][drum][step]
bool patterns[NUM_PATTERNS][NUM_DRUMS][NUM_STEPS];

// Current state
int      currentPattern = 0;
DrumVoice currentDrum   = VOICE_KICK;
int      currentStep    = -1;
int      editStep       = 0;
int      paramPage      = 0;

// Timing
Metro         metro;
volatile float stepPulse = 0.0f;
float         tempo      = 120.0f;
float         swing      = 0.0f;
uint32_t      lastStepMs = 0;

// Drum parameters
float kickDecay  = 0.5f;
float snareDecay = 0.5f;
float chatDecay  = 0.2f;
float ohatDecay  = 0.5f;
float tomTune    = 200.0f;
float clapDecay  = 0.3f;

// Serial logging state
float    prevKnobVals[2] = {0.0f, 0.0f};
bool     knobLogInit     = false;
uint32_t lastStatusLogMs = 0;

int WrapStep(int in)
{
    while(in < 0)
        in += NUM_STEPS;
    return in % NUM_STEPS;
}

void InitDrums()
{
    const float sampleRate = hw.AudioSampleRate();

    kick.Init(sampleRate);
    kick.SetFreq(50.0f);
    kick.SetTone(0.5f);
    kick.SetDecay(kickDecay);

    snare.Init(sampleRate);
    snare.SetFreq(200.0f);
    snare.SetSnappy(0.5f);
    snare.SetDecay(snareDecay);

    hihatClosed.Init(sampleRate);
    hihatClosed.SetFreq(4000.0f);
    hihatClosed.SetDecay(chatDecay);

    hihatOpen.Init(sampleRate);
    hihatOpen.SetFreq(4000.0f);
    hihatOpen.SetDecay(ohatDecay);

    tom.Init(sampleRate);
    tom.SetFreq(tomTune);
    tom.SetSnappy(0.3f);
    tom.SetDecay(0.6f);

    clap.Init(sampleRate);
    clap.SetFreq(800.0f);
    clap.SetSnappy(0.8f);
    clap.SetDecay(clapDecay);

    metro.Init(tempo / 60.0f * 4.0f, sampleRate);
}

void TriggerDrum(DrumVoice drum)
{
    switch(drum)
    {
        case VOICE_KICK:
            kick.SetDecay(kickDecay);
            kick.Trig();
            break;
        case VOICE_SNARE:
            snare.SetDecay(snareDecay);
            snare.Trig();
            break;
        case VOICE_CHAT:
            hihatClosed.SetDecay(chatDecay);
            hihatClosed.Trig();
            break;
        case VOICE_OHAT:
            hihatOpen.SetDecay(ohatDecay);
            hihatOpen.Trig();
            break;
        case VOICE_TOM:
            tom.SetFreq(tomTune);
            tom.Trig();
            break;
        case VOICE_CLAP:
            clap.SetDecay(clapDecay);
            clap.Trig();
            break;
    }
}

void AdvanceSequencer()
{
    currentStep = (currentStep + 1) % NUM_STEPS;

    if((currentStep & 1) != 0 && swing > 0.0f)
    {
        const uint32_t swingDelayMs = static_cast<uint32_t>(swing * 16.0f);
        if(System::GetNow() - lastStepMs < swingDelayMs)
            return;
    }

    lastStepMs = System::GetNow();
    stepPulse  = 1.0f;

    for(int drum = 0; drum < NUM_DRUMS; drum++)
    {
        if(patterns[currentPattern][drum][currentStep])
            TriggerDrum(static_cast<DrumVoice>(drum));
    }
}

void LogKnobChanges(float k1, float k2)
{
    if(!knobLogInit)
    {
        prevKnobVals[0] = k1;
        prevKnobVals[1] = k2;
        knobLogInit     = true;
        return;
    }

    if(fabsf(k1 - prevKnobVals[0]) > 0.02f || fabsf(k2 - prevKnobVals[1]) > 0.02f)
    {
        prevKnobVals[0] = k1;
        prevKnobVals[1] = k2;
        hw.seed.PrintLine("[KNOB] page=%d k1=%d%% k2=%d%%",
                          paramPage + 1,
                          static_cast<int>(k1 * 100.0f),
                          static_cast<int>(k2 * 100.0f));
    }
}

void LogSystemStatus(bool force)
{
    const uint32_t now = System::GetNow();
    if(!force && (now - lastStatusLogMs) < 1000)
        return;

    lastStatusLogMs = now;
    hw.seed.PrintLine("[STATUS] drum=%s step=%02d play=%02d page=%d bpm=%d swing=%d%%",
                      drumNames[currentDrum],
                      currentStep + 1,
                      editStep + 1,
                      paramPage + 1,
                      static_cast<int>(tempo),
                      static_cast<int>(swing * 100.0f));
}

void ProcessControls()
{
    hw.ProcessDigitalControls();
    hw.ProcessAnalogControls();

    const int inc = hw.encoder.Increment();
    if(inc != 0)
    {
        editStep = WrapStep(editStep + inc);
        hw.seed.PrintLine("[ENC] edit-step=%02d", editStep + 1);
    }

    if(hw.encoder.RisingEdge())
    {
        paramPage = (paramPage + 1) % 4;
        hw.seed.PrintLine("[ENC] page=%d", paramPage + 1);
    }

    if(hw.button2.RisingEdge())
    {
        currentDrum = static_cast<DrumVoice>((currentDrum + 1) % NUM_DRUMS);
        hw.seed.PrintLine("[BTN2] drum=%s", drumNames[currentDrum]);
    }

    if(hw.button1.RisingEdge())
    {
        patterns[currentPattern][currentDrum][editStep]
            = !patterns[currentPattern][currentDrum][editStep];
        hw.seed.PrintLine("[BTN1] drum=%s step=%02d state=%d",
                          drumNames[currentDrum],
                          editStep + 1,
                          patterns[currentPattern][currentDrum][editStep] ? 1 : 0);
    }

    const float k1 = hw.knob1.Process();
    const float k2 = hw.knob2.Process();

    switch(paramPage)
    {
        case 0:
            kickDecay  = 0.1f + (k1 * 0.9f);
            snareDecay = 0.1f + (k2 * 0.9f);
            break;
        case 1:
            chatDecay = 0.05f + (k1 * 0.5f);
            ohatDecay = 0.1f + (k2 * 0.9f);
            break;
        case 2:
            tomTune   = 100.0f + (k1 * 300.0f);
            clapDecay = 0.1f + (k2 * 0.9f);
            break;
        case 3:
            tempo = 40.0f + (k1 * 200.0f);
            swing = k2 * 0.75f;
            metro.SetFreq(tempo / 60.0f * 4.0f);
            break;
    }

    LogKnobChanges(k1, k2);
}

void UpdateLeds()
{
    if(stepPulse > 0.01f)
        stepPulse *= 0.92f;
    else
        stepPulse = 0.0f;

    const float editPos   = static_cast<float>(editStep) / static_cast<float>(NUM_STEPS - 1);
    const float led1Pulse = 0.05f + (stepPulse * 0.85f);
    hw.led1.Set(led1Pulse, 0.02f, 0.10f + (0.45f * editPos));

    static const float drumColor[NUM_DRUMS][3] = {
        {0.85f, 0.10f, 0.10f},
        {0.80f, 0.45f, 0.10f},
        {0.75f, 0.75f, 0.10f},
        {0.10f, 0.70f, 0.10f},
        {0.10f, 0.35f, 0.75f},
        {0.70f, 0.20f, 0.70f},
    };

    const float pageScale = 0.25f + (0.17f * static_cast<float>(paramPage));
    hw.led2.Set(drumColor[currentDrum][0] * pageScale,
                drumColor[currentDrum][1] * pageScale,
                drumColor[currentDrum][2] * pageScale);

    hw.UpdateLeds();
}

static void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                          AudioHandle::InterleavingOutputBuffer out,
                          size_t                                size)
{
    for(size_t i = 0; i < size; i += 2)
    {
        if(metro.Process())
            AdvanceSequencer();

        float mixL = 0.0f;
        float mixR = 0.0f;

        const float kickSig = kick.Process();
        mixL += kickSig * 0.70f;
        mixR += kickSig * 0.70f;

        const float snareSig = snare.Process();
        mixL += snareSig * 0.60f;
        mixR += snareSig * 0.40f;

        const float chatSig = hihatClosed.Process();
        mixL += chatSig * 0.30f;
        mixR += chatSig * 0.60f;

        const float ohatSig = hihatOpen.Process();
        mixL += ohatSig * 0.30f;
        mixR += ohatSig * 0.60f;

        const float tomSig = tom.Process();
        mixL += tomSig * 0.60f;
        mixR += tomSig * 0.40f;

        const float clapSig = clap.Process();
        mixL += clapSig * 0.40f;
        mixR += clapSig * 0.50f;

        mixL = fclamp(mixL, -0.95f, 0.95f);
        mixR = fclamp(mixR, -0.95f, 0.95f);

        out[i]     = mixL;
        out[i + 1] = mixR;
    }
}

int main(void)
{
    hw.Init();
    hw.SetAudioBlockSize(48);

    // Block until serial terminal is connected.
    hw.seed.StartLog(true);
    hw.seed.PrintLine("=== Pod_DrumMachinePro Boot ===");
    hw.seed.PrintLine("[BOOT] block=%d samplerate=%d", 48, static_cast<int>(hw.AudioSampleRate()));

    InitDrums();

    for(int p = 0; p < NUM_PATTERNS; p++)
    {
        for(int d = 0; d < NUM_DRUMS; d++)
        {
            for(int s = 0; s < NUM_STEPS; s++)
                patterns[p][d][s] = false;
        }
    }

    hw.seed.PrintLine("[BOOT] patterns cleared");
    hw.StartAdc();
    hw.StartAudio(AudioCallback);

    LogSystemStatus(true);

    while(1)
    {
        ProcessControls();
        UpdateLeds();
        LogSystemStatus(false);
        System::Delay(1);
    }
}
