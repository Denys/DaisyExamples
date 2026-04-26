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
struct DaisyPolyOscParameter
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
};

class DaisyPolyOscCore
{
  public:
    static constexpr std::size_t kPreferredBlockSize = 48;
    static constexpr std::size_t kOscillatorCount    = 3;

    DaisyPolyOscCore();
    ~DaisyPolyOscCore();

    void Prepare(double sampleRate, std::size_t maxBlockSize);
    void Process(float*      output1,
                 float*      output2,
                 float*      output3,
                 float*      mixOutput,
                 std::size_t frameCount);

    void ResetToDefaultState(std::uint32_t seed = 0);
    bool SetParameterValue(const std::string& parameterId, float normalizedValue);
    bool GetParameterValue(const std::string& parameterId,
                           float*             normalizedValue) const;
    bool GetEffectiveParameterValue(const std::string& parameterId,
                                    float*             normalizedValue) const;

    const DaisyPolyOscParameter* FindParameter(
        const std::string& parameterId) const;
    const std::vector<DaisyPolyOscParameter>& GetParameters() const;

    std::unordered_map<std::string, float> CaptureStatefulParameterValues() const;
    void RestoreStatefulParameterValues(
        const std::unordered_map<std::string, float>& values);

    void        IncrementWaveform(int delta);
    int         GetWaveformIndex() const;
    std::string GetWaveformLabel() const;
    float       GetOscillatorFrequencyHz(std::size_t zeroBasedIndex) const;

  private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace daisyhost
