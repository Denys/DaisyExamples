#include "daisyhost/DaisyBraidsCore.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string_view>

#include "braids/macro_oscillator.h"
#include "braids/settings.h"
#include "braids/signature_waveshaper.h"
#include "stmlib/utils/random.h"

namespace daisyhost
{
namespace
{
constexpr std::size_t kBraidsInnerBlockSize = 24;
constexpr int         kDefaultMidiNote      = 36;
constexpr int         kModelCount           = 6;
constexpr int         kResolutionCount      = 7;
constexpr int         kSampleRateCount      = 7;
constexpr int         kTuneStepCount        = 49;

float Clamp01(float value)
{
    return std::clamp(value, 0.0f, 1.0f);
}

float QuantizedChoiceNormalized(float normalizedValue, int choiceCount)
{
    if(choiceCount <= 1)
    {
        return 0.0f;
    }

    const int index = std::clamp(static_cast<int>(
                                     std::round(Clamp01(normalizedValue)
                                                * static_cast<float>(choiceCount - 1))),
                                 0,
                                 choiceCount - 1);
    return static_cast<float>(index) / static_cast<float>(choiceCount - 1);
}

int QuantizeChoice(float normalizedValue, int choiceCount)
{
    if(choiceCount <= 1)
    {
        return 0;
    }
    return std::clamp(static_cast<int>(
                          std::round(Clamp01(normalizedValue)
                                     * static_cast<float>(choiceCount - 1))),
                      0,
                      choiceCount - 1);
}

float QuantizedTuneNormalized(float normalizedValue)
{
    return QuantizedChoiceNormalized(normalizedValue, kTuneStepCount);
}

int TuneSemitoneOffset(float normalizedValue)
{
    return QuantizeChoice(normalizedValue, kTuneStepCount) - 24;
}

braids::MacroOscillatorShape ModelShapeForIndex(int modelIndex)
{
    switch(modelIndex)
    {
        case 0: return braids::MACRO_OSC_SHAPE_KICK;
        case 1: return braids::MACRO_OSC_SHAPE_SNARE;
        case 2: return braids::MACRO_OSC_SHAPE_CYMBAL;
        case 3: return braids::MACRO_OSC_SHAPE_STRUCK_DRUM;
        case 4: return braids::MACRO_OSC_SHAPE_STRUCK_BELL;
        default: return braids::MACRO_OSC_SHAPE_FILTERED_NOISE;
    }
}

const char* ModelLabelForIndex(int modelIndex)
{
    switch(modelIndex)
    {
        case 0: return "Kick";
        case 1: return "Snare";
        case 2: return "Cymbal";
        case 3: return "Drum";
        case 4: return "Bell";
        default: return "Filtered Noise";
    }
}

int ResolutionBitsForIndex(int resolutionIndex)
{
    static constexpr std::array<int, kResolutionCount> kBits = {
        2, 3, 4, 6, 8, 12, 16};
    return kBits[static_cast<std::size_t>(std::clamp(
        resolutionIndex, 0, static_cast<int>(kBits.size() - 1)))];
}

int SampleRateHzForIndex(int sampleRateIndex)
{
    static constexpr std::array<int, kSampleRateCount> kRates = {
        4000, 8000, 16000, 24000, 32000, 48000, 96000};
    return kRates[static_cast<std::size_t>(std::clamp(
        sampleRateIndex, 0, static_cast<int>(kRates.size() - 1)))];
}

std::string FormatBitsLabel(int resolutionIndex)
{
    char buffer[16];
    std::snprintf(buffer, sizeof(buffer), "%db", ResolutionBitsForIndex(resolutionIndex));
    return std::string(buffer);
}

std::string FormatSampleRateLabel(int sampleRateIndex)
{
    const int rate = SampleRateHzForIndex(sampleRateIndex);
    char      buffer[16];
    if(rate >= 1000)
    {
        std::snprintf(buffer, sizeof(buffer), "%dk", rate / 1000);
    }
    else
    {
        std::snprintf(buffer, sizeof(buffer), "%d", rate);
    }
    return std::string(buffer);
}

float EnvelopeDecayForModel(int modelIndex)
{
    switch(modelIndex)
    {
        case 0: return 0.99955f;
        case 1: return 0.99920f;
        case 2: return 0.99985f;
        case 3: return 0.99945f;
        case 4: return 0.99990f;
        default: return 0.99890f;
    }
}

float QuantizeToBitDepth(float sample, int bits)
{
    if(bits >= 16)
    {
        return sample;
    }

    const float levels = static_cast<float>((1 << bits) - 1);
    const float scaled = ((Clamp01((sample * 0.5f) + 0.5f)) * levels);
    const float quantized = std::round(scaled) / levels;
    return (quantized * 2.0f) - 1.0f;
}

std::uint32_t NextRandom(std::uint32_t* state)
{
    *state = (*state * 1664525u) + 1013904223u;
    return *state;
}
} // namespace

struct DaisyBraidsCore::Impl
{
    double      sampleRate   = 48000.0;
    std::size_t maxBlockSize = DaisyBraidsCore::kPreferredBlockSize;
    bool        prepared     = false;

