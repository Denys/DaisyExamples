/**
 * DrumMachine v2.2 - Daisy Field
 * 
 * 16-step drum machine with 3 voices.
 * 
 * Controls:
 * Knob 0: Tempo (40-240 BPM)
 * Knob 1: Swing (0-50%)
 * Knob 2: Kick Decaym
 * Knob 3: Kick Tone
 * Knob 4: Snare Decay
 * Knob 5: Snare Snappy
 * Knob 6: HiHat Decay
 * Knob 7: Master Volume
 * 
 * Keys 0-15: Toggle steps for selected drum
 * SW1: Cycle selected drum (Kick → Snare → HiHat)
 * SW2: Play/Pause
 */

#include "daisy_field.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

DaisyField hw;

// Drum voices
AnalogBassDrum     kick;
SyntheticSnareDrum snare;
HiHat<>            hihat;

// Constants
constexpr int NUM_DRUMS = 3;
constexpr int NUM_STEPS = 16;

// Patterns
bool patterns[NUM_DRUMS][NUM_STEPS];

// State
uint8_t currentStep  = 0;
uint8_t selectedDrum = 0;
bool    playing      = true;
uint8_t swingStep    = 0;

// Timing
float    tempo       = 120.f;
float    swing       = 0.f;
uint32_t lastStepTime = 0;

// Knob values cache
float kvals[8];

// Smoothed parameters
float kickDecay = 0.5f, kickDecayCur = 0.5f;
float kickTone = 0.5f, kickToneCur = 0.5f;
float snareDecay = 0.5f, snareDecayCur = 0.5f;
float snareSnappy = 0.5f, snareSnappyCur = 0.5f;
float hihatDecay = 0.5f, hihatDecayCur = 0.5f;
float masterVol = 0.8f, masterVolCur = 0.8f;

// LED arrays
const size_t knob_leds[8] = {
    DaisyField::LED_KNOB_1, DaisyField::LED_KNOB_2,
    DaisyField::LED_KNOB_3, DaisyField::LED_KNOB_4,
    DaisyField::LED_KNOB_5, DaisyField::LED_KNOB_6,
    DaisyField::LED_KNOB_7, DaisyField::LED_KNOB_8,
};

const size_t keyboard_leds[16] = {
    DaisyField::LED_KEY_A1, DaisyField::LED_KEY_A2,
    DaisyField::LED_KEY_A3, DaisyField::LED_KEY_A4,
    DaisyField::LED_KEY_A5, DaisyField::LED_KEY_A6,
    DaisyField::LED_KEY_A7, DaisyField::LED_KEY_A8,
    DaisyField::LED_KEY_B1, DaisyField::LED_KEY_B2,
    DaisyField::LED_KEY_B3, DaisyField::LED_KEY_B4,
    DaisyField::LED_KEY_B5, DaisyField::LED_KEY_B6,
    DaisyField::LED_KEY_B7, DaisyField::LED_KEY_B8,
};

const char* drumNames[3] = {"KICK", "SNARE", "HIHAT"};

void InitPatterns()
{
    for(int d = 0; d < NUM_DRUMS; d++)
        for(int s = 0; s < NUM_STEPS; s++)
            patterns[d][s] = false;
    
    // Kick: 4-on-the-floor
    patterns[0][0] = patterns[0][4] = patterns[0][8] = patterns[0][12] = true;
    
    // Snare: backbeat
    patterns[1][4] = patterns[1][12] = true;
    
    // HiHat: 8th notes
    for(int s = 0; s < NUM_STEPS; s += 2)
        patterns[2][s] = true;
}

void UpdateLeds()
{
    for(int i = 0; i < 8; i++)
        hw.led_driver.SetLed(knob_leds[i], kvals[i]);
    
    for(int s = 0; s < NUM_STEPS; s++)
    {
        float brightness = 0.1f;
        if(patterns[selectedDrum][s]) brightness = 0.5f;
        if(s == currentStep && playing) brightness = 1.0f;
        hw.led_driver.SetLed(keyboard_leds[s], brightness);
    }
    
    hw.led_driver.SwapBuffersAndTransmit();
}

