#include "daisyhost/DaisyCloudSeedCore.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <memory>
#include <sstream>
#include <utility>

#include "DSP/ReverbController.h"
#include "Parameters.h"
#include "Programs.h"

namespace daisyhost
{
namespace
{
float Clamp01(float value)
{
    return std::clamp(value, 0.0f, 1.0f);
}

float QuantizeNormalized(float value, int stepCount)
{
    const float clamped = Clamp01(value);
    if(stepCount <= 0)
    {
        return clamped;
    }
    if(stepCount == 1)
    {
        return 0.0f;
    }

    const float steps = static_cast<float>(stepCount - 1);
    return std::round(clamped * steps) / steps;
}

bool HasMeaningfulChange(float left, float right)
{
    return std::abs(left - right) > 0.0001f;
}

void EnsureProgramsInitialized()
{
    static bool initialized = []() {
        Cloudseed::initPrograms();
        return true;
    }();
    static_cast<void>(initialized);
}

enum class ParameterIndex : std::size_t
{
    kMix = 0,
    kSize,
    kDecay,
    kDiffusion,
    kPreDelay,
    kDamping,
    kModAmount,
    kModRate,
    kBypass,
    kProgram,
    kArpEnabled,
    kArpRate,
    kArpPattern,
    kArpDepth,
    kArpTarget,
    kGlobalInterpolation,
    kGlobalLowCutEnabled,
    kGlobalHighCutEnabled,
    kGlobalInputMix,
    kGlobalLowCut,
    kGlobalHighCut,
    kGlobalDryOut,
    kGlobalEarlyOut,
    kGlobalLateOut,
    kTapEnabled,
    kTapCount,
    kTapDecay,
    kTapPredelay,
    kTapLength,
    kEarlyDiffusionEnabled,
    kEarlyDiffusionCount,
    kEarlyDiffusionDelay,
    kEarlyDiffusionModAmount,
    kEarlyDiffusionFeedback,
    kEarlyDiffusionModRate,
    kLateMode,
    kLateLineCount,
    kLateDiffusionEnabled,
    kLateDiffusionCount,
    kLateLineSize,
    kLateLineModAmount,
    kLateDiffusionDelay,
    kLateDiffusionModAmount,
    kLateLineDecay,
    kLateLineModRate,
    kLateDiffusionFeedback,
    kLateDiffusionModRate,
    kEqLowShelfEnabled,
    kEqHighShelfEnabled,
    kEqLowpassEnabled,
    kEqLowFreq,
    kEqHighFreq,
    kEqCutoff,
    kEqLowGain,
    kEqHighGain,
    kEqCrossSeed,
    kSeedTap,
    kSeedDiffusion,
    kSeedDelay,
    kSeedPostDiffusion,
    kCount,
};

std::size_t ToIndex(ParameterIndex index)
{
    return static_cast<std::size_t>(index);
}

struct ExpertParameterDefinition
{
    ParameterIndex index;
    int            rawIndex;
    const char*    id;
    const char*    label;
    const char*    groupLabel;
    int            stepCount;
};

constexpr std::array<ParameterIndex, 4> kArpSpaceTargets = {{
    ParameterIndex::kMix,
    ParameterIndex::kSize,
    ParameterIndex::kDecay,
    ParameterIndex::kDiffusion,
}};

constexpr std::array<ParameterIndex, 4> kArpMotionTargets = {{
    ParameterIndex::kPreDelay,
    ParameterIndex::kDamping,
    ParameterIndex::kModAmount,
    ParameterIndex::kModRate,
}};

constexpr std::array<ExpertParameterDefinition, 45> kExpertDefinitions = {{
    {ParameterIndex::kGlobalInterpolation,
     Cloudseed::Parameter::Interpolation,
     "global_interpolation",
     "Interpolation",
     "Global",
     2},
    {ParameterIndex::kGlobalLowCutEnabled,
     Cloudseed::Parameter::LowCutEnabled,
     "global_low_cut_enabled",
     "Low Cut",
     "Global",
     2},
    {ParameterIndex::kGlobalHighCutEnabled,
     Cloudseed::Parameter::HighCutEnabled,
     "global_high_cut_enabled",
     "High Cut",
     "Global",
     2},
    {ParameterIndex::kGlobalInputMix,
     Cloudseed::Parameter::InputMix,
     "global_input_mix",
     "Input Mix",
     "Global",
     0},
    {ParameterIndex::kGlobalLowCut,
     Cloudseed::Parameter::LowCut,
     "global_low_cut",
     "Low Cut Freq",
     "Global",
     0},
    {ParameterIndex::kGlobalHighCut,
     Cloudseed::Parameter::HighCut,
     "global_high_cut",
     "High Cut Freq",
     "Global",
     0},
    {ParameterIndex::kGlobalDryOut,
     Cloudseed::Parameter::DryOut,
     "global_dry_out",
     "Dry Out",
     "Global",
     0},
    {ParameterIndex::kGlobalEarlyOut,
     Cloudseed::Parameter::EarlyOut,
     "global_early_out",
     "Early Out",
     "Global",
     0},
    {ParameterIndex::kGlobalLateOut,
     Cloudseed::Parameter::LateOut,
     "global_late_out",
     "Late Out",
     "Global",
     0},
    {ParameterIndex::kTapEnabled,
     Cloudseed::Parameter::TapEnabled,
     "tap_enabled",
     "Tap Enabled",
     "Tap",
     2},
    {ParameterIndex::kTapCount,
     Cloudseed::Parameter::TapCount,
     "tap_count",
     "Tap Count",
     "Tap",
     256},
    {ParameterIndex::kTapDecay,
     Cloudseed::Parameter::TapDecay,
     "tap_decay",
     "Tap Decay",
     "Tap",
     0},
    {ParameterIndex::kTapPredelay,
     Cloudseed::Parameter::TapPredelay,
     "tap_predelay",
     "Tap Pre-Delay",
     "Tap",
     0},
    {ParameterIndex::kTapLength,
     Cloudseed::Parameter::TapLength,
     "tap_length",
     "Tap Length",
     "Tap",
     0},
    {ParameterIndex::kEarlyDiffusionEnabled,
     Cloudseed::Parameter::EarlyDiffuseEnabled,
     "early_diffusion_enabled",
     "Enabled",
     "EarlyDiffusion",
     2},
    {ParameterIndex::kEarlyDiffusionCount,
     Cloudseed::Parameter::EarlyDiffuseCount,
     "early_diffusion_count",
     "Count",
     "EarlyDiffusion",
     12},
    {ParameterIndex::kEarlyDiffusionDelay,
     Cloudseed::Parameter::EarlyDiffuseDelay,
     "early_diffusion_delay",
     "Delay",
     "EarlyDiffusion",
     0},
    {ParameterIndex::kEarlyDiffusionModAmount,
     Cloudseed::Parameter::EarlyDiffuseModAmount,
     "early_diffusion_mod_amount",
     "Mod Amount",
     "EarlyDiffusion",
     0},
    {ParameterIndex::kEarlyDiffusionFeedback,
     Cloudseed::Parameter::EarlyDiffuseFeedback,
     "early_diffusion_feedback",
     "Feedback",
     "EarlyDiffusion",
     0},
    {ParameterIndex::kEarlyDiffusionModRate,
     Cloudseed::Parameter::EarlyDiffuseModRate,
     "early_diffusion_mod_rate",
     "Mod Rate",
     "EarlyDiffusion",
     0},
    {ParameterIndex::kLateMode,
     Cloudseed::Parameter::LateMode,
     "late_mode",
     "Mode",
     "LateReverb",
     2},
    {ParameterIndex::kLateLineCount,
     Cloudseed::Parameter::LateLineCount,
     "late_line_count",
     "Line Count",
     "LateReverb",
     12},
    {ParameterIndex::kLateDiffusionEnabled,
     Cloudseed::Parameter::LateDiffuseEnabled,
     "late_diffusion_enabled",
     "Diffusion Enabled",
     "LateReverb",
     2},
    {ParameterIndex::kLateDiffusionCount,
     Cloudseed::Parameter::LateDiffuseCount,
     "late_diffusion_count",
     "Diffusion Count",
     "LateReverb",
     8},
    {ParameterIndex::kLateLineSize,
     Cloudseed::Parameter::LateLineSize,
     "late_line_size",
     "Line Size",
     "LateReverb",
     0},
    {ParameterIndex::kLateLineModAmount,
     Cloudseed::Parameter::LateLineModAmount,
     "late_line_mod_amount",
     "Line Mod Amt",
     "LateReverb",
     0},
    {ParameterIndex::kLateDiffusionDelay,
     Cloudseed::Parameter::LateDiffuseDelay,
     "late_diffusion_delay",
     "Diffusion Delay",
     "LateReverb",
     0},
    {ParameterIndex::kLateDiffusionModAmount,
     Cloudseed::Parameter::LateDiffuseModAmount,
     "late_diffusion_mod_amount",
     "Diffusion Mod Amt",
     "LateReverb",
     0},
    {ParameterIndex::kLateLineDecay,
     Cloudseed::Parameter::LateLineDecay,
     "late_line_decay",
     "Line Decay",
     "LateReverb",
     0},
    {ParameterIndex::kLateLineModRate,
     Cloudseed::Parameter::LateLineModRate,
     "late_line_mod_rate",
     "Line Mod Rate",
     "LateReverb",
     0},
    {ParameterIndex::kLateDiffusionFeedback,
     Cloudseed::Parameter::LateDiffuseFeedback,
     "late_diffusion_feedback",
     "Diffusion Feedback",
     "LateReverb",
     0},
    {ParameterIndex::kLateDiffusionModRate,
     Cloudseed::Parameter::LateDiffuseModRate,
     "late_diffusion_mod_rate",
     "Diffusion Mod Rate",
     "LateReverb",
     0},
    {ParameterIndex::kEqLowShelfEnabled,
     Cloudseed::Parameter::EqLowShelfEnabled,
     "eq_low_shelf_enabled",
     "Low Shelf",
     "Eq",
     2},
    {ParameterIndex::kEqHighShelfEnabled,
     Cloudseed::Parameter::EqHighShelfEnabled,
     "eq_high_shelf_enabled",
     "High Shelf",
     "Eq",
     2},
    {ParameterIndex::kEqLowpassEnabled,
     Cloudseed::Parameter::EqLowpassEnabled,
     "eq_lowpass_enabled",
     "Lowpass",
     "Eq",
     2},
    {ParameterIndex::kEqLowFreq,
     Cloudseed::Parameter::EqLowFreq,
     "eq_low_freq",
     "Low Freq",
     "Eq",
     0},
    {ParameterIndex::kEqHighFreq,
     Cloudseed::Parameter::EqHighFreq,
     "eq_high_freq",
     "High Freq",
     "Eq",
     0},
    {ParameterIndex::kEqCutoff,
     Cloudseed::Parameter::EqCutoff,
     "eq_cutoff",
     "Cutoff",
     "Eq",
     0},
    {ParameterIndex::kEqLowGain,
     Cloudseed::Parameter::EqLowGain,
     "eq_low_gain",
     "Low Gain",
     "Eq",
     0},
    {ParameterIndex::kEqHighGain,
     Cloudseed::Parameter::EqHighGain,
     "eq_high_gain",
     "High Gain",
     "Eq",
     0},
    {ParameterIndex::kEqCrossSeed,
     Cloudseed::Parameter::EqCrossSeed,
     "eq_cross_seed",
     "Cross Seed",
     "Eq",
     0},
    {ParameterIndex::kSeedTap,
     Cloudseed::Parameter::SeedTap,
     "seed_tap",
     "Tap Seed",
     "Seeds",
     1000},
    {ParameterIndex::kSeedDiffusion,
     Cloudseed::Parameter::SeedDiffusion,
     "seed_diffusion",
     "Diffusion Seed",
     "Seeds",
     1000},
    {ParameterIndex::kSeedDelay,
     Cloudseed::Parameter::SeedDelay,
     "seed_delay",
     "Delay Seed",
     "Seeds",
     1000},
    {ParameterIndex::kSeedPostDiffusion,
     Cloudseed::Parameter::SeedPostDiffusion,
     "seed_post_diffusion",
     "Post Diffusion Seed",
     "Seeds",
     1000},
}};

bool IsSafeAdvancedFieldParameter(ParameterIndex index)
{
    switch(index)
    {
        case ParameterIndex::kEqLowFreq:
        case ParameterIndex::kEqHighFreq:
        case ParameterIndex::kEqCutoff:
        case ParameterIndex::kEqLowGain:
        case ParameterIndex::kEqHighGain:
        case ParameterIndex::kEqCrossSeed:
        case ParameterIndex::kSeedDiffusion:
        case ParameterIndex::kSeedDelay: return true;
        default: return false;
    }
}

std::size_t SeedNumber(float normalizedValue)
{
    const float clamped = Clamp01(normalizedValue);
    return static_cast<std::size_t>(std::round(clamped * 999.0f));
}

std::string FormatSeedValue(float normalizedValue)
{
    char buffer[8];
    std::snprintf(buffer, sizeof(buffer), "%03d",
                  static_cast<int>(SeedNumber(normalizedValue)));
    return std::string(buffer);
}
} // namespace

struct DaisyCloudSeedCore::Impl
{
    double                                        sampleRate   = 48000.0;
    std::size_t                                   maxBlockSize = kPreferredBlockSize;
    std::unique_ptr<Cloudseed::ReverbController>  controller;
    std::vector<DaisyCloudSeedParameter>          parameters;
    std::array<float, Cloudseed::Parameter::COUNT> effectiveEngineParameters{};
    DaisyCloudSeedPage                            activePage = DaisyCloudSeedPage::kSpace;
    std::uint32_t                                 randomState = 1u;
    std::uint64_t                                 arpSamplesIntoStep = 0;
    std::uint32_t                                 arpStepIndex = 0;
    std::vector<float>                            tempInLeft;
    std::vector<float>                            tempInRight;
    std::vector<float>                            tempOutLeft;
    std::vector<float>                            tempOutRight;

