#pragma once
/**
 * field_defaults.h
 * 
 * Standard Daisy Field hardware constants and helpers.
 * Eliminates repetitive LED mapping, keyboard indices, toggle state management,
 * and OLED display code across Field projects.
 * 
 * Usage:
 *   #include "field_defaults.h"
 *   using namespace FieldDefaults;
 *   
 *   // Access LED constants
 *   hw.led_driver.SetLed(kLedKnobs[0], brightness);
 *   
 *   // Use keyboard mappings
 *   int key_idx = kKeyAIndices[3];  // Physical key A4 -> array index 12
 *   
 *   // Toggle keyboard LEDs on/off
 *   FieldKeyboardLEDs leds;
 *   leds.Init(&hw);
 *   // In your main loop:
 *   for(int i = 0; i < 8; i++) {
 *       if(hw.KeyboardRisingEdge(kKeyAIndices[i])) leds.ToggleA(i);
 *       if(hw.KeyboardRisingEdge(kKeyBIndices[i])) leds.ToggleB(i);
 *   }
 *   leds.Update();
 *   
 *   // Display all parameters with auto-highlighting
 *   FieldOLEDDisplay display;
 *   display.Init(&hw);
 *   display.SetTitle("My Synth");
 *   display.SetLabel(0, "Cutoff");
 *   // In your main loop:
 *   for(int i = 0; i < 8; i++) {
 *       display.SetValue(i, hw.knob[i].Process());
 *   }
 *   display.Update();  // Shows active param large + list
 */

#include "daisy_field.h"

