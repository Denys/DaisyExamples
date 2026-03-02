#pragma once
#include "daisy_field.h"
#include "daisysp.h"
#include "voice.h"
#include "morph_processor.h"
#include "voice.h"
#include "morph_processor.h"
#include "wavetables.h"
#include "field_ux.h"

namespace synth
{

class UiHandler
{
  public:
    UiHandler() {}
    ~UiHandler() {}

    void Init(daisy::DaisyField* hw);
    void ProcessControls();
    void SetVoice(Voice* voice) { voice_ = voice; }
    void UpdateDisplay();

  private:
    daisy::DaisyField* hw_;
    Voice*             voice_;

    // Current control values
    WavetableBank current_bank_;
    MorphCurve    current_curve_;
    bool          lfo_enabled_;

    // Smoothing
    // Smoothing
    // (Handled by FieldUX now, but we keep local copies for synth parameters)
    float smooth_position_;
    float smooth_morph_speed_;
    float smooth_filter_cutoff_;
    float smooth_attack_;
    float smooth_decay_;
    float smooth_sustain_;
    float smooth_release_;
    float smooth_output_level_; // [CHANGED] Was smooth_fx_amount_

    // UI State
    int8_t   active_param_;      // -1 = none, 0-7 = knob
    uint32_t active_param_time_; // Time of last change
    float    prev_knob_vals_[8]; // For change detection

    void ProcessKnobs();
    void ProcessKeys();
    void ProcessSwitches();
    void UpdateVoiceParameters();

    // UI Helpers
    void CheckParameterChanges();
    void DrawFocusedParam();
    void DrawOverview();
    void UpdateLeds(); // Restored wrapper

    FieldUX field_ux_;
};

} // namespace synth