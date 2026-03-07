/**
 * DrumMachine v2.2 - Daisy Field
 * 
 * 16-step drum machine with 3 voices.
 * 
 * Controls:
 * Knob 0: Tempo (40-240 BPM)
 * Knob 1: Swing (0-50%)
 * Knob 2: Kick Decay
 * Knob 3: Kick Tone
 * Knob 4: Snare Decay
 * Knob 5: Snare Snappy
 * Knob 6: HiHat Decay
 * Knob 7: Hi-Hat Tone (2000-8000 Hz)
 * 
 * Keys 0-15: Toggle steps for selected drum
 * SW1: Cycle selected drum (Kick → Snare → HiHat)
 * SW2: Play/Pause
 *
 * Kick Voice Select (patch CV OUT1 to):
 *   CV IN1: Analog Kick  — 808-style resonant filter [default]
 *   CV IN3: Synth Kick   — 909-ish modulated oscillator
 *
 * Snare Voice Select (patch CV OUT2 to):
 *   CV IN2: Analog Snare — 808-style multi-mode resonators
 *   CV IN4: Synth Snare  — noise + oscillator mix [default]
 */

#include "daisy_field.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

// ── Voice selection defaults ─────────────────────────────────────────────────
// Edit these two lines and recompile to switch voices.
// CV patch (OUT1→IN3 / OUT2→IN2) overrides these at runtime when CV outs work.
#define DEFAULT_SYNTH_KICK   1  // 0=AnalogBassDrum (808)    1=SyntheticBassDrum (909)
#define DEFAULT_ANALOG_SNARE 1  // 0=SyntheticSnareDrum       1=AnalogSnareDrum (808)
// ─────────────────────────────────────────────────────────────────────────────

DaisyField hw;

// Drum voices
AnalogBassDrum     kick;        // CV IN1 → Analog Kick (default)
SyntheticBassDrum  synthKick;   // CV IN3 → Synth Kick
SyntheticSnareDrum snare;       // CV IN4 → Synth Snare (default)
AnalogSnareDrum    analogSnare; // CV IN2 → Analog Snare
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

// Voice selection (overridden by CV patch at runtime when CV outs reach >0.5 normalized)
volatile bool useSynthKick   = DEFAULT_SYNTH_KICK;   // false=AnalogBassDrum, true=SyntheticBassDrum
volatile bool useAnalogSnare = DEFAULT_ANALOG_SNARE; // false=SyntheticSnareDrum, true=AnalogSnareDrum

// Timing
float    tempo       = 120.f;
float    swing       = 0.f;
uint32_t lastStepTime = 0;

// Knob values cache
float kvals[8];

// Smoothed parameters
float kickDecay = 0.0f, kickDecayCur = 0.0f;
float kickTone = 0.5f, kickToneCur = 0.5f;
float snareDecay = 0.5f, snareDecayCur = 0.5f;
float snareSnappy = 0.5f, snareSnappyCur = 0.5f;
float hihatDecay = 0.5f, hihatDecayCur = 0.5f;
float hihatTone = 0.5f, hihatToneCur = 0.5f;

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
    sprintf(buf, "K:%s S:%s %s %dBPM",
        useSynthKick ? "S" : "A", useAnalogSnare ? "A" : "S",
        drumNames[selectedDrum], (int)tempo);
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
    sprintf(buf, "Stp:%d/16 Sw:%d%%", currentStep + 1, (int)(swing * 100.f));
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
        fonepole(hihatToneCur, hihatTone, 0.001f);

        kick.SetDecay(kickDecayCur);
        kick.SetTone(kickToneCur);
        synthKick.SetDecay(kickDecayCur);
        synthKick.SetTone(kickToneCur);
        snare.SetDecay(snareDecayCur);
        snare.SetSnappy(snareSnappyCur);
        analogSnare.SetDecay(snareDecayCur);
        analogSnare.SetSnappy(snareSnappyCur);
        hihat.SetDecay(hihatDecayCur);
        hihat.SetFreq(2000.f + hihatToneCur * 6000.f);

        float kickOut  = useSynthKick   ? synthKick.Process(false)   : kick.Process(false);
        float snareOut = useAnalogSnare ? analogSnare.Process(false) : snare.Process(false);
        float hihatOut = hihat.Process(false);

        float mix = (kickOut + snareOut * 0.8f + hihatOut * 0.4f) * 0.8f;
        if(!isfinite(mix)) mix = 0.f;
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
    kick.SetSelfFmAmount(0.1f);    // limit self-FM: swing 50–444 Hz vs default 53–3990 Hz
    kick.SetAttackFmAmount(0.2f);  // reduce FM sweep on attack

    synthKick.Init(sr);
    synthKick.SetFreq(50.f);
    synthKick.SetAccent(0.8f);

    snare.Init(sr);
    snare.SetFreq(200.f);
    snare.SetAccent(0.8f);

    analogSnare.Init(sr);
    analogSnare.SetFreq(200.f);
    analogSnare.SetAccent(0.8f);

    hihat.Init(sr);
    hihat.SetFreq(3000.f);
    hihat.SetAccent(0.8f);

    // CV OUT1/OUT2 at 5V — patch to CV IN1/IN3 (kick) or CV IN2/IN4 (snare) to select voice
    hw.SetCvOut1(4095);
    hw.SetCvOut2(4095);

    // Gate out held HIGH — use as 5V patch source if CV outs don't reach threshold
    dsy_gpio_write(&hw.gate_out, 1);

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

        // CV voice selection (CV outs measured ~1V, normalized threshold is 0.5 — disabled)
        // Re-enable when CV OUT voltage issue is resolved:
        // useSynthKick   = (hw.GetCvValue(DaisyField::CV_3) > 0.5f);
        // useAnalogSnare = (hw.GetCvValue(DaisyField::CV_2) > 0.5f);

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
        kickDecay   = fclamp(kvals[2], 0.0f, 0.4f);
        kickTone    = kvals[3];
        snareDecay  = kvals[4];
        snareSnappy = kvals[5];
        hihatDecay  = kvals[6];
        hihatTone   = kvals[7];
        
        uint32_t now = System::GetNow();
        uint32_t interval = GetStepInterval();
        
        if(playing && (now - lastStepTime >= interval))
        {
            currentStep = (currentStep + 1) % NUM_STEPS;
            swingStep++;
            lastStepTime = now;
            
            // Trigger drums from main loop (timing-sensitive)
            if(patterns[0][currentStep]) { if(useSynthKick)   synthKick.Trig();   else kick.Trig();  }
            if(patterns[1][currentStep]) { if(useAnalogSnare) analogSnare.Trig(); else snare.Trig(); }
            if(patterns[2][currentStep]) hihat.Trig();
        }
        
        UpdateDisplay();
        UpdateLeds();
        System::Delay(5);
    }
}