namespace FieldDefaults
{

//==============================================================================
// HARDWARE CONSTANTS
//==============================================================================

constexpr int kNumKnobs      = 8;
constexpr int kNumKeys       = 16;
constexpr int kNumKeysPerRow = 8;
constexpr int kNumSwitches   = 2;
constexpr int kNumCVInputs   = 4;
constexpr int kNumCVOutputs  = 2;

constexpr size_t kRecommendedBlockSize = 48;
constexpr float  kDefaultSampleRate    = 48000.0f;

//==============================================================================
// LED CONSTANTS
//==============================================================================

/** 8 Knob LEDs (left to right)
 * From pinout: LED_POT_1=LED0 through LED_POT_8=LED7
 * NOTE: These are on a separate PCA9685 driver from the keys
 */
constexpr size_t kLedKnobs[8] = {
    daisy::DaisyField::LED_KNOB_1, // 16
    daisy::DaisyField::LED_KNOB_2, // 17
    daisy::DaisyField::LED_KNOB_3, // 18
    daisy::DaisyField::LED_KNOB_4, // 19
    daisy::DaisyField::LED_KNOB_5, // 20
    daisy::DaisyField::LED_KNOB_6, // 21
    daisy::DaisyField::LED_KNOB_7, // 22
    daisy::DaisyField::LED_KNOB_8, // 23
};

/** 8 Keyboard LEDs - Top Row (A1-A8, left to right)
 * Based on working field_wavetable_morph_synth: LED index = 15 - position
 * A1→15, A2→14, A3→13, A4→12, A5→11, A6→10, A7→9, A8→8
 */
constexpr size_t kLedKeysA[8] = {15, 14, 13, 12, 11, 10, 9, 8};

/** 8 Keyboard LEDs - Bottom Row (B1-B8, left to right)
 * Based on working field_wavetable_morph_synth: LED index = position
 * B1→0, B2→1, B3→2, B4→3, B5→4, B6→5, B7→6, B8→7
 */
constexpr size_t kLedKeysB[8] = {0, 1, 2, 3, 4, 5, 6, 7};

/** 2 Switch LEDs */
constexpr size_t kLedSwitches[2] = {
    daisy::DaisyField::LED_SW_1,
    daisy::DaisyField::LED_SW_2,
};

//------------------------------------------------------------------------------
// CHROMATIC SCALE KEYBOARD MAPPING (Optional - for piano-style use)
//------------------------------------------------------------------------------
// When using the keybed as a chromatic piano:
//   - B row (closer to user) = WHITE keys: C D E F G A B C (all 8)
//   - A row (further from user) = BLACK keys: C# D# F# G# A# (only 4 used)
//   - A1, A4, A5, A8 are GREY (gaps) = not played in chromatic mode
//
// NOTE: By default we use EXTERNAL MIDI keyboard for notes,
//       so A1-8 and B1-8 are FREE for presets, modes, LFO shapes, etc.
//------------------------------------------------------------------------------

/** Chromatic scale: 12 playable notes (excludes A1, A4, A5, A8 gaps)
 * B row: white keys, A row: black keys at A2, A3, A6, A7
 */
constexpr size_t kLedKeysChromaticScale[12] = {
    daisy::DaisyField::LED_KEY_B1, // C
    daisy::DaisyField::LED_KEY_B2, // D
    daisy::DaisyField::LED_KEY_B3, // E
    daisy::DaisyField::LED_KEY_B4, // F
    daisy::DaisyField::LED_KEY_B5, // G
    daisy::DaisyField::LED_KEY_B6, // A
    daisy::DaisyField::LED_KEY_B7, // B
    daisy::DaisyField::LED_KEY_B8, // C (octave)
    daisy::DaisyField::LED_KEY_A2, // C#
    daisy::DaisyField::LED_KEY_A3, // D#
    daisy::DaisyField::LED_KEY_A6, // G#
    daisy::DaisyField::LED_KEY_A7, // A#
};

/** All 16 keyboard LEDs (for non-chromatic applications like presets, modes)
 * Order: A1-A8, then B1-B8
 */
constexpr size_t kLedKeysAll[16] = {
    daisy::DaisyField::LED_KEY_A1,
    daisy::DaisyField::LED_KEY_A2,
    daisy::DaisyField::LED_KEY_A3,
    daisy::DaisyField::LED_KEY_A4,
    daisy::DaisyField::LED_KEY_A5,
    daisy::DaisyField::LED_KEY_A6,
    daisy::DaisyField::LED_KEY_A7,
    daisy::DaisyField::LED_KEY_A8,
    daisy::DaisyField::LED_KEY_B1,
    daisy::DaisyField::LED_KEY_B2,
    daisy::DaisyField::LED_KEY_B3,
    daisy::DaisyField::LED_KEY_B4,
    daisy::DaisyField::LED_KEY_B5,
    daisy::DaisyField::LED_KEY_B6,
    daisy::DaisyField::LED_KEY_B7,
    daisy::DaisyField::LED_KEY_B8,
};

//==============================================================================
// KEYBOARD INDEX MAPPINGS
//==============================================================================

/**
 * Physical keyboard to array index mapping.
 * 
 * Based on working field_wavetable_morph_synth implementation:
 *   - Row A (A1-A8): KeyboardRisingEdge indices 0-7
 *   - Row B (B1-B8): KeyboardRisingEdge indices 8-15
 * 
 * Physical Layout:
 *   A1  A2  A3  A4  A5  A6  A7  A8   (Top Row) → indices 0-7
 *   B1  B2  B3  B4  B5  B6  B7  B8   (Bottom Row) → indices 8-15
 */

// A1=0, A2=1, ..., A8=7
constexpr int kKeyAIndices[8] = {0, 1, 2, 3, 4, 5, 6, 7};

// B1=8, B2=9, ..., B8=15
constexpr int kKeyBIndices[8] = {8, 9, 10, 11, 12, 13, 14, 15};

//==============================================================================
// MUSICAL SCALES (Optional)
//==============================================================================

/**
 * Major scale for 16-key keyboard
 * Indices 8, 11, 15 are unused (black key gaps)
 */
constexpr float kScaleMajor[16] = {
    0.f,
    2.f,
    4.f,
    5.f,
    7.f,
    9.f,
    11.f,
    12.f, // B row
    0.f,
    1.f,
    3.f,
    0.f,
    6.f,
    8.f,
    10.f,
    0.f // A row
};

/** Chromatic scale (12 semitones) */
constexpr float kScaleChromatic[12] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

//==============================================================================
// KEYBOARD LED TOGGLE HELPER
//==============================================================================

/**
 * @brief Helper class to manage keyboard LED toggle state.
 * 
 * Provides toggle on/off behavior for keyboard LEDs:
 * First press = LED on, second press = LED off.
 * 
 * Example usage:
 *   FieldKeyboardLEDs leds;
 *   leds.Init(&hw);
 *   
 *   // In main loop:
 *   for(int i = 0; i < 8; i++) {
 *       if(hw.KeyboardRisingEdge(kKeyAIndices[i])) leds.ToggleA(i);
 *       if(hw.KeyboardRisingEdge(kKeyBIndices[i])) leds.ToggleB(i);
 *   }
 *   leds.Update();
 */
class FieldKeyboardLEDs
{
  public:
    FieldKeyboardLEDs() : hw_(nullptr)
    {
        for(int i = 0; i < 8; i++)
        {
            state_a_[i] = false;
            state_b_[i] = false;
        }
    }

    /** Initialize with hardware reference */
    void Init(daisy::DaisyField* hw) { hw_ = hw; }

