#include "daisyhost/DaisySubharmoniqCore.h"

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
    return value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value);
}

int ClampInt(int value, int minValue, int maxValue)
{
    return std::max(minValue, std::min(maxValue, value));
}

float MidiNoteToFrequency(int midiNote)
{
    return 440.0f
           * std::pow(2.0f,
                      (static_cast<float>(midiNote) - 69.0f) / 12.0f);
}

float NormalizedToLog(float normalized, float minValue, float maxValue)
{
    const float clamped = Clamp01(normalized);
    return std::exp(std::log(minValue)
                    + clamped * (std::log(maxValue) - std::log(minValue)));
}

float NormalizedToTempoBpm(float normalized)
{
    return 30.0f + Clamp01(normalized) * 210.0f;
}

bool DecodeSequencerStepId(const char* parameterId,
                           std::size_t* sequencer,
                           std::size_t* step)
{
    if(parameterId == nullptr || sequencer == nullptr || step == nullptr)
    {
        return false;
    }
    if(parameterId[0] != 's' || parameterId[1] != 'e' || parameterId[2] != 'q'
       || parameterId[4] != '_' || parameterId[5] != 's'
       || parameterId[6] != 't' || parameterId[7] != 'e'
       || parameterId[8] != 'p' || parameterId[10] != '\0')
    {
        return false;
    }
    const int seqIndex  = parameterId[3] - '1';
    const int stepIndex = parameterId[9] - '1';
    if(seqIndex < 0
       || seqIndex >= static_cast<int>(DaisySubharmoniqCore::kSequencerCount)
       || stepIndex < 0
       || stepIndex >= static_cast<int>(DaisySubharmoniqCore::kStepsPerSequencer))
    {
        return false;
    }
    *sequencer = static_cast<std::size_t>(seqIndex);
    *step      = static_cast<std::size_t>(stepIndex);
    return true;
}

float SoftSaw(float phase)
{
    const float normalized = phase / kTwoPi;
    return (2.0f * normalized) - 1.0f;
}

float Square(float phase)
{
    return phase < 3.14159265358979323846f ? 1.0f : -1.0f;
}

class SimpleSvfLowpass
{
  public:
    void Init(float sampleRate)
    {
        sampleRate_ = sampleRate > 1000.0f ? sampleRate : 48000.0f;
        low_ = 0.0f;
        band_ = 0.0f;
        output_ = 0.0f;
    }

    void SetFreq(float frequency)
    {
        const float maxFreq = sampleRate_ * 0.33f;
        const float clamped = frequency < 20.0f
                                  ? 20.0f
                                  : (frequency > maxFreq ? maxFreq : frequency);
        freq_ = 2.0f * std::sin(3.14159265358979323846f * clamped / sampleRate_);
    }

    void SetRes(float resonance)
    {
        const float clamped = resonance < 0.0f
                                  ? 0.0f
                                  : (resonance > 0.98f ? 0.98f : resonance);
        damping_ = 1.95f - clamped * 1.65f;
    }

    void SetDrive(float drive)
    {
        drive_ = drive < 0.1f ? 0.1f : (drive > 2.0f ? 2.0f : drive);
    }

    void Process(float input)
    {
        const float driven = std::tanh(input * drive_);
        low_ += freq_ * band_;
        const float high = driven - low_ - damping_ * band_;
        band_ += freq_ * high;
        output_ = low_;
    }

    float Low() const { return output_; }

  private:
    float sampleRate_ = 48000.0f;
    float freq_       = 0.05f;
    float damping_    = 1.0f;
    float drive_      = 1.0f;
    float low_        = 0.0f;
    float band_       = 0.0f;
    float output_     = 0.0f;
};

