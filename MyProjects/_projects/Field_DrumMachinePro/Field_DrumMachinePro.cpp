/**
 * Field_DrumMachinePro
 *
 * 6-voice drum machine with 16-step patterns.
 *
 * Platform: Daisy Field
 *
 * Controls:
 * - Keys A1-A8: Steps 1-8 toggle for current drum
 * - Keys B1-B8: Steps 9-16 toggle for current drum
 * - Switch 1: Cycle through drum voices (Kick/Snare/CHat/OHat/Tom/Clap)
 * - Switch 2: Unused (reserved for future)
 * - Knob 1-6: Drum parameters (decay/tune/tone per voice)
 * - Knob 7: Tempo (40-240 BPM)
 * - Knob 8: Swing (0-75%)
 *
 * OLED: Pattern grid, current step, BPM, active drum
 * Serial: Startup status, knob/key/switch logs, periodic system status
 */

#include "daisy_field.h"
#include "daisysp.h"
#include <cmath>
#include <cstdio>

using namespace daisy;
using namespace daisysp;

DaisyField hw;

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

// Pattern memory: [pattern][drum][step]
bool patterns[NUM_PATTERNS][NUM_DRUMS][NUM_STEPS];

// Current state
int      currentPattern = 0;
DrumVoice currentDrum   = VOICE_KICK;
int      currentStep    = 0;
bool     playing        = true;

// Timing
Metro    metro;
float    tempo        = 120.0f;
float    swing        = 0.0f;
uint32_t lastStepTime = 0;
bool     oddStep      = false;

// Drum parameters
float kickDecay  = 0.5f;
float snareDecay = 0.5f;
float chatDecay  = 0.2f;
float ohatDecay  = 0.5f;
float tomTune    = 200.0f;
float clapDecay  = 0.3f;

const char* drumNames[NUM_DRUMS] = {
    "Kick", "Snare", "CHat", "OHat", "Tom", "Clap"
};

// Serial logging state
float    prevKnobVals[8] = {0};
bool     knobLogInit     = false;
uint32_t lastStatusLogMs = 0;

void InitDrums();
void UpdateControls();
void UpdateOLED();
void AdvanceSequencer();
void TriggerDrum(DrumVoice drum);
void LogSystemStatus(bool force = false);
void LogKnobChanges();

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    // Audio processing only - NO control processing or serial logs here.
    for(size_t i = 0; i < size; i++)
    {
        if(playing && metro.Process())
        {
            AdvanceSequencer();
        }

        float mixL = 0.0f;
        float mixR = 0.0f;

        float kickSig = kick.Process();
        mixL += kickSig * 0.7f;
        mixR += kickSig * 0.7f;

        float snareSig = snare.Process();
        mixL += snareSig * 0.6f;
        mixR += snareSig * 0.4f;

        float chatSig = hihatClosed.Process();
        mixL += chatSig * 0.3f;
        mixR += chatSig * 0.6f;

        float ohatSig = hihatOpen.Process();
        mixL += ohatSig * 0.3f;
        mixR += ohatSig * 0.6f;

        float tomSig = tom.Process();
        mixL += tomSig * 0.6f;
        mixR += tomSig * 0.4f;

        float clapSig = clap.Process();
        mixL += clapSig * 0.4f;
        mixR += clapSig * 0.5f;

        mixL = fclamp(mixL, -0.95f, 0.95f);
        mixR = fclamp(mixR, -0.95f, 0.95f);

        out[0][i] = mixL;
        out[1][i] = mixR;
    }
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
    if(oddStep)
    {
        const uint32_t swingDelay = (uint32_t)(swing * 100.0f);
        if(System::GetNow() - lastStepTime < swingDelay)
        {
            return;
        }
    }

    lastStepTime = System::GetNow();
    oddStep      = !oddStep;

    currentStep = (currentStep + 1) % NUM_STEPS;

    for(int drum = 0; drum < NUM_DRUMS; drum++)
    {
        if(patterns[currentPattern][drum][currentStep])
        {
            TriggerDrum((DrumVoice)drum);
        }
    }
}

void LogKnobChanges()
{
    if(!knobLogInit)
    {
        for(int i = 0; i < 8; i++)
            prevKnobVals[i] = hw.knob[i].Value();

        knobLogInit = true;
        return;
    }

    for(int i = 0; i < 8; i++)
    {
        const float current = hw.knob[i].Value();
        if(fabsf(current - prevKnobVals[i]) > 0.02f)
        {
            prevKnobVals[i] = current;
            hw.seed.PrintLine("[KNOB] K%d=%d%%", i + 1, (int)(current * 100.0f));
        }
    }
}

void LogSystemStatus(bool force)
{
    const uint32_t now = System::GetNow();
    if(!force && (now - lastStatusLogMs) < 1000)
        return;

    lastStatusLogMs = now;

    hw.seed.PrintLine("[STATUS] play=%d step=%02d drum=%s bpm=%d swing=%d%%",
                 playing ? 1 : 0,
                 currentStep + 1,
                 drumNames[currentDrum],
                 (int)tempo,
                 (int)(swing * 100.0f));
}

