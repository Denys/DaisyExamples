#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace daisyhost
{
enum class DaisyHarmoniqsPage
{
    kSpectrum = 0,
    kEnvelope,
};

struct DaisyHarmoniqsParameter
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

struct DaisyHarmoniqsPageBinding
{
    DaisyHarmoniqsPage         page = DaisyHarmoniqsPage::kSpectrum;
    std::string                pageLabel;
    std::array<std::string, 4> parameterIds{};
    std::array<std::string, 4> parameterLabels{};
};

class DaisyHarmoniqsCore
{
  public:
    static constexpr std::size_t kPreferredBlockSize = 48;

    DaisyHarmoniqsCore();
    ~DaisyHarmoniqsCore();

    void Prepare(double sampleRate, std::size_t maxBlockSize);
    void Process(float* outputLeft, float* outputRight, std::size_t frameCount);

    void ResetToDefaultState(std::uint32_t seed = 0);
    bool SetParameterValue(const std::string& parameterId, float normalizedValue);
    bool SetEffectiveParameterValue(const std::string& parameterId,
                                    float normalizedValue);
    bool GetParameterValue(const std::string& parameterId,
                           float*             normalizedValue) const;
    bool GetEffectiveParameterValue(const std::string& parameterId,
                                    float*             normalizedValue) const;
    bool TriggerMomentaryAction(const std::string& actionId);
    void TriggerMidiNote(std::uint8_t midiNote, std::uint8_t velocity);
    void ReleaseMidiNote(std::uint8_t midiNote);
    void TriggerGate(bool high);

    const DaisyHarmoniqsParameter* FindParameter(
        const std::string& parameterId) const;
    const std::vector<DaisyHarmoniqsParameter>& GetParameters() const;

    std::unordered_map<std::string, float> CaptureStatefulParameterValues() const;
    void RestoreStatefulParameterValues(
        const std::unordered_map<std::string, float>& values);

    DaisyHarmoniqsPage GetActivePage() const;
    bool SetActivePage(DaisyHarmoniqsPage page);
    DaisyHarmoniqsPageBinding GetActivePageBinding() const;

    int GetCurrentMidiNote() const;
    int GetCurrentVelocity() const;

  private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace daisyhost
