#pragma once
#include "src/dsp/Primitives.h"
#include <array>
#include <cstdint>

namespace hydrapulse::dsp
{
enum class VoiceId : std::uint8_t
{
    Hammer = 0,
    Crack = 1,
    Steel = 2,
    Arc = 3
};
struct VoiceParameters
{
    float tune{0.4f}, decay{0.3f}, character{0.35f}, level{0.75f};
};
// Fixed work and no ownership of memory outside a read-only initialized sine table.
class PercussionVoice
{
  public:
    void Init(VoiceId id, float sample_rate, const SineTable &table) noexcept;
    void SetParameters(const VoiceParameters &p, bool immediate = false) noexcept;
    void Trigger(float velocity) noexcept;
    void Release() noexcept
    {
        amplitude_.Release();
    }
    void Reset() noexcept;
    float Process() noexcept;
    VoiceParameters Parameters() const noexcept
    {
        return parameters_;
    }
    float DecaySeconds() const noexcept
    {
        return decay_seconds_;
    }
    std::uint32_t Faults() const noexcept
    {
        return faults_;
    }

  private:
    float Osc(std::size_t oscillator, float frequency, float phase_mod = 0) noexcept;
    VoiceId id_{VoiceId::Hammer};
    float sr_{48000}, inverse_sr_{1.0f / 48000};
    float noise_low_{0}, decay_seconds_{0.2f};
    std::uint32_t faults_{0};
    std::array<float, 6> phase_{};
    const SineTable *table_{nullptr};
    Noise noise_;
    Ramp frequency_, character_, level_, noise_alpha_;
    Envelope amplitude_, pitch_;
    VoiceParameters parameters_{};
};
} // namespace hydrapulse::dsp
