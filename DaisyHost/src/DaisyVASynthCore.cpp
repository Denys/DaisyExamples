#include "daisyhost/DaisyVASynthCore.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace daisyhost
{
namespace
{
constexpr float kTwoPi      = 6.28318530717958647692f;
constexpr int   kVoiceCount = 7;

float Clamp01(float value)
{
    return std::clamp(value, 0.0f, 1.0f);
}

float MidiNoteToFrequency(int midiNote)
{
    return 440.0f * std::pow(2.0f, (static_cast<float>(midiNote) - 69.0f) / 12.0f);
}

float AttackSeconds(float normalizedValue)
{
    const float shaped = Clamp01(normalizedValue);
    return 0.001f + shaped * shaped * 1.5f;
}

float ReleaseSeconds(float normalizedValue)
{
    const float shaped = Clamp01(normalizedValue);
    return 0.02f + shaped * shaped * 2.8f;
}

float DetuneSemitones(float normalizedValue)
{
    return (Clamp01(normalizedValue) - 0.5f) * 12.0f;
}

int WaveIndex(float normalizedValue)
{
    return std::clamp(static_cast<int>(std::round(Clamp01(normalizedValue) * 3.0f)),
                      0,
                      3);
}

const char* WaveLabel(int waveIndex)
{
    switch(waveIndex)
    {
        case 0:
            return "Sine";
        case 1:
            return "Tri";
        case 2:
            return "Saw";
        case 3:
            return "Square";
        default:
            return "Sine";
    }
}

float RenderWave(int waveIndex, float phase)
{
    switch(waveIndex)
    {
        case 0:
            return std::sin(kTwoPi * phase);
        case 1:
            return 1.0f - 4.0f * std::abs(phase - 0.5f);
        case 2:
            return (2.0f * phase) - 1.0f;
        case 3:
            return phase < 0.5f ? 1.0f : -1.0f;
        default:
            return 0.0f;
    }
}

std::vector<DaisyVASynthParameter> MakeDefaultParameters()
{
    return {
        {"osc_mix", "Mix", "Osc", 0.5f, 0.5f, 0.5f, 0, 0, true, true, true},
        {"detune", "Detune", "Osc", 0.55f, 0.55f, 0.55f, 0, 1, true, true, true},
        {"osc1_wave", "Osc 1", "Osc", 2.0f / 3.0f, 2.0f / 3.0f, 2.0f / 3.0f, 3, 2, true, true, true},
        {"osc2_wave", "Osc 2", "Osc", 1.0f, 1.0f, 1.0f, 3, 3, true, true, true},
        {"filter_cutoff", "Cutoff", "Filter", 0.62f, 0.62f, 0.62f, 0, 4, true, true, true},
        {"resonance", "Resonance", "Filter", 0.18f, 0.18f, 0.18f, 0, 5, true, true, true},
        {"filter_env_amount", "Env Amt", "Filter", 0.45f, 0.45f, 0.45f, 0, 6, true, true, true},
        {"level", "Level", "Filter", 0.75f, 0.75f, 0.75f, 0, 7, true, true, true},
        {"lfo_rate", "LFO Rate", "Motion", 0.3f, 0.3f, 0.3f, 0, 8, false, true, true},
        {"lfo_amount", "LFO Amt", "Motion", 0.18f, 0.18f, 0.18f, 0, 9, false, true, true},
        {"attack", "Attack", "Motion", 0.06f, 0.06f, 0.06f, 0, 10, false, true, true},
        {"release", "Release", "Motion", 0.2f, 0.2f, 0.2f, 0, 11, false, true, true},
        {"stereo_sim", "Stereo Sim", "Utilities", 0.0f, 0.0f, 0.0f, 1, 12, false, true, false},
    };
}
} // namespace

struct DaisyVASynthCore::Impl
{
    struct Voice
    {
        bool          active       = false;
        bool          releasing    = false;
        int           midiNote     = 48;
        int           velocity     = 100;
        std::uint64_t age          = 0;
        float         phase1       = 0.0f;
        float         phase2       = 0.0f;
        float         envelope     = 0.0f;
        float         filterState  = 0.0f;
        float         filterState2 = 0.0f;
    };

    double                             sampleRate   = 48000.0;
    std::size_t                        maxBlockSize = DaisyVASynthCore::kPreferredBlockSize;
    std::vector<DaisyVASynthParameter> parameters   = MakeDefaultParameters();
    DaisyVASynthPage                   activePage   = DaisyVASynthPage::kOsc;
    std::array<Voice, kVoiceCount>     voices{};
    std::uint64_t                      voiceAge     = 0;
    bool                               gateHigh     = false;
    int                                lastMidiNote = 48;
    int                                lastVelocity = 100;
    float                              lfoPhase     = 0.0f;
    int                                auditionSamplesRemaining = 0;
    int                                auditionNote = 48;

    DaisyVASynthParameter* FindMutable(const std::string& parameterId)
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

    const DaisyVASynthParameter* Find(const std::string& parameterId) const
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

DaisyVASynthCore::DaisyVASynthCore() : impl_(std::make_unique<Impl>()) {}

DaisyVASynthCore::~DaisyVASynthCore() = default;

void DaisyVASynthCore::Prepare(double sampleRate, std::size_t maxBlockSize)
{
    impl_->sampleRate   = sampleRate > 1.0 ? sampleRate : 48000.0;
    impl_->maxBlockSize = maxBlockSize > 0 ? maxBlockSize : kPreferredBlockSize;
}

void DaisyVASynthCore::Process(float* outputLeft,
                               float* outputRight,
                               std::size_t frameCount)
{
    if(outputLeft != nullptr)
    {
        std::fill(outputLeft, outputLeft + frameCount, 0.0f);
    }
    if(outputRight != nullptr)
    {
        std::fill(outputRight, outputRight + frameCount, 0.0f);
    }

    const float sr            = static_cast<float>(impl_->sampleRate);
    const float oscMix        = Clamp01(impl_->Value("osc_mix"));
    const float detuneAmount  = DetuneSemitones(impl_->Value("detune"));
    const float cutoffBase    = 40.0f + std::pow(impl_->Value("filter_cutoff"), 2.0f) * 12000.0f;
    const float resonance     = Clamp01(impl_->Value("resonance"));
    const float envAmount     = Clamp01(impl_->Value("filter_env_amount"));
    const float outputLevel   = 0.2f + Clamp01(impl_->Value("level")) * 0.9f;
    const float attackStepDiv = std::max(0.0001f, AttackSeconds(impl_->Value("attack")) * sr);
    const float releaseStepDiv
        = std::max(0.0001f, ReleaseSeconds(impl_->Value("release")) * sr);
    const float lfoRate   = 0.05f + impl_->Value("lfo_rate") * 12.0f;
    const float lfoAmount = Clamp01(impl_->Value("lfo_amount"));
    const bool  stereoSim = impl_->Value("stereo_sim") >= 0.5f;
    const int   osc1Wave  = WaveIndex(impl_->Value("osc1_wave"));
    const int   osc2Wave  = WaveIndex(impl_->Value("osc2_wave"));

    for(std::size_t frame = 0; frame < frameCount; ++frame)
    {
        const float lfoValue = std::sin(kTwoPi * impl_->lfoPhase);
        impl_->lfoPhase += lfoRate / sr;
        if(impl_->lfoPhase >= 1.0f)
        {
            impl_->lfoPhase -= std::floor(impl_->lfoPhase);
        }

        float leftSample  = 0.0f;
        float rightSample = 0.0f;

        for(std::size_t voiceIndex = 0; voiceIndex < impl_->voices.size(); ++voiceIndex)
        {
            auto& voice = impl_->voices[voiceIndex];
            if(!voice.active)
            {
                continue;
            }

            if(voice.releasing)
            {
                voice.envelope = std::max(0.0f, voice.envelope - (1.0f / releaseStepDiv));
            }
            else
            {
                voice.envelope = std::min(1.0f, voice.envelope + (1.0f / attackStepDiv));
            }

            if(voice.releasing && voice.envelope <= 0.0001f)
            {
                voice = Impl::Voice{};
                continue;
            }

            const float lfoPitchOffset = lfoValue * lfoAmount * 0.35f;
            const float baseFrequency
                = MidiNoteToFrequency(voice.midiNote)
                  * std::pow(2.0f, (detuneAmount + lfoPitchOffset) / 12.0f);
            const float velocityScale
                = 0.3f + 0.7f * (static_cast<float>(voice.velocity) / 127.0f);

            voice.phase1 += baseFrequency / sr;
            voice.phase2 += (baseFrequency * std::pow(2.0f, detuneAmount / 24.0f)) / sr;
            voice.phase1 -= std::floor(voice.phase1);
            voice.phase2 -= std::floor(voice.phase2);

            const float osc1 = RenderWave(osc1Wave, voice.phase1);
            const float osc2 = RenderWave(osc2Wave, voice.phase2);
            const float raw  = ((1.0f - oscMix) * osc1 + oscMix * osc2) * velocityScale;

            const float cutoffHz
                = std::clamp(cutoffBase + envAmount * voice.envelope * 6000.0f
                                 + lfoValue * lfoAmount * 1200.0f,
                             30.0f,
                             18000.0f);
            const float alpha = cutoffHz / (cutoffHz + sr);
            voice.filterState += alpha * (raw - voice.filterState);
            voice.filterState2 += alpha
                                  * ((voice.filterState
                                      + (voice.filterState - voice.filterState2) * resonance)
                                     - voice.filterState2);

            float sample = std::tanh(voice.filterState2 * 1.6f) * voice.envelope * outputLevel;
            float pan    = 0.0f;
            if(stereoSim)
            {
                pan = (static_cast<float>(voiceIndex)
                       / static_cast<float>(impl_->voices.size() - 1))
                          * 2.0f
                      - 1.0f;
                pan *= 0.35f;
            }

            leftSample += sample * (1.0f - pan);
            rightSample += sample * (1.0f + pan);
        }

        leftSample  = std::tanh(leftSample * 0.8f);
        rightSample = std::tanh(rightSample * 0.8f);

        if(outputLeft != nullptr)
        {
            outputLeft[frame] = leftSample;
        }
        if(outputRight != nullptr)
        {
            outputRight[frame] = rightSample;
        }

        if(impl_->auditionSamplesRemaining > 0)
        {
            --impl_->auditionSamplesRemaining;
            if(impl_->auditionSamplesRemaining == 0)
            {
                ReleaseMidiNote(static_cast<std::uint8_t>(impl_->auditionNote));
            }
        }
    }
}

void DaisyVASynthCore::ResetToDefaultState(std::uint32_t)
{
    impl_->ResetParameterDefaults();
    impl_->activePage               = DaisyVASynthPage::kOsc;
    impl_->voices.fill({});
    impl_->voiceAge                 = 0;
    impl_->gateHigh                 = false;
    impl_->lastMidiNote             = 48;
    impl_->lastVelocity             = 100;
    impl_->lfoPhase                 = 0.0f;
    impl_->auditionSamplesRemaining = 0;
    impl_->auditionNote             = 48;
}

bool DaisyVASynthCore::SetParameterValue(const std::string& parameterId,
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

bool DaisyVASynthCore::GetParameterValue(const std::string& parameterId,
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

bool DaisyVASynthCore::GetEffectiveParameterValue(
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

bool DaisyVASynthCore::TriggerMomentaryAction(const std::string& actionId)
{
    if(actionId == "audition")
    {
        impl_->auditionNote = impl_->lastMidiNote;
        TriggerMidiNote(static_cast<std::uint8_t>(impl_->lastMidiNote),
                        static_cast<std::uint8_t>(impl_->lastVelocity));
        impl_->auditionSamplesRemaining = static_cast<int>(impl_->sampleRate * 0.25);
        return true;
    }
    if(actionId == "panic")
    {
        Panic();
        return true;
    }
    return false;
}

void DaisyVASynthCore::TriggerMidiNote(std::uint8_t midiNote, std::uint8_t velocity)
{
    impl_->lastMidiNote = static_cast<int>(midiNote);
    impl_->lastVelocity = std::max<int>(1, velocity);

    auto voiceIt = std::find_if(impl_->voices.begin(),
                                impl_->voices.end(),
                                [](const Impl::Voice& voice) {
                                    return !voice.active;
                                });
    if(voiceIt == impl_->voices.end())
    {
        voiceIt = std::find_if(impl_->voices.begin(),
                               impl_->voices.end(),
                               [](const Impl::Voice& voice) {
                                   return voice.releasing;
                               });
    }
    if(voiceIt == impl_->voices.end())
    {
        voiceIt = std::min_element(
            impl_->voices.begin(),
            impl_->voices.end(),
            [](const Impl::Voice& a, const Impl::Voice& b) { return a.age < b.age; });
    }

    *voiceIt = {};
    voiceIt->active    = true;
    voiceIt->releasing = false;
    voiceIt->midiNote  = static_cast<int>(midiNote);
    voiceIt->velocity  = std::max<int>(1, velocity);
    voiceIt->age       = ++impl_->voiceAge;
}

void DaisyVASynthCore::ReleaseMidiNote(std::uint8_t midiNote)
{
    for(auto& voice : impl_->voices)
    {
        if(voice.active && voice.midiNote == static_cast<int>(midiNote))
        {
            voice.releasing = true;
        }
    }
}

void DaisyVASynthCore::TriggerGate(bool high)
{
    if(high && !impl_->gateHigh)
    {
        TriggerMidiNote(static_cast<std::uint8_t>(impl_->lastMidiNote),
                        static_cast<std::uint8_t>(impl_->lastVelocity));
    }
    else if(!high)
    {
        ReleaseMidiNote(static_cast<std::uint8_t>(impl_->lastMidiNote));
    }
    impl_->gateHigh = high;
}

void DaisyVASynthCore::Panic()
{
    impl_->voices.fill({});
    impl_->auditionSamplesRemaining = 0;
}

const DaisyVASynthParameter* DaisyVASynthCore::FindParameter(
    const std::string& parameterId) const
{
    return impl_->Find(parameterId);
}

const std::vector<DaisyVASynthParameter>& DaisyVASynthCore::GetParameters() const
{
    return impl_->parameters;
}

std::unordered_map<std::string, float>
DaisyVASynthCore::CaptureStatefulParameterValues() const
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

void DaisyVASynthCore::RestoreStatefulParameterValues(
    const std::unordered_map<std::string, float>& values)
{
    for(const auto& [parameterId, normalizedValue] : values)
    {
        SetParameterValue(parameterId, normalizedValue);
    }
}

DaisyVASynthPage DaisyVASynthCore::GetActivePage() const
{
    return impl_->activePage;
}

bool DaisyVASynthCore::SetActivePage(DaisyVASynthPage page)
{
    if(impl_->activePage == page)
    {
        return false;
    }
    impl_->activePage = page;
    return true;
}

DaisyVASynthPageBinding DaisyVASynthCore::GetActivePageBinding() const
{
    DaisyVASynthPageBinding binding;
    binding.page = impl_->activePage;
    switch(impl_->activePage)
    {
        case DaisyVASynthPage::kFilter:
            binding.pageLabel       = "Filter";
            binding.parameterIds    = {"filter_cutoff", "resonance", "filter_env_amount", "level"};
            binding.parameterLabels = {"Cutoff", "Resonance", "Env Amt", "Level"};
            break;
        case DaisyVASynthPage::kMotion:
            binding.pageLabel       = "Motion";
            binding.parameterIds    = {"lfo_rate", "lfo_amount", "attack", "release"};
            binding.parameterLabels = {"LFO Rate", "LFO Amt", "Attack", "Release"};
            break;
        case DaisyVASynthPage::kOsc:
        default:
            binding.pageLabel       = "Osc";
            binding.parameterIds    = {"osc_mix", "detune", "osc1_wave", "osc2_wave"};
            binding.parameterLabels = {"Mix", "Detune", "Osc 1", "Osc 2"};
            break;
    }
    return binding;
}

int DaisyVASynthCore::GetCurrentVoiceCount() const
{
    return static_cast<int>(std::count_if(
        impl_->voices.begin(), impl_->voices.end(), [](const Impl::Voice& voice) {
            return voice.active;
        }));
}

int DaisyVASynthCore::GetLastMidiNote() const
{
    return impl_->lastMidiNote;
}

std::string DaisyVASynthCore::GetWaveLabel(const std::string& parameterId) const
{
    return WaveLabel(WaveIndex(impl_->Value(parameterId)));
}
} // namespace daisyhost
