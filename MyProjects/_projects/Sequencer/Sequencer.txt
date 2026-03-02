/**
 * 8-Step Sequencer - Daisy Field Adaptation
 * 
 * Ported from: Daisy Patch Sequencer (5-step)
 * Platform: Daisy Field
 * 
 * Enhanced 8-step sequencer using keyboard for step selection.
 * OLED shows all parameters with focus on active control.
 * 
 * Control Mapping:
 * Knob 0: Selected Step Pitch (0-127 MIDI)
 * Knob 1: Selected Step Gate Length (0-100%)
 * Knob 2: Tempo/BPM (30-300)
 * Knob 3: Swing Amount (0-50%)
 * Knob 4: Global Pitch Offset
 * Knob 5: Gate Output Length
 * Knob 6: Sequence Length (1-8 steps)
 * Knob 7: Output Level
 * sw[0]: Play/Stop toggle
 * sw[1]: Record mode toggle
 * Keyboard 0-7: Select step for editing / Toggle step active
 * Gate In: External clock
 * Gate Out: Trigger on each step
 * CV Out 1: Pitch (1V/oct)
 * CV Out 2: Gate
 */

#include "daisy_field.h"
#include "daisysp.h"
#include <cstring>

using namespace daisy;
using namespace daisysp;

DaisyField hw;

Oscillator osc;

struct Step
{
    float pitch;      // MIDI note
    float gateLen;    // 0-1
    bool  active;     // Step on/off
};

Step sequence[8];

uint8_t  currentStep = 0;
uint8_t  selectedStep = 0;
uint8_t  seqLength = 8;
float    tempo = 120.f;
float    swing = 0.f;
bool     playing = false;
bool     recording = false;
uint32_t lastStepTime = 0;
uint32_t stepInterval = 500;  // ms
bool     gateHigh = false;
uint32_t gateStartTime = 0;
float    pitchOffset = 0.f;
float    outputLevel = 0.8f;
float    gateLenMult = 0.5f;

// Parameter tracking for focused display
float    prevKnobVals[8] = {0};
int8_t   activeParam = -1;          // -1 = none, 0-7 = knob index
uint32_t activeParamTime = 0;
const uint32_t FOCUS_TIMEOUT = 1500; // ms before returning to overview
const float CHANGE_THRESHOLD = 0.02f;

// Parameter names for display
const char* paramNames[8] = {
    "PITCH",
    "GATE LEN",
    "TEMPO",
    "SWING",
    "OFFSET",
    "GATE OUT",
    "SEQ LEN",
    "LEVEL"
};

void UpdateLeds()
{
    size_t knob_leds[] = {
        DaisyField::LED_KNOB_1, DaisyField::LED_KNOB_2,
        DaisyField::LED_KNOB_3, DaisyField::LED_KNOB_4,
        DaisyField::LED_KNOB_5, DaisyField::LED_KNOB_6,
        DaisyField::LED_KNOB_7, DaisyField::LED_KNOB_8,
    };
    
    size_t keyboard_leds[] = {
        DaisyField::LED_KEY_A1, DaisyField::LED_KEY_A2,
        DaisyField::LED_KEY_A3, DaisyField::LED_KEY_A4,
        DaisyField::LED_KEY_A5, DaisyField::LED_KEY_A6,
        DaisyField::LED_KEY_A7, DaisyField::LED_KEY_A8,
        DaisyField::LED_KEY_B1, DaisyField::LED_KEY_B2,
        DaisyField::LED_KEY_B3, DaisyField::LED_KEY_B4,
        DaisyField::LED_KEY_B5, DaisyField::LED_KEY_B6,
        DaisyField::LED_KEY_B7, DaisyField::LED_KEY_B8,
    };
    
    // Knob LEDs - brighten active param
    for(int i = 0; i < 8; i++)
    {
        float brightness = hw.knob[i].Process();
        if(activeParam == i)
            brightness = 1.0f;  // Full brightness for active
        hw.led_driver.SetLed(knob_leds[i], brightness);
    }
    
    // Bottom row (0-7): Sequence steps
    for(size_t i = 0; i < 8; i++)
    {
        float brightness = 0.1f;  // Inactive step
        
        if(sequence[i].active)
            brightness = 0.4f;    // Active step
        
        if(i == selectedStep)
            brightness = 0.7f;    // Selected step
        
        if(playing && i == currentStep)
            brightness = 1.0f;    // Current playing step
        
        if(i >= seqLength)
            brightness = 0.0f;    // Beyond sequence length
        
        hw.led_driver.SetLed(keyboard_leds[i], brightness);
    }
    
    // Top row (8-15): Status indicators
    hw.led_driver.SetLed(keyboard_leds[8], playing ? 1.0f : 0.2f);
    hw.led_driver.SetLed(keyboard_leds[9], recording ? 1.0f : 0.2f);
    
    // Keys 10-15: Pitch indicator for selected step
    float pitchNorm = sequence[selectedStep].pitch / 127.f;
    for(int i = 10; i < 16; i++)
    {
        float thresh = (float)(i - 10) / 6.f;
        hw.led_driver.SetLed(keyboard_leds[i], pitchNorm > thresh ? 0.5f : 0.1f);
    }
    
    hw.led_driver.SwapBuffersAndTransmit();
}

