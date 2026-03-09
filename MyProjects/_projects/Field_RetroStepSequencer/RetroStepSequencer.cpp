// Retro Step Sequencer for Daisy Field
// Ported from Arduino for Musicians by Brent Edstrom (Chapter 15)
// Date: 2026-01-19
//
// A classic 8-step sequencer with per-step pitch control via keyboard.
// Features pentatonic scale quantization, 6 waveforms, Moog filter with
// envelope, and MIDI output.
//
// DAISY FIELD CONTROLS:
// - Keys A1-A8: Set pitch for each step (touch to assign current knob1 value)
// - Keys B1-B8: Toggle step on/off
// - SW1: Play/Pause
// - SW2: Edit mode (hold to access per-step parameters)
// - Knob 1-4 (Play mode): Tempo, Filter Env, Filter Cutoff, Resonance
// - Knob 5-8 (Play mode): LFO Rate, LFO Amount, Attack, Decay
// - Knob 1-4 (Edit+Key): Pitch, Velocity, Duration, ---

#include "daisy_field.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

// ============================================================================
// CONSTANTS
// ============================================================================
constexpr int NUM_STEPS     = 8;
constexpr int NUM_WAVEFORMS = 6;

// Pentatonic scale intervals (semitones from root)
const int PENTATONIC[] = {0, 2, 4, 7, 9, 12, 14, 16, 19, 21, 24};
const int PENT_SIZE    = 11;

// ============================================================================
// STEP DATA STRUCTURE
// ============================================================================
struct Step
{
    float pitch;    // 0.0-1.0
    float velocity; // 0.0-1.0
    float duration; // 0.1-1.0
    bool  active;   // Step on/off
};

// ============================================================================
// HARDWARE & DSP
// ============================================================================
DaisyField hw;

// Sound generation
Oscillator osc;
Oscillator lfo;
AdEnv      ampEnv;
AdEnv      filterEnv;
MoogLadder filter;
Metro      metro;

// Sequencer state
Step steps[NUM_STEPS];
int  currentStep  = 0;
int  selectedStep = -1; // -1 = none selected
bool playing      = false;
int  waveform     = 2; // Default: SAW

// Parameters
float tempo           = 120.0f;
float filterCutoff    = 5000.0f;
float filterRes       = 0.5f;
float filterEnvAmount = 0.5f;
float lfoRate         = 0.5f;
float lfoAmount       = 0.0f;
int   rootNote        = 48; // C3

// Gate timing
float gateTimer    = 0.0f;
float stepDuration = 0.0f;
bool  gateActive   = false;

// Knob values
float knobVals[8];

// Display
char displayLine1[32];
char displayLine2[32];

// ============================================================================
// MIDI
// ============================================================================
MidiUsbHandler midi;
uint8_t        lastMidiNote = 0;

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

float PitchToFreq(float pitch, int root)
{
    int index    = static_cast<int>(pitch * (PENT_SIZE - 1));
    index        = DSY_MIN(DSY_MAX(index, 0), PENT_SIZE - 1);
    int midiNote = root + PENTATONIC[index];
    return mtof(midiNote);
}

uint8_t PitchToMidiNote(float pitch, int root)
{
    int index = static_cast<int>(pitch * (PENT_SIZE - 1));
    index     = DSY_MIN(DSY_MAX(index, 0), PENT_SIZE - 1);
    return static_cast<uint8_t>(root + PENTATONIC[index]);
}

void UpdateDisplay()
{
    hw.display.Fill(false);

    // Line 1: Mode and tempo
    snprintf(displayLine1,
             32,
             "%s %.0fBPM W:%d",
             playing ? "PLAY>" : "STOP",
             tempo,
             waveform);
    hw.display.SetCursor(0, 0);
    hw.display.WriteString(displayLine1, Font_6x8, true);

    // Line 2: Selected step or filter info
    if(selectedStep >= 0)
    {
        snprintf(displayLine2,
                 32,
                 "Step%d P:%.0f V:%.0f",
                 selectedStep + 1,
                 steps[selectedStep].pitch * 100,
                 steps[selectedStep].velocity * 100);
    }
    else
    {
        snprintf(displayLine2,
                 32,
                 "Flt:%.0f Res:%.0f%%",
                 filterCutoff,
                 filterRes * 100);
    }
    hw.display.SetCursor(0, 12);
    hw.display.WriteString(displayLine2, Font_6x8, true);

    // Step visualization (8 boxes)
    for(int i = 0; i < NUM_STEPS; i++)
    {
        int x = i * 16;
        int y = 26;
        int w = 14;
        int h = 10;

        hw.display.DrawRect(x, y, x + w, y + h, true, false);

        if(steps[i].active)
        {
            int fillH = static_cast<int>(steps[i].velocity * h);
            hw.display.DrawRect(
                x + 1, y + h - fillH, x + w - 1, y + h - 1, true, true);
        }

        // Highlight current step in play mode
        if(i == currentStep && playing && gateActive)
        {
            hw.display.DrawRect(x, y, x + w, y + h, true, true);
        }
    }

    hw.display.Update();
}

