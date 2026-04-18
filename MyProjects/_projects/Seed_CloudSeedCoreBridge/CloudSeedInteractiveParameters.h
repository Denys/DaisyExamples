#pragma once

namespace cloudseedbridge
{
/**
 * BKS CloudSeed parameter breakdown.
 *
 * Derived from:
 * https://github.com/bkshepherd/DaisySeedProjects/
 *   tree/main/Software/GuitarPedal/dependencies/CloudSeed/Parameter.h
 *
 * Source enum in that project is `Parameter2`.
 * This file organizes those parameters into Rack-style per-block classes.
 */

struct CloudSeedGlobal
{
    enum ParamIds
    {
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
        NUM_PARAMS
    };

    enum InputIds
    {
        GLOBAL_CV_INPUT,
        INPUT_MIX_CV_INPUT,
        PRE_DELAY_CV_INPUT,
        FILTER_CV_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        DRY_OUTPUT,
        PREDELAY_OUTPUT,
        EARLY_OUTPUT,
        MAIN_OUTPUT,
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
        TAP_COUNT_PARAM,
        TAP_LENGTH_PARAM,
        TAP_GAIN_PARAM,
        TAP_DECAY_PARAM,
        NUM_PARAMS
    };

    enum InputIds
    {
        TAP_COUNT_CV_INPUT,
        TAP_LENGTH_CV_INPUT,
        TAP_GAIN_CV_INPUT,
        TAP_DECAY_CV_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        TAP_DEBUG_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        TAP_ACTIVE_LIGHT,
        NUM_LIGHTS
    };
};

struct CloudSeedEarlyDiffusion
{
    enum ParamIds
    {
        DIFFUSION_ENABLED_PARAM,
        DIFFUSION_STAGES_PARAM,
        DIFFUSION_DELAY_PARAM,
        DIFFUSION_FEEDBACK_PARAM,
        EARLY_DIFFUSION_MOD_AMOUNT_PARAM,
        EARLY_DIFFUSION_MOD_RATE_PARAM,
        NUM_PARAMS
    };

    enum InputIds
    {
        EARLY_DIFFUSION_AMOUNT_INPUT,
        EARLY_DIFFUSION_TIME_INPUT,
        EARLY_DIFFUSION_MOD_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        EARLY_DIFFUSION_DEBUG_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        EARLY_DIFFUSION_ENABLED_LIGHT,
        NUM_LIGHTS
    };
};

struct CloudSeedLateReverb
{
    enum ParamIds
    {
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
        LATE_DIFFUSION_ENABLED_LIGHT,
        LATE_STAGE_TAP_LIGHT,
        NUM_LIGHTS
    };
};

struct CloudSeedEq
{
    enum ParamIds
    {
        POST_LOWSHELF_GAIN_PARAM,
        POST_LOWSHELF_FREQUENCY_PARAM,
        POST_HIGHSHELF_GAIN_PARAM,
        POST_HIGHSHELF_FREQUENCY_PARAM,
        POST_CUTOFF_FREQUENCY_PARAM,
        LOWSHELF_ENABLED_PARAM,
        HIGHSHELF_ENABLED_PARAM,
        CUTOFF_ENABLED_PARAM,
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
        TAP_SEED_PARAM,
        DIFFUSION_SEED_PARAM,
        DELAY_SEED_PARAM,
        POST_DIFFUSION_SEED_PARAM,
        CROSS_SEED_PARAM,
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
 * These are macro controls mapped onto the Parameter2 groups above.
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