std::vector<DaisySubharmoniqParameter> MakeDefaultParameters()
{
    return {
        {"vco1_pitch", "VCO 1", "Voice", 0.25f, 0.25f, 0.25f, 0, 0, true, true, true},
        {"vco2_pitch", "VCO 2", "Voice", 0.38f, 0.38f, 0.38f, 0, 1, true, true, true},
        {"vco1_sub1_div", "VCO1 Sub1", "Voice", 0.20f, 0.20f, 0.20f, 16, 2, true, true, true},
        {"vco2_sub1_div", "VCO2 Sub1", "Voice", 0.27f, 0.27f, 0.27f, 16, 3, true, true, true},
        {"vco1_sub2_div", "VCO1 Sub2", "Voice", 0.40f, 0.40f, 0.40f, 16, 4, true, true, true},
        {"vco2_sub2_div", "VCO2 Sub2", "Voice", 0.47f, 0.47f, 0.47f, 16, 5, true, true, true},
        {"tempo", "Tempo", "Home", 0.34f, 0.34f, 0.34f, 0, 6, true, true, true},
        {"cutoff", "Cutoff", "Filter", 0.72f, 0.72f, 0.72f, 0, 7, true, true, true},
        {"resonance", "Res", "Filter", 0.20f, 0.20f, 0.20f, 0, 8, true, true, true},
        {"vca_decay", "VCA Decay", "Filter", 0.50f, 0.50f, 0.50f, 0, 9, true, true, true},
        {"vcf_attack", "VCF Attack", "Filter", 0.02f, 0.02f, 0.02f, 0, 10, true, true, true},
        {"vcf_decay", "VCF Decay", "Filter", 0.32f, 0.32f, 0.32f, 0, 11, true, true, true},
        {"vcf_env_amt", "VCF Env", "Filter", 0.70f, 0.70f, 0.70f, 0, 12, true, true, true},
        {"drive", "Drive", "Mix", 0.35f, 0.35f, 0.35f, 0, 13, true, true, true},
        {"output", "Output", "Mix", 0.90f, 0.90f, 0.90f, 0, 14, true, true, true},
        {"vco1_level", "VCO1 Lvl", "Mix", 0.65f, 0.65f, 0.65f, 0, 15, true, true, true},
        {"vco1_sub1_level", "V1S1 Lvl", "Mix", 0.55f, 0.55f, 0.55f, 0, 16, true, true, true},
        {"vco1_sub2_level", "V1S2 Lvl", "Mix", 0.48f, 0.48f, 0.48f, 0, 17, true, true, true},
        {"vco2_level", "VCO2 Lvl", "Mix", 0.58f, 0.58f, 0.58f, 0, 18, true, true, true},
        {"vco2_sub1_level", "V2S1 Lvl", "Mix", 0.50f, 0.50f, 0.50f, 0, 19, true, true, true},
        {"vco2_sub2_level", "V2S2 Lvl", "Mix", 0.42f, 0.42f, 0.42f, 0, 20, true, true, true},
        {"seq1_step1", "S1 Step1", "Seq", 0.50f, 0.50f, 0.50f, 0, 21, true, true, true},
        {"seq1_step2", "S1 Step2", "Seq", 0.58f, 0.58f, 0.58f, 0, 22, true, true, true},
        {"seq1_step3", "S1 Step3", "Seq", 0.42f, 0.42f, 0.42f, 0, 23, true, true, true},
        {"seq1_step4", "S1 Step4", "Seq", 0.67f, 0.67f, 0.67f, 0, 24, true, true, true},
        {"seq2_step1", "S2 Step1", "Seq", 0.50f, 0.50f, 0.50f, 0, 25, true, true, true},
        {"seq2_step2", "S2 Step2", "Seq", 0.33f, 0.33f, 0.33f, 0, 26, true, true, true},
        {"seq2_step3", "S2 Step3", "Seq", 0.58f, 0.58f, 0.58f, 0, 27, true, true, true},
        {"seq2_step4", "S2 Step4", "Seq", 0.25f, 0.25f, 0.25f, 0, 28, true, true, true},
        {"rhythm1_div", "Rhythm 1", "Rhythm", 0.00f, 0.00f, 0.00f, 16, 29, true, true, true},
        {"rhythm2_div", "Rhythm 2", "Rhythm", 0.07f, 0.07f, 0.07f, 16, 30, true, true, true},
        {"rhythm3_div", "Rhythm 3", "Rhythm", 0.20f, 0.20f, 0.20f, 16, 31, true, true, true},
        {"rhythm4_div", "Rhythm 4", "Rhythm", 0.40f, 0.40f, 0.40f, 16, 32, true, true, true},
        {"root_cv", "Root CV", "Patch", 0.50f, 0.50f, 0.50f, 0, 33, true, true, false},
        {"cutoff_cv", "Cut CV", "Patch", 0.00f, 0.00f, 0.00f, 0, 34, true, true, false},
        {"rhythm_cv", "Rhythm CV", "Patch", 0.00f, 0.00f, 0.00f, 0, 35, true, true, false},
        {"sub_cv", "Sub CV", "Patch", 0.00f, 0.00f, 0.00f, 0, 36, true, true, false},
    };
}

