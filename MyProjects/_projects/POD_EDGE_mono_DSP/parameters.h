#pragma once
#include <stdint.h>

// ============================================================
// Beat subdivision table — Knob 1 zone snap (sync mode)
// ============================================================

struct BeatDiv {
    const char* label;  // display string
    float       beats;  // fraction of one beat (1.0 = quarter note)
};

static constexpr int kNumDivs = 10;

static constexpr BeatDiv kBeatDivs[kNumDivs] = {
    { "1/16 ", 0.25f   },   // zone 0: K1 0.00-0.09
    { "1/16d", 0.375f  },   // zone 1: K1 0.10-0.19  (dotted sixteenth)
    { "1/16t", 0.1667f },   // zone 2: K1 0.20-0.29  (sixteenth triplet)
    { "1/8  ", 0.5f    },   // zone 3: K1 0.30-0.39
    { "1/8d ", 0.75f   },   // zone 4: K1 0.40-0.49  (dotted eighth)
    { "1/8t ", 0.3333f },   // zone 5: K1 0.50-0.59  (eighth triplet)
    { "1/4  ", 1.0f    },   // zone 6: K1 0.60-0.69  (quarter note — default)
    { "1/4d ", 1.5f    },   // zone 7: K1 0.70-0.79  (dotted quarter)
    { "1/2  ", 2.0f    },   // zone 8: K1 0.80-0.89
    { "1/1  ", 4.0f    },   // zone 9: K1 0.90-1.00
};

// Map 0-1 knob value to subdivision index
inline int KnobToSubdivIdx(float k) {
    int idx = static_cast<int>(k * kNumDivs);
    if (idx >= kNumDivs) idx = kNumDivs - 1;
    return idx;
}

// Compute delay time in ms from BPM and subdivision index
inline float BpmToDelayMs(float bpm, int idx) {
    float beat_ms = 60000.0f / bpm;
    return beat_ms * kBeatDivs[idx].beats;
}

// ============================================================
// FxParams — central parameter store
//
// Written by main loop (with __DSB() barrier before flag swap).
// Read by AudioCallback as a cached snapshot — no hw.* calls inside ISR.
// ============================================================

struct FxParams {

    // ---- P1: Perform (live controls) ----
    float input_gain;       // 0.5 – 2.0 (Input Trim)
    float drive;            // 0.0 – 1.0 (Soft Drive amount)
    int   subdiv_idx;       // 0-9 (delay subdivision, sync mode)
    float delay_time_ms;    // 10 – 2000 ms (free mode)
    float feedback;         // 0.0 – 0.98 (hard-clamped)
    float wet;              // 0.0 – 1.0 (Wet/Dry mix)
    float bpm;              // 40 – 240 BPM (tap tempo)
    bool  sync_mode;        // true = delay time from BPM+subdiv, false = free ms

    // Freeze (fast access from P1 regardless of active page)
    bool  freeze_momentary; // true while SW1 held short (not latched)
    bool  freeze_latched;   // Pod Enc push toggles this latch
    bool  freeze_latch_mode;// P4: if true, Pod Enc push enables latch; false = momentary only

    // ---- P2: Tone ----
    float hp_hz;            // Input HP cutoff:  20 – 500 Hz
    float fb_lp_hz;         // Feedback LP:     500 – 18000 Hz
    float fb_hp_hz;         // Feedback HP:      20 – 500 Hz
    float diffuse_damping;  // Reverb/diffusion LP: 1000 – 20000 Hz
    float output_tilt_db;   // Output Tilt:      -6 – +6 dB

    // ---- P3: Motion ----
    float wow_depth;        // 0.0 – 1.0
    float wow_rate_hz;      // 0.1 – 5.0 Hz
    bool  wow_enabled;

    // ---- P4: Freeze detail ----
    bool  freeze_loop_mode; // true = loop frozen buffer, false = hold-last
    float freeze_size_ms;   // 50 – 2000 ms loop size

    // ---- P6: System ----
    uint8_t brightness_idx;   // 0..3 -> 25/50/75/100%
    bool  bypass;           // true = audio bypass
    bool  enc_flipped;      // Ext Enc direction flip

    // Factory defaults
    static FxParams Defaults() {
        FxParams p = {};
        p.input_gain        = 1.0f;
        p.drive             = 0.25f;
        p.subdiv_idx        = 6;         // 1/4 note
        p.delay_time_ms     = 500.0f;
        p.feedback          = 0.50f;
        p.wet               = 0.40f;
        p.bpm               = 120.0f;
        p.sync_mode         = true;
        p.freeze_momentary  = false;
        p.freeze_latched    = false;
        p.freeze_latch_mode = false;
        p.hp_hz             = 80.0f;
        p.fb_lp_hz          = 6000.0f;
        p.fb_hp_hz          = 60.0f;
        p.diffuse_damping   = 8000.0f;
        p.output_tilt_db    = 0.0f;
        p.wow_depth         = 0.0f;
        p.wow_rate_hz       = 1.2f;
        p.wow_enabled       = false;
        p.freeze_loop_mode  = true;
        p.freeze_size_ms    = 500.0f;
        p.brightness_idx    = 2;         // 75%
        p.bypass            = false;
        p.enc_flipped       = false;
        return p;
    }
};
