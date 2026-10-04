#include "src/dsp/Voices.h"
#include <algorithm>
#include <cmath>

namespace hydrapulse::dsp
{
void PercussionVoice::Init(VoiceId id, float sample_rate, const SineTable &table) noexcept
{
    id_ = id;
    sr_ = std::isfinite(sample_rate) ? std::clamp(sample_rate, 8000.0f, 192000.0f) : 48000.0f;
    inverse_sr_ = 1.0f / sr_;
    table_ = &table;
    amplitude_.Init(sr_);
    pitch_.Init(sr_);
    faults_ = 0;
    Reset();
    SetParameters({}, true);
}

void PercussionVoice::Reset() noexcept
{
    phase_.fill(0);
    noise_low_ = 0;
    noise_.Seed(0x48504631u + static_cast<std::uint32_t>(id_) * 0x19A3u);
    amplitude_.Reset();
    pitch_.Reset();
}

void PercussionVoice::SetParameters(const VoiceParameters &p, bool immediate) noexcept
{
    parameters_ = {Unit(p.tune, parameters_.tune), Unit(p.decay, parameters_.decay),
                   Unit(p.character, parameters_.character), Unit(p.level, parameters_.level)};
    float low = 32, high = 110, min_decay = 0.06f, max_decay = 1.4f;
    switch (id_)
    {
    case VoiceId::Hammer:
        break;
    case VoiceId::Crack:
        low = 120;
        high = 330;
        min_decay = 0.04f;
        max_decay = 0.8f;
        break;
    case VoiceId::Steel:
        low = 700;
        high = 2300;
        min_decay = 0.02f;
        max_decay = 0.3f;
        break;
    case VoiceId::Arc:
        low = 55;
        high = 900;
        min_decay = 0.05f;
        max_decay = 2.4f;
        break;
    }
    const float frequency = ExpMap(parameters_.tune, low, high);
    decay_seconds_ = ExpMap(parameters_.decay, min_decay, max_decay);
    amplitude_.SetDecay(decay_seconds_);
    pitch_.SetDecay(0.02f + 0.12f * parameters_.decay);
    // Convex one-pole noise filter; coefficient cannot create a resonant feedback loop.
    const float cutoff = 900.0f + 4500.0f * parameters_.character;
    const float noise_alpha = 1.0f - std::exp(-2.0f * kPi * cutoff / sr_);
    const auto ramp_samples = std::max<std::uint32_t>(1u, static_cast<std::uint32_t>(sr_ * 0.005f));
    if (immediate)
    {
        frequency_.Reset(frequency);
        noise_alpha_.Reset(noise_alpha);
        character_.Reset(parameters_.character);
        level_.Reset(parameters_.level);
    }
    else
    {
        frequency_.Set(frequency, ramp_samples);
        noise_alpha_.Set(noise_alpha, ramp_samples);
        character_.Set(parameters_.character, ramp_samples);
        level_.Set(parameters_.level, ramp_samples);
    }
}

void PercussionVoice::Trigger(float velocity) noexcept
{
    if (!std::isfinite(velocity) || velocity <= 0 || !table_)
        return;
    // Keep oscillator phases and the current envelope value on retrigger.
    amplitude_.Trigger(velocity, id_ == VoiceId::Steel ? 0.0005f : 0.001f);
    // Smooth the transient depth as well as amplitude: PM index must not jump on retrigger.
    pitch_.Trigger(1.0f);
}

float PercussionVoice::Osc(std::size_t oscillator, float frequency, float phase_mod) noexcept
{
    const float value = table_->Read(phase_[oscillator] + phase_mod);
    const float increment = std::clamp(frequency * inverse_sr_, 0.0f, 0.42f);
    phase_[oscillator] += increment;
    if (phase_[oscillator] >= 1)
        phase_[oscillator] -= 1;
    return value;
}

float PercussionVoice::Process() noexcept
{
    const float f = frequency_.Process();
    const float c = character_.Process();
    const float level = level_.Process();
    const float envelope = amplitude_.Process();
    const float pitch_env = pitch_.Process();
    const float noise_alpha = noise_alpha_.Process();
    if (!table_ || envelope == 0)
        return 0;

    float signal = 0;
    switch (id_)
    {
    case VoiceId::Hammer:
    {
        const float pitched = std::min(f * (1.0f + (2.0f + 10.0f * c) * pitch_env), 1800.0f);
        const float body = Osc(0, pitched);
        const float overtone = Osc(1, pitched * 2.0f);
        signal = (body + 0.18f * c * overtone) / (1.0f + 0.18f * c);
        break;
    }
    case VoiceId::Crack:
    {
        const float n = noise_.Process();
        noise_low_ += noise_alpha * (n - noise_low_);
        const float highpass = (n - noise_low_) * 0.65f;
        const float body = 0.65f * Osc(0, f * (1.0f + 0.35f * pitch_env)) + 0.35f * Osc(1, f * 1.47f);
        signal = body * (0.55f - 0.25f * c) + highpass * (0.45f + 0.25f * c);
        break;
    }
    case VoiceId::Steel:
    {
        constexpr float ratios[6] = {1.0f, 1.342f, 1.893f, 2.511f, 3.117f, 4.017f};
        float partials[6]{};
        for (std::size_t i = 0; i < 6; ++i)
            partials[i] = Osc(i, f * ratios[i]);
        const float metal =
            (partials[0] * partials[1] + partials[2] * partials[3] + partials[4] * partials[5]) *
            (1.0f / 3.0f);
        const float n = noise_.Process();
        noise_low_ += noise_alpha * (n - noise_low_);
        signal = metal * (0.8f - 0.35f * c) + (n - noise_low_) * (0.15f + 0.2f * c);
        break;
    }
    case VoiceId::Arc:
    {
        const float modulator = Osc(1, f * (2.0f + 4.5f * c));
        // Finite, feed-forward phase modulation. This is not feedback FM.
        const float index = (0.5f + 4.5f * c) * (0.15f + 0.85f * pitch_env);
        signal = Osc(0, f, modulator * index / (2.0f * kPi));
        break;
    }
    }
    const float result = signal * envelope * level;
    if (!std::isfinite(result))
    {
        ++faults_;
        Reset();
        return 0;
    }
    return std::clamp(result, -1.0f, 1.0f);
}
} // namespace hydrapulse::dsp