    DaisyCloudSeedParameter& Param(ParameterIndex index)
    {
        return parameters[ToIndex(index)];
    }

    const DaisyCloudSeedParameter& Param(ParameterIndex index) const
    {
        return parameters[ToIndex(index)];
    }

    void BuildParameters()
    {
        EnsureProgramsInitialized();
        parameters.clear();
        parameters.reserve(ToIndex(ParameterIndex::kCount));

        auto addParameter = [this](const std::string& id,
                                   const std::string& label,
                                   const std::string& groupLabel,
                                   float              defaultNormalizedValue,
                                   int                stepCount,
                                   int                importanceRank,
                                   bool               automatable,
                                   bool               stateful,
                                   bool               performanceTier) {
            DaisyCloudSeedParameter parameter;
            parameter.id                      = id;
            parameter.label                   = label;
            parameter.groupLabel              = groupLabel;
            parameter.normalizedValue         = defaultNormalizedValue;
            parameter.defaultNormalizedValue  = defaultNormalizedValue;
            parameter.effectiveNormalizedValue = defaultNormalizedValue;
            parameter.stepCount              = stepCount;
            parameter.importanceRank         = importanceRank;
            parameter.automatable            = automatable;
            parameter.stateful               = stateful;
            parameter.performanceTier        = performanceTier;
            parameters.push_back(std::move(parameter));
        };

        addParameter("mix", "Mix", "Performance", 0.45f, 0, 0, true, true, true);
        addParameter("size", "Size", "Performance", 0.46f, 0, 1, true, true, true);
        addParameter("decay", "Decay", "Performance", 0.63f, 0, 2, true, true, true);
        addParameter("diffusion",
                     "Diffusion",
                     "Performance",
                     0.54f,
                     0,
                     3,
                     true,
                     true,
                     true);
        addParameter("pre_delay",
                     "Pre-Delay",
                     "Performance",
                     0.04f,
                     0,
                     4,
                     true,
                     true,
                     true);
        addParameter("damping",
                     "Damping",
                     "Performance",
                     0.42f,
                     0,
                     5,
                     true,
                     true,
                     true);
        addParameter("mod_amount",
                     "Mod Amount",
                     "Performance",
                     0.28f,
                     0,
                     6,
                     true,
                     true,
                     true);
        addParameter("mod_rate",
                     "Mod Rate",
                     "Performance",
                     0.23f,
                     0,
                     7,
                     true,
                     true,
                     true);
        addParameter("bypass", "Bypass", "Utilities", 0.0f, 2, 100, false, true, false);
        addParameter("program", "Program", "Program", 0.0f, 1, 101, false, true, false);
        addParameter("arp_enabled", "Arp Enable", "Arp", 0.0f, 2, 120, false, true, false);
        addParameter("arp_rate", "Arp Rate", "Arp", 0.33333334f, 4, 121, false, true, false);
        addParameter("arp_pattern", "Arp Pattern", "Arp", 0.0f, 3, 122, false, true, false);
        addParameter("arp_depth", "Arp Depth", "Arp", 0.35f, 0, 123, false, true, false);
        addParameter("arp_target", "Arp Target", "Arp", 0.0f, 2, 124, false, true, false);

        for(const auto& definition : kExpertDefinitions)
        {
            addParameter(definition.id,
                         definition.label,
                         definition.groupLabel,
                         static_cast<float>(Cloudseed::ProgramDarkPlate[definition.rawIndex]),
                         definition.stepCount,
                         200 + definition.rawIndex,
                         IsSafeAdvancedFieldParameter(definition.index),
                         true,
                         false);
        }
    }