void AdvanceStep()
{
    currentStep = (currentStep + 1) % seqLength;
    lastStepTime = System::GetNow();
    
    // Trigger gate if step is active
    if(sequence[currentStep].active)
    {
        gateHigh = true;
        gateStartTime = System::GetNow();
        
        // Set oscillator frequency
        float note = sequence[currentStep].pitch + pitchOffset;
        osc.SetFreq(mtof(note));
    }
}

void DetectActiveParam()
{
    uint32_t now = System::GetNow();
    
    // Check each knob for movement
    for(int i = 0; i < 8; i++)
    {
        float currentVal = hw.knob[i].Process();
        float delta = fabsf(currentVal - prevKnobVals[i]);
        
        if(delta > CHANGE_THRESHOLD)
        {
            activeParam = i;
            activeParamTime = now;
        }
        prevKnobVals[i] = currentVal;
    }
    
    // Timeout check - return to overview
    if(activeParam >= 0 && (now - activeParamTime > FOCUS_TIMEOUT))
    {
        activeParam = -1;
    }
}

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    hw.ProcessAllControls();
    
    // Detect which parameter is being modified
    DetectActiveParam();
    
    // Play/Stop toggle
    if(hw.sw[0].RisingEdge())
    {
        playing = !playing;
        if(playing)
        {
            currentStep = 0;
            lastStepTime = System::GetNow();
        }
    }
    
    // Record mode toggle
    if(hw.sw[1].RisingEdge())
    {
        recording = !recording;
    }
    
    // Keyboard step selection (bottom row 0-7)
    for(size_t i = 0; i < 8; i++)
    {
        if(hw.KeyboardRisingEdge(i))
        {
            if(recording)
            {
                sequence[i].active = !sequence[i].active;
            }
            else
            {
                selectedStep = i;
            }
        }
    }
    
    // Read parameters
    if(!playing || currentStep != selectedStep)
    {
        sequence[selectedStep].pitch = hw.knob[0].Process() * 127.f;
        sequence[selectedStep].gateLen = hw.knob[1].Process();
    }
    
    tempo = 30.f + hw.knob[2].Process() * 270.f;
    swing = hw.knob[3].Process() * 0.5f;
    pitchOffset = (hw.knob[4].Process() - 0.5f) * 24.f;
    gateLenMult = hw.knob[5].Process();
    seqLength = 1 + static_cast<uint8_t>(hw.knob[6].Process() * 7.f);
    outputLevel = hw.knob[7].Process();
    
    // Calculate step interval with swing
    float baseInterval = 60000.f / tempo / 4.f;
    bool swingStep = (currentStep % 2 == 1);
    stepInterval = static_cast<uint32_t>(baseInterval * (swingStep ? (1.f + swing) : (1.f - swing * 0.5f)));
    
    // CV outputs
    float pitchCV = (sequence[currentStep].pitch + pitchOffset) / 127.f;
    uint16_t cv1 = static_cast<uint16_t>(fclamp(pitchCV, 0.f, 1.f) * 4095.f);
    hw.seed.dac.WriteValue(DacHandle::Channel::ONE, cv1);
    
    uint16_t cv2 = gateHigh ? 4095 : 0;
    hw.seed.dac.WriteValue(DacHandle::Channel::TWO, cv2);
    
    // Process audio
    for(size_t i = 0; i < size; i++)
    {
        float sig = 0.f;
        
        if(gateHigh && sequence[currentStep].active)
        {
            sig = osc.Process() * outputLevel;
        }
        
        out[0][i] = sig;
        out[1][i] = sig;
    }
}

void UpdateSequencer()
{
    uint32_t now = System::GetNow();
    
    // Check gate off
    if(gateHigh)
    {
        float gateLen = sequence[currentStep].gateLen * gateLenMult;
        uint32_t gateDuration = static_cast<uint32_t>(stepInterval * gateLen);
        if(now - gateStartTime > gateDuration)
        {
            gateHigh = false;
        }
    }
    
    // Check step advance
    if(playing && (now - lastStepTime >= stepInterval))
    {
        AdvanceStep();
    }
}

