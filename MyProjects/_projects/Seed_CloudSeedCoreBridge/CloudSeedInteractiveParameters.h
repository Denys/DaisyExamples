#pragma once

namespace cloudseedbridge
{
/**
<<<<<<< ours
<<<<<<< ours
 * Rack-style interactive parameter organization.
 *
 * Mirrors the enum layout style requested by the user:
 *   - ParamIds
 *   - InputIds
 *   - OutputIds
 *   - LightIds
 *
 * Grouped per CloudSeed control class.
 */
=======
=======
>>>>>>> theirs
 * BKS CloudSeed parameter breakdown.
 *
 * Derived from:
 * https://github.com/bkshepherd/DaisySeedProjects/
 *   tree/main/Software/GuitarPedal/dependencies/CloudSeed/Parameter.h
 *
 * Source enum in that project is `Parameter2`.
 * This file organizes those parameters into Rack-style per-block classes.
 */

<<<<<<< ours
>>>>>>> theirs
=======
>>>>>>> theirs
struct CloudSeedGlobal
{
    enum ParamIds
    {
<<<<<<< ours
<<<<<<< ours
        INTERPOLATION_PARAM,
        INPUT_MIX_PARAM,
        LOW_CUT_ENABLED_PARAM,
        LOW_CUT_PARAM,
        HIGH_CUT_ENABLED_PARAM,
        HIGH_CUT_PARAM,
        DRY_OUT_PARAM,
        EARLY_OUT_PARAM,
        LATE_OUT_PARAM,
=======
=======
>>>>>>> theirs
        INPUT_MIX_PARAM,
        PRE_DELAY_PARAM,
        HIGH_PASS_PARAM,
        LOW_PASS_PARAM,
        DRY_OUT_PARAM,
        PREDELAY_OUT_PARAM,
        EARLY_OUT_PARAM,
        MAIN_OUT_PARAM,
        HIGH_PASS_ENABLED_PARAM,
        LOW_PASS_ENABLED_PARAM,
        INTERPOLATION_PARAM,
<<<<<<< ours
>>>>>>> theirs
=======
>>>>>>> theirs
        NUM_PARAMS
    };

    enum InputIds
    {
        GLOBAL_CV_INPUT,
        INPUT_MIX_CV_INPUT,
<<<<<<< ours
<<<<<<< ours
        LOW_CUT_CV_INPUT,
        HIGH_CUT_CV_INPUT,
=======
        PRE_DELAY_CV_INPUT,
        FILTER_CV_INPUT,
>>>>>>> theirs
=======
        PRE_DELAY_CV_INPUT,
        FILTER_CV_INPUT,
>>>>>>> theirs
        NUM_INPUTS
    };

    enum OutputIds
    {
        DRY_OUTPUT,
<<<<<<< ours
<<<<<<< ours
        EARLY_OUTPUT,
        LATE_OUTPUT,
=======
        PREDELAY_OUTPUT,
        EARLY_OUTPUT,
        MAIN_OUTPUT,
>>>>>>> theirs
=======
        PREDELAY_OUTPUT,
        EARLY_OUTPUT,
        MAIN_OUTPUT,
>>>>>>> theirs
        NUM_OUTPUTS
    };

    enum LightIds
    {
        INTERPOLATION_LIGHT,
        FILTERS_ENABLED_LIGHT,
        NUM_LIGHTS
    };
};

struct CloudSeedTap
{
    enum ParamIds
    {
<<<<<<< ours
<<<<<<< ours
        TAP_ENABLED_PARAM,
        TAP_COUNT_PARAM,
        TAP_DECAY_PARAM,
        TAP_PREDELAY_PARAM,
        TAP_LENGTH_PARAM,
=======
=======
>>>>>>> theirs
        TAP_COUNT_PARAM,
        TAP_LENGTH_PARAM,
        TAP_GAIN_PARAM,
        TAP_DECAY_PARAM,
<<<<<<< ours
>>>>>>> theirs
=======
>>>>>>> theirs
        NUM_PARAMS
    };

    enum InputIds
    {
<<<<<<< ours
<<<<<<< ours
        TAP_AMOUNT_INPUT,
        TAP_DECAY_INPUT,
        TAP_TIME_INPUT,
=======
=======
>>>>>>> theirs
        TAP_COUNT_CV_INPUT,
        TAP_LENGTH_CV_INPUT,
        TAP_GAIN_CV_INPUT,
        TAP_DECAY_CV_INPUT,
<<<<<<< ours
>>>>>>> theirs
=======
>>>>>>> theirs
        NUM_INPUTS
    };

