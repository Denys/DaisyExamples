#pragma once

namespace cloudseedbridge
{
/**
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
struct CloudSeedGlobal
{
    enum ParamIds
    {
        INTERPOLATION_PARAM,
        INPUT_MIX_PARAM,
        LOW_CUT_ENABLED_PARAM,
        LOW_CUT_PARAM,
        HIGH_CUT_ENABLED_PARAM,
        HIGH_CUT_PARAM,
        DRY_OUT_PARAM,
        EARLY_OUT_PARAM,
        LATE_OUT_PARAM,
        NUM_PARAMS
    };

    enum InputIds
    {
        GLOBAL_CV_INPUT,
        INPUT_MIX_CV_INPUT,
        LOW_CUT_CV_INPUT,
        HIGH_CUT_CV_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        DRY_OUTPUT,
        EARLY_OUTPUT,
        LATE_OUTPUT,
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
        TAP_ENABLED_PARAM,
        TAP_COUNT_PARAM,
        TAP_DECAY_PARAM,
        TAP_PREDELAY_PARAM,
        TAP_LENGTH_PARAM,
        NUM_PARAMS
    };

    enum InputIds
    {
        TAP_AMOUNT_INPUT,
        TAP_DECAY_INPUT,
        TAP_TIME_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        TAP_DEBUG_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        TAP_ENABLED_LIGHT,
        NUM_LIGHTS
    };
};

struct CloudSeedEarlyDiffusion
{
    enum ParamIds
    {
        EARLY_DIFFUSE_ENABLED_PARAM,
        EARLY_DIFFUSE_COUNT_PARAM,
        EARLY_DIFFUSE_DELAY_PARAM,
        EARLY_DIFFUSE_MOD_AMOUNT_PARAM,
        EARLY_DIFFUSE_FEEDBACK_PARAM,
        EARLY_DIFFUSE_MOD_RATE_PARAM,
        NUM_PARAMS
    };

    enum InputIds
    {
        EARLY_DIFFUSE_AMOUNT_INPUT,
        EARLY_DIFFUSE_TIME_INPUT,
        EARLY_DIFFUSE_MOD_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        EARLY_DIFFUSE_DEBUG_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        EARLY_DIFFUSE_ENABLED_LIGHT,
        NUM_LIGHTS
    };
};

struct CloudSeedLateReverb
{
    enum ParamIds
    {
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
        LATE_MODE_LIGHT,
        LATE_DIFFUSE_ENABLED_LIGHT,
        NUM_LIGHTS
    };
};

struct CloudSeedEq
{
    enum ParamIds
    {
        EQ_LOW_SHELF_ENABLED_PARAM,
        EQ_HIGH_SHELF_ENABLED_PARAM,
        EQ_LOWPASS_ENABLED_PARAM,
        EQ_LOW_FREQ_PARAM,
        EQ_HIGH_FREQ_PARAM,
        EQ_CUTOFF_PARAM,
        EQ_LOW_GAIN_PARAM,
        EQ_HIGH_GAIN_PARAM,
        EQ_CROSS_SEED_PARAM,
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
        SEED_TAP_PARAM,
        SEED_DIFFUSION_PARAM,
        SEED_DELAY_PARAM,
        SEED_POST_DIFFUSION_PARAM,
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
