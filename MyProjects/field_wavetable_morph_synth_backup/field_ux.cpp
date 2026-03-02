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
        daisysp::fonepole(smooth_knobs_[i], hw_->knob[i].Process(), coeff);
        values_out[i] = smooth_knobs_[i];
    }
}

void FieldUX::UpdateLeds(int          active_bank_idx,
                         int          active_curve_idx,
                         const float* knob_values)
{
    // Turn off all LEDs
    for(int i = 0; i < 26; i++)
    {
        hw_->led_driver.SetLed(i, 0.0f);
    }

    // A-Row (Bank): Indices 8-15 (reversed: A1=15, A8=8)
    if(active_bank_idx >= 0 && active_bank_idx < 8)
    {
        int led_idx = 15 - active_bank_idx;
        hw_->led_driver.SetLed(led_idx, 1.0f); // Full Brightness
    }

    // B-Row (Curve): Indices 0-7 (B1=0, B8=7)
    if(active_curve_idx >= 0 && active_curve_idx < 8)
    {
        hw_->led_driver.SetLed(active_curve_idx, 1.0f); // Full Brightness
    }

    // Knob LEDs: Indices 16-23
    for(int i = 0; i < 8; i++)
    {
        // LED_KNOB_1 starts at index 16
        hw_->led_driver.SetLed(16 + i, knob_values[i]);
    }

    hw_->led_driver.SwapBuffersAndTransmit();
}

void FieldUX::DrawHeader(const char* title,
                         const char* subtitle,
                         bool        lfo_active)
{
    // Reusable header logic can go here if needed,
    // for now just placeholder to show library capability.
}

} // namespace synth
