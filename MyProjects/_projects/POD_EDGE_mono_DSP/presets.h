#pragma once
#include "parameters.h"

// ============================================================
// Presets — factory data for all 10 preset slots
//
// Slots 1-5 : Performance presets  (voiced for EDGE material)
// Slots 6-10: Functionality-test presets (systematic verification)
//
// Call InitPresets() once at startup, then pass the array to UIState.
// Loading a preset copies FxParams and returns the UI to P1.
// ============================================================

static constexpr int kNumPresets = 10;

// Short names for OLED display — max 10 chars including null
// __attribute__((unused)): header included in TUs that only need kNumPresets/InitPresets
static const char* kPresetNames[kNumPresets] __attribute__((unused)) = {
    // ---- Performance ----
    "KickDub   ",   // P01
    "SnareSpace",   // P02
    "MetalBurst",   // P03
    "NoiseWash ",   // P04
    "BassEchDrv",   // P05
    // ---- Test ----
    "T:InitChek",   // P06 — dry passthrough, all params at known state
    "T:MaxFeedb",   // P07 — feedback stability at 95%
    "T:WowDeep ",   // P08 — wow engine at full depth
    "T:FullDriv",   // P09 — drive + limiter at maximum
    "T:FreezePd",   // P10 — designed to be frozen immediately
};

