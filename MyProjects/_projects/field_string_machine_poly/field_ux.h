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
     * @brief Update keyboard and knob LEDs based on basic state.
     * @param page_idx 0-3 (page selection for A1-A4)
     * @param note_leds Array of 4 float brightness values (A5-A8)
     * @param latch SW1 state
     * @param paused SW2 state
     * @param knob_values Array of 8 floats for knob brightness
     */
    void UpdateLeds(int          page_idx,
                    const float* note_leds,
                    bool         latch,
                    bool         paused,
                    const float* knob_values);

    /** @brief Helper to draw a consistent header on the OLED */
    void DrawHeader(const char* title, const char* subtitle, bool active);

  private:
    daisy::DaisyField* hw_;
    float              smooth_knobs_[8];
};

} // namespace synth