void UpdateLEDs()
{
    // Use keyboard LEDs A1-A8 for step indication
    size_t key_leds_a[] = {
        DaisyField::LED_KEY_A1,
        DaisyField::LED_KEY_A2,
        DaisyField::LED_KEY_A3,
        DaisyField::LED_KEY_A4,
        DaisyField::LED_KEY_A5,
        DaisyField::LED_KEY_A6,
        DaisyField::LED_KEY_A7,
        DaisyField::LED_KEY_A8,
    };

    // B-row LEDs for step active/muted
    size_t key_leds_b[] = {
        DaisyField::LED_KEY_B1,
        DaisyField::LED_KEY_B2,
        DaisyField::LED_KEY_B3,
        DaisyField::LED_KEY_B4,
        DaisyField::LED_KEY_B5,
        DaisyField::LED_KEY_B6,
        DaisyField::LED_KEY_B7,
        DaisyField::LED_KEY_B8,
    };

    for(int i = 0; i < NUM_STEPS; i++)
    {
        // A-row: show current step position
        float a_brightness = 0.0f;
        if(i == currentStep && playing && gateActive)
        {
            a_brightness = 1.0f;
        }
        else if(steps[i].active)
        {
            a_brightness = steps[i].velocity * 0.3f;
        }
        hw.led_driver.SetLed(key_leds_a[i], a_brightness);

        // B-row: show active/muted status
        float b_brightness = steps[i].active ? 0.5f : 0.0f;
        hw.led_driver.SetLed(key_leds_b[i], b_brightness);
    }

    // Knob LEDs show knob values
    size_t knob_leds[] = {
        DaisyField::LED_KNOB_1,
        DaisyField::LED_KNOB_2,
        DaisyField::LED_KNOB_3,
        DaisyField::LED_KNOB_4,
        DaisyField::LED_KNOB_5,
        DaisyField::LED_KNOB_6,
        DaisyField::LED_KNOB_7,
        DaisyField::LED_KNOB_8,
    };
    for(int i = 0; i < 8; i++)
    {
        hw.led_driver.SetLed(knob_leds[i], knobVals[i]);
    }

    // SW LEDs
    hw.led_driver.SetLed(DaisyField::LED_SW_1, playing ? 1.0f : 0.0f);
    hw.led_driver.SetLed(DaisyField::LED_SW_2,
                         (selectedStep >= 0) ? 1.0f : 0.0f);

    hw.led_driver.SwapBuffersAndTransmit();
}

void TriggerStep(int step)
{
    if(!steps[step].active)
        return;

    float freq = PitchToFreq(steps[step].pitch, rootNote);
    osc.SetFreq(freq);
    osc.SetAmp(steps[step].velocity);

    ampEnv.Trigger();
    filterEnv.Trigger();

    float stepTime = 60.0f / tempo;
    stepDuration   = stepTime * steps[step].duration;
    gateTimer      = 0.0f;
    gateActive     = true;

    // MIDI
    uint8_t midiNote = PitchToMidiNote(steps[step].pitch, rootNote);
    uint8_t vel      = static_cast<uint8_t>(steps[step].velocity * 127);

    if(lastMidiNote > 0)
    {
        uint8_t noteOff[3] = {0x80, lastMidiNote, 0};
        midi.SendMessage(noteOff, 3);
    }
    uint8_t noteOn[3] = {0x90, midiNote, vel};
    midi.SendMessage(noteOn, 3);
    lastMidiNote = midiNote;
}

void InitDefaultPattern()
{
    for(int i = 0; i < NUM_STEPS; i++)
    {
        steps[i].pitch    = static_cast<float>(i) / (NUM_STEPS - 1);
        steps[i].velocity = 0.8f;
        steps[i].duration = 0.5f;
        steps[i].active   = true;
    }
}