    const DaisyCloudSeedParameter* FindParameter(const std::string& parameterId) const
    {
        const auto it = std::find_if(parameters.begin(),
                                     parameters.end(),
                                     [&parameterId](const DaisyCloudSeedParameter& parameter) {
                                         return parameter.id == parameterId;
                                     });
        return it != parameters.end() ? &(*it) : nullptr;
    }

    DaisyCloudSeedParameter* FindParameter(const std::string& parameterId)
    {
        const auto it = std::find_if(parameters.begin(),
                                     parameters.end(),
                                     [&parameterId](const DaisyCloudSeedParameter& parameter) {
                                         return parameter.id == parameterId;
                                     });
        return it != parameters.end() ? &(*it) : nullptr;
    }

    bool IsArpEnabled() const
    {
        return Param(ParameterIndex::kArpEnabled).normalizedValue >= 0.5f
               && Param(ParameterIndex::kArpDepth).normalizedValue > 0.0001f;
    }

    bool IsArpControlParameter(ParameterIndex index) const
    {
        return index == ParameterIndex::kArpEnabled
               || index == ParameterIndex::kArpRate
               || index == ParameterIndex::kArpPattern
               || index == ParameterIndex::kArpDepth
               || index == ParameterIndex::kArpTarget;
    }

    std::uint64_t ArpSamplesPerStep() const
    {
        static constexpr std::array<double, 4> kSecondsPerStep = {
            {0.03125, 0.0625, 0.125, 0.25}};
        const int index = std::clamp(
            static_cast<int>(
                std::round(Param(ParameterIndex::kArpRate).normalizedValue
                           * static_cast<float>(kSecondsPerStep.size() - 1))),
            0,
            static_cast<int>(kSecondsPerStep.size()) - 1);
        return std::max<std::uint64_t>(
            1u,
            static_cast<std::uint64_t>(
                std::llround(sampleRate * kSecondsPerStep[static_cast<std::size_t>(index)])));
    }

