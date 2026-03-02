#pragma once
#include "daisy_field.h"
#include "daisysp.h"

namespace synth
{

/**
 * @brief Portable User Interface handler for Daisy Field.
 * Handles LED mapping, Knob smoothing, and general UI helpers.
 */
class FieldUX
{
  public:
    FieldUX() {}
    ~FieldUX() {}

    /** @brief Initialize the UX handler with hardware pointer */
    void Init(daisy::DaisyField* hw);

    /** 
     * @brief Process and smooth all 8 knobs.
     * @param values_out Array of 8 floats to store smoothed values (0.0-1.0)
     */
    void ProcessKnobs(float* values_out);

    /**
     * @brief Update keyboard and knob LEDs based on synth state.
     * @param active_bank_idx 0-7 Index of active bank (A-row)
     * @param active_curve_idx 0-7 Index of active curve (B-row)
     * @param knob_values Array of 8 floats for knob brightness
     */
    void UpdateLeds(int          active_bank_idx,
                    int          active_curve_idx,
                    const float* knob_values);

    /** @brief Helper to draw a consistent header on the OLED */
    void DrawHeader(const char* title, const char* subtitle, bool lfo_active);

  private:
    daisy::DaisyField* hw_;
    float              smooth_knobs_[8];
};

} // namespace synth