int QuantizedSemitone(float normalized,
                      int   octaveRange,
                      DaisySubharmoniqQuantizeMode mode)
{
    const float raw = (Clamp01(normalized) - 0.5f)
                      * static_cast<float>(octaveRange * 24);
    if(mode == DaisySubharmoniqQuantizeMode::kOff)
    {
        return static_cast<int>(std::round(raw));
    }

    int semitone = static_cast<int>(std::round(raw));
    if(mode == DaisySubharmoniqQuantizeMode::kEightEqual
       || mode == DaisySubharmoniqQuantizeMode::kEightJust)
    {
        static constexpr std::array<int, 8> kMajorScale = {0, 2, 4, 5, 7, 9, 11, 12};
        const int octave = semitone >= 0 ? semitone / 12 : -((-semitone + 11) / 12);
        const int within = semitone - octave * 12;
        int best = kMajorScale[0];
        int bestDistance = 99;
        for(const int degree : kMajorScale)
        {
            const int distance = std::abs(degree - within);
            if(distance <= bestDistance)
            {
                best = degree;
                bestDistance = distance;
            }
        }
        semitone = octave * 12 + best;
    }
    return semitone;
}

float JustRatioForSemitone(int semitone)
{
    const int octave = semitone >= 0 ? semitone / 12 : -((-semitone + 11) / 12);
    int within = semitone - octave * 12;
    if(within < 0)
    {
        within += 12;
    }

    static constexpr std::array<float, 12> kRatios = {
        1.0f, 16.0f / 15.0f, 9.0f / 8.0f, 6.0f / 5.0f,
        5.0f / 4.0f, 4.0f / 3.0f, 3.0f / 2.0f, 3.0f / 2.0f,
        8.0f / 5.0f, 5.0f / 3.0f, 9.0f / 5.0f, 15.0f / 8.0f};
    return kRatios[static_cast<std::size_t>(within)] * std::pow(2.0f, octave);
}
} // namespace

struct DaisySubharmoniqCore::Impl
{
    double                              sampleRate   = 48000.0;
    std::size_t                         maxBlockSize = DaisySubharmoniqCore::kPreferredBlockSize;
    std::vector<DaisySubharmoniqParameter> parameters = MakeDefaultParameters();
    DaisySubharmoniqPage                activePage   = DaisySubharmoniqPage::kHome;
    DaisySubharmoniqQuantizeMode        quantizeMode = DaisySubharmoniqQuantizeMode::kTwelveEqual;
    int                                 seqOctaveRange = 2;
    std::array<std::array<float, DaisySubharmoniqCore::kStepsPerSequencer>,
               DaisySubharmoniqCore::kSequencerCount>
        sequencerSteps{};
    std::array<int, DaisySubharmoniqCore::kSequencerCount> stepIndices{};
    std::array<int, DaisySubharmoniqCore::kRhythmCount> rhythmDivisors{{1, 2, 3, 5}};
    std::array<DaisySubharmoniqRhythmTarget, DaisySubharmoniqCore::kRhythmCount>
        rhythmTargets{{DaisySubharmoniqRhythmTarget::kSeq1,
                       DaisySubharmoniqRhythmTarget::kSeq2,
                       DaisySubharmoniqRhythmTarget::kBoth,
                       DaisySubharmoniqRhythmTarget::kOff}};
    std::array<float, DaisySubharmoniqCore::kSourceCount> phases{};
    SimpleSvfLowpass filter;
    std::uint32_t currentSeed = 0;
    std::mt19937  rng{0u};
    bool          playing = false;
    bool          gateHigh = false;
    bool          hardMuted = false;
    bool          squareWave1 = false;
    bool          squareWave2 = false;
    int           currentMidiNote = 48;
    int           currentVelocity = 100;
    int           clockPulseCount = 0;
    float         internalClockSamplesUntilPulse = 0.0f;
    std::uint32_t triggerCount = 0;
    int           gatePulseSamples = 0;
    float         vcaEnvelope = 0.0f;
    float         vcfEnvelope = 0.0f;
    bool          envelopeGate = false;

    DaisySubharmoniqParameter* FindMutable(const std::string& parameterId)
    {
        return FindMutable(parameterId.c_str());
    }

    DaisySubharmoniqParameter* FindMutable(const char* parameterId)
    {
        if(parameterId == nullptr)
        {
            return nullptr;
        }
        for(auto& parameter : parameters)
        {
            if(parameter.id == parameterId)
            {
                return &parameter;
            }
        }
        return nullptr;
    }

    const DaisySubharmoniqParameter* Find(const std::string& parameterId) const
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

    void ResetSequencerDefaults()
    {
        for(std::size_t seq = 0; seq < sequencerSteps.size(); ++seq)
        {
            for(std::size_t step = 0; step < sequencerSteps[seq].size(); ++step)
            {
                const std::string id = "seq" + std::to_string(seq + 1)
                                       + "_step" + std::to_string(step + 1);
                sequencerSteps[seq][step] = Value(id);
            }
        }
    }

    int DivisorFromParameter(const std::string& parameterId) const
    {
        return 1 + static_cast<int>(std::round(Value(parameterId) * 15.0f));
    }

