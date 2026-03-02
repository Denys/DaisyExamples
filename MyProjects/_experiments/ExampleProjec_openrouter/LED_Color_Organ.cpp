// Reactive LED Color Organ - Daisy Pod Project
// Proof of concept for interactive LED control demonstrating all physical I/O
// Based on DaisyPod-Creative-Projects.md specifications

#include "daisysp.h"
#include "daisy_pod.h"

using namespace daisy;
using namespace daisysp;

// Global objects
static DaisyPod pod;

// Color mode enumeration
enum ColorMode
{
    MODE_INDEPENDENT = 0,  // Each knob controls its respective LED
    MODE_MIRROR      = 1,  // Both LEDs show same color
    MODE_COMPLEMENT  = 2,  // LED2 shows complement of LED1
    MODE_RAINBOW     = 3,  // Automatic rainbow cycle
    NUM_MODES        = 4
};

// Global state variables
ColorMode currentMode         = MODE_INDEPENDENT;
float     encoderValue        = 0.5f; // Master brightness (0-1)
float     rainbowPhase        = 0.0f; // For rainbow mode animation
bool      button1ToggleState  = false;
bool      button2ToggleState  = false;
uint32_t  button1LastTapTime  = 0;
uint32_t  button2LastTapTime  = 0;
bool      button1WasPressed   = false;
bool      button2WasPressed   = false;
float     brightnessSmooth1Val = 0.5f;
float     brightnessSmooth2Val = 0.5f;

// HSV to RGB conversion structure
struct LedColor
{
    float r, g, b;
};

// Convert HSV (Hue: 0-1, Saturation: 0-1, Value: 0-1) to RGB (0-1)
LedColor HsvToRgb(float h, float s, float v)
{
    LedColor result;
    
    if(s <= 0.0f)
    {
        result.r = result.g = result.b = v;
        return result;
    }
    
    // Normalize hue to 0-6 range
    float hh = h * 6.0f;
    if(hh >= 6.0f)
        hh = 0.0f;
    
    int   i  = (int)hh;
    float ff = hh - i;
    float p  = v * (1.0f - s);
    float q  = v * (1.0f - (s * ff));
    float t  = v * (1.0f - (s * (1.0f - ff)));
    
    switch(i)
    {
        case 0:
            result.r = v;
            result.g = t;
            result.b = p;
            break;
        case 1:
            result.r = q;
            result.g = v;
            result.b = p;
            break;
        case 2:
            result.r = p;
            result.g = v;
            result.b = t;
            break;
        case 3:
            result.r = p;
            result.g = q;
            result.b = v;
            break;
        case 4:
            result.r = t;
            result.g = p;
            result.b = v;
            break;
        case 5:
        default:
            result.r = v;
            result.g = p;
            result.b = q;
            break;
    }
    
    return result;
}

// Clamp value between 0 and 1
float Clamp01(float value)
{
    if(value < 0.0f)
        return 0.0f;
    if(value > 1.0f)
        return 1.0f;
    return value;
}

// Update encoder value for brightness control
void UpdateEncoder()
{
    int32_t increment = pod.encoder.Increment();
    encoderValue += increment * 0.02f; // Sensitivity adjustment
    encoderValue = Clamp01(encoderValue);
    
    // Mode switching on encoder press
    if(pod.encoder.RisingEdge())
    {
        currentMode = (ColorMode)((currentMode + 1) % NUM_MODES);
    }
}

// Handle button interactions with double-tap detection
void UpdateButtons()
{
    uint32_t currentTime = System::GetNow();
    
    // Button 1 logic - LED1 intensity boost
    if(pod.button1.RisingEdge())
    {
        // Check for double-tap (within 300ms)
        if(currentTime - button1LastTapTime < 300)
        {
            button1ToggleState = !button1ToggleState;
        }
        button1LastTapTime = currentTime;
    }
    
    // Button 2 logic - LED2 intensity boost
    if(pod.button2.RisingEdge())
    {
        // Check for double-tap (within 300ms)
        if(currentTime - button2LastTapTime < 300)
        {
            button2ToggleState = !button2ToggleState;
        }
        button2LastTapTime = currentTime;
    }
}