    std::vector<DaisyBraidsParameter> parameters;
    std::vector<std::string>          modelLabels = {
        "Kick", "Snare", "Cymbal", "Drum", "Bell", "Filtered Noise"};

    braids::MacroOscillator    oscillator;
    braids::SignatureWaveshaper waveshaper;

    std::uint32_t randomSeed       = 1u;
    std::uint32_t randomState      = 1u;
    std::uint32_t braidsRandomState = 0x21u;

    DaisyBraidsPage activePage = DaisyBraidsPage::kDrum;

    int   currentMidiNote = kDefaultMidiNote;
    int   currentVelocity = 100;
    float currentAccent   = 0.85f;
    bool  pendingTrigger  = false;
    bool  gateHigh        = false;

    float envelopeLevel      = 0.0f;
    float heldSample         = 0.0f;
    int   heldSamplesRemain  = 0;

    std::array<std::int16_t, kBraidsInnerBlockSize> braidsBuffer{};
    std::array<std::uint8_t, kBraidsInnerBlockSize> syncBuffer{};

    Impl()
    {
        parameters = {
            {"model", "Model", "Drum", 0.0f, 0.0f, 0.0f, kModelCount, 0, true, true, true},
            {"tune", "Tune", "Drum", 0.5f, 0.5f, 0.5f, kTuneStepCount, 1, true, true, true},
            {"timbre", "Timbre", "Drum", 0.55f, 0.55f, 0.55f, 0, 2, true, true, true},
            {"color", "Color", "Drum", 0.35f, 0.35f, 0.35f, 0, 3, true, true, true},
            {"resolution",
             "Resolution",
             "Finish",
             QuantizedChoiceNormalized(1.0f, kResolutionCount),
             QuantizedChoiceNormalized(1.0f, kResolutionCount),
             QuantizedChoiceNormalized(1.0f, kResolutionCount),
             kResolutionCount,
             5,
             false,
             true,
             false},
            {"sample_rate",
             "Sample Rate",
             "Finish",
             QuantizedChoiceNormalized(5.0f / 6.0f, kSampleRateCount),
             QuantizedChoiceNormalized(5.0f / 6.0f, kSampleRateCount),
             QuantizedChoiceNormalized(5.0f / 6.0f, kSampleRateCount),
             kSampleRateCount,
             6,
             false,
             true,
             false},
            {"signature", "Signature", "Finish", 0.0f, 0.0f, 0.0f, 0, 4, true, true, false},
            {"level", "Level", "Finish", 0.85f, 0.85f, 0.85f, 0, 7, false, true, false},
        };
    }

    DaisyBraidsParameter* FindParameter(const std::string& parameterId)
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

    const DaisyBraidsParameter* FindParameter(const std::string& parameterId) const
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

    int ModelIndex() const
    {
        return QuantizeChoice(
            FindParameter("model")->normalizedValue, kModelCount);
    }

    int ResolutionIndex() const
    {
        return QuantizeChoice(
            FindParameter("resolution")->normalizedValue, kResolutionCount);
    }

    int SampleRateIndex() const
    {
        return QuantizeChoice(
            FindParameter("sample_rate")->normalizedValue, kSampleRateCount);
    }

    int TuneOffset() const
    {
        return TuneSemitoneOffset(FindParameter("tune")->normalizedValue);
    }

    int Bits() const
    {
        return ResolutionBitsForIndex(ResolutionIndex());
    }

