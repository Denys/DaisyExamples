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
enum class DaisyCloudSeedPage
{
    kSpace = 0,
    kMotion,
};

struct DaisyCloudSeedParameter
{
    std::string id;
    std::string label;
    std::string groupLabel;
    float       normalizedValue        = 0.0f;
    float       defaultNormalizedValue = 0.0f;
    float       effectiveNormalizedValue = 0.0f;
    int         stepCount             = 0;
    int         importanceRank        = 100;
    bool        automatable           = false;
    bool        stateful              = true;
    bool        performanceTier       = false;
};

struct DaisyCloudSeedPageBinding
{
    DaisyCloudSeedPage           page = DaisyCloudSeedPage::kSpace;
    std::string                  pageLabel;
    std::array<std::string, 4>   parameterIds{};
    std::array<std::string, 4>   parameterLabels{};
};

class DaisyCloudSeedCore
{
  public:
    static constexpr std::size_t kPreferredBlockSize = 48;

    DaisyCloudSeedCore();
    ~DaisyCloudSeedCore();

    void Prepare(double sampleRate, std::size_t maxBlockSize);
    void Process(const float* inputLeft,
                 const float* inputRight,
                 float*       outputLeft,
                 float*       outputRight,
                 std::size_t  frameCount);

    void ResetToDefaultState(std::uint32_t seed = 0);
    bool SetParameterValue(const std::string& parameterId, float normalizedValue);
    bool GetParameterValue(const std::string& parameterId,
                           float*             normalizedValue) const;
    bool GetEffectiveParameterValue(const std::string& parameterId,
                                    float*             normalizedValue) const;
    bool TriggerMomentaryAction(const std::string& actionId);

    const DaisyCloudSeedParameter* FindParameter(
        const std::string& parameterId) const;
    const std::vector<DaisyCloudSeedParameter>& GetParameters() const;

    std::unordered_map<std::string, float> CaptureStatefulParameterValues()
        const;
    void RestoreStatefulParameterValues(
        const std::unordered_map<std::string, float>& values);

    DaisyCloudSeedPage GetActivePage() const;
    bool SetActivePage(DaisyCloudSeedPage page);
    DaisyCloudSeedPageBinding GetActivePageBinding() const;

    std::string GetProgramLabel() const;
    std::string GetSeedSummary() const;

  private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace daisyhost
