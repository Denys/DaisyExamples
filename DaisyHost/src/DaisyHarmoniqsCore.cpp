#include "daisyhost/DaisyHarmoniqsCore.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <random>

namespace daisyhost
{
namespace
{
constexpr float kTwoPi = 6.28318530717958647692f;

float Clamp01(float value)
{
    return std::clamp(value, 0.0f, 1.0f);
}

float MidiNoteToFrequency(int midiNote)
{
    return 440.0f * std::pow(2.0f, (static_cast<float>(midiNote) - 69.0f) / 12.0f);
}

float SecondsFromAttack(float normalizedValue)
{
    const float shaped = Clamp01(normalizedValue);
    return 0.001f + shaped * shaped * 2.0f;
}

float SecondsFromRelease(float normalizedValue)
{
    const float shaped = Clamp01(normalizedValue);
    return 0.01f + shaped * shaped * 3.0f;
}

float DetuneSemitones(float normalizedValue)
{
    return (Clamp01(normalizedValue) - 0.5f) * 12.0f;
}

std::vector<DaisyHarmoniqsParameter> MakeDefaultParameters()
{
    return {
        {"brightness", "Brightness", "Spectrum", 0.5f, 0.5f, 0.5f, 0, 0, true, true, true},
        {"tilt", "Tilt", "Spectrum", 0.5f, 0.5f, 0.5f, 0, 1, true, true, true},
        {"odd_even", "Odd/Even", "Spectrum", 0.5f, 0.5f, 0.5f, 0, 3, true, true, true},
        {"spread", "Spread", "Spectrum", 0.25f, 0.25f, 0.25f, 0, 2, true, true, true},
        {"attack", "Attack", "Envelope", 0.08f, 0.08f, 0.08f, 0, 4, true, true, true},
        {"release", "Release", "Envelope", 0.2f, 0.2f, 0.2f, 0, 5, true, true, true},
        {"detune", "Detune", "Envelope", 0.5f, 0.5f, 0.5f, 0, 6, true, true, true},
        {"level", "Level", "Envelope", 0.75f, 0.75f, 0.75f, 0, 7, true, true, true},
        {"harmonic_1", "H1", "Harmonics", 1.0f, 1.0f, 1.0f, 0, 50, false, true, false},
        {"harmonic_2", "H2", "Harmonics", 0.72f, 0.72f, 0.72f, 0, 51, false, true, false},
        {"harmonic_3", "H3", "Harmonics", 0.55f, 0.55f, 0.55f, 0, 52, false, true, false},
        {"harmonic_4", "H4", "Harmonics", 0.38f, 0.38f, 0.38f, 0, 53, false, true, false},
        {"harmonic_5", "H5", "Harmonics", 0.24f, 0.24f, 0.24f, 0, 54, false, true, false},
        {"harmonic_6", "H6", "Harmonics", 0.16f, 0.16f, 0.16f, 0, 55, false, true, false},
        {"harmonic_7", "H7", "Harmonics", 0.10f, 0.10f, 0.10f, 0, 56, false, true, false},
        {"harmonic_8", "H8", "Harmonics", 0.06f, 0.06f, 0.06f, 0, 57, false, true, false},
    };
}
} // namespace

struct DaisyHarmoniqsCore::Impl
{
    double                                sampleRate = 48000.0;
    std::size_t                           maxBlockSize = DaisyHarmoniqsCore::kPreferredBlockSize;
    std::vector<DaisyHarmoniqsParameter>  parameters = MakeDefaultParameters();
    DaisyHarmoniqsPage                    activePage = DaisyHarmoniqsPage::kSpectrum;
    std::array<float, 8>                  phases{};
    std::uint32_t                         currentSeed = 0;
    std::mt19937                          rng{0u};
    bool                                  noteGate = false;
    bool                                  gateHigh = false;
    bool                                  retriggerPending = false;
    int                                   auditionSamplesRemaining = 0;
    int                                   currentMidiNote = 48;
    int                                   lastMidiNote = 48;
    int                                   currentVelocity = 100;
    int                                   lastVelocity = 100;
    float                                 envelope = 0.0f;