// Fill a FxParams[kNumPresets] array with factory data.
// All presets start from Defaults() and override specific fields.
inline void InitPresets(FxParams presets[kNumPresets]) {
    for(int i = 0; i < kNumPresets; i++) presets[i] = FxParams::Defaults();

    // ================================================================
    // P01: Kick Dub
    // Punchy sub-bass echo. Dotted-eighth delay at 120 BPM, warm
    // LP on repeats, mild drive to thicken the transient.
    // ================================================================
    presets[0].subdiv_idx      = 4;       // 1/8d
    presets[0].feedback        = 0.40f;
    presets[0].wet             = 0.45f;
    presets[0].drive           = 0.20f;
    presets[0].hp_hz           = 100.f;
    presets[0].fb_lp_hz        = 3000.f;
    presets[0].fb_hp_hz        = 80.f;
    presets[0].diffuse_damping = 5000.f;
    presets[0].output_tilt_db  = -0.5f;
    presets[0].bpm             = 120.f;

    // ================================================================
    // P02: Snare Space
    // Wide snare reinforcement. Quarter-note echo, moderate feedback,
    // diffusion adds room without obscuring transient identity.
    // ================================================================
    presets[1].subdiv_idx      = 6;       // 1/4
    presets[1].feedback        = 0.35f;
    presets[1].wet             = 0.40f;
    presets[1].drive           = 0.10f;
    presets[1].hp_hz           = 120.f;
    presets[1].fb_lp_hz        = 5000.f;
    presets[1].fb_hp_hz        = 100.f;
    presets[1].diffuse_damping = 6000.f;
    presets[1].output_tilt_db  = 0.0f;
    presets[1].bpm             = 120.f;

    // ================================================================
    // P03: Metal Burst
    // Tight metallic sixteenth echo with biting feedback.
    // Bright tone, moderate drive, output tilt +1.5 dB for presence.
    // ================================================================
    presets[2].subdiv_idx      = 0;       // 1/16
    presets[2].feedback        = 0.65f;
    presets[2].wet             = 0.35f;
    presets[2].drive           = 0.30f;
    presets[2].hp_hz           = 150.f;
    presets[2].fb_lp_hz        = 9000.f;
    presets[2].fb_hp_hz        = 150.f;
    presets[2].diffuse_damping = 12000.f;
    presets[2].output_tilt_db  = 1.5f;
    presets[2].bpm             = 140.f;

    // ================================================================
    // P04: Noise Wash
    // Long ambient texture. Half-note delay, heavy wet, strong
    // diffusion damping. Pair with freeze latch for sustained pads.
    // ================================================================
    presets[3].subdiv_idx      = 8;       // 1/2
    presets[3].feedback        = 0.72f;
    presets[3].wet             = 0.70f;
    presets[3].drive           = 0.05f;
    presets[3].hp_hz           = 60.f;
    presets[3].fb_lp_hz        = 4000.f;
    presets[3].fb_hp_hz        = 40.f;
    presets[3].diffuse_damping = 3500.f;
    presets[3].output_tilt_db  = -1.0f;
    presets[3].bpm             = 90.f;

    // ================================================================
    // P05: Bass Echo Drive
    // Saturated bass double. Eighth-note delay, driven input, low
    // feedback LP keeps only low-end weight in the repeats.
    // ================================================================
    presets[4].subdiv_idx      = 3;       // 1/8
    presets[4].feedback        = 0.40f;
    presets[4].wet             = 0.35f;
    presets[4].drive           = 0.65f;
    presets[4].hp_hz           = 80.f;
    presets[4].fb_lp_hz        = 4000.f;
    presets[4].fb_hp_hz        = 60.f;
    presets[4].diffuse_damping = 7000.f;
    presets[4].output_tilt_db  = -0.5f;
    presets[4].bpm             = 120.f;

    // ================================================================
    // P06: T: Init Check
    // All DSP disabled. Completely dry signal.
    // Expected result: input = output, no delay, no colouration.
    // Use to verify the audio signal path is clean.
    // ================================================================
    presets[5].drive           = 0.0f;
    presets[5].feedback        = 0.0f;
    presets[5].wet             = 0.0f;    // 100% dry
    presets[5].input_gain      = 1.0f;
    presets[5].hp_hz           = 20.f;   // HP at subsonic — effectively bypass
    presets[5].fb_lp_hz        = 18000.f;
    presets[5].diffuse_damping = 20000.f;
    presets[5].output_tilt_db  = 0.0f;
    presets[5].wow_enabled     = false;
    presets[5].bypass          = false;  // bypass=false; wet=0 gives same dry result

    // ================================================================
    // P07: T: Max Feedback
    // Feedback at 95% — near-infinite repeats.
    // Expected: repeats sustain many seconds without runaway.
    // Verify feedback hard-clamp protects against oscillation.
    // ================================================================
    presets[6].subdiv_idx      = 6;       // 1/4
    presets[6].feedback        = 0.95f;
    presets[6].wet             = 0.40f;
    presets[6].drive           = 0.0f;
    presets[6].fb_lp_hz        = 8000.f;
    presets[6].diffuse_damping = 10000.f;
    presets[6].bpm             = 100.f;

    // ================================================================
    // P08: T: Wow Deep
    // Maximum wow/flutter depth, audible pitch modulation at 2 Hz.
    // Expected: slow wavering pitch on sustained delay repeats.
    // Verify the wow Phasor engine is running correctly.
    // ================================================================
    presets[7].subdiv_idx      = 6;       // 1/4
    presets[7].feedback        = 0.50f;
    presets[7].wet             = 0.55f;
    presets[7].wow_depth       = 0.80f;
    presets[7].wow_rate_hz     = 2.0f;
    presets[7].wow_enabled     = true;
    presets[7].fb_lp_hz        = 7000.f;
    presets[7].bpm             = 80.f;

    // ================================================================
    // P09: T: Full Drive
    // Drive at 100%, input gain boosted.
    // Expected: heavy saturation audible, output limiter active,
    // no clipping artifacts at output. Verify drive + limiter chain.
    // ================================================================
    presets[8].drive           = 1.0f;
    presets[8].input_gain      = 1.5f;
    presets[8].feedback        = 0.20f;
    presets[8].wet             = 0.25f;
    presets[8].hp_hz           = 80.f;
    presets[8].output_tilt_db  = -1.0f;  // slightly dark to counteract drive brightness
    presets[8].bpm             = 120.f;

    // ================================================================
    // P10: T: Freeze Pad
    // Long half-note delay, high wet, latch mode enabled.
    // Expected: load preset → hit any sound → tap Pod Enc push to freeze.
    // Verify freeze engages, FRZ badge appears, audio sustains cleanly.
    // ================================================================
    presets[9].subdiv_idx      = 8;       // 1/2
    presets[9].feedback        = 0.60f;
    presets[9].wet             = 0.65f;
    presets[9].drive           = 0.05f;
    presets[9].diffuse_damping = 4000.f;
    presets[9].fb_lp_hz        = 5000.f;
    presets[9].freeze_latch_mode = true;  // Pod Enc push latches freeze
    presets[9].freeze_loop_mode  = true;  // loop the frozen buffer
    presets[9].freeze_size_ms    = 800.f;
    presets[9].bpm             = 90.f;
    presets[9].output_tilt_db  = -0.5f;
}