    int SampleHoldCount() const
    {
        const int desiredRate = SampleRateHzForIndex(SampleRateIndex());
        if(desiredRate <= 0)
        {
            return 1;
        }
        const int hold = static_cast<int>(std::ceil(sampleRate / static_cast<double>(
                                                           std::min(desiredRate,
                                                                    static_cast<int>(sampleRate)))));
        return std::max(1, hold);
    }

    std::int16_t Pitch() const
    {
        const int midiNote = std::clamp(currentMidiNote + TuneOffset(), 0, 127);
        return static_cast<std::int16_t>(midiNote << 7);
    }

    std::int16_t BraidsParameter(std::string_view id) const
    {
        const auto* parameter = FindParameter(std::string(id));
        return static_cast<std::int16_t>(std::round(
            Clamp01(parameter->normalizedValue) * 32767.0f));
    }

    void RefreshEffectiveValues()
    {
        for(auto& parameter : parameters)
        {
            if(parameter.id == "model")
            {
                parameter.effectiveNormalizedValue
                    = QuantizedChoiceNormalized(parameter.normalizedValue, kModelCount);
            }
            else if(parameter.id == "resolution")
            {
                parameter.effectiveNormalizedValue
                    = QuantizedChoiceNormalized(parameter.normalizedValue,
                                                kResolutionCount);
            }
            else if(parameter.id == "sample_rate")
            {
                parameter.effectiveNormalizedValue
                    = QuantizedChoiceNormalized(parameter.normalizedValue,
                                                kSampleRateCount);
            }
            else if(parameter.id == "tune")
            {
                parameter.effectiveNormalizedValue
                    = QuantizedTuneNormalized(parameter.normalizedValue);
            }
            else
            {
                parameter.effectiveNormalizedValue = Clamp01(parameter.normalizedValue);
            }
        }
    }

    void ConfigureOscillator()
    {
        oscillator.set_shape(ModelShapeForIndex(ModelIndex()));
        oscillator.set_pitch(Pitch());
        oscillator.set_parameters(BraidsParameter("timbre"), BraidsParameter("color"));
    }

