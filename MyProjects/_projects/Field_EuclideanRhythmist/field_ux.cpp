#include "field_ux.h"
#include <stdio.h>

namespace synth
{

void FieldUX::Init(daisy::DaisyField* hw)
{
    hw_ = hw;
    for(int i = 0; i < 8; i++)
    {
        smooth_knobs_[i] = 0.0f;
    }
}

void FieldUX::ProcessKnobs(float* values_out)
{
    float coeff = 0.1f; // Standard smoothing
    for(int i = 0; i < 8; i++)
    {
        // Use Value() since ProcessAllControls() already calls Process()
        daisysp::fonepole(smooth_knobs_[i], hw_->knob[i].Value(), coeff);
        values_out[i] = smooth_knobs_[i];
    }
}

void FieldUX::UpdateLeds(int          mode_idx,
                         int          octave_shift,
                         const float* note_leds,
                         bool         latch,
                         bool         paused,
                         const float* knob_values)
{
    // Clear all LEDs initially
    for(int i = 0; i < 26; i++)
    {
        hw_->led_driver.SetLed(i, 0.0f);
    }

    // Modes A1-A4 (Indices 15, 14, 13, 12 reversed)
    for(int i = 0; i < 4; i++)
    {
        int   led_idx    = 15 - i;
        float brightness = (i == mode_idx) ? 1.0f : 0.1f;
        hw_->led_driver.SetLed(led_idx, brightness);
    }

    // Note Activity A5-A8 (Indices 11, 10, 9, 8 reversed)
    for(int i = 0; i < 4; i++)
    {
        int led_idx = 11 - i;
        hw_->led_driver.SetLed(led_idx, note_leds[i]);
    }

    // Octave Shift B1-B4 (Indices 0, 1, 2, 3)
    // octave_shift is -1(B1), 0(B2), +1(B3), +2(B4)
    for(int i = 0; i < 4; i++)
    {
        int   led_idx    = i; // B1..B4 are 0..3
        float brightness = (i == (octave_shift + 1)) ? 1.0f : 0.1f;
        hw_->led_driver.SetLed(led_idx, brightness);
    }

    // Switch LEDs
    hw_->led_driver.SetLed(daisy::DaisyField::LED_SW_1, latch ? 1.0f : 0.0f);
    hw_->led_driver.SetLed(daisy::DaisyField::LED_SW_2, paused ? 1.0f : 0.0f);

    // Knob LEDs 16-23
    for(int i = 0; i < 8; i++)
    {
        hw_->led_driver.SetLed(16 + i, knob_values[i]);
    }

    hw_->led_driver.SwapBuffersAndTransmit();
}

void FieldUX::DrawHeader(const char* title,
                         const char* subtitle,
                         bool        lfo_active)
{
    // Placeholder
}

} // namespace synth
