#pragma once
#include "parameters.h"
#include "presets.h"
#include <stdint.h>

// ============================================================
// UIState — event-driven page/cursor/edit state machine
//
// Owns the FxParams pointer and is the only place that writes
// parameters from user input. Audio ISR reads the cached copy
// in main.cpp after a __DSB() barrier.
//
// Usage in main loop:
//   1. Call PollControls() every iteration (continuous controls)
//   2. Dispatch discrete events (button presses, encoder push)
//   3. Read current_page(), cursor(), edit_mode() for display
// ============================================================

// Page IDs (matches display page order)
enum class Page : uint8_t {
    PERFORM = 0,
    TONE    = 1,
    MOTION  = 2,
    FREEZE  = 3,
    PRESETS = 4,
    SYSTEM  = 5,
    COUNT   = 6
};

// Discrete UI events dispatched from main loop
enum class UIEvent : uint8_t {
    EXT_ENC_CW,         // External encoder clockwise
    EXT_ENC_CCW,        // External encoder counter-clockwise
    EXT_PSH_SHORT,      // Ext encoder push (short tap)
    EXT_PSH_LONG,       // Ext encoder push (500 ms hold)
    BAK,                // BAK button
    CON,                // CON button
    POD_ENC_CW,         // Pod encoder CW (wet/dry, or Shift: drive)
    POD_ENC_CCW,        // Pod encoder CCW
    POD_ENC_PUSH,       // Pod encoder push (page-aware action)
    SW2_PRESS,          // SW2 unshifted (page-aware)
    PAGE_JUMP,          // Shift + Pod Enc push: cycle pages P1→...→P6
    FREEZE_INSTANT,     // SW1 short: toggle momentary freeze
    SW2_SHIFT,          // Shift + SW2 (page-aware)
};

class UIState {
public:
    // params   — live parameter store (written here, read by ISR via double-buffer)
    // presets  — array of kNumPresets factory preset data (read-only after init)
    void Init(FxParams* params, FxParams* presets);

    // Dispatch a single discrete event
    void Dispatch(UIEvent ev);

    // Call every main loop iteration with current knob/encoder values.
    // shift_active: SW1 held > 200 ms
    void PollControls(float knob1, float knob2,
                      int pod_enc_delta, bool shift_active);

    // State accessors for Display
    Page  current_page()    const { return page_; }
    int   cursor()          const { return cursor_; }
    bool  edit_mode()       const { return edit_mode_; }
    bool  shift_active()    const { return shift_active_; }
    bool  preset_selected() const { return preset_selected_; }

private:
    FxParams* params_  = nullptr;
    FxParams* presets_ = nullptr;   // points to main.cpp's presets array
    Page      page_    = Page::PERFORM;
    int       cursor_  = 0;
    bool      edit_mode_      = false;
    bool      shift_active_   = false;
    bool      preset_selected_= false;  // P5: Ext PSH pressed, awaiting CON to load

    // Number of selectable items per page
    int MaxCursor() const;

    // Adjust the currently selected parameter by delta steps
    void AdjustValue(int delta);

    // Load preset idx into *params_ and return to P1
    void LoadPreset(int idx);

    // Cycle pages forward P1→P2→…→P6→P1 (Shift + Pod Enc push)
    void QuickPageJump();

    // Page wrap helpers — called when Ext Enc scrolls past page boundary
    void NextPage();
    void PrevPage();

    // ---- Tap tempo state ------------------------------------
    uint32_t tap_times_[4] = {};
    int      tap_idx_      = 0;
    int      tap_count_    = 0;
    void     HandleTap();

    // Movement-based knob handling keeps page-aware controls usable
    // without full pickup mode and avoids fighting encoder fine edits.
    float prev_knob1_ = -1.0f;
    float prev_knob2_ = -1.0f;

    static constexpr float kKnobDeadband = 0.01f;
};