    void TriggerEnvelopes()
    {
        envelopeGate = true;
        vcaEnvelope = std::max(vcaEnvelope, 0.15f);
        vcfEnvelope = std::max(vcfEnvelope, 0.15f);
        gatePulseSamples = static_cast<int>(sampleRate * 0.001);
        ++triggerCount;
        hardMuted = false;
    }

    void StepInternalClock()
    {
        if(!playing)
        {
            return;
        }

        if(internalClockSamplesUntilPulse <= 0.0f)
        {
            const float tempoBpm = NormalizedToTempoBpm(Value("tempo"));
            const float pulsesPerSecond = std::max(1.0f, tempoBpm * 4.0f / 60.0f);
            internalClockSamplesUntilPulse
                += static_cast<float>(sampleRate) / pulsesPerSecond;
        }

        internalClockSamplesUntilPulse -= 1.0f;
    }
};

DaisySubharmoniqCore::DaisySubharmoniqCore() : impl_(new Impl)
{
    impl_->ResetSequencerDefaults();
}

DaisySubharmoniqCore::~DaisySubharmoniqCore()
{
    delete impl_;
}

void DaisySubharmoniqCore::Prepare(double sampleRate, std::size_t maxBlockSize)
{
    impl_->sampleRate   = sampleRate > 1000.0 ? sampleRate : 48000.0;
    impl_->maxBlockSize = maxBlockSize > 0 ? maxBlockSize : kPreferredBlockSize;
    impl_->filter.Init(static_cast<float>(impl_->sampleRate));
}

void DaisySubharmoniqCore::Process(float* outputLeft,
                                   float* outputRight,
                                   std::size_t frameCount)
{
    if(outputLeft == nullptr && outputRight == nullptr)
    {
        return;
    }

    if(impl_->hardMuted)
    {
        for(std::size_t frame = 0; frame < frameCount; ++frame)
        {
            if(outputLeft != nullptr)
            {
                outputLeft[frame] = 0.0f;
            }
            if(outputRight != nullptr)
            {
                outputRight[frame] = 0.0f;
            }
        }
        return;
    }

    const float sr = static_cast<float>(impl_->sampleRate);
    const float baseNote = static_cast<float>(impl_->currentMidiNote)
                           + ((impl_->Value("root_cv") - 0.5f) * 24.0f);
    const int seq1Semitones = GetSequencerStepSemitones(
        0, static_cast<std::size_t>(impl_->stepIndices[0]));
    const int seq2Semitones = GetSequencerStepSemitones(
        1, static_cast<std::size_t>(impl_->stepIndices[1]));
    const float vco1Freq = std::min(
        10000.0f,
        MidiNoteToFrequency(static_cast<int>(baseNote + seq1Semitones))
            * std::pow(2.0f, (impl_->Value("vco1_pitch") - 0.25f) * 4.0f));
    const float vco2Freq = std::min(
        10000.0f,
        MidiNoteToFrequency(static_cast<int>(baseNote + 7.0f + seq2Semitones))
            * std::pow(2.0f, (impl_->Value("vco2_pitch") - 0.38f) * 4.0f));
    const std::array<float, kSourceCount> freqs = {
        vco1Freq,
        vco1Freq / static_cast<float>(impl_->DivisorFromParameter("vco1_sub1_div")),
        vco1Freq / static_cast<float>(impl_->DivisorFromParameter("vco1_sub2_div")),
        vco2Freq,
        vco2Freq / static_cast<float>(impl_->DivisorFromParameter("vco2_sub1_div")),
        vco2Freq / static_cast<float>(impl_->DivisorFromParameter("vco2_sub2_div")),
    };
    const std::array<float, kSourceCount> levels = {
        impl_->Value("vco1_level"),
        impl_->Value("vco1_sub1_level"),
        impl_->Value("vco1_sub2_level"),
        impl_->Value("vco2_level"),
        impl_->Value("vco2_sub1_level"),
        impl_->Value("vco2_sub2_level"),
    };

    const float cutoffBase = NormalizedToLog(
        Clamp01(impl_->Value("cutoff") + impl_->Value("cutoff_cv") * 0.35f),
        60.0f,
        12000.0f);
    const float resonance = std::min(0.95f, impl_->Value("resonance") * 0.9f);
    impl_->filter.SetRes(resonance);
    impl_->filter.SetDrive(0.35f + impl_->Value("drive") * 0.65f);

    const float attackStep = 1.0f / std::max(
        1.0f,
        sr * NormalizedToLog(impl_->Value("vcf_attack"), 0.001f, 2.5f));
    const float vcfDecayStep = 1.0f / std::max(
        1.0f,
        sr * NormalizedToLog(impl_->Value("vcf_decay"), 0.005f, 5.0f));
    const float vcaDecayStep = 1.0f / std::max(
        1.0f,
        sr * NormalizedToLog(impl_->Value("vca_decay"), 0.005f, 5.0f));
    const float outputGain = impl_->Value("output") * 0.75f;

    for(std::size_t frame = 0; frame < frameCount; ++frame)
    {
        if(impl_->playing && impl_->internalClockSamplesUntilPulse <= 0.0f)
        {
            AdvanceClockPulse();
        }
        impl_->StepInternalClock();

        if(impl_->envelopeGate)
        {
            impl_->vcaEnvelope = std::min(1.0f, impl_->vcaEnvelope + attackStep);
            impl_->vcfEnvelope = std::min(1.0f, impl_->vcfEnvelope + attackStep);
            if(impl_->vcaEnvelope >= 0.999f && impl_->vcfEnvelope >= 0.999f)
            {
                impl_->envelopeGate = false;
            }
        }
        else
        {
            impl_->vcaEnvelope = std::max(0.0f, impl_->vcaEnvelope - vcaDecayStep);
            impl_->vcfEnvelope = std::max(0.0f, impl_->vcfEnvelope - vcfDecayStep);
        }

        float mixed = 0.0f;
        for(std::size_t source = 0; source < kSourceCount; ++source)
        {
            impl_->phases[source] += (kTwoPi * freqs[source]) / sr;
            if(impl_->phases[source] >= kTwoPi)
            {
                impl_->phases[source] -= kTwoPi
                                        * std::floor(impl_->phases[source] / kTwoPi);
            }
            const bool useSquare = source < 3 ? impl_->squareWave1 : impl_->squareWave2;
            mixed += (useSquare ? Square(impl_->phases[source])
                                : SoftSaw(impl_->phases[source]))
                     * levels[source];
        }

        mixed = std::tanh(mixed * (0.35f + impl_->Value("drive") * 1.8f));
        const float envelopeCutoff
            = cutoffBase
              * std::pow(2.0f,
                         (impl_->Value("vcf_env_amt") - 0.5f)
                             * 4.0f * impl_->vcfEnvelope);
        impl_->filter.SetFreq(std::min(16000.0f, std::max(20.0f, envelopeCutoff)));
        impl_->filter.Process(mixed);
        const float sample = impl_->filter.Low() * impl_->vcaEnvelope * outputGain;

        if(outputLeft != nullptr)
        {
            outputLeft[frame] = sample;
        }
        if(outputRight != nullptr)
        {
            outputRight[frame] = sample;
        }

        if(impl_->gatePulseSamples > 0)
        {
            --impl_->gatePulseSamples;
        }
    }
}