    /** Toggle LED state for key A[0-7] (A1-A8) */
    void ToggleA(int idx)
    {
        if(idx >= 0 && idx < 8)
            state_a_[idx] = !state_a_[idx];
    }

    /** Toggle LED state for key B[0-7] (B1-B8) */
    void ToggleB(int idx)
    {
        if(idx >= 0 && idx < 8)
            state_b_[idx] = !state_b_[idx];
    }

    /** Set LED state for key A[0-7] directly */
    void SetA(int idx, bool on)
    {
        if(idx >= 0 && idx < 8)
            state_a_[idx] = on;
    }

    /** Set LED state for key B[0-7] directly */
    void SetB(int idx, bool on)
    {
        if(idx >= 0 && idx < 8)
            state_b_[idx] = on;
    }

    /** Get current state of key A[0-7] */
    bool GetA(int idx) const
    {
        return (idx >= 0 && idx < 8) ? state_a_[idx] : false;
    }

    /** Get current state of key B[0-7] */
    bool GetB(int idx) const
    {
        return (idx >= 0 && idx < 8) ? state_b_[idx] : false;
    }

    /** Clear all LED states (all off) */
    void Clear()
    {
        for(int i = 0; i < 8; i++)
        {
            state_a_[i] = false;
            state_b_[i] = false;
        }
    }

    /**
     * Update hardware LEDs based on current state.
     * Call this in your main loop after processing key presses.
     * 
     * @param brightness Brightness for ON LEDs (0.0-1.0), default 1.0
     */
    void Update(float brightness = 1.0f)
    {
        if(!hw_)
            return;

        for(int i = 0; i < 8; i++)
        {
            hw_->led_driver.SetLed(kLedKeysA[i],
                                   state_a_[i] ? brightness : 0.0f);
            hw_->led_driver.SetLed(kLedKeysB[i],
                                   state_b_[i] ? brightness : 0.0f);
        }
        hw_->led_driver.SwapBuffersAndTransmit();
    }

  private:
    daisy::DaisyField* hw_;
    bool               state_a_[8];
    bool               state_b_[8];
};

//==============================================================================
// OLED DISPLAY HELPER
//==============================================================================

/**
 * @brief Helper class to display all settings on OLED with active parameter highlighting.
 * 
 * Automatically tracks which parameter was most recently modified and displays it
 * prominently. Shows up to 8 parameters with labels and values.
 * 
 * Example usage:
 *   FieldOLEDDisplay display;
 *   display.Init(&hw);
 *   display.SetTitle("My Synth");
 *   
 *   // Set parameter labels
 *   display.SetLabel(0, "Cutoff");
 *   display.SetLabel(1, "Resonance");
 *   
 *   // In main loop:
 *   for(int i = 0; i < 8; i++) {
 *       float val = hw.knob[i].Process();
 *       display.SetValue(i, val);  // Automatically detects changes
 *   }
 *   display.Update();
 */
class FieldOLEDDisplay
{
  public:
    FieldOLEDDisplay() : hw_(nullptr), active_param_(-1), last_change_time_(0)
    {
        title_[0] = '\0';
        for(int i = 0; i < kMaxParams; i++)
        {
            labels_[i][0]   = '\0';
            values_[i]      = 0.0f;
            last_values_[i] = -999.0f; // Force initial update
        }
    }

    /** Initialize with hardware reference */
    void Init(daisy::DaisyField* hw)
    {
        hw_               = hw;
        last_change_time_ = daisy::System::GetNow();
    }

    /** Set display title (shown at top) */
    void SetTitle(const char* title)
    {
        if(title)
        {
            strncpy(title_, title, kMaxTitleLen - 1);
            title_[kMaxTitleLen - 1] = '\0';
        }
    }

    /** Set label for parameter 0-7 */
    void SetLabel(int param_idx, const char* label)
    {
        if(param_idx >= 0 && param_idx < kMaxParams && label)
        {
            strncpy(labels_[param_idx], label, kMaxLabelLen - 1);
            labels_[param_idx][kMaxLabelLen - 1] = '\0';
        }
    }

    /**
     * Set value for parameter 0-7.
     * Automatically detects changes and updates active parameter.
     * 
     * @param param_idx Parameter index (0-7)
     * @param value New value (0.0-1.0 typical)
     * @param threshold Change detection threshold (default 0.01)
     */
    void SetValue(int param_idx, float value, float threshold = 0.01f)
    {
        if(param_idx < 0 || param_idx >= kMaxParams)
            return;

        // Detect significant change
        if(fabsf(value - last_values_[param_idx]) > threshold)
        {
            active_param_           = param_idx;
            last_change_time_       = daisy::System::GetNow();
            last_values_[param_idx] = value;
        }

        values_[param_idx] = value;
    }

