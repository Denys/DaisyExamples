#include "field_ux.h"

namespace synth
{

void FieldUX::Init(daisy::DaisyField* hw)
{
    hw_ = hw;
    for(int i = 0; i < 8; i++)
        smooth_knobs_[i] = 0.0f;
}

void FieldUX::ProcessKnobs(float* values_out)
{
    const float coeff = 0.1f;
    for(int i = 0; i < 8; i++)
    {
        daisysp::fonepole(smooth_knobs_[i], hw_->knob[i].Process(), coeff);
        values_out[i] = smooth_knobs_[i];
    }
}

void FieldUX::UpdateLeds(int          page_idx,
                         const float* note_leds,
                         bool         latch,
                         bool         paused,
                         const float* knob_values)
{
    for(int i = 0; i < 26; i++)
        hw_->led_driver.SetLed(i, 0.0f);

    for(int i = 0; i < 4; i++)
    {
        int   led_idx    = 15 - i; // A1-A4
        float brightness = (i == page_idx) ? 1.0f : 0.1f;
        hw_->led_driver.SetLed(led_idx, brightness);
    }

    for(int i = 0; i < 4; i++)
    {
        int led_idx = 11 - i; // A5-A8
        hw_->led_driver.SetLed(led_idx, note_leds[i]);
    }

    hw_->led_driver.SetLed(daisy::DaisyField::LED_SW_1, latch ? 1.0f : 0.0f);
    hw_->led_driver.SetLed(daisy::DaisyField::LED_SW_2, paused ? 1.0f : 0.0f);

    for(int i = 0; i < 8; i++)
        hw_->led_driver.SetLed(16 + i, knob_values[i]);

    hw_->led_driver.SwapBuffersAndTransmit();
}

void FieldUX::DrawHeader(const char* title, const char* subtitle, bool active)
{
    hw_->display.Fill(false);
    hw_->display.SetCursor(0, 0);
    hw_->display.WriteString(title, Font_7x10, true);
    hw_->display.SetCursor(0, 12);
    hw_->display.WriteString(subtitle, Font_6x8, true);
    if(active)
        hw_->display.DrawCircle(110, 10, 5, true);
    hw_->display.DrawLine(0, 22, 128, 22, true);
    hw_->display.Update();
}

} // namespace synth