void UpdateDisplay()
{
    hw.display.Fill(false);
    
    hw.display.SetCursor(0, 0);
    hw.display.WriteString(playing ? "DRUM MACHINE" : "DRUMS [STOP]", Font_7x10, true);
    
    char buf[40];
    hw.display.SetCursor(0, 12);
    sprintf(buf, "%s %dBPM Sw:%d%%", drumNames[selectedDrum], (int)tempo, (int)(swing * 100.f));
    hw.display.WriteString(buf, Font_6x8, true);
    
    // Pattern boxes row 1 (steps 0-7)
    for(int s = 0; s < 8; s++)
    {
        int x = 2 + s * 15;
        hw.display.DrawRect(x, 24, x + 12, 34, true, patterns[selectedDrum][s]);
        if(s == currentStep && playing)
            hw.display.DrawRect(x + 3, 26, x + 9, 32, true, !patterns[selectedDrum][s]);
    }
    
    // Pattern boxes row 2 (steps 8-15)
    for(int s = 0; s < 8; s++)
    {
        int x = 2 + s * 15;
        hw.display.DrawRect(x, 38, x + 12, 48, true, patterns[selectedDrum][s + 8]);
        if((s + 8) == currentStep && playing)
            hw.display.DrawRect(x + 3, 40, x + 9, 46, true, !patterns[selectedDrum][s + 8]);
    }
    
    hw.display.SetCursor(0, 54);
    sprintf(buf, "Step: %d/16", currentStep + 1);
    hw.display.WriteString(buf, Font_6x8, true);
    
    hw.display.Update();
}

uint32_t GetStepInterval()
{
    float baseInterval = 60000.f / tempo / 4.f;
    bool swingBeat = (swingStep % 2 == 1);
    return (uint32_t)(baseInterval * (swingBeat ? (1.f + swing) : (1.f - swing * 0.5f)));
}

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    // Audio processing only - all control processing moved to main loop
    for(size_t i = 0; i < size; i++)
    {
        // Smooth parameters
        fonepole(kickDecayCur, kickDecay, 0.001f);
        fonepole(kickToneCur, kickTone, 0.001f);
        fonepole(snareDecayCur, snareDecay, 0.001f);
        fonepole(snareSnappyCur, snareSnappy, 0.001f);
        fonepole(hihatDecayCur, hihatDecay, 0.001f);
        fonepole(masterVolCur, masterVol, 0.001f);
        
        kick.SetDecay(kickDecayCur);
        kick.SetTone(kickToneCur);
        snare.SetDecay(snareDecayCur);
        snare.SetSnappy(snareSnappyCur);
        hihat.SetDecay(hihatDecayCur);
        
        float kickOut  = kick.Process(false);
        float snareOut = snare.Process(false);
        float hihatOut = hihat.Process(false);
        
        float mix = (kickOut + snareOut * 0.8f + hihatOut * 0.4f) * masterVolCur;
        mix = fclamp(mix, -1.f, 1.f);
        
        out[0][i] = mix;
        out[1][i] = mix;
    }
}

int main(void)
{
    hw.Init();
    float sr = hw.AudioSampleRate();
    
    kick.Init(sr);
    kick.SetFreq(50.f);
    kick.SetAccent(0.8f);
    
    snare.Init(sr);
    snare.SetFreq(200.f);
    snare.SetAccent(0.8f);
    
    hihat.Init(sr);
    hihat.SetFreq(3000.f);
    hihat.SetAccent(0.8f);
    
    InitPatterns();
    
    hw.StartAdc();
    hw.StartAudio(AudioCallback);
    
    lastStepTime = System::GetNow();
    
    while(1)
    {
        // Process controls in main loop (per DAISY_DEVELOPMENT_STANDARDS.md)
        hw.ProcessAnalogControls();
        hw.ProcessDigitalControls();
        
        // Read knob values
        for(int i = 0; i < 8; i++)
            kvals[i] = hw.GetKnobValue(i);
        
        // Switch handling
        if(hw.GetSwitch(DaisyField::SW_1)->RisingEdge())
        {
            selectedDrum = (selectedDrum + 1) % NUM_DRUMS;
        }
        
        if(hw.GetSwitch(DaisyField::SW_2)->RisingEdge())
        {
            playing = !playing;
        }
        
        // Keyboard handling - pattern toggle
        for(size_t k = 0; k < NUM_STEPS; k++)
        {
            if(hw.KeyboardRisingEdge(k))
            {
                patterns[selectedDrum][k] = !patterns[selectedDrum][k];
            }
        }
        
        // Parameter mapping from knobs
        tempo       = 40.f + kvals[0] * 200.f;
        swing       = kvals[1] * 0.5f;
        kickDecay   = kvals[2];
        kickTone    = kvals[3];
        snareDecay  = kvals[4];
        snareSnappy = kvals[5];
        hihatDecay  = kvals[6];
        masterVol   = kvals[7];
        
        uint32_t now = System::GetNow();
        uint32_t interval = GetStepInterval();
        
        if(playing && (now - lastStepTime >= interval))
        {
            currentStep = (currentStep + 1) % NUM_STEPS;
            swingStep++;
            lastStepTime = now;
            
            // Trigger drums from main loop (timing-sensitive)
            if(patterns[0][currentStep]) kick.Trig();
            if(patterns[1][currentStep]) snare.Trig();
            if(patterns[2][currentStep]) hihat.Trig();
        }
        
        UpdateDisplay();
        UpdateLeds();
        System::Delay(5);
    }
}
