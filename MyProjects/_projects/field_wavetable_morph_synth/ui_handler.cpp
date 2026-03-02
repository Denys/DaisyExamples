#include "ui_handler.h"
#include "wavetables.h"

namespace synth
{

void UiHandler::Init(daisy::DaisyField* hw)
{
    hw_            = hw;
    voice_         = nullptr;
    current_bank_  = BANK_SINE;
    current_curve_ = MORPH_LINEAR;
    lfo_enabled_   = false;

    // UI State Init
    active_param_      = -1;
    active_param_time_ = 0;
    for(int i = 0; i < 8; i++)
        prev_knob_vals_[i] = 0.0f;

    // Initialize Field UX Helper
    field_ux_.Init(hw);

    // Initialize smoothed values (Voice params)
    smooth_position_      = 0.0f;
    smooth_morph_speed_   = 1.0f;
    smooth_filter_cutoff_ = 1000.0f;
    smooth_attack_        = 0.1f;
    smooth_decay_         = 0.1f;
    smooth_sustain_       = 0.8f;
    smooth_release_       = 0.2f;
    smooth_output_level_  = 0.5f; // [CHANGED] Default volume
}

void UiHandler::ProcessControls()
{
    hw_->ProcessAllControls();
    ProcessKnobs();
    ProcessKeys();
    ProcessSwitches();
    UpdateVoiceParameters();
    UpdateLeds(); // Update keyboard LEDs to show active bank/curve
    // Note: UpdateDisplay() called separately in main loop
}

void UiHandler::ProcessKnobs()
{
    CheckParameterChanges();

    // Use FieldUX to get smoothed values for all knobs
    float knob_values[8];
    field_ux_.ProcessKnobs(knob_values);

    // Map knobs to parameters
    smooth_position_      = knob_values[0];
    smooth_morph_speed_   = knob_values[1] * 10.0f;            // 0-10 Hz
    smooth_filter_cutoff_ = 20.0f + knob_values[2] * 19980.0f; // 20Hz-20kHz
    smooth_attack_        = knob_values[3] * 5.0f;             // 0-5s
    smooth_decay_         = knob_values[4] * 5.0f;             // 0-5s
    smooth_sustain_       = knob_values[5];                    // 0-1
    smooth_release_       = knob_values[6] * 10.0f;            // 0-10s
    smooth_output_level_  = knob_values[7]; // [CHANGED] Volume 0-1
}

void UiHandler::ProcessKeys()
{
    // A1-A8: Wavetable bank select
    for(int i = 0; i < 8; i++)
    {
        if(hw_->KeyboardRisingEdge(i))
        {
            current_bank_ = static_cast<WavetableBank>(i);
        }
    }

    // B1-B8: Morph curve select
    for(int i = 0; i < 8; i++)
    {
        if(hw_->KeyboardRisingEdge(i + 8))
        { // B keys are 8-15
            current_curve_ = static_cast<MorphCurve>(i);
        }
    }
}

void UiHandler::ProcessSwitches()
{
    // SW1: LFO On/Off
    if(hw_->sw[0].RisingEdge())
    {
        lfo_enabled_ = !lfo_enabled_;
    }

    // SW2: FX Bypass (handled in voice)
}

void UiHandler::UpdateVoiceParameters()
{
    if(!voice_)
        return;

    // Set wavetable bank
    const float* bank_ptr = GetWavetableBank(current_bank_);
    voice_->SetWavetable(bank_ptr);

    // Set morph parameters
    voice_->SetPosition(smooth_position_);
    voice_->SetMorphCurve(current_curve_);
    voice_->SetMorphSpeed(smooth_morph_speed_);
    voice_->SetLfoEnabled(lfo_enabled_);

    // Set filter
    voice_->SetFilterCutoff(smooth_filter_cutoff_);

    // Set ADSR
    voice_->SetAdsr(
        smooth_attack_, smooth_decay_, smooth_sustain_, smooth_release_);

    // Set FX (hardcoded for now as knob 8 is volume)
    voice_->SetFxAmount(0.0f); // Default FX off until Shift-Knob implemented

    // Set Output Level
    voice_->SetOutputLevel(smooth_output_level_);
}

void UiHandler::CheckParameterChanges()
{
    float    threshold = 0.01f; // Sensitivity
    uint32_t now       = daisy::System::GetNow();

    for(int i = 0; i < 8; i++)
    {
        float val = hw_->knob[i].Value();
        if(fabsf(val - prev_knob_vals_[i]) > threshold)
        {
            active_param_      = i;
            active_param_time_ = now;
            prev_knob_vals_[i] = val;
        }
    }

    // Timeout check (1.5 seconds)
    if(active_param_ != -1 && (now - active_param_time_ > 1500))
    {
        active_param_ = -1;
    }
}

void UiHandler::UpdateDisplay()
{
    if(!hw_)
        return;

    // Dispatch display mode
    if(active_param_ != -1)
    {
        DrawFocusedParam();
    }
    else
    {
        DrawOverview();
    }

    hw_->display.Update();
}

void UiHandler::DrawFocusedParam()
{
    hw_->display.Fill(false);

    char        buf[32];
    const char* label = "";
    float       val   = 0.0f;
    const char* unit  = "";

    // Map knob index to parameter info
    switch(active_param_)
    {
        case 0: // Position
            label = "WAVETABLE";
            val   = smooth_position_;
            break;
        case 1: // Speed
            label = "MORPH SPD";
            val   = smooth_morph_speed_; // 0-10
            unit  = "Hz";
            break;
        case 2: // Cutoff
            label = "CUTOFF";
            val   = smooth_filter_cutoff_; // 20-20k
            unit  = "Hz";
            break;
        case 3: // Attack
            label = "ATTACK";
            val   = smooth_attack_;
            unit  = "s";
            break;
        case 4: // Decay
            label = "DECAY";
            val   = smooth_decay_;
            unit  = "s";
            break;
        case 5: // Sustain
            label = "SUSTAIN";
            val   = smooth_sustain_;
            break;
        case 6: // Release
            label = "RELEASE";
            val   = smooth_release_;
            unit  = "s";
            break;
        case 7: // Volume matches Knob 8 now
            label = "OUTPUT VOL";
            val   = smooth_output_level_;
            break;
    }

    // Draw Large Label
    hw_->display.SetCursor(0, 0);
    hw_->display.WriteString(label, Font_11x18, true);

    // Draw Large Value
    hw_->display.SetCursor(0, 24);
    if(active_param_ == 2) // Cutoff (large integer)
        sprintf(buf, "%.0f %s", val, unit);
    else
        sprintf(buf, "%.2f %s", val, unit);

    hw_->display.WriteString(buf, Font_16x26, true);

    // Draw Progress Bar
    float normalized = hw_->knob[active_param_].Value();
    int   width      = (int)(normalized * 128.0f);
    hw_->display.DrawRect(0, 52, 127, 60, true, false);  // Frame
    hw_->display.DrawRect(0, 52, width, 60, true, true); // Fill
}

void UiHandler::DrawOverview()
{
    hw_->display.Fill(false);
    char buf[48];

    // --- Line 1: Curve Type (Full Name) ---
    // User requested "complete curve parameters" and swapped lines.
    // "Curve" is now top header.
    const char* crv[] = {"Linear",
                         "Exponential",
                         "Logarithmic",
                         "Sine",
                         "Triangle",
                         "Step",
                         "Random",
                         "Custom"};

    hw_->display.SetCursor(0, 0);
    sprintf(
        buf,
        "%s",
        crv[current_curve_]); // Removed "Curve:" prefix to fit full names like "Exponential"
    hw_->display.WriteString(
        buf, Font_11x18, true); // Larger font for main header

    // --- Line 2: Bank Name & LFO ---
    const char* bank_names[] = {"Sine",
                                "Sawtooth",
                                "Square",
                                "Triangle",
                                "User 1",
                                "User 2",
                                "User 3",
                                "User 4"};

    hw_->display.SetCursor(0, 20); // Moved down due to larger header
    // Ensure index is safe
    int bank_idx = current_bank_ % 8;
    sprintf(buf, "Bank:%s", bank_names[bank_idx]);
    hw_->display.WriteString(buf, Font_6x8, true);

    // LFO Indicator (moved to Line 2)
    if(lfo_enabled_)
    {
        hw_->display.SetCursor(100, 20);
        hw_->display.WriteString("LFO", Font_6x8, true);
    }

    // --- Parameter Grid ---
    // Left Column: P (Pos), A (Atk), S (Sus), X (FX)
    // Right Column: F (Cut), D (Dec), R (Rel), M (Morph)

    int col1_x    = 0;
    int col2_x    = 64;
    int row_start = 30; // Shifted down
    int row_h     = 8;  // Tighter vertical spacing

    // Row 1: Position | Filter
    hw_->display.SetCursor(col1_x, row_start);
    sprintf(buf, "P:%.2f", smooth_position_);
    hw_->display.WriteString(buf, Font_6x8, true);

    hw_->display.SetCursor(col2_x, row_start);
    sprintf(buf, "F:%.0f", smooth_filter_cutoff_);
    hw_->display.WriteString(buf, Font_6x8, true);

    // Row 2: Attack | Decay
    hw_->display.SetCursor(col1_x, row_start + row_h);
    sprintf(buf, "A:%.2f", smooth_attack_);
    hw_->display.WriteString(buf, Font_6x8, true);

    hw_->display.SetCursor(col2_x, row_start + row_h);
    sprintf(buf, "D:%.2f", smooth_decay_);
    hw_->display.WriteString(buf, Font_6x8, true);

    // Row 3: Sustain | Release
    hw_->display.SetCursor(col1_x, row_start + row_h * 2);
    sprintf(buf, "S:%.2f", smooth_sustain_);
    hw_->display.WriteString(buf, Font_6x8, true);

    hw_->display.SetCursor(col2_x, row_start + row_h * 2);
    sprintf(buf, "R:%.2f", smooth_release_);
    hw_->display.WriteString(buf, Font_6x8, true);

    // Row 4: Vol | Morph Spd
    hw_->display.SetCursor(col1_x, row_start + row_h * 3);
    sprintf(buf, "V:%.2f", smooth_output_level_);
    hw_->display.WriteString(buf, Font_6x8, true);

    hw_->display.SetCursor(col2_x, row_start + row_h * 3);
    sprintf(buf, "M:%.1f", smooth_morph_speed_);
    hw_->display.WriteString(buf, Font_6x8, true);
}

void UiHandler::UpdateLeds()
{
    // Gather knob values specifically for LED brightness
    float knob_brightness[8];
    for(int i = 0; i < 8; i++)
    {
        knob_brightness[i] = hw_->knob[i].Value();
    }

    // Delegate LED update to FieldUX
    field_ux_.UpdateLeds(current_bank_, current_curve_, knob_brightness);
}

} // namespace synth