    void ResetArpPhase()
    {
        arpSamplesIntoStep = 0;
        arpStepIndex       = 0;
    }

    ParameterIndex SelectArpTarget() const
    {
        const auto& targets = Param(ParameterIndex::kArpTarget).normalizedValue >= 0.5f
                                  ? kArpMotionTargets
                                  : kArpSpaceTargets;
        const int pattern = std::clamp(
            static_cast<int>(
                std::round(Param(ParameterIndex::kArpPattern).normalizedValue * 2.0f)),
            0,
            2);

        std::size_t position = 0;
        if(pattern == 1)
        {
            position = targets.size() - 1u
                       - (static_cast<std::size_t>(arpStepIndex) % targets.size());
        }
        else if(pattern == 2)
        {
            static constexpr std::array<std::size_t, 6> kPendulum = {{0, 1, 2, 3, 2, 1}};
            position = kPendulum[static_cast<std::size_t>(arpStepIndex)
                                 % kPendulum.size()];
        }
        else
        {
            position = static_cast<std::size_t>(arpStepIndex) % targets.size();
        }

        return targets[position];
    }

    void ApplyArpBump(ParameterIndex selected,
                      float&         mix,
                      float&         size,
                      float&         decay,
                      float&         diffusion,
                      float&         preDelay,
                      float&         damping,
                      float&         modAmount,
                      float&         modRate) const
    {
        const float bump = 0.5f * Param(ParameterIndex::kArpDepth).normalizedValue;
        auto apply = [selected, bump](ParameterIndex index, float& value) {
            if(selected == index)
            {
                value = Clamp01(value + bump);
            }
        };

        apply(ParameterIndex::kMix, mix);
        apply(ParameterIndex::kSize, size);
        apply(ParameterIndex::kDecay, decay);
        apply(ParameterIndex::kDiffusion, diffusion);
        apply(ParameterIndex::kPreDelay, preDelay);
        apply(ParameterIndex::kDamping, damping);
        apply(ParameterIndex::kModAmount, modAmount);
        apply(ParameterIndex::kModRate, modRate);
    }