    void ResetSynthesisState(std::uint32_t seed)
    {
        randomSeed        = seed == 0 ? 1u : seed;
        randomState       = randomSeed;
        braidsRandomState = randomSeed ^ 0x9E3779B9u;
        currentMidiNote   = kDefaultMidiNote;
        currentVelocity   = 100;
        currentAccent     = 0.85f;
        pendingTrigger    = false;
        gateHigh          = false;
        envelopeLevel     = 0.0f;
        heldSample        = 0.0f;
        heldSamplesRemain = 0;
        oscillator.Init();
        waveshaper.Init(randomSeed ^ 0xA341316Cu);
        stmlib::Random::Seed(braidsRandomState);
    }
};

DaisyBraidsCore::DaisyBraidsCore()
: impl_(std::make_unique<Impl>())
{
    impl_->RefreshEffectiveValues();
    impl_->ResetSynthesisState(1u);
}

DaisyBraidsCore::~DaisyBraidsCore() = default;

void DaisyBraidsCore::Prepare(double sampleRate, std::size_t maxBlockSize)
{
    impl_->sampleRate   = sampleRate > 1000.0 ? sampleRate : 48000.0;
    impl_->maxBlockSize = maxBlockSize > 0 ? maxBlockSize : kPreferredBlockSize;
    impl_->prepared     = true;
    impl_->ConfigureOscillator();
}

void DaisyBraidsCore::Process(float* outputLeft,
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

    if(frameCount == 0)
    {
        return;
    }

    impl_->ConfigureOscillator();
    const float signature = impl_->FindParameter("signature")->normalizedValue;
    const float level     = impl_->FindParameter("level")->normalizedValue;
    const float decay     = EnvelopeDecayForModel(impl_->ModelIndex());
    const int   holdCount = impl_->SampleHoldCount();
    const int   bits      = impl_->Bits();

    std::size_t frameOffset = 0;
    while(frameOffset < frameCount)
    {
        const std::size_t chunkSize
            = std::min(kBraidsInnerBlockSize, frameCount - frameOffset);

        std::fill(impl_->syncBuffer.begin(), impl_->syncBuffer.end(), 0u);
        if(impl_->pendingTrigger)
        {
            impl_->syncBuffer[0] = 1u;
            impl_->oscillator.Strike();
            impl_->pendingTrigger = false;
        }

        if(impl_->envelopeLevel > 0.00001f || impl_->syncBuffer[0] != 0u)
        {
            stmlib::Random::Seed(impl_->braidsRandomState);
            impl_->oscillator.Render(
                impl_->syncBuffer.data(), impl_->braidsBuffer.data(), chunkSize);
            impl_->braidsRandomState = stmlib::Random::state();

            for(std::size_t i = 0; i < chunkSize; ++i)
            {
                const float drySample
                    = static_cast<float>(impl_->braidsBuffer[i]) / 32768.0f;
                const float wetSample
                    = static_cast<float>(impl_->waveshaper.Transform(impl_->braidsBuffer[i]))
                      / 32768.0f;
                const float shaped = drySample + ((wetSample - drySample) * signature);
                const float enveloped = shaped * impl_->envelopeLevel * level;
                impl_->envelopeLevel *= decay;
                if(impl_->envelopeLevel < 0.00001f)
                {
                    impl_->envelopeLevel = 0.0f;
                }

                if(impl_->heldSamplesRemain <= 0)
                {
                    impl_->heldSample = QuantizeToBitDepth(enveloped, bits);
                    impl_->heldSamplesRemain = holdCount - 1;
                }
                else
                {
                    --impl_->heldSamplesRemain;
                }

                if(outputLeft != nullptr)
                {
                    outputLeft[frameOffset + i] = impl_->heldSample;
                }
                if(outputRight != nullptr)
                {
                    outputRight[frameOffset + i] = impl_->heldSample;
                }
            }
        }

        frameOffset += chunkSize;
    }
}

void DaisyBraidsCore::ResetToDefaultState(std::uint32_t seed)
{
    impl_->activePage = DaisyBraidsPage::kDrum;
    for(auto& parameter : impl_->parameters)
    {
        parameter.normalizedValue = parameter.defaultNormalizedValue;
    }
    impl_->RefreshEffectiveValues();
    impl_->ResetSynthesisState(seed);
    impl_->ConfigureOscillator();
}

bool DaisyBraidsCore::SetParameterValue(const std::string& parameterId,
                                        float              normalizedValue)
{
    auto* parameter = impl_->FindParameter(parameterId);
    if(parameter == nullptr)
    {
        return false;
    }

    if(parameterId == "model")
    {
        parameter->normalizedValue
            = QuantizedChoiceNormalized(normalizedValue, kModelCount);
    }
    else if(parameterId == "resolution")
    {
        parameter->normalizedValue
            = QuantizedChoiceNormalized(normalizedValue, kResolutionCount);
    }
    else if(parameterId == "sample_rate")
    {
        parameter->normalizedValue
            = QuantizedChoiceNormalized(normalizedValue, kSampleRateCount);
    }
    else if(parameterId == "tune")
    {
        parameter->normalizedValue = QuantizedTuneNormalized(normalizedValue);
    }
    else
    {
        parameter->normalizedValue = Clamp01(normalizedValue);
    }

    impl_->RefreshEffectiveValues();
    return true;
}

bool DaisyBraidsCore::GetParameterValue(const std::string& parameterId,
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

bool DaisyBraidsCore::GetEffectiveParameterValue(const std::string& parameterId,
                                                 float* normalizedValue) const
{
    const auto* parameter = impl_->FindParameter(parameterId);
    if(parameter == nullptr || normalizedValue == nullptr)
    {
        return false;
    }
    *normalizedValue = parameter->effectiveNormalizedValue;
    return true;
}

bool DaisyBraidsCore::TriggerMomentaryAction(const std::string& actionId)
{
    if(actionId == "audition")
    {
        TriggerMidiNote(static_cast<std::uint8_t>(impl_->currentMidiNote),
                        static_cast<std::uint8_t>(impl_->currentVelocity));
        return true;
    }
    if(actionId == "randomize_model")
    {
        const float previous = impl_->FindParameter("model")->normalizedValue;
        float       next     = previous;
        while(next == previous)
        {
            next = QuantizedChoiceNormalized(
                static_cast<float>(NextRandom(&impl_->randomState) & 0xffffu) / 65535.0f,
                kModelCount);
        }
        SetParameterValue("model", next);
        return true;
    }
    if(actionId == "panic")
    {
        Panic();
        return true;
    }
    return false;
}

void DaisyBraidsCore::TriggerMidiNote(std::uint8_t midiNote, std::uint8_t velocity)
{
    impl_->currentMidiNote = static_cast<int>(midiNote);
    impl_->currentVelocity = static_cast<int>(velocity);
    impl_->currentAccent = 0.2f + (0.8f * (static_cast<float>(velocity) / 127.0f));
    impl_->envelopeLevel = std::max(impl_->envelopeLevel, impl_->currentAccent);
    impl_->pendingTrigger = true;
    impl_->heldSamplesRemain = 0;
}

void DaisyBraidsCore::TriggerGate(bool high)
{
    if(high && !impl_->gateHigh)
    {
        TriggerMidiNote(static_cast<std::uint8_t>(impl_->currentMidiNote),
                        static_cast<std::uint8_t>(impl_->currentVelocity));
    }
    impl_->gateHigh = high;
}

void DaisyBraidsCore::Panic()
{
    impl_->pendingTrigger = false;
    impl_->gateHigh       = false;
    impl_->envelopeLevel  = 0.0f;
    impl_->heldSample     = 0.0f;
    impl_->heldSamplesRemain = 0;
    impl_->oscillator.Init();
    impl_->ConfigureOscillator();
}

const DaisyBraidsParameter* DaisyBraidsCore::FindParameter(
    const std::string& parameterId) const
{
    return impl_->FindParameter(parameterId);
}

const std::vector<DaisyBraidsParameter>& DaisyBraidsCore::GetParameters() const
{
    return impl_->parameters;
}

std::unordered_map<std::string, float>
DaisyBraidsCore::CaptureStatefulParameterValues() const
{
    std::unordered_map<std::string, float> values;
    for(const auto& parameter : impl_->parameters)
    {
        if(parameter.stateful)
        {
            values[parameter.id] = parameter.normalizedValue;
        }
    }
    return values;
}

void DaisyBraidsCore::RestoreStatefulParameterValues(
    const std::unordered_map<std::string, float>& values)
{
    for(const auto& entry : values)
    {
        SetParameterValue(entry.first, entry.second);
    }
}

DaisyBraidsPage DaisyBraidsCore::GetActivePage() const
{
    return impl_->activePage;
}

bool DaisyBraidsCore::SetActivePage(DaisyBraidsPage page)
{
    if(page != DaisyBraidsPage::kDrum && page != DaisyBraidsPage::kFinish)
    {
        return false;
    }
    impl_->activePage = page;
    return true;
}

DaisyBraidsPageBinding DaisyBraidsCore::GetActivePageBinding() const
{
    DaisyBraidsPageBinding binding;
    binding.page = impl_->activePage;
    if(impl_->activePage == DaisyBraidsPage::kFinish)
    {
        binding.pageLabel          = "Finish";
        binding.parameterIds       = {"resolution", "sample_rate", "signature", "level"};
        binding.parameterLabels    = {"Resolution", "Rate", "Signature", "Level"};
    }
    else
    {
        binding.pageLabel          = "Drum";
        binding.parameterIds       = {"tune", "timbre", "color", "model"};
        binding.parameterLabels    = {"Tune", "Timbre", "Color", "Model"};
    }
    return binding;
}

const std::vector<std::string>& DaisyBraidsCore::GetModelLabels() const
{
    return impl_->modelLabels;
}

std::string DaisyBraidsCore::GetCurrentModelLabel() const
{
    return std::string(ModelLabelForIndex(impl_->ModelIndex()));
}

int DaisyBraidsCore::GetCurrentMidiNote() const
{
    return impl_->currentMidiNote;
}

int DaisyBraidsCore::GetCurrentVelocity() const
{
    return impl_->currentVelocity;
}

int DaisyBraidsCore::GetCurrentTuneSemitoneOffset() const
{
    return impl_->TuneOffset();
}

std::string DaisyBraidsCore::GetResolutionLabel() const
{
    return FormatBitsLabel(impl_->ResolutionIndex());
}

std::string DaisyBraidsCore::GetSampleRateLabel() const
{
    return FormatSampleRateLabel(impl_->SampleRateIndex());
}

} // namespace daisyhost