    /** Get current value for parameter */
    float GetValue(int param_idx) const
    {
        return (param_idx >= 0 && param_idx < kMaxParams) ? values_[param_idx]
                                                          : 0.0f;
    }

    /** Manually set active parameter (highlighted) */
    void SetActiveParam(int param_idx)
    {
        if(param_idx >= -1 && param_idx < kMaxParams)
        {
            active_param_     = param_idx;
            last_change_time_ = daisy::System::GetNow();
        }
    }

    /** Get currently active parameter index (-1 if none) */
    int GetActiveParam() const { return active_param_; }

    /** Get timestamp of last parameter change (for timeout logic) */
    uint32_t GetLastChangeTime() const { return last_change_time_; }

    /**
     * Update OLED display.
     * Call this in your main loop.
     * 
     * Layout:
     * - Line 0: Title
     * - Lines 1-3: Active parameter (large, highlighted)
     * - Lines 4-7: All 8 parameters (small, scrollable)
     */
    void Update()
    {
        if(!hw_)
            return;

        hw_->display.Fill(false);

        // Title
        if(title_[0] != '\0')
        {
            hw_->display.SetCursor(0, 0);
            hw_->display.WriteString(title_, Font_7x10, true);
        }

        // Active parameter (highlighted, large font)
        if(active_param_ >= 0 && active_param_ < kMaxParams)
        {
            char buf[32];

            // Parameter name
            hw_->display.SetCursor(0, 12);
            if(labels_[active_param_][0] != '\0')
            {
                snprintf(buf, sizeof(buf), "> %s", labels_[active_param_]);
            }
            else
            {
                snprintf(buf, sizeof(buf), "> Param %d", active_param_ + 1);
            }
            hw_->display.WriteString(buf, Font_7x10, true);

            // Value (large)
            hw_->display.SetCursor(0, 24);
            snprintf(buf, sizeof(buf), "%.2f", values_[active_param_]);
            hw_->display.WriteString(buf, Font_11x18, true);
        }

        // All parameters (compact list)
        hw_->display.SetCursor(0, 46);
        hw_->display.WriteString("---", Font_6x8, true);

        int y = 52;
        for(int i = 0; i < kMaxParams && i < 2; i++)
        {
            char        buf[22];
            const char* label = labels_[i][0] != '\0' ? labels_[i] : "?";

            // Truncate label if needed
            char short_label[6];
            strncpy(short_label, label, 5);
            short_label[5] = '\0';

            snprintf(buf, sizeof(buf), "%s:%.2f", short_label, values_[i]);

            hw_->display.SetCursor(0, y);
            hw_->display.WriteString(buf, Font_6x8, i == active_param_);
            y += 6;
        }

        hw_->display.Update();
    }

    /**
     * Alternative compact layout showing all 8 parameters.
     * No large active parameter display.
     */
    void UpdateCompact()
    {
        if(!hw_)
            return;

        hw_->display.Fill(false);

        // Title
        if(title_[0] != '\0')
        {
            hw_->display.SetCursor(0, 0);
            hw_->display.WriteString(title_, Font_7x10, true);
        }

        // Display all 8 parameters in 2 columns
        int y = 12;
        for(int i = 0; i < kMaxParams; i++)
        {
            char        buf[16];
            const char* label = labels_[i][0] != '\0' ? labels_[i] : "?";

            // Truncate label
            char short_label[5];
            strncpy(short_label, label, 4);
            short_label[4] = '\0';

            snprintf(buf, sizeof(buf), "%s:%.2f", short_label, values_[i]);

            // Left column (0-3), Right column (4-7)
            int x   = (i < 4) ? 0 : 64;
            int row = i % 4;

            hw_->display.SetCursor(x, y + (row * 13));
            hw_->display.WriteString(buf, Font_6x8, i == active_param_);
        }

        hw_->display.Update();
    }

  private:
    static constexpr int kMaxParams   = 8;
    static constexpr int kMaxLabelLen = 12;
    static constexpr int kMaxTitleLen = 20;

    daisy::DaisyField* hw_;
    char               title_[kMaxTitleLen];
    char               labels_[kMaxParams][kMaxLabelLen];
    float              values_[kMaxParams];
    float              last_values_[kMaxParams];
    int                active_param_;
    uint32_t           last_change_time_;
};

} // namespace FieldDefaults