// ============================================================================
// AUDIO CALLBACK
// ============================================================================
void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    hw.ProcessAnalogControls();
    hw.ProcessDigitalControls();

    // Read knob values
    for(int i = 0; i < 8; i++)
    {
        knobVals[i] = hw.GetKnobValue(i);
    }

    // Check if SW2 is held for edit mode
    bool editMode = hw.sw[1].Pressed();

    // Check keyboard for step selection (A-row) or toggle (B-row)
    selectedStep = -1; // Reset selection
    for(int i = 0; i < 8; i++)
    {
        // A-row (0-7): Select step to edit
        if(hw.KeyboardState(i + 8)) // A-row is keys 8-15 (top row)
        {
            selectedStep = i;
        }

        // B-row (8-15 in hardware, but we use 0-7): Toggle step active
        if(hw.KeyboardRisingEdge(i)) // B-row is keys 0-7 (bottom row)
        {
            steps[i].active = !steps[i].active;
        }
    }

    // Parameter mapping based on mode
    if(editMode && selectedStep >= 0)
    {
        // Edit mode: modify selected step
        steps[selectedStep].pitch    = knobVals[0];
        steps[selectedStep].velocity = knobVals[1];
        steps[selectedStep].duration = 0.1f + knobVals[2] * 0.9f;
        // Knob 4-8 unused in edit mode
    }
    else
    {
        // Play mode: global parameters
        tempo           = 40.0f + knobVals[0] * 200.0f; // 40-240 BPM
        filterEnvAmount = knobVals[1];
        filterCutoff    = 100.0f + knobVals[2] * 9900.0f;
        filterRes       = knobVals[3];
        lfoRate         = 0.1f + knobVals[4] * 20.0f;
        lfoAmount       = knobVals[5];
        ampEnv.SetTime(ADENV_SEG_ATTACK, 0.001f + knobVals[6] * 0.5f);
        ampEnv.SetTime(ADENV_SEG_DECAY, 0.05f + knobVals[7] * 2.0f);

        metro.SetFreq(tempo / 60.0f);
    }

    // SW1: Play/Pause
    if(hw.sw[0].RisingEdge())
    {
        playing = !playing;
        if(playing)
        {
            currentStep = 0;
            TriggerStep(currentStep);
        }
        else if(lastMidiNote > 0)
        {
            uint8_t noteOff[3] = {0x80, lastMidiNote, 0};
            midi.SendMessage(noteOff, 3);
            lastMidiNote = 0;
        }
    }

    // SW2 rising edge: Cycle waveform
    if(hw.sw[1].RisingEdge() && !editMode)
    {
        waveform = (waveform + 1) % NUM_WAVEFORMS;
        osc.SetWaveform(static_cast<uint8_t>(waveform));
    }

    // Audio processing
    float sampleRate = hw.AudioSampleRate();
    lfo.SetFreq(lfoRate);

    for(size_t i = 0; i < size; i++)
    {
        // Metro tick
        if(playing && metro.Process())
        {
            currentStep = (currentStep + 1) % NUM_STEPS;
            TriggerStep(currentStep);
        }

        // Gate timing
        if(gateActive)
        {
            gateTimer += 1.0f / sampleRate;
            if(gateTimer >= stepDuration)
            {
                gateActive = false;
            }
        }

        // LFO (for future use)
        float lfoVal = lfo.Process();
        (void)lfoVal;

        // Envelopes
        float ampEnvVal    = ampEnv.Process();
        float filterEnvVal = filterEnv.Process();

        // Oscillator
        float sig = osc.Process();

        // Filter
        float filterFreq
            = filterCutoff + filterEnvVal * filterEnvAmount * 5000.0f;
        filterFreq = DSY_MIN(DSY_MAX(filterFreq, 20.0f), 18000.0f);
        filter.SetFreq(filterFreq);
        filter.SetRes(filterRes);
        sig = filter.Process(sig);

        // VCA
        sig *= ampEnvVal;

        // Output
        out[0][i] = sig;
        out[1][i] = sig;
    }
}

// ============================================================================
// MAIN
// ============================================================================
int main(void)
{
    hw.Init();
    hw.SetAudioBlockSize(48);
    float sr = hw.AudioSampleRate();

    // Initialize DSP
    osc.Init(sr);
    osc.SetWaveform(Oscillator::WAVE_SAW);
    osc.SetAmp(0.5f);

    lfo.Init(sr);
    lfo.SetWaveform(Oscillator::WAVE_SIN);
    lfo.SetAmp(1.0f);
    lfo.SetFreq(0.5f);

    ampEnv.Init(sr);
    ampEnv.SetTime(ADENV_SEG_ATTACK, 0.01f);
    ampEnv.SetTime(ADENV_SEG_DECAY, 0.3f);
    ampEnv.SetMin(0.0f);
    ampEnv.SetMax(1.0f);

    filterEnv.Init(sr);
    filterEnv.SetTime(ADENV_SEG_ATTACK, 0.001f);
    filterEnv.SetTime(ADENV_SEG_DECAY, 0.2f);
    filterEnv.SetMin(0.0f);
    filterEnv.SetMax(1.0f);

    filter.Init(sr);
    filter.SetFreq(5000.0f);
    filter.SetRes(0.5f);

    metro.Init(2.0f, sr);

    // Initialize MIDI
    MidiUsbHandler::Config midiCfg;
    midiCfg.transport_config.periph = MidiUsbTransport::Config::INTERNAL;
    midi.Init(midiCfg);

    InitDefaultPattern();

    hw.StartAdc();
    hw.StartAudio(AudioCallback);

    while(1)
    {
        midi.Listen();
        UpdateDisplay();
        UpdateLEDs();
        System::Delay(33);
    }
}