void UpdateControls()
{
    hw.ProcessAllControls();

    // SW1: Cycle drum select
    if(hw.sw[0].RisingEdge())
    {
        currentDrum = (DrumVoice)((currentDrum + 1) % NUM_DRUMS);
        hw.seed.PrintLine("[SW] SW1 drum=%s", drumNames[currentDrum]);
    }

    // SW2 currently unused; still logged for hardware verification.
    if(hw.sw[1].RisingEdge())
    {
        hw.seed.PrintLine("[SW] SW2 rising (unused)");
    }

    // Keys A1-A8: Toggle steps 1-8 for current drum
    for(int i = 0; i < 8; i++)
    {
        if(hw.KeyboardRisingEdge(i))
        {
            patterns[currentPattern][currentDrum][i]
                = !patterns[currentPattern][currentDrum][i];
            hw.seed.PrintLine("[KEY] A%d step=%02d state=%d drum=%s",
                         i + 1,
                         i + 1,
                         patterns[currentPattern][currentDrum][i] ? 1 : 0,
                         drumNames[currentDrum]);
        }
    }

    // Keys B1-B8: Toggle steps 9-16 for current drum
    for(int i = 0; i < 8; i++)
    {
        if(hw.KeyboardRisingEdge(i + 8))
        {
            patterns[currentPattern][currentDrum][i + 8]
                = !patterns[currentPattern][currentDrum][i + 8];
            hw.seed.PrintLine("[KEY] B%d step=%02d state=%d drum=%s",
                         i + 1,
                         i + 9,
                         patterns[currentPattern][currentDrum][i + 8] ? 1 : 0,
                         drumNames[currentDrum]);
        }
    }

    // Knobs 1-6: Drum parameters
    kickDecay  = hw.knob[DaisyField::KNOB_1].Process() * 0.9f + 0.1f;
    snareDecay = hw.knob[DaisyField::KNOB_2].Process() * 0.9f + 0.1f;
    chatDecay  = hw.knob[DaisyField::KNOB_3].Process() * 0.5f + 0.05f;
    ohatDecay  = hw.knob[DaisyField::KNOB_4].Process() * 0.9f + 0.1f;
    tomTune    = hw.knob[DaisyField::KNOB_5].Process() * 300.0f + 100.0f;
    clapDecay  = hw.knob[DaisyField::KNOB_6].Process() * 0.9f + 0.1f;

    // Knob 7: Tempo (40-240 BPM)
    tempo = 40.0f + hw.knob[DaisyField::KNOB_7].Process() * 200.0f;
    metro.SetFreq(tempo / 60.0f * 4.0f);

    // Knob 8: Swing (0-75%)
    swing = hw.knob[DaisyField::KNOB_8].Process() * 0.75f;

    LogKnobChanges();

    static uint32_t lastOLEDUpdate = 0;
    if(System::GetNow() - lastOLEDUpdate > 100)
    {
        UpdateOLED();
        lastOLEDUpdate = System::GetNow();
    }
}

void UpdateOLED()
{
    hw.display.Fill(false);

    char buf[32];
    snprintf(buf, sizeof(buf), "P%d: %s", currentPattern + 1, drumNames[currentDrum]);
    hw.display.SetCursor(0, 0);
    hw.display.WriteString(buf, Font_6x8, true);

    snprintf(buf, sizeof(buf), "%d BPM  SW:%d%%", (int)tempo, (int)(swing * 100));
    hw.display.SetCursor(0, 10);
    hw.display.WriteString(buf, Font_6x8, true);

    hw.display.SetCursor(0, 22);
    hw.display.WriteString("A:", Font_6x8, true);
    for(int i = 0; i < 8; i++)
    {
        hw.display.DrawRect(20 + i * 12,
                            22,
                            20 + i * 12 + 10,
                            30,
                            patterns[currentPattern][currentDrum][i],
                            true);
        if(currentStep == i)
        {
            hw.display.DrawRect(20 + i * 12 - 1,
                                21,
                                20 + i * 12 + 11,
                                31,
                                true,
                                false);
        }
    }

    hw.display.SetCursor(0, 35);
    hw.display.WriteString("B:", Font_6x8, true);
    for(int i = 0; i < 8; i++)
    {
        hw.display.DrawRect(20 + i * 12,
                            35,
                            20 + i * 12 + 10,
                            43,
                            patterns[currentPattern][currentDrum][i + 8],
                            true);
        if(currentStep == i + 8)
        {
            hw.display.DrawRect(20 + i * 12 - 1,
                                34,
                                20 + i * 12 + 11,
                                44,
                                true,
                                false);
        }
    }

    snprintf(buf, sizeof(buf), "Step: %02d/%d", currentStep + 1, NUM_STEPS);
    hw.display.SetCursor(0, 50);
    hw.display.WriteString(buf, Font_6x8, true);

    hw.display.Update();
}

int main(void)
{
    // Initialize hardware first.
    hw.Init();

    // Block here until serial terminal connects.
    hw.seed.StartLog(true);
    hw.seed.PrintLine("=== Field_DrumMachinePro Boot ===");

    hw.SetAudioBlockSize(48);
    hw.seed.PrintLine("[BOOT] block=%d samplerate=%d", 48, (int)hw.AudioSampleRate());

    InitDrums();

    // Initialize all patterns OFF.
    for(int p = 0; p < NUM_PATTERNS; p++)
    {
        for(int d = 0; d < NUM_DRUMS; d++)
        {
            for(int s = 0; s < NUM_STEPS; s++)
            {
                patterns[p][d][s] = false;
            }
        }
    }

    hw.seed.PrintLine("[BOOT] patterns cleared, playing=%d", playing ? 1 : 0);

    hw.StartAdc();
    hw.StartAudio(AudioCallback);

    UpdateOLED();
    LogSystemStatus(true);

    while(1)
    {
        UpdateControls();
        LogSystemStatus(false);
        System::Delay(1);
    }
}