void DrawFocusedParam()
{
    char buf[32];
    float value = 0.f;
    const char* unit = "";
    
    // Get parameter value and format
    switch(activeParam)
    {
        case 0:  // Pitch
            value = sequence[selectedStep].pitch;
            unit = "MIDI";
            break;
        case 1:  // Gate Length
            value = sequence[selectedStep].gateLen * 100.f;
            unit = "%";
            break;
        case 2:  // Tempo
            value = tempo;
            unit = "BPM";
            break;
        case 3:  // Swing
            value = swing * 100.f;
            unit = "%";
            break;
        case 4:  // Pitch Offset
            value = pitchOffset;
            unit = "st";
            break;
        case 5:  // Gate Out
            value = gateLenMult * 100.f;
            unit = "%";
            break;
        case 6:  // Seq Length
            value = (float)seqLength;
            unit = "steps";
            break;
        case 7:  // Level
            value = outputLevel * 100.f;
            unit = "%";
            break;
    }
    
    hw.display.Fill(false);
    
    // Large parameter name at top
    hw.display.SetCursor(0, 0);
    hw.display.WriteString(paramNames[activeParam], Font_11x18, true);
    
    // Large value in center
    if(activeParam == 4)  // Pitch offset shows sign
        sprintf(buf, "%+.1f %s", value, unit);
    else if(activeParam == 6)  // Seq length is integer
        sprintf(buf, "%.0f %s", value, unit);
    else
        sprintf(buf, "%.1f %s", value, unit);
    
    hw.display.SetCursor(0, 24);
    hw.display.WriteString(buf, Font_11x18, true);
    
    // Progress bar visualization
    float normalized = hw.knob[activeParam].Process();
    int barWidth = static_cast<int>(normalized * 120.f);
    hw.display.DrawRect(3, 48, 124, 58, true, false);  // Outline
    hw.display.DrawRect(4, 49, 4 + barWidth, 57, true, true);  // Fill
    
    // Knob number indicator
    sprintf(buf, "K%d", activeParam + 1);
    hw.display.SetCursor(105, 0);
    hw.display.WriteString(buf, Font_7x10, true);
    
    hw.display.Update();
}

void DrawOverview()
{
    hw.display.Fill(false);
    
    char buf[32];
    
    // Header: Status
    hw.display.SetCursor(0, 0);
    hw.display.WriteString(playing ? "PLAY" : "STOP", Font_7x10, true);
    if(recording)
    {
        hw.display.SetCursor(35, 0);
        hw.display.WriteString("REC", Font_7x10, true);
    }
    sprintf(buf, "S%d/%d", currentStep + 1, seqLength);
    hw.display.SetCursor(70, 0);
    hw.display.WriteString(buf, Font_7x10, true);
    sprintf(buf, "%.0f", tempo);
    hw.display.SetCursor(105, 0);
    hw.display.WriteString(buf, Font_7x10, true);
    
    // Step pattern (large)
    hw.display.SetCursor(0, 14);
    char pattern[9];
    for(int i = 0; i < 8; i++)
    {
        if(i >= seqLength)
            pattern[i] = '.';
        else if(i == currentStep && playing)
            pattern[i] = 'O';
        else if(sequence[i].active)
            pattern[i] = 'X';
        else
            pattern[i] = '-';
    }
    pattern[8] = '\0';
    hw.display.WriteString(pattern, Font_11x18, true);
    
    // Selected step info
    hw.display.SetCursor(0, 34);
    sprintf(buf, "Ed:%d P:%.0f G:%.0f%%", 
            selectedStep + 1,
            sequence[selectedStep].pitch,
            sequence[selectedStep].gateLen * 100.f);
    hw.display.WriteString(buf, Font_6x8, true);
    
    // Bottom row: All params compact
    hw.display.SetCursor(0, 44);
    sprintf(buf, "Sw:%.0f%% Of:%+.0f Lv:%.0f%%", 
            swing * 100.f, pitchOffset, outputLevel * 100.f);
    hw.display.WriteString(buf, Font_6x8, true);
    
    // Gate out
    hw.display.SetCursor(0, 54);
    sprintf(buf, "GateOut:%.0f%%", gateLenMult * 100.f);
    hw.display.WriteString(buf, Font_6x8, true);
    
    hw.display.Update();
}

void UpdateDisplay()
{
    if(activeParam >= 0)
    {
        DrawFocusedParam();
    }
    else
    {
        DrawOverview();
    }
}

int main(void)
{
    hw.Init();
    float sr = hw.AudioSampleRate();
    
    osc.Init(sr);
    osc.SetWaveform(Oscillator::WAVE_SAW);
    osc.SetAmp(0.5f);
    osc.SetFreq(440.f);
    
    // Initialize sequence with default values
    for(int i = 0; i < 8; i++)
    {
        sequence[i].pitch = 48.f + i * 2.f;
        sequence[i].gateLen = 0.5f;
        sequence[i].active = (i < 4);
    }
    
    // Initialize previous knob values
    for(int i = 0; i < 8; i++)
    {
        prevKnobVals[i] = hw.knob[i].Process();
    }
    
    hw.StartAdc();
    hw.StartAudio(AudioCallback);
    
    while(1)
    {
        UpdateSequencer();
        UpdateDisplay();
        UpdateLeds();
        System::Delay(10);
    }
}