void DaisySubharmoniqCore::ResetToDefaultState(std::uint32_t seed)
{
    impl_->currentSeed = seed;
    impl_->rng.seed(seed);
    impl_->parameters = MakeDefaultParameters();
    impl_->activePage = DaisySubharmoniqPage::kHome;
    impl_->quantizeMode = DaisySubharmoniqQuantizeMode::kTwelveEqual;
    impl_->seqOctaveRange = 2;
    impl_->rhythmDivisors = {1, 2, 3, 5};
    impl_->rhythmTargets = {DaisySubharmoniqRhythmTarget::kSeq1,
                            DaisySubharmoniqRhythmTarget::kSeq2,
                            DaisySubharmoniqRhythmTarget::kBoth,
                            DaisySubharmoniqRhythmTarget::kOff};
    impl_->stepIndices.fill(0);
    impl_->phases.fill(0.0f);
    impl_->playing = false;
    impl_->gateHigh = false;
    impl_->hardMuted = false;
    impl_->currentMidiNote = 48;
    impl_->currentVelocity = 100;
    impl_->clockPulseCount = 0;
    impl_->internalClockSamplesUntilPulse = 0.0f;
    impl_->triggerCount = 0;
    impl_->gatePulseSamples = 0;
    impl_->vcaEnvelope = 0.0f;
    impl_->vcfEnvelope = 0.0f;
    impl_->envelopeGate = false;
    impl_->ResetSequencerDefaults();
}

bool DaisySubharmoniqCore::SetParameterValue(const std::string& parameterId,
                                             float              normalizedValue)
{
    return SetParameterValue(parameterId.c_str(), normalizedValue);
}

bool DaisySubharmoniqCore::SetParameterValue(const char* parameterId,
                                             float       normalizedValue)
{
    auto* parameter = impl_->FindMutable(parameterId);
    if(parameter == nullptr)
    {
        return false;
    }
    parameter->normalizedValue = Clamp01(normalizedValue);
    parameter->effectiveNormalizedValue = parameter->normalizedValue;

    std::size_t seq  = 0;
    std::size_t step = 0;
    if(DecodeSequencerStepId(parameterId, &seq, &step))
    {
        impl_->sequencerSteps[seq][step] = parameter->normalizedValue;
    }
    return true;
}