    void UpdateEffectiveState()
    {
        for(const auto& definition : kExpertDefinitions)
        {
            effectiveEngineParameters[definition.rawIndex]
                = Param(definition.index).normalizedValue;
        }

        float mix        = Param(ParameterIndex::kMix).normalizedValue;
        float size       = Param(ParameterIndex::kSize).normalizedValue;
        float decay      = Param(ParameterIndex::kDecay).normalizedValue;
        float diffusion  = Param(ParameterIndex::kDiffusion).normalizedValue;
        float preDelay   = Param(ParameterIndex::kPreDelay).normalizedValue;
        float damping    = Param(ParameterIndex::kDamping).normalizedValue;
        float modAmount  = Param(ParameterIndex::kModAmount).normalizedValue;
        float modRate    = Param(ParameterIndex::kModRate).normalizedValue;

        if(IsArpEnabled())
        {
            ApplyArpBump(SelectArpTarget(),
                         mix,
                         size,
                         decay,
                         diffusion,
                         preDelay,
                         damping,
                         modAmount,
                         modRate);
        }

        // v1 keeps the performance layer authoritative and derives the
        // underlying CloudSeed parameter set from it while preserving the
        // raw expert parameters as canonical state for later expansion.
        effectiveEngineParameters[Cloudseed::Parameter::InputMix]            = 0.18f;
        effectiveEngineParameters[Cloudseed::Parameter::DryOut]              = 0.0f;
        effectiveEngineParameters[Cloudseed::Parameter::EarlyOut]            = Clamp01(0.30f + 0.18f * diffusion);
        effectiveEngineParameters[Cloudseed::Parameter::LateOut]             = 0.76f;
        effectiveEngineParameters[Cloudseed::Parameter::TapPredelay]         = preDelay;
        effectiveEngineParameters[Cloudseed::Parameter::TapLength]           = Clamp01(0.15f + 0.80f * size);
        effectiveEngineParameters[Cloudseed::Parameter::TapDecay]            = Clamp01(0.20f + 0.75f * decay);
        effectiveEngineParameters[Cloudseed::Parameter::EarlyDiffuseEnabled] = 1.0f;
        effectiveEngineParameters[Cloudseed::Parameter::EarlyDiffuseCount]   = Clamp01(0.10f + 0.55f * diffusion);
        effectiveEngineParameters[Cloudseed::Parameter::EarlyDiffuseDelay]   = Clamp01(0.08f + 0.48f * diffusion);
        effectiveEngineParameters[Cloudseed::Parameter::EarlyDiffuseModAmount] = Clamp01(0.60f * modAmount);
        effectiveEngineParameters[Cloudseed::Parameter::EarlyDiffuseFeedback]  = Clamp01(0.28f + 0.55f * diffusion);
        effectiveEngineParameters[Cloudseed::Parameter::EarlyDiffuseModRate]   = modRate;
        effectiveEngineParameters[Cloudseed::Parameter::LateMode]              = 1.0f;
        effectiveEngineParameters[Cloudseed::Parameter::LateLineCount]         = 0.55f;
        effectiveEngineParameters[Cloudseed::Parameter::LateDiffuseEnabled]    = 1.0f;
        effectiveEngineParameters[Cloudseed::Parameter::LateDiffuseCount]      = Clamp01(0.24f + 0.55f * diffusion);
        effectiveEngineParameters[Cloudseed::Parameter::LateLineSize]          = Clamp01(0.12f + 0.78f * size);
        effectiveEngineParameters[Cloudseed::Parameter::LateLineModAmount]     = Clamp01(0.70f * modAmount);
        effectiveEngineParameters[Cloudseed::Parameter::LateDiffuseDelay]      = Clamp01(0.14f + 0.40f * diffusion);
        effectiveEngineParameters[Cloudseed::Parameter::LateDiffuseModAmount]  = Clamp01(0.50f * modAmount);
        effectiveEngineParameters[Cloudseed::Parameter::LateLineDecay]         = Clamp01(0.20f + 0.75f * decay);
        effectiveEngineParameters[Cloudseed::Parameter::LateLineModRate]       = modRate;
        effectiveEngineParameters[Cloudseed::Parameter::LateDiffuseFeedback]   = Clamp01(0.45f + 0.40f * diffusion);
        effectiveEngineParameters[Cloudseed::Parameter::LateDiffuseModRate]    = modRate;
        effectiveEngineParameters[Cloudseed::Parameter::HighCut]               = Clamp01(0.18f + 0.80f * (1.0f - damping));

        Param(ParameterIndex::kMix).effectiveNormalizedValue        = mix;
        Param(ParameterIndex::kSize).effectiveNormalizedValue       = size;
        Param(ParameterIndex::kDecay).effectiveNormalizedValue      = decay;
        Param(ParameterIndex::kDiffusion).effectiveNormalizedValue  = diffusion;
        Param(ParameterIndex::kPreDelay).effectiveNormalizedValue   = preDelay;
        Param(ParameterIndex::kDamping).effectiveNormalizedValue    = damping;
        Param(ParameterIndex::kModAmount).effectiveNormalizedValue  = modAmount;
        Param(ParameterIndex::kModRate).effectiveNormalizedValue    = modRate;
        Param(ParameterIndex::kBypass).effectiveNormalizedValue     = Param(ParameterIndex::kBypass).normalizedValue;
        Param(ParameterIndex::kProgram).effectiveNormalizedValue    = Param(ParameterIndex::kProgram).normalizedValue;
        Param(ParameterIndex::kArpEnabled).effectiveNormalizedValue = Param(ParameterIndex::kArpEnabled).normalizedValue;
        Param(ParameterIndex::kArpRate).effectiveNormalizedValue    = Param(ParameterIndex::kArpRate).normalizedValue;
        Param(ParameterIndex::kArpPattern).effectiveNormalizedValue = Param(ParameterIndex::kArpPattern).normalizedValue;
        Param(ParameterIndex::kArpDepth).effectiveNormalizedValue   = Param(ParameterIndex::kArpDepth).normalizedValue;
        Param(ParameterIndex::kArpTarget).effectiveNormalizedValue  = Param(ParameterIndex::kArpTarget).normalizedValue;

        for(const auto& definition : kExpertDefinitions)
        {
            Param(definition.index).effectiveNormalizedValue
                = effectiveEngineParameters[definition.rawIndex];
        }

        if(controller == nullptr)
        {
            return;
        }

        controller->SetSamplerate(static_cast<int>(sampleRate));
        for(int rawIndex = 0; rawIndex < Cloudseed::Parameter::COUNT; ++rawIndex)
        {
            controller->SetParameter(rawIndex, effectiveEngineParameters[rawIndex]);
        }
    }

