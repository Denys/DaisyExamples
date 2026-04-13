#include "ui_state.h"
#include "daisy_pod.h"   // for System::GetNow()
#include <algorithm>
#include <cmath>

using namespace daisy;

// ---- Cursor counts per page --------------------------------
// Matches the item count in each DrawP*() renderer in display.cpp

static constexpr int kMaxCursor[static_cast<int>(Page::COUNT)] = {
    5,           // P1 PERFORM:  Time, Feedback, Wet/Dry, Drive, Freeze
    5,           // P2 TONE:     InputHP, FbLoPss, FbHiPss, WetDamp, OutTilt
    3,           // P3 MOTION:   WowDepth, WowRate, ModEnable
    3,           // P4 FREEZE:   Mode, Behavior, LoopSize
    kNumPresets, // P5 PRESETS:  one cursor position per preset slot
    3,           // P6 SYSTEM:   Brightness, EncDir, Bypass
};

// ---- clamp helper ------------------------------------------

static inline float fclamp(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

// ---- Init --------------------------------------------------

void UIState::Init(FxParams* params, FxParams* presets) {
    params_       = params;
    presets_      = presets;
    page_         = Page::PERFORM;
    cursor_       = 0;
    edit_mode_    = false;
    shift_active_ = false;
    tap_idx_      = 0;
    tap_count_    = 0;
    prev_knob1_   = -1.0f;
    prev_knob2_   = -1.0f;
}

// ---- MaxCursor ---------------------------------------------

int UIState::MaxCursor() const {
    return kMaxCursor[static_cast<int>(page_)];
}

// ---- Dispatch ----------------------------------------------

void UIState::Dispatch(UIEvent ev) {
    switch (ev) {

    // External encoder: navigate cursor (normal) or adjust value (edit mode).
    // PRESETS page always navigates — edit_mode_ is explicitly cleared there
    // so there is no ambiguity about which action the encoder takes.
    case UIEvent::EXT_ENC_CW:
        if (edit_mode_ && page_ != Page::PRESETS) {
            AdjustValue(+1);
        } else {
            edit_mode_ = false;         // clear any spurious edit state on PRESETS
            cursor_++;
            if (cursor_ >= MaxCursor()) NextPage();
        }
        break;

    case UIEvent::EXT_ENC_CCW:
        if (edit_mode_ && page_ != Page::PRESETS) {
            AdjustValue(-1);
        } else {
            edit_mode_ = false;
            cursor_--;
            if (cursor_ < 0) PrevPage();
        }
        break;

    // Ext encoder push on PRESETS: mark preset as "selected" (visual only — awaits CON to load).
    // On other pages: enter/exit edit mode as usual.
    // This prevents the accidental-load problem where muscle memory causes
    // Ext PSH to immediately jump to P1 mid-navigation.
    case UIEvent::EXT_PSH_SHORT:
        if (page_ == Page::PRESETS) {
            preset_selected_ = true;    // OLED shows "CON=LOAD" hint; CON actually loads
        } else {
            edit_mode_ = !edit_mode_;
        }
        break;

    // Ext encoder long hold: exit edit (preset quick-save is Phase 6)
    case UIEvent::EXT_PSH_LONG:
        edit_mode_ = false;
        break;

    // BAK: exit edit mode, or go to P1 from any page
    case UIEvent::BAK:
        if (edit_mode_) {
            edit_mode_ = false;
        } else if (page_ != Page::PERFORM) {
            page_   = Page::PERFORM;
            cursor_ = 0;
        }
        break;

    // CON: on P5 loads the highlighted preset; elsewhere toggles edit mode.
    // Also clears preset_selected_ flag (whether or not a load happens).
    case UIEvent::CON:
        if (page_ == Page::PRESETS) {
            LoadPreset(cursor_);        // always load on CON — no two-step required
            preset_selected_ = false;
        } else {
            edit_mode_ = !edit_mode_;
        }
        break;

    // Pod encoder CW/CCW are still supported for menu-style code paths, but
    // the page-aware live control flow now happens in PollControls().
    case UIEvent::POD_ENC_CW:
        if (shift_active_) {
            params_->drive = fclamp(params_->drive + 0.02f, 0.f, 1.f);
        } else {
            params_->wet   = fclamp(params_->wet   + 0.02f, 0.f, 1.f);
        }
        break;

    case UIEvent::POD_ENC_CCW:
        if (shift_active_) {
            params_->drive = fclamp(params_->drive - 0.02f, 0.f, 1.f);
        } else {
            params_->wet   = fclamp(params_->wet   - 0.02f, 0.f, 1.f);
        }
        break;

    // Pod encoder push: page-aware action
    case UIEvent::POD_ENC_PUSH:
        switch(page_) {
        case Page::PERFORM:
        case Page::FREEZE:
            params_->freeze_latched = !params_->freeze_latched;
            break;
        case Page::TONE:
            params_->output_tilt_db = 0.0f;
            break;
        case Page::MOTION:
            params_->wow_enabled = !params_->wow_enabled;
            break;
        case Page::PRESETS:
            LoadPreset(cursor_);
            preset_selected_ = false;
            break;
        case Page::SYSTEM:
            params_->bypass = !params_->bypass;
            break;
        default:
            break;
        }
        break;

    // SW2: page-aware primary action
    case UIEvent::SW2_PRESS:
        switch(page_) {
        case Page::PERFORM:
            HandleTap();
            break;
        case Page::TONE:
        case Page::SYSTEM:
            params_->bypass = !params_->bypass;
            break;
        case Page::MOTION:
            params_->wow_enabled = !params_->wow_enabled;
            break;
        case Page::FREEZE:
            params_->freeze_loop_mode = !params_->freeze_loop_mode;
            break;
        case Page::PRESETS:
            LoadPreset(cursor_);
            preset_selected_ = false;
            break;
        default:
            break;
        }
        break;

    // Shift + Pod Enc push: quick page jump through all 6 pages.
    case UIEvent::PAGE_JUMP:
        QuickPageJump();
        break;

    // SW1 short: toggle freeze momentary (on until SW1 released — see main.cpp)
    case UIEvent::FREEZE_INSTANT:
        params_->freeze_momentary = !params_->freeze_momentary;
        break;

    // Shift + SW2: page-aware secondary action
    case UIEvent::SW2_SHIFT:
        switch(page_) {
        case Page::PERFORM:
        case Page::TONE:
            params_->sync_mode = !params_->sync_mode;
            break;
        case Page::MOTION:
            HandleTap();
            break;
        case Page::FREEZE:
            params_->freeze_latch_mode = !params_->freeze_latch_mode;
            break;
        case Page::SYSTEM:
            params_->enc_flipped = !params_->enc_flipped;
            break;
        case Page::PRESETS:
        default:
            break;
        }
        break;
    }
}

// ---- PollControls ------------------------------------------
// Called every main loop iteration with current hardware values.
// Knobs update only when moved enough to avoid fighting page-aware
// encoder controls on the same page.

void UIState::PollControls(float knob1, float knob2,
                           int pod_enc_delta, bool shift_active) {
    shift_active_ = shift_active;

    bool k1_moved = prev_knob1_ < 0.0f || std::fabs(knob1 - prev_knob1_) > kKnobDeadband;
    bool k2_moved = prev_knob2_ < 0.0f || std::fabs(knob2 - prev_knob2_) > kKnobDeadband;
    
    if (k1_moved) prev_knob1_ = knob1;
    if (k2_moved) prev_knob2_ = knob2;

    switch(page_) {
    case Page::PERFORM:
        if(shift_active) {
            if(k1_moved) params_->subdiv_idx = KnobToSubdivIdx(knob1);
            if(k2_moved) params_->fb_lp_hz   = 500.f + knob2 * 17500.f;
            if(pod_enc_delta != 0) {
                params_->drive = fclamp(
                    params_->drive + (float)pod_enc_delta * 0.02f, 0.f, 1.f);
            }
        } else {
            if(k1_moved) {
                if(params_->sync_mode)
                    params_->subdiv_idx = KnobToSubdivIdx(knob1);
                else
                    params_->delay_time_ms = 10.f + knob1 * 1990.f;
            }
            if(k2_moved) params_->feedback = knob2 * 0.98f;
            if(pod_enc_delta != 0) {
                params_->wet = fclamp(
                    params_->wet + (float)pod_enc_delta * 0.02f, 0.f, 1.f);
            }
        }
        break;

    case Page::TONE:
        if(shift_active) {
            if(k1_moved) params_->fb_hp_hz        = 20.f + knob1 * 480.f;
            if(k2_moved) params_->diffuse_damping = 1000.f + knob2 * 19000.f;
            if(pod_enc_delta != 0) {
                params_->input_gain = fclamp(
                    params_->input_gain + (float)pod_enc_delta * 0.05f, 0.5f, 2.0f);
            }
        } else {
            if(k1_moved) params_->hp_hz    = 20.f + knob1 * 480.f;
            if(k2_moved) params_->fb_lp_hz = 500.f + knob2 * 17500.f;
            if(pod_enc_delta != 0) {
                params_->output_tilt_db = fclamp(
                    params_->output_tilt_db + (float)pod_enc_delta * 0.5f, -6.f, 6.f);
            }
        }
        break;

    case Page::MOTION:
        if(shift_active) {
            if(k1_moved) params_->feedback = knob1 * 0.98f;
            if(k2_moved) params_->wet      = knob2;
            if(pod_enc_delta != 0) {
                params_->drive = fclamp(
                    params_->drive + (float)pod_enc_delta * 0.02f, 0.f, 1.f);
            }
        } else {
            if(k1_moved) params_->wow_depth   = knob1;
            if(k2_moved) params_->wow_rate_hz = 0.1f + knob2 * 4.9f;
            if(pod_enc_delta != 0) {
                params_->wow_rate_hz = fclamp(
                    params_->wow_rate_hz + (float)pod_enc_delta * 0.1f, 0.1f, 5.0f);
            }
        }
        break;

    case Page::FREEZE:
        if(shift_active) {
            if(k1_moved) {
                if(params_->sync_mode)
                    params_->subdiv_idx = KnobToSubdivIdx(knob1);
                else
                    params_->delay_time_ms = 10.f + knob1 * 1990.f;
            }
            if(k2_moved) params_->wet = knob2;
            if(pod_enc_delta != 0) {
                params_->diffuse_damping = fclamp(
                    params_->diffuse_damping + (float)pod_enc_delta * 200.f,
                    1000.f, 20000.f);
            }
        } else {
            if(k1_moved) params_->freeze_size_ms = 50.f + knob1 * 1950.f;
            if(k2_moved) params_->feedback       = knob2 * 0.98f;
            if(pod_enc_delta != 0) {
                params_->freeze_size_ms = fclamp(
                    params_->freeze_size_ms + (float)pod_enc_delta * 50.f, 50.f, 2000.f);
            }
        }
        break;

    case Page::PRESETS:
        if(k1_moved) {
            int idx = static_cast<int>(knob1 * kNumPresets);
            if(idx >= kNumPresets) idx = kNumPresets - 1;
            if(idx < 0) idx = 0;
            cursor_ = idx;
            preset_selected_ = false;
        }
        if(pod_enc_delta != 0) {
            cursor_ = std::max(0, std::min(kNumPresets - 1, cursor_ + pod_enc_delta));
            preset_selected_ = false;
        }
        break;

    case Page::SYSTEM:
        if(k1_moved) {
            int idx = static_cast<int>(knob1 * 4.0f);
            if(idx > 3) idx = 3;
            if(idx < 0) idx = 0;
            params_->brightness_idx = static_cast<uint8_t>(idx);
        }
        if(pod_enc_delta != 0) {
            int idx = static_cast<int>(params_->brightness_idx) + pod_enc_delta;
            params_->brightness_idx = static_cast<uint8_t>(std::max(0, std::min(3, idx)));
        }
        break;
    default:
        break;
    }
}

// ---- AdjustValue -------------------------------------------
// Adjusts the parameter currently under the cursor.
// Step sizes chosen to feel natural with the Ext Enc detent.

void UIState::AdjustValue(int delta) {
    switch (page_) {

    case Page::PERFORM:
        switch (cursor_) {
        case 0:  // Delay time
            if (params_->sync_mode) {
                params_->subdiv_idx = std::max(0, std::min(kNumDivs - 1,
                    params_->subdiv_idx + delta));
            } else {
                params_->delay_time_ms = fclamp(
                    params_->delay_time_ms + delta * 10.f, 10.f, 2000.f);
            }
            break;
        case 1:  // Feedback
            params_->feedback = fclamp(params_->feedback + delta * 0.02f, 0.f, 0.98f);
            break;
        case 2:  // Wet/Dry
            params_->wet      = fclamp(params_->wet      + delta * 0.02f, 0.f, 1.f);
            break;
        case 3:  // Drive
            params_->drive    = fclamp(params_->drive    + delta * 0.02f, 0.f, 1.f);
            break;
        case 4:  // Freeze mode toggle
            if (delta != 0) params_->freeze_latch_mode = !params_->freeze_latch_mode;
            break;
        }
        break;

    case Page::TONE:
        switch (cursor_) {
        case 0:  // Input HP cutoff
            params_->hp_hz = fclamp(params_->hp_hz + delta * 5.f, 20.f, 500.f);
            break;
        case 1:  // Feedback LP
            params_->fb_lp_hz = fclamp(params_->fb_lp_hz + delta * 200.f, 500.f, 18000.f);
            break;
        case 2:  // Feedback HP
            params_->fb_hp_hz = fclamp(params_->fb_hp_hz + delta * 5.f, 20.f, 500.f);
            break;
        case 3:  // Wet tone/damping
            params_->diffuse_damping = fclamp(
                params_->diffuse_damping + delta * 200.f, 1000.f, 20000.f);
            break;
        case 4:  // Output tilt
            params_->output_tilt_db = fclamp(
                params_->output_tilt_db + delta * 0.5f, -6.f, 6.f);
            break;
        }
        break;

    case Page::MOTION:
        switch (cursor_) {
        case 0:  // Wow depth
            params_->wow_depth = fclamp(params_->wow_depth + delta * 0.05f, 0.f, 1.f);
            break;
        case 1:  // Wow rate
            params_->wow_rate_hz = fclamp(
                params_->wow_rate_hz + delta * 0.1f, 0.1f, 5.0f);
            break;
        case 2:  // Mod enable toggle
            if (delta != 0) params_->wow_enabled = !params_->wow_enabled;
            break;
        }
        break;

    case Page::FREEZE:
        switch (cursor_) {
        case 0:  // Freeze mode
            if (delta != 0) params_->freeze_latch_mode = !params_->freeze_latch_mode;
            break;
        case 1:  // Hold behavior
            if (delta != 0) params_->freeze_loop_mode = !params_->freeze_loop_mode;
            break;
        case 2:  // Loop size
            params_->freeze_size_ms = fclamp(
                params_->freeze_size_ms + delta * 50.f, 50.f, 2000.f);
            break;
        }
        break;

    case Page::PRESETS:
        // Preset navigation only — load/save triggered by CON/PSH
        // (stubbed — no persistent storage in Phase 1)
        break;

    case Page::SYSTEM:
        switch (cursor_) {
        case 0: {  // Brightness index (OLED driver hook still pending)
            int idx = static_cast<int>(params_->brightness_idx) + delta;
            params_->brightness_idx = static_cast<uint8_t>(std::max(0, std::min(3, idx)));
            break;
        }
        case 1:  // Encoder direction
            if (delta != 0) params_->enc_flipped = !params_->enc_flipped;
            break;
        case 2:  // Bypass
            if (delta != 0) params_->bypass = !params_->bypass;
            break;
        }
        break;

    default:
        break;
    }
}

// ---- LoadPreset --------------------------------------------

void UIState::LoadPreset(int idx) {
    if (presets_ == nullptr)       return;
    if (idx < 0 || idx >= kNumPresets) return;

    *params_          = presets_[idx];    // copy all FxParams fields
    page_             = Page::PERFORM;    // return to P1 so user sees the loaded state
    cursor_           = 0;
    edit_mode_        = false;
    preset_selected_  = false;
}

// ---- QuickPageJump -----------------------------------------
// Shift + Pod Enc push: cycle forward through all 6 pages.

void UIState::QuickPageJump() {
    int next = (static_cast<int>(page_) + 1) % static_cast<int>(Page::COUNT);
    page_      = static_cast<Page>(next);
    cursor_    = 0;
    edit_mode_ = false;
}

// ---- NextPage / PrevPage -----------------------------------
// Called when Ext Enc scrolls past the boundary of the current page.

void UIState::NextPage() {
    int next = (static_cast<int>(page_) + 1) % static_cast<int>(Page::COUNT);
    page_           = static_cast<Page>(next);
    cursor_         = 0;
    edit_mode_      = false;
    preset_selected_= false;
}

void UIState::PrevPage() {
    int prev = (static_cast<int>(page_) - 1 + static_cast<int>(Page::COUNT))
               % static_cast<int>(Page::COUNT);
    page_           = static_cast<Page>(prev);
    cursor_         = MaxCursor() - 1;
    edit_mode_      = false;
    preset_selected_= false;
}

// ---- HandleTap ---------------------------------------------
// Rolling average of the last 4 inter-tap intervals.
// Resets if gap > 3 seconds.

void UIState::HandleTap() {
    uint32_t now = System::GetNow();

    if (tap_count_ > 0) {
        int prev = (tap_idx_ - 1 + 4) % 4;
        uint32_t gap = now - tap_times_[prev];
        if (gap > 3000) {
            tap_count_ = 0;  // Reset on long gap
        }
    }

    tap_times_[tap_idx_ % 4] = now;
    tap_idx_ = (tap_idx_ + 1) % 4;
    if (tap_count_ < 4) tap_count_++;

    if (tap_count_ >= 2) {
        // Average the last (tap_count_-1) intervals
        float sum_ms = 0.f;
        int   n      = tap_count_ - 1;
        for (int i = 0; i < n; i++) {
            int ia = (tap_idx_ - 1 - i + 4) % 4;
            int ib = (tap_idx_ - 2 - i + 4) % 4;
            sum_ms += (float)(tap_times_[ia] - tap_times_[ib]);
        }
        float avg_ms = sum_ms / (float)n;
        if (avg_ms > 0.f) {
            float new_bpm = 60000.f / avg_ms;
            params_->bpm  = fclamp(new_bpm, 40.f, 240.f);
        }
    }
}