    enum OutputIds
    {
        TAP_DEBUG_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds
    {
<<<<<<< ours
<<<<<<< ours
        TAP_ENABLED_LIGHT,
=======
        TAP_ACTIVE_LIGHT,
>>>>>>> theirs
=======
        TAP_ACTIVE_LIGHT,
>>>>>>> theirs
        NUM_LIGHTS
    };
};

struct CloudSeedEarlyDiffusion
{
    enum ParamIds
    {
<<<<<<< ours
<<<<<<< ours
        EARLY_DIFFUSE_ENABLED_PARAM,
        EARLY_DIFFUSE_COUNT_PARAM,
        EARLY_DIFFUSE_DELAY_PARAM,
        EARLY_DIFFUSE_MOD_AMOUNT_PARAM,
        EARLY_DIFFUSE_FEEDBACK_PARAM,
        EARLY_DIFFUSE_MOD_RATE_PARAM,
=======
=======
>>>>>>> theirs
        DIFFUSION_ENABLED_PARAM,
        DIFFUSION_STAGES_PARAM,
        DIFFUSION_DELAY_PARAM,
        DIFFUSION_FEEDBACK_PARAM,
        EARLY_DIFFUSION_MOD_AMOUNT_PARAM,
        EARLY_DIFFUSION_MOD_RATE_PARAM,
<<<<<<< ours
>>>>>>> theirs
=======
>>>>>>> theirs
        NUM_PARAMS
    };

    enum InputIds
    {
<<<<<<< ours
<<<<<<< ours
        EARLY_DIFFUSE_AMOUNT_INPUT,
        EARLY_DIFFUSE_TIME_INPUT,
        EARLY_DIFFUSE_MOD_INPUT,
=======
        EARLY_DIFFUSION_AMOUNT_INPUT,
        EARLY_DIFFUSION_TIME_INPUT,
        EARLY_DIFFUSION_MOD_INPUT,
>>>>>>> theirs
=======
        EARLY_DIFFUSION_AMOUNT_INPUT,
        EARLY_DIFFUSION_TIME_INPUT,
        EARLY_DIFFUSION_MOD_INPUT,
>>>>>>> theirs
        NUM_INPUTS
    };

    enum OutputIds
    {
<<<<<<< ours
<<<<<<< ours
        EARLY_DIFFUSE_DEBUG_OUTPUT,
=======
        EARLY_DIFFUSION_DEBUG_OUTPUT,
>>>>>>> theirs
=======
        EARLY_DIFFUSION_DEBUG_OUTPUT,
>>>>>>> theirs
        NUM_OUTPUTS
    };

    enum LightIds
    {
<<<<<<< ours
<<<<<<< ours
        EARLY_DIFFUSE_ENABLED_LIGHT,
=======
        EARLY_DIFFUSION_ENABLED_LIGHT,
>>>>>>> theirs
=======
        EARLY_DIFFUSION_ENABLED_LIGHT,
>>>>>>> theirs
        NUM_LIGHTS
    };
};

struct CloudSeedLateReverb
{
    enum ParamIds
    {
<<<<<<< ours
<<<<<<< ours
        LATE_MODE_PARAM,
        LATE_LINE_COUNT_PARAM,
        LATE_DIFFUSE_ENABLED_PARAM,
        LATE_DIFFUSE_COUNT_PARAM,
        LATE_LINE_SIZE_PARAM,
        LATE_LINE_MOD_AMOUNT_PARAM,
        LATE_DIFFUSE_DELAY_PARAM,
        LATE_DIFFUSE_MOD_AMOUNT_PARAM,
        LATE_LINE_DECAY_PARAM,
        LATE_LINE_MOD_RATE_PARAM,
        LATE_DIFFUSE_FEEDBACK_PARAM,
        LATE_DIFFUSE_MOD_RATE_PARAM,
=======
=======
>>>>>>> theirs
        LINE_COUNT_PARAM,
        LINE_DELAY_PARAM,
        LINE_DECAY_PARAM,
        LATE_DIFFUSION_ENABLED_PARAM,
        LATE_DIFFUSION_STAGES_PARAM,
        LATE_DIFFUSION_DELAY_PARAM,
        LATE_DIFFUSION_FEEDBACK_PARAM,
        LINE_MOD_AMOUNT_PARAM,
        LINE_MOD_RATE_PARAM,
        LATE_DIFFUSION_MOD_AMOUNT_PARAM,
        LATE_DIFFUSION_MOD_RATE_PARAM,
        LATE_STAGE_TAP_PARAM,
<<<<<<< ours
>>>>>>> theirs
=======
>>>>>>> theirs
        NUM_PARAMS
    };

