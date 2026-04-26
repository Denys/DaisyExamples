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
enum class DaisyVASynthPage
{
    kOsc = 0,
    kFilter,
    kMotion,
};

struct DaisyVASynthParameter
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

struct DaisyVASynthPageBinding
{
    DaisyVASynthPage           page = DaisyVASynthPage::kOsc;
    std::string                pageLabel;
    std::array<std::string, 4> parameterIds{};
    std::array<std::string, 4> parameterLabels{};
};

class DaisyVASynthCore
{
  public:
    static constexpr std::size_t kPreferredBlockSize = 48;

    DaisyVASynthCore();
    ~DaisyVASynthCore();

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
    void Panic();

    const DaisyVASynthParameter* FindParameter(const std::string& parameterId) const;
    const std::vector<DaisyVASynthParameter>& GetParameters() const;

    std::unordered_map<std::string, float> CaptureStatefulParameterValues() const;
    void RestoreStatefulParameterValues(
        const std::unordered_map<std::string, float>& values);

    DaisyVASynthPage GetActivePage() const;
    bool             SetActivePage(DaisyVASynthPage page);
    DaisyVASynthPageBinding GetActivePageBinding() const;

    int         GetCurrentVoiceCount() const;
    int         GetLastMidiNote() const;
    std::string GetWaveLabel(const std::string& parameterId) const;

  private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace daisyhost