    DaisyHarmoniqsParameter* FindMutable(const std::string& parameterId)
    {
        for(auto& parameter : parameters)
        {
            if(parameter.id == parameterId)
            {
                return &parameter;
            }
        }
        return nullptr;
    }

    const DaisyHarmoniqsParameter* Find(const std::string& parameterId) const
    {
        for(const auto& parameter : parameters)
        {
            if(parameter.id == parameterId)
            {
                return &parameter;
            }
        }
        return nullptr;
    }

    float Value(const std::string& parameterId) const
    {
        const auto* parameter = Find(parameterId);
        return parameter != nullptr ? parameter->normalizedValue : 0.0f;
    }

    void ResetParameterDefaults()
    {
        parameters = MakeDefaultParameters();
    }
};

DaisyHarmoniqsCore::DaisyHarmoniqsCore() : impl_(std::make_unique<Impl>()) {}

DaisyHarmoniqsCore::~DaisyHarmoniqsCore() = default;

void DaisyHarmoniqsCore::Prepare(double sampleRate, std::size_t maxBlockSize)
{
    impl_->sampleRate   = sampleRate > 1.0 ? sampleRate : 48000.0;
    impl_->maxBlockSize = maxBlockSize > 0 ? maxBlockSize : kPreferredBlockSize;
}

void DaisyHarmoniqsCore::Process(float* outputLeft,
                                 float* outputRight,
                                 std::size_t frameCount)
{
    if(impl_->retriggerPending)
    {
        impl_->phases.fill(0.0f);
        impl_->envelope = 0.0f;
        impl_->retriggerPending = false;
    }

    const float sr = static_cast<float>(impl_->sampleRate);
    const float baseFrequency
        = MidiNoteToFrequency(impl_->currentMidiNote)
          * std::pow(2.0f, DetuneSemitones(impl_->Value("detune")) / 12.0f);
    const float brightness = impl_->Value("brightness");
    const float tilt       = (impl_->Value("tilt") - 0.5f) * 2.0f;
    const float oddEven    = (impl_->Value("odd_even") - 0.5f) * 2.0f;
    const float spread     = impl_->Value("spread");
    const float level
        = impl_->Value("level")
          * (0.25f + 0.75f * (static_cast<float>(impl_->currentVelocity) / 127.0f));

    for(std::size_t frame = 0; frame < frameCount; ++frame)
    {
        const bool gate = impl_->noteGate || impl_->auditionSamplesRemaining > 0;
        const float attackSeconds  = SecondsFromAttack(impl_->Value("attack"));
        const float releaseSeconds = SecondsFromRelease(impl_->Value("release"));
        const float attackStep
            = attackSeconds > 0.0f ? 1.0f / (attackSeconds * sr) : 1.0f;
        const float releaseStep
            = releaseSeconds > 0.0f ? 1.0f / (releaseSeconds * sr) : 1.0f;

        if(gate)
        {
            impl_->envelope = std::min(1.0f, impl_->envelope + attackStep);
        }
        else
        {
            impl_->envelope = std::max(0.0f, impl_->envelope - releaseStep);
        }

        float sample = 0.0f;
        for(std::size_t harmonic = 0; harmonic < impl_->phases.size(); ++harmonic)
        {
            const int   harmonicNumber = static_cast<int>(harmonic) + 1;
            const float harmonicPos
                = static_cast<float>(harmonic) / static_cast<float>(impl_->phases.size() - 1);
            const float harmonicValue
                = impl_->Value("harmonic_" + std::to_string(harmonicNumber));
            const float lowBias  = 1.15f - 0.1f * static_cast<float>(harmonic);
            const float highBias = 0.35f + harmonicPos * 1.3f;
            const float brightWeight
                = (1.0f - brightness) * lowBias + brightness * highBias;
            const float tiltWeight
                = tilt >= 0.0f ? (1.0f + tilt * harmonicPos)
                               : (1.0f + (-tilt) * (1.0f - harmonicPos));
            const bool oddHarmonic = (harmonicNumber % 2) == 1;
            const float oddEvenWeight
                = oddHarmonic ? (1.0f + 0.75f * oddEven)
                              : (1.0f - 0.75f * oddEven);
            const float spreadFactor
                = 1.0f + spread * 0.02f * static_cast<float>(harmonic);
            const float phaseIncrement
                = (kTwoPi * baseFrequency * static_cast<float>(harmonicNumber)
                   * spreadFactor)
                  / sr;

            impl_->phases[harmonic] += phaseIncrement;
            if(impl_->phases[harmonic] >= kTwoPi)
            {
                impl_->phases[harmonic]
                    -= kTwoPi * std::floor(impl_->phases[harmonic] / kTwoPi);
            }

            sample += std::sin(impl_->phases[harmonic]) * harmonicValue
                      * brightWeight * tiltWeight
                      * std::max(0.1f, oddEvenWeight);
        }

        sample = std::tanh(sample * 0.25f) * impl_->envelope * level;

        if(outputLeft != nullptr)
        {
            outputLeft[frame] = sample;
        }
        if(outputRight != nullptr)
        {
            outputRight[frame] = sample;
        }

        if(impl_->auditionSamplesRemaining > 0)
        {
            --impl_->auditionSamplesRemaining;
        }
    }
}

void DaisyHarmoniqsCore::ResetToDefaultState(std::uint32_t seed)
{
    impl_->currentSeed = seed;
    impl_->rng.seed(seed);
    impl_->ResetParameterDefaults();
    impl_->activePage               = DaisyHarmoniqsPage::kSpectrum;
    impl_->phases.fill(0.0f);
    impl_->noteGate                 = false;
    impl_->gateHigh                 = false;
    impl_->retriggerPending         = false;
    impl_->auditionSamplesRemaining = 0;
    impl_->currentMidiNote          = 48;
    impl_->lastMidiNote             = 48;
    impl_->currentVelocity          = 100;
    impl_->lastVelocity             = 100;
    impl_->envelope                 = 0.0f;
}

bool DaisyHarmoniqsCore::SetParameterValue(const std::string& parameterId,
                                           float              normalizedValue)
{
    auto* parameter = impl_->FindMutable(parameterId);
    if(parameter == nullptr)
    {
        return false;
    }

    parameter->normalizedValue          = Clamp01(normalizedValue);
    parameter->effectiveNormalizedValue = parameter->normalizedValue;
    return true;
}

bool DaisyHarmoniqsCore::SetEffectiveParameterValue(
    const std::string& parameterId,
    float              normalizedValue)
{
    auto* parameter = impl_->FindMutable(parameterId);
    if(parameter == nullptr || parameter->stepCount > 0)
    {
        return false;
    }

    parameter->effectiveNormalizedValue = Clamp01(normalizedValue);
    return true;
}

bool DaisyHarmoniqsCore::GetParameterValue(const std::string& parameterId,
                                           float*             normalizedValue) const
{
    const auto* parameter = impl_->Find(parameterId);
    if(parameter == nullptr || normalizedValue == nullptr)
    {
        return false;
    }

    *normalizedValue = parameter->normalizedValue;
    return true;
}

bool DaisyHarmoniqsCore::GetEffectiveParameterValue(
    const std::string& parameterId,
    float*             normalizedValue) const
{
    const auto* parameter = impl_->Find(parameterId);
    if(parameter == nullptr || normalizedValue == nullptr)
    {
        return false;
    }

    *normalizedValue = parameter->effectiveNormalizedValue;
    return true;
}

bool DaisyHarmoniqsCore::TriggerMomentaryAction(const std::string& actionId)
{
    if(actionId == "audition")
    {
        impl_->currentMidiNote          = impl_->lastMidiNote;
        impl_->currentVelocity          = impl_->lastVelocity;
        impl_->auditionSamplesRemaining = static_cast<int>(impl_->sampleRate * 0.25);
        impl_->retriggerPending         = true;
        return true;
    }
    if(actionId == "init_spectrum")
    {
        const auto defaultParameters = MakeDefaultParameters();
        for(int harmonic = 1; harmonic <= 8; ++harmonic)
        {
            SetParameterValue("harmonic_" + std::to_string(harmonic),
                              defaultParameters[7 + harmonic].defaultNormalizedValue);
        }
        return true;
    }
    if(actionId == "randomize_spectrum")
    {
        std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
        for(int harmonic = 1; harmonic <= 8; ++harmonic)
        {
            SetParameterValue("harmonic_" + std::to_string(harmonic),
                              distribution(impl_->rng));
        }
        return true;
    }
    if(actionId == "panic")
    {
        impl_->noteGate                 = false;
        impl_->gateHigh                 = false;
        impl_->auditionSamplesRemaining = 0;
        impl_->retriggerPending         = false;
        impl_->envelope                 = 0.0f;
        impl_->phases.fill(0.0f);
        return true;
    }
    return false;
}

void DaisyHarmoniqsCore::TriggerMidiNote(std::uint8_t midiNote, std::uint8_t velocity)
{
    impl_->currentMidiNote  = static_cast<int>(midiNote);
    impl_->lastMidiNote     = static_cast<int>(midiNote);
    impl_->currentVelocity  = std::max<int>(1, velocity);
    impl_->lastVelocity     = impl_->currentVelocity;
    impl_->noteGate         = true;
    impl_->gateHigh         = true;
    impl_->retriggerPending = true;
}

void DaisyHarmoniqsCore::ReleaseMidiNote(std::uint8_t midiNote)
{
    if(static_cast<int>(midiNote) == impl_->currentMidiNote)
    {
        impl_->noteGate = false;
        impl_->gateHigh = false;
    }
}

void DaisyHarmoniqsCore::TriggerGate(bool high)
{
    if(high && !impl_->gateHigh)
    {
        TriggerMidiNote(static_cast<std::uint8_t>(impl_->lastMidiNote),
                        static_cast<std::uint8_t>(impl_->lastVelocity));
    }
    else if(!high)
    {
        impl_->noteGate = false;
        impl_->gateHigh = false;
    }
}

const DaisyHarmoniqsParameter* DaisyHarmoniqsCore::FindParameter(
    const std::string& parameterId) const
{
    return impl_->Find(parameterId);
}

const std::vector<DaisyHarmoniqsParameter>& DaisyHarmoniqsCore::GetParameters() const
{
    return impl_->parameters;
}

std::unordered_map<std::string, float>
DaisyHarmoniqsCore::CaptureStatefulParameterValues() const
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

void DaisyHarmoniqsCore::RestoreStatefulParameterValues(
    const std::unordered_map<std::string, float>& values)
{
    for(const auto& [parameterId, normalizedValue] : values)
    {
        SetParameterValue(parameterId, normalizedValue);
    }
}

DaisyHarmoniqsPage DaisyHarmoniqsCore::GetActivePage() const
{
    return impl_->activePage;
}

bool DaisyHarmoniqsCore::SetActivePage(DaisyHarmoniqsPage page)
{
    if(impl_->activePage == page)
    {
        return false;
    }
    impl_->activePage = page;
    return true;
}

DaisyHarmoniqsPageBinding DaisyHarmoniqsCore::GetActivePageBinding() const
{
    DaisyHarmoniqsPageBinding binding;
    binding.page = impl_->activePage;
    if(impl_->activePage == DaisyHarmoniqsPage::kEnvelope)
    {
        binding.pageLabel       = "Envelope";
        binding.parameterIds    = {"attack", "release", "detune", "level"};
        binding.parameterLabels = {"Attack", "Release", "Detune", "Level"};
        return binding;
    }

    binding.pageLabel       = "Spectrum";
    binding.parameterIds    = {"brightness", "tilt", "odd_even", "spread"};
    binding.parameterLabels = {"Brightness", "Tilt", "Odd/Even", "Spread"};
    return binding;
}

int DaisyHarmoniqsCore::GetCurrentMidiNote() const
{
    return impl_->currentMidiNote;
}

int DaisyHarmoniqsCore::GetCurrentVelocity() const
{
    return impl_->currentVelocity;
}
} // namespace daisyhost