    enum InputIds
    {
        SIZE_INPUT,
        DECAY_INPUT,
        DIFFUSION_INPUT,
        MOD_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        WET_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds
    {
<<<<<<< ours
<<<<<<< ours
        LATE_MODE_LIGHT,
        LATE_DIFFUSE_ENABLED_LIGHT,
=======
        LATE_DIFFUSION_ENABLED_LIGHT,
        LATE_STAGE_TAP_LIGHT,
>>>>>>> theirs
=======
        LATE_DIFFUSION_ENABLED_LIGHT,
        LATE_STAGE_TAP_LIGHT,
>>>>>>> theirs
        NUM_LIGHTS
    };
};

struct CloudSeedEq
{
    enum ParamIds
    {
<<<<<<< ours
<<<<<<< ours
        EQ_LOW_SHELF_ENABLED_PARAM,
        EQ_HIGH_SHELF_ENABLED_PARAM,
        EQ_LOWPASS_ENABLED_PARAM,
        EQ_LOW_FREQ_PARAM,
        EQ_HIGH_FREQ_PARAM,
        EQ_CUTOFF_PARAM,
        EQ_LOW_GAIN_PARAM,
        EQ_HIGH_GAIN_PARAM,
        EQ_CROSS_SEED_PARAM,
=======
=======
>>>>>>> theirs
        POST_LOWSHELF_GAIN_PARAM,
        POST_LOWSHELF_FREQUENCY_PARAM,
        POST_HIGHSHELF_GAIN_PARAM,
        POST_HIGHSHELF_FREQUENCY_PARAM,
        POST_CUTOFF_FREQUENCY_PARAM,
        LOWSHELF_ENABLED_PARAM,
        HIGHSHELF_ENABLED_PARAM,
        CUTOFF_ENABLED_PARAM,
<<<<<<< ours
>>>>>>> theirs
=======
>>>>>>> theirs
        NUM_PARAMS
    };

    enum InputIds
    {
        EQ_TILT_INPUT,
        EQ_FREQ_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        EQ_DEBUG_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        EQ_ENABLED_LIGHT,
        NUM_LIGHTS
    };
};

struct CloudSeedSeeds
{
    enum ParamIds
    {
<<<<<<< ours
<<<<<<< ours
        SEED_TAP_PARAM,
        SEED_DIFFUSION_PARAM,
        SEED_DELAY_PARAM,
        SEED_POST_DIFFUSION_PARAM,
=======
=======
>>>>>>> theirs
        TAP_SEED_PARAM,
        DIFFUSION_SEED_PARAM,
        DELAY_SEED_PARAM,
        POST_DIFFUSION_SEED_PARAM,
        CROSS_SEED_PARAM,
<<<<<<< ours
>>>>>>> theirs
=======
>>>>>>> theirs
        NUM_PARAMS
    };

    enum InputIds
    {
        RANDOMIZE_TRIGGER_INPUT,
        SEED_SPREAD_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        SEED_STATE_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        RANDOMIZE_LIGHT,
        NUM_LIGHTS
    };
};

/**
 * Performance layer used by the current Daisy Seed 8-knob prototype.
<<<<<<< ours
<<<<<<< ours
=======
 * These are macro controls mapped onto the Parameter2 groups above.
>>>>>>> theirs
=======
 * These are macro controls mapped onto the Parameter2 groups above.
>>>>>>> theirs
 */
struct CloudSeedPerformance
{
    enum ParamIds
    {
        MIX_PARAM,
        SIZE_PARAM,
        DECAY_PARAM,
        DIFFUSION_PARAM,
        PRE_DELAY_PARAM,
        DAMPING_PARAM,
        MODULATION_PARAM,
        MODULATION_RATE_PARAM,
        NUM_PARAMS
    };

    enum InputIds
    {
        AUDIO_L_INPUT,
        AUDIO_R_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        AUDIO_L_OUTPUT,
        AUDIO_R_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        CLIP_LIGHT,
        ACTIVE_LIGHT,
        NUM_LIGHTS
    };
};

} // namespace cloudseedbridge