bool DaisySubharmoniqCore::GetParameterValue(const std::string& parameterId,
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

bool DaisySubharmoniqCore::GetEffectiveParameterValue(
    const std::string& parameterId,
    float*             normalizedValue) const
{
    return GetParameterValue(parameterId, normalizedValue);
}

bool DaisySubharmoniqCore::TriggerMomentaryAction(const std::string& actionId)
{
    if(actionId == "play_toggle")
    {
        impl_->playing = !impl_->playing;
        if(impl_->playing)
        {
            impl_->internalClockSamplesUntilPulse = 0.0f;
            impl_->hardMuted = false;
        }
        return true;
    }
    if(actionId == "reset")
    {
        impl_->stepIndices.fill(0);
        impl_->clockPulseCount = 0;
        impl_->internalClockSamplesUntilPulse = 0.0f;
        return true;
    }
    if(actionId == "next")
    {
        impl_->stepIndices[0] = (impl_->stepIndices[0] + 1) % 4;
        impl_->stepIndices[1] = (impl_->stepIndices[1] + 1) % 4;
        impl_->TriggerEnvelopes();
        return true;
    }
    if(actionId == "panic")
    {
        impl_->playing = false;
        impl_->gateHigh = false;
        impl_->hardMuted = true;
        impl_->vcaEnvelope = 0.0f;
        impl_->vcfEnvelope = 0.0f;
        impl_->envelopeGate = false;
        impl_->gatePulseSamples = 0;
        impl_->internalClockSamplesUntilPulse = 0.0f;
        return true;
    }
    if(actionId == "wave1_toggle")
    {
        impl_->squareWave1 = !impl_->squareWave1;
        return true;
    }
    if(actionId == "wave2_toggle")
    {
        impl_->squareWave2 = !impl_->squareWave2;
        return true;
    }
    return false;
}

void DaisySubharmoniqCore::TriggerGate(bool high)
{
    if(high && !impl_->gateHigh)
    {
        AdvanceClockPulse();
    }
    impl_->gateHigh = high;
}

void DaisySubharmoniqCore::HandleMidiEvent(std::uint8_t status,
                                           std::uint8_t data1,
                                           std::uint8_t data2)
{
    const std::uint8_t statusNibble = status & 0xF0;
    if(statusNibble == 0x90 && data2 > 0)
    {
        impl_->currentMidiNote = static_cast<int>(data1);
        impl_->currentVelocity = static_cast<int>(data2);
        impl_->hardMuted = false;
        impl_->TriggerEnvelopes();
    }
    else if(statusNibble == 0x80 || (statusNibble == 0x90 && data2 == 0))
    {
        impl_->gateHigh = false;
    }
    else if(status == 0xFA)
    {
        impl_->playing = true;
    }
    else if(status == 0xFC)
    {
        impl_->playing = false;
    }
    else if(status == 0xF8)
    {
        AdvanceClockPulse();
    }
}

const DaisySubharmoniqParameter* DaisySubharmoniqCore::FindParameter(
    const std::string& parameterId) const
{
    return impl_->Find(parameterId);
}

const std::vector<DaisySubharmoniqParameter>& DaisySubharmoniqCore::GetParameters()
    const
{
    return impl_->parameters;
}

std::unordered_map<std::string, float>
DaisySubharmoniqCore::CaptureStatefulParameterValues() const
{
    std::unordered_map<std::string, float> values;
    for(const auto& parameter : impl_->parameters)
    {
        if(parameter.stateful)
        {
            values[parameter.id] = parameter.normalizedValue;
        }
    }
    values["state/play"] = impl_->playing ? 1.0f : 0.0f;
    values["state/page"] = static_cast<float>(static_cast<int>(impl_->activePage));
    values["state/quantize"] = static_cast<float>(static_cast<int>(impl_->quantizeMode));
    values["state/seq_oct"] = static_cast<float>(impl_->seqOctaveRange);
    return values;
}

void DaisySubharmoniqCore::RestoreStatefulParameterValues(
    const std::unordered_map<std::string, float>& values)
{
    for(const auto& entry : values)
    {
        SetParameterValue(entry.first, entry.second);
    }
    const auto page = values.find("state/page");
    if(page != values.end())
    {
        SetActivePage(static_cast<DaisySubharmoniqPage>(
            ClampInt(static_cast<int>(std::round(page->second)), 0, 8)));
    }
    const auto quantize = values.find("state/quantize");
    if(quantize != values.end())
    {
        SetQuantizeMode(static_cast<DaisySubharmoniqQuantizeMode>(
            ClampInt(static_cast<int>(std::round(quantize->second)), 0, 4)));
    }
}

DaisySubharmoniqPage DaisySubharmoniqCore::GetActivePage() const
{
    return impl_->activePage;
}

bool DaisySubharmoniqCore::SetActivePage(DaisySubharmoniqPage page)
{
    const int index = static_cast<int>(page);
    if(index < 0 || index > static_cast<int>(DaisySubharmoniqPage::kAbout))
    {
        return false;
    }
    impl_->activePage = page;
    return true;
}

DaisySubharmoniqPageBinding DaisySubharmoniqCore::GetActivePageBinding() const
{
    DaisySubharmoniqPageBinding binding;
    binding.page = impl_->activePage;

    auto set = [&binding](const std::string& label,
                          const std::array<std::string, 8>& ids,
                          const std::array<std::string, 8>& labels) {
        binding.pageLabel = label;
        binding.parameterIds = ids;
        binding.parameterLabels = labels;
    };

    switch(impl_->activePage)
    {
        case DaisySubharmoniqPage::kVoice:
            set("Voice",
                {"vco1_pitch", "vco2_pitch", "vco1_sub1_div", "vco2_sub1_div",
                 "vco1_sub2_div", "vco2_sub2_div", "", ""},
                {"VCO 1", "VCO 2", "VCO1 Sub1", "VCO2 Sub1",
                 "VCO1 Sub2", "VCO2 Sub2", "", ""});
            break;
        case DaisySubharmoniqPage::kMix:
            set("Mix",
                {"vco1_level", "vco1_sub1_level", "vco1_sub2_level", "vco2_level",
                 "vco2_sub1_level", "vco2_sub2_level", "drive", "output"},
                {"VCO1 Lvl", "V1S1 Lvl", "V1S2 Lvl", "VCO2 Lvl",
                 "V2S1 Lvl", "V2S2 Lvl", "Drive", "Output"});
            break;
        case DaisySubharmoniqPage::kSeq:
            set("Seq",
                {"seq1_step1", "seq1_step2", "seq1_step3", "seq1_step4",
                 "seq2_step1", "seq2_step2", "seq2_step3", "seq2_step4"},
                {"S1 Step1", "S1 Step2", "S1 Step3", "S1 Step4",
                 "S2 Step1", "S2 Step2", "S2 Step3", "S2 Step4"});
            break;
        case DaisySubharmoniqPage::kRhythm:
            set("Rhythm",
                {"rhythm1_div", "rhythm2_div", "rhythm3_div", "rhythm4_div",
                 "", "", "", ""},
                {"Rhythm 1", "Rhythm 2", "Rhythm 3", "Rhythm 4",
                 "", "", "", ""});
            break;
        case DaisySubharmoniqPage::kFilter:
            set("Filter",
                {"cutoff", "resonance", "vcf_env_amt", "vca_decay",
                 "vcf_attack", "vcf_decay", "drive", "output"},
                {"Cutoff", "Res", "VCF Env", "VCA Decay",
                 "VCF Attack", "VCF Decay", "Drive", "Output"});
            break;
        case DaisySubharmoniqPage::kPatch:
            set("Patch",
                {"root_cv", "cutoff_cv", "rhythm_cv", "sub_cv", "", "", "", ""},
                {"Root CV", "Cut CV", "Rhythm CV", "Sub CV", "", "", "", ""});
            break;
        default:
            set("Home",
                {"tempo", "cutoff", "resonance", "vca_decay",
                 "vco1_pitch", "vco2_pitch", "drive", "output"},
                {"Tempo", "Cutoff", "Res", "VCA Decay",
                 "VCO 1", "VCO 2", "Drive", "Output"});
            break;
    }
    return binding;
}

DaisySubharmoniqQuantizeMode DaisySubharmoniqCore::GetQuantizeMode() const
{
    return impl_->quantizeMode;
}

void DaisySubharmoniqCore::SetQuantizeMode(DaisySubharmoniqQuantizeMode mode)
{
    impl_->quantizeMode = mode;
}

int DaisySubharmoniqCore::GetSeqOctaveRange() const
{
    return impl_->seqOctaveRange;
}

void DaisySubharmoniqCore::SetSeqOctaveRange(int octaveRange)
{
    if(octaveRange <= 1)
    {
        impl_->seqOctaveRange = 1;
    }
    else if(octaveRange <= 2)
    {
        impl_->seqOctaveRange = 2;
    }
    else
    {
        impl_->seqOctaveRange = 5;
    }
}

std::size_t DaisySubharmoniqCore::GetSourceCount() const
{
    return kSourceCount;
}

void DaisySubharmoniqCore::SetSequencerStepValue(std::size_t sequencer,
                                                 std::size_t step,
                                                 float       normalizedValue)
{
    if(sequencer >= kSequencerCount || step >= kStepsPerSequencer)
    {
        return;
    }
    impl_->sequencerSteps[sequencer][step] = Clamp01(normalizedValue);
    SetParameterValue("seq" + std::to_string(sequencer + 1) + "_step"
                          + std::to_string(step + 1),
                      normalizedValue);
}

int DaisySubharmoniqCore::GetSequencerStepIndex(std::size_t sequencer) const
{
    if(sequencer >= kSequencerCount)
    {
        return 0;
    }
    return impl_->stepIndices[sequencer];
}

int DaisySubharmoniqCore::GetSequencerStepSemitones(std::size_t sequencer,
                                                    std::size_t step) const
{
    if(sequencer >= kSequencerCount || step >= kStepsPerSequencer)
    {
        return 0;
    }
    return QuantizedSemitone(impl_->sequencerSteps[sequencer][step],
                             impl_->seqOctaveRange,
                             impl_->quantizeMode);
}

float DaisySubharmoniqCore::GetSequencerStepRatio(std::size_t sequencer,
                                                  std::size_t step) const
{
    const int semitone = GetSequencerStepSemitones(sequencer, step);
    if(impl_->quantizeMode == DaisySubharmoniqQuantizeMode::kTwelveJust
       || impl_->quantizeMode == DaisySubharmoniqQuantizeMode::kEightJust)
    {
        return JustRatioForSemitone(semitone);
    }
    return std::pow(2.0f, static_cast<float>(semitone) / 12.0f);
}

void DaisySubharmoniqCore::SetRhythmDivisor(std::size_t rhythm, int divisor)
{
    if(rhythm >= kRhythmCount)
    {
        return;
    }
    impl_->rhythmDivisors[rhythm] = ClampInt(divisor, 1, 16);
    SetParameterValue("rhythm" + std::to_string(rhythm + 1) + "_div",
                      static_cast<float>(impl_->rhythmDivisors[rhythm] - 1)
                          / 15.0f);
}

int DaisySubharmoniqCore::GetRhythmDivisor(std::size_t rhythm) const
{
    return rhythm < kRhythmCount ? impl_->rhythmDivisors[rhythm] : 1;
}

void DaisySubharmoniqCore::SetRhythmTarget(
    std::size_t rhythm,
    DaisySubharmoniqRhythmTarget target)
{
    if(rhythm < kRhythmCount)
    {
        impl_->rhythmTargets[rhythm] = target;
    }
}

DaisySubharmoniqRhythmTarget DaisySubharmoniqCore::GetRhythmTarget(
    std::size_t rhythm) const
{
    return rhythm < kRhythmCount ? impl_->rhythmTargets[rhythm]
                                 : DaisySubharmoniqRhythmTarget::kOff;
}

void DaisySubharmoniqCore::AdvanceClockPulse()
{
    if(!impl_->playing)
    {
        return;
    }

    ++impl_->clockPulseCount;
    bool trigger = false;
    for(std::size_t rhythm = 0; rhythm < kRhythmCount; ++rhythm)
    {
        if((impl_->clockPulseCount % impl_->rhythmDivisors[rhythm]) != 0)
        {
            continue;
        }

        const auto target = impl_->rhythmTargets[rhythm];
        if(target == DaisySubharmoniqRhythmTarget::kSeq1
           || target == DaisySubharmoniqRhythmTarget::kBoth)
        {
            impl_->stepIndices[0] = (impl_->stepIndices[0] + 1) % 4;
            trigger = true;
        }
        if(target == DaisySubharmoniqRhythmTarget::kSeq2
           || target == DaisySubharmoniqRhythmTarget::kBoth)
        {
            impl_->stepIndices[1] = (impl_->stepIndices[1] + 1) % 4;
            trigger = true;
        }
    }

    if(trigger)
    {
        impl_->TriggerEnvelopes();
    }
}

std::uint32_t DaisySubharmoniqCore::GetTriggerCount() const
{
    return impl_->triggerCount;
}

bool DaisySubharmoniqCore::IsPlaying() const
{
    return impl_->playing;
}

int DaisySubharmoniqCore::GetCurrentMidiNote() const
{
    return impl_->currentMidiNote;
}

float DaisySubharmoniqCore::GetGateOutputPulse() const
{
    return impl_->gatePulseSamples > 0 ? 1.0f : 0.0f;
}

float DaisySubharmoniqCore::GetSequencerCv(std::size_t sequencer) const
{
    if(sequencer >= kSequencerCount)
    {
        return 0.0f;
    }
    const int semitone = GetSequencerStepSemitones(
        sequencer, static_cast<std::size_t>(impl_->stepIndices[sequencer]));
    return Clamp01((static_cast<float>(semitone) + 60.0f) / 120.0f);
}
} // namespace daisyhost