    void AdvanceArp(std::size_t frameCount)
    {
        if(!IsArpEnabled())
        {
            return;
        }

        arpSamplesIntoStep += static_cast<std::uint64_t>(frameCount);
        const auto samplesPerStep = ArpSamplesPerStep();
        bool       advanced       = false;
        while(arpSamplesIntoStep >= samplesPerStep)
        {
            arpSamplesIntoStep -= samplesPerStep;
            ++arpStepIndex;
            advanced = true;
        }

        if(advanced)
        {
            UpdateEffectiveState();
        }
    }

    std::uint32_t NextRandom()
    {
        randomState = randomState * 1664525u + 1013904223u;
        return randomState;
    }

    void RandomizeSeedParameters()
    {
        auto randomSeedNormalized = [this]() {
            return static_cast<float>(NextRandom() % 1000u) / 999.0f;
        };

        Param(ParameterIndex::kSeedTap).normalizedValue
            = QuantizeNormalized(randomSeedNormalized(), 1000);
        Param(ParameterIndex::kSeedDiffusion).normalizedValue
            = QuantizeNormalized(randomSeedNormalized(), 1000);
        Param(ParameterIndex::kSeedDelay).normalizedValue
            = QuantizeNormalized(randomSeedNormalized(), 1000);
        Param(ParameterIndex::kSeedPostDiffusion).normalizedValue
            = QuantizeNormalized(randomSeedNormalized(), 1000);
    }
};

DaisyCloudSeedCore::DaisyCloudSeedCore()
: impl_(std::make_unique<Impl>())
{
    impl_->BuildParameters();
    impl_->UpdateEffectiveState();
}

DaisyCloudSeedCore::~DaisyCloudSeedCore() = default;

void DaisyCloudSeedCore::Prepare(double sampleRate, std::size_t maxBlockSize)
{
    impl_->sampleRate   = sampleRate > 1000.0 ? sampleRate : 48000.0;
    impl_->maxBlockSize = std::max<std::size_t>(1, maxBlockSize);

    if(impl_->controller == nullptr)
    {
        impl_->controller = std::make_unique<Cloudseed::ReverbController>(
            static_cast<int>(impl_->sampleRate));
    }
    else
    {
        impl_->controller->SetSamplerate(static_cast<int>(impl_->sampleRate));
    }

    impl_->UpdateEffectiveState();
    impl_->controller->ClearBuffers();
}

void DaisyCloudSeedCore::Process(const float* inputLeft,
                                 const float* inputRight,
                                 float*       outputLeft,
                                 float*       outputRight,
                                 std::size_t  frameCount)
{
    if(frameCount == 0 || outputLeft == nullptr || outputRight == nullptr)
    {
        return;
    }

    if(impl_->controller == nullptr)
    {
        Prepare(impl_->sampleRate, std::max(impl_->maxBlockSize, frameCount));
    }

    impl_->AdvanceArp(frameCount);

    impl_->tempInLeft.resize(frameCount, 0.0f);
    impl_->tempInRight.resize(frameCount, 0.0f);
    impl_->tempOutLeft.resize(frameCount, 0.0f);
    impl_->tempOutRight.resize(frameCount, 0.0f);

    for(std::size_t i = 0; i < frameCount; ++i)
    {
        impl_->tempInLeft[i]  = inputLeft != nullptr ? inputLeft[i] : 0.0f;
        impl_->tempInRight[i] = inputRight != nullptr ? inputRight[i]
                                                      : impl_->tempInLeft[i];
    }

    const bool bypassed = impl_->Param(ParameterIndex::kBypass).normalizedValue >= 0.5f;
    if(bypassed)
    {
        for(std::size_t i = 0; i < frameCount; ++i)
        {
            outputLeft[i]  = impl_->tempInLeft[i];
            outputRight[i] = impl_->tempInRight[i];
        }
        return;
    }

    impl_->controller->Process(impl_->tempInLeft.data(),
                               impl_->tempInRight.data(),
                               impl_->tempOutLeft.data(),
                               impl_->tempOutRight.data(),
                               static_cast<int>(frameCount));

    const float mix = impl_->Param(ParameterIndex::kMix).effectiveNormalizedValue;
    const float dry = 1.0f - mix;
    for(std::size_t i = 0; i < frameCount; ++i)
    {
        outputLeft[i]  = dry * impl_->tempInLeft[i] + mix * impl_->tempOutLeft[i];
        outputRight[i] = dry * impl_->tempInRight[i] + mix * impl_->tempOutRight[i];
    }
}

void DaisyCloudSeedCore::ResetToDefaultState(std::uint32_t seed)
{
    impl_->activePage  = DaisyCloudSeedPage::kSpace;
    impl_->randomState = seed == 0 ? 0xC10D5EEDu : seed;
    impl_->ResetArpPhase();

    for(auto& parameter : impl_->parameters)
    {
        parameter.normalizedValue         = parameter.defaultNormalizedValue;
        parameter.effectiveNormalizedValue = parameter.defaultNormalizedValue;
    }

    if(seed != 0)
    {
        impl_->RandomizeSeedParameters();
    }

    impl_->UpdateEffectiveState();
    if(impl_->controller != nullptr)
    {
        impl_->controller->ClearBuffers();
    }
}

bool DaisyCloudSeedCore::SetParameterValue(const std::string& parameterId,
                                           float              normalizedValue)
{
    auto* parameter = impl_->FindParameter(parameterId);
    if(parameter == nullptr)
    {
        return false;
    }

    const float quantized = QuantizeNormalized(normalizedValue, parameter->stepCount);
    if(!HasMeaningfulChange(parameter->normalizedValue, quantized))
    {
        return true;
    }

    parameter->normalizedValue = quantized;
    const auto parameterIndex = static_cast<ParameterIndex>(
        static_cast<std::size_t>(parameter - impl_->parameters.data()));
    if(impl_->IsArpControlParameter(parameterIndex))
    {
        impl_->ResetArpPhase();
    }
    impl_->UpdateEffectiveState();
    return true;
}

bool DaisyCloudSeedCore::GetParameterValue(const std::string& parameterId,
                                           float*             normalizedValue) const
{
    const auto* parameter = impl_->FindParameter(parameterId);
    if(parameter == nullptr || normalizedValue == nullptr)
    {
        return false;
    }

    *normalizedValue = parameter->normalizedValue;
    return true;
}

bool DaisyCloudSeedCore::GetEffectiveParameterValue(
    const std::string& parameterId,
    float*             normalizedValue) const
{
    const auto* parameter = impl_->FindParameter(parameterId);
    if(parameter == nullptr || normalizedValue == nullptr)
    {
        return false;
    }

    *normalizedValue = parameter->effectiveNormalizedValue;
    return true;
}

bool DaisyCloudSeedCore::TriggerMomentaryAction(const std::string& actionId)
{
    if(actionId == "randomize_seeds")
    {
        impl_->RandomizeSeedParameters();
        impl_->UpdateEffectiveState();
        return true;
    }
    if(actionId == "clear_tails")
    {
        if(impl_->controller != nullptr)
        {
            impl_->controller->ClearBuffers();
        }
        return true;
    }
    return false;
}

const DaisyCloudSeedParameter* DaisyCloudSeedCore::FindParameter(
    const std::string& parameterId) const
{
    return impl_->FindParameter(parameterId);
}

const std::vector<DaisyCloudSeedParameter>& DaisyCloudSeedCore::GetParameters()
    const
{
    return impl_->parameters;
}

std::unordered_map<std::string, float>
DaisyCloudSeedCore::CaptureStatefulParameterValues() const
{
    std::unordered_map<std::string, float> values;
    for(const auto& parameter : impl_->parameters)
    {
        if(parameter.stateful)
        {
            values.emplace(parameter.id, parameter.normalizedValue);
        }
    }
    return values;
}

void DaisyCloudSeedCore::RestoreStatefulParameterValues(
    const std::unordered_map<std::string, float>& values)
{
    for(const auto& entry : values)
    {
        auto* parameter = impl_->FindParameter(entry.first);
        if(parameter == nullptr || !parameter->stateful)
        {
            continue;
        }
        parameter->normalizedValue = QuantizeNormalized(entry.second, parameter->stepCount);
    }
    impl_->ResetArpPhase();
    impl_->UpdateEffectiveState();
}

DaisyCloudSeedPage DaisyCloudSeedCore::GetActivePage() const
{
    return impl_->activePage;
}

bool DaisyCloudSeedCore::SetActivePage(DaisyCloudSeedPage page)
{
    if(impl_->activePage == page)
    {
        return false;
    }

    impl_->activePage = page;
    return true;
}

DaisyCloudSeedPageBinding DaisyCloudSeedCore::GetActivePageBinding() const
{
    DaisyCloudSeedPageBinding binding;
    binding.page = impl_->activePage;

    if(impl_->activePage == DaisyCloudSeedPage::kAdvanced)
    {
        binding.pageLabel      = "Advanced";
        binding.parameterIds   = {{"eq_low_freq", "eq_high_freq", "eq_cutoff", "eq_low_gain"}};
        binding.parameterLabels = {{"Low Freq", "High Freq", "Cutoff", "Low Gain"}};
        binding.fieldParameterIds = {{"eq_low_freq",
                                      "eq_high_freq",
                                      "eq_cutoff",
                                      "eq_low_gain",
                                      "eq_high_gain",
                                      "eq_cross_seed",
                                      "seed_diffusion",
                                      "seed_delay"}};
        binding.fieldParameterLabels = {{"Low Freq",
                                         "High Freq",
                                         "Cutoff",
                                         "Low Gain",
                                         "High Gain",
                                         "Cross Seed",
                                         "Diff Seed",
                                         "Delay Seed"}};
        return binding;
    }

    if(impl_->activePage == DaisyCloudSeedPage::kArp)
    {
        binding.pageLabel      = "Arp";
        binding.parameterIds   = {{"arp_enabled", "arp_rate", "arp_pattern", "arp_target"}};
        binding.parameterLabels = {{"Arp Enable", "Arp Rate", "Pattern", "Target"}};
        binding.fieldParameterIds = {{"arp_enabled",
                                      "arp_rate",
                                      "arp_pattern",
                                      "arp_target",
                                      "arp_depth",
                                      "",
                                      "",
                                      ""}};
        binding.fieldParameterLabels = {{"Arp Enable",
                                         "Arp Rate",
                                         "Pattern",
                                         "Target",
                                         "Arp Depth",
                                         "",
                                         "",
                                         ""}};
        return binding;
    }

    if(impl_->activePage == DaisyCloudSeedPage::kMotion)
    {
        binding.pageLabel      = "Motion";
        binding.parameterIds   = {{"pre_delay", "damping", "mod_amount", "mod_rate"}};
        binding.parameterLabels = {{"Pre-Delay", "Damping", "Mod Amt", "Mod Rate"}};
        binding.fieldParameterIds = {{"pre_delay",
                                      "damping",
                                      "mod_amount",
                                      "mod_rate",
                                      "mix",
                                      "size",
                                      "decay",
                                      "diffusion"}};
        binding.fieldParameterLabels = {{"Pre-Delay",
                                         "Damping",
                                         "Mod Amt",
                                         "Mod Rate",
                                         "Mix",
                                         "Size",
                                         "Decay",
                                         "Diffusion"}};
        return binding;
    }

    binding.pageLabel      = "Space";
    binding.parameterIds   = {{"mix", "size", "decay", "diffusion"}};
    binding.parameterLabels = {{"Mix", "Size", "Decay", "Diffusion"}};
    binding.fieldParameterIds = {{"mix",
                                  "size",
                                  "decay",
                                  "diffusion",
                                  "pre_delay",
                                  "damping",
                                  "mod_amount",
                                  "mod_rate"}};
    binding.fieldParameterLabels = {{"Mix",
                                     "Size",
                                     "Decay",
                                     "Diffusion",
                                     "Pre-Delay",
                                     "Damping",
                                     "Mod Amt",
                                     "Mod Rate"}};
    return binding;
}

std::string DaisyCloudSeedCore::GetProgramLabel() const
{
    return "Dark Plate";
}

std::string DaisyCloudSeedCore::GetSeedSummary() const
{
    std::ostringstream stream;
    stream << FormatSeedValue(impl_->Param(ParameterIndex::kSeedTap).normalizedValue)
           << " "
           << FormatSeedValue(
                  impl_->Param(ParameterIndex::kSeedDiffusion).normalizedValue)
           << " "
           << FormatSeedValue(impl_->Param(ParameterIndex::kSeedDelay).normalizedValue)
           << " "
           << FormatSeedValue(
                  impl_->Param(ParameterIndex::kSeedPostDiffusion).normalizedValue);
    return stream.str();
}
} // namespace daisyhost
