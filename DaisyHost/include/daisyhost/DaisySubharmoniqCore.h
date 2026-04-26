#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace daisyhost
{
enum class DaisySubharmoniqPage
{
    kHome = 0,
    kVoice,
    kMix,
    kSeq,
    kRhythm,
    kFilter,
    kPatch,
    kMidi,
    kAbout,
};

enum class DaisySubharmoniqQuantizeMode
{
    kOff = 0,
    kTwelveEqual,
    kEightEqual,
    kTwelveJust,
    kEightJust,
};

enum class DaisySubharmoniqRhythmTarget
{
    kOff = 0,
    kSeq1,
    kSeq2,
    kBoth,
};

struct DaisySubharmoniqParameter
{
    std::string id;
    std::string label;
    std::string groupLabel;
    float       normalizedValue          = 0.0f;
    float       defaultNormalizedValue   = 0.0f;
    float       effectiveNormalizedValue = 0.0f;
    int         stepCount                = 0;
    int         importanceRank           = 100;
    bool        automatable              = false;
    bool        stateful                 = true;
    bool        performanceTier          = false;
};

struct DaisySubharmoniqPageBinding
{
    DaisySubharmoniqPage       page = DaisySubharmoniqPage::kHome;
    std::string                pageLabel;
    std::array<std::string, 8> parameterIds{};
    std::array<std::string, 8> parameterLabels{};
};

class DaisySubharmoniqCore
{
  public:
    static constexpr std::size_t kPreferredBlockSize = 48;
    static constexpr std::size_t kSourceCount        = 6;
    static constexpr std::size_t kSequencerCount     = 2;
    static constexpr std::size_t kStepsPerSequencer  = 4;
    static constexpr std::size_t kRhythmCount        = 4;

    DaisySubharmoniqCore();
    ~DaisySubharmoniqCore();

    void Prepare(double sampleRate, std::size_t maxBlockSize);
    void Process(float* outputLeft, float* outputRight, std::size_t frameCount);

    void ResetToDefaultState(std::uint32_t seed = 0);
    bool SetParameterValue(const std::string& parameterId, float normalizedValue);
    bool SetParameterValue(const char* parameterId, float normalizedValue);
    bool GetParameterValue(const std::string& parameterId,
                           float*             normalizedValue) const;
    bool GetEffectiveParameterValue(const std::string& parameterId,
                                    float*             normalizedValue) const;
    bool TriggerMomentaryAction(const std::string& actionId);
    void TriggerGate(bool high);
    void HandleMidiEvent(std::uint8_t status,
                         std::uint8_t data1,
                         std::uint8_t data2);

    const DaisySubharmoniqParameter* FindParameter(
        const std::string& parameterId) const;
    const std::vector<DaisySubharmoniqParameter>& GetParameters() const;
    std::unordered_map<std::string, float> CaptureStatefulParameterValues() const;
    void RestoreStatefulParameterValues(
        const std::unordered_map<std::string, float>& values);

    DaisySubharmoniqPage GetActivePage() const;
    bool                 SetActivePage(DaisySubharmoniqPage page);
    DaisySubharmoniqPageBinding GetActivePageBinding() const;

    DaisySubharmoniqQuantizeMode GetQuantizeMode() const;
    void SetQuantizeMode(DaisySubharmoniqQuantizeMode mode);
    int  GetSeqOctaveRange() const;
    void SetSeqOctaveRange(int octaveRange);

    std::size_t GetSourceCount() const;
    void        SetSequencerStepValue(std::size_t sequencer,
                                      std::size_t step,
                                      float       normalizedValue);
    int         GetSequencerStepIndex(std::size_t sequencer) const;
    int         GetSequencerStepSemitones(std::size_t sequencer,
                                          std::size_t step) const;
    float       GetSequencerStepRatio(std::size_t sequencer,
                                      std::size_t step) const;
    void        SetRhythmDivisor(std::size_t rhythm, int divisor);
    int         GetRhythmDivisor(std::size_t rhythm) const;
    void        SetRhythmTarget(std::size_t rhythm,
                                DaisySubharmoniqRhythmTarget target);
    DaisySubharmoniqRhythmTarget GetRhythmTarget(std::size_t rhythm) const;
    void        AdvanceClockPulse();
    std::uint32_t GetTriggerCount() const;

    bool IsPlaying() const;
    int  GetCurrentMidiNote() const;
    float GetGateOutputPulse() const;
    float GetSequencerCv(std::size_t sequencer) const;

  private:
    struct Impl;
    Impl* impl_;
};
} // namespace daisyhost