// Main LED update logic
void UpdateLEDs()
{
    // Read knob values (0-1)
    float knob1Value = pod.knob1.Process();
    float knob2Value = pod.knob2.Process();
    
    // Calculate intensity boost from buttons
    float boost1 = 1.0f;
    float boost2 = 1.0f;
    
    // Momentary boost when held
    if(pod.button1.Pressed())
    {
        boost1 = 1.5f;
    }
    if(pod.button2.Pressed())
    {
        boost2 = 1.5f;
    }
    
    // Toggle boost if double-tapped
    if(button1ToggleState)
    {
        boost1 = 1.5f;
    }
    if(button2ToggleState)
    {
        boost2 = 1.5f;
    }
    
    // Calculate hue values based on mode
    float hue1, hue2;
    
    switch(currentMode)
    {
        case MODE_INDEPENDENT:
            // Each knob controls its respective LED
            hue1 = knob1Value;
            hue2 = knob2Value;
            break;
            
        case MODE_MIRROR:
            // Both LEDs show same color (average of knobs)
            hue1 = hue2 = (knob1Value + knob2Value) * 0.5f;
            break;
            
        case MODE_COMPLEMENT:
            // LED2 shows complement (180° offset) of LED1
            hue1 = knob1Value;
            hue2 = hue1 + 0.5f;
            if(hue2 >= 1.0f)
                hue2 -= 1.0f;
            break;
            
        case MODE_RAINBOW:
            // Automatic rainbow cycle
            // Encoder controls speed, knobs control phase offset
            rainbowPhase += 0.0005f * (encoderValue + 0.1f);
            if(rainbowPhase >= 1.0f)
                rainbowPhase -= 1.0f;
            
            hue1 = rainbowPhase + knob1Value * 0.3f;
            hue2 = rainbowPhase + knob2Value * 0.3f + 0.2f;
            
            if(hue1 >= 1.0f)
                hue1 -= 1.0f;
            if(hue2 >= 1.0f)
                hue2 -= 1.0f;
            break;
            
        default:
            hue1 = hue2 = 0.0f;
            break;
    }
    
    // Calculate brightness with smoothing
    float brightness1 = encoderValue * boost1;
    float brightness2 = encoderValue * boost2;
    
    // Apply smoothing to brightness using one-pole filter
    fonepole(brightnessSmooth1Val, brightness1, 0.01f);
    fonepole(brightnessSmooth2Val, brightness2, 0.01f);
    brightness1 = Clamp01(brightnessSmooth1Val);
    brightness2 = Clamp01(brightnessSmooth2Val);
    
    // Convert HSV to RGB
    LedColor color1 = HsvToRgb(hue1, 1.0f, brightness1);
    LedColor color2 = HsvToRgb(hue2, 1.0f, brightness2);
    
    // Update LED colors
    pod.led1.Set(color1.r, color1.g, color1.b);
    pod.led2.Set(color2.r, color2.g, color2.b);
    pod.UpdateLeds();
}

// Controls processing
void ProcessControls()
{
    // Process analog and digital controls
    pod.ProcessAnalogControls();
    pod.ProcessDigitalControls();
    
    // Update encoder
    UpdateEncoder();
    
    // Update buttons
    UpdateButtons();
    
    // Update LEDs
    UpdateLEDs();
}

// Audio callback (not used for audio in this project, but required)
static void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                          AudioHandle::InterleavingOutputBuffer out,
                          size_t                                size)
{
    // Process controls at audio rate for smooth LED response
    ProcessControls();
    
    // Pass through audio (zero output in this demo)
    for(size_t i = 0; i < size; i++)
    {
        out[i] = 0.0f;
    }
}

int main(void)
{
    // Initialize Pod hardware
    pod.Init();
    pod.SetAudioBlockSize(48); // Larger block size since we're not processing audio
    
    float sample_rate = pod.AudioSampleRate();
    (void)sample_rate;
    
    // Set initial LED state
    pod.ClearLeds();
    pod.UpdateLeds();
    
    // Start ADC for knob readings
    pod.StartAdc();
    
    // Start audio callback
    pod.StartAudio(AudioCallback);
    
    // Main loop does nothing - all work happens in callback
    while(1)
    {
        // Infinite loop
    }
}