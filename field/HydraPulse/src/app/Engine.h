#pragma once
#include "src/app/Types.h"
#include "src/core/GrooveClock.h"
#include "src/dsp/Primitives.h"
#include <array>
#include <cstdint>

namespace hydrapulse
{
// Single-owner object: only the audio callback (or the serial host renderer) may mutate it.
class Engine
{
  public:
    void Init(std::uint32_t sample_rate = 48000) noexcept;
    StereoFrame Process() noexcept;
    void Start() noexcept;
    void Stop() noexcept;
    void Panic() noexcept;
    bool Running() const noexcept
    {
        return running_;
    }
    bool Loading() const noexcept
    {
        return pending_preset_ >= 0;
    }
    bool RequestPreset(std::uint8_t id) noexcept;
    std::uint8_t PresetId() const noexcept
    {
        return preset_;
    }
    std::uint32_t PresetRevision() const noexcept
    {
        return preset_revision_;
    }
    const char *PresetName() const noexcept;
    void Trigger(std::size_t track, float velocity = 0.75f) noexcept;
    void HandleMidi(const MidiCommand &command) noexcept;
    bool SetTempo(double bpm) noexcept
    {
        return clock_.SetTempo(bpm);
    }
    bool SetSwing(float amount) noexcept
    {
        return clock_.SetSwing(amount);
    }
    void SetDrive(float drive) noexcept;
    void SetMaster(float level) noexcept;
    float Master() const noexcept
    {
        return master_.Target();
    }
    float Drive() const noexcept
    {
        return drive_.Target();
    }
    double Tempo() const noexcept
    {
        return clock_.Tempo();
    }
    float Swing() const noexcept
    {
        return clock_.Swing();
    }
    void SetVoice(std::size_t track, const dsp::VoiceParameters &parameters) noexcept;
    dsp::VoiceParameters Voice(std::size_t track) const noexcept;
    bool ToggleStep(Bank bank, std::size_t track, std::size_t step) noexcept;
    bool ToggleAccent(Bank bank, std::size_t track, std::size_t step) noexcept;
    bool ClearTrack(Bank bank, std::size_t track) noexcept;
    std::uint16_t Notes(Bank bank, std::size_t track) const noexcept;
    std::uint16_t Accents(Bank bank, std::size_t track) const noexcept;
    void QueueBank(Bank bank) noexcept;
    void SetFill(bool held) noexcept
    {
        fill_ = held;
    }
    bool Fill() const noexcept
    {
        return fill_;
    }
    Bank ActiveBank() const noexcept
    {
        return active_bank_;
    }
    Bank QueuedBank() const noexcept
    {
        return queued_bank_;
    }
    Bank PlayingBank() const noexcept
    {
        return play_bank_;
    }
    std::uint8_t CurrentStep() const noexcept
    {
        return current_step_;
    }
    void ToggleMute(std::size_t track) noexcept;
    std::uint8_t Mutes() const noexcept
    {
        return mute_mask_;
    }
    std::uint32_t Faults() const noexcept
    {
        std::uint32_t total = faults_;
        for (const auto &voice : voices_)
            total += voice.Faults();
        return total;
    }
    std::uint64_t StepEvents() const noexcept
    {
        return step_events_;
    }

  private:
    void ApplyPreset(std::uint8_t id) noexcept;
    void EmitStep() noexcept;
    static bool ValidBank(Bank bank) noexcept
    {
        return static_cast<unsigned>(bank) < 3u;
    }
    std::uint32_t sample_rate_{48000}, ramp_samples_{240}, preset_revision_{0}, faults_{0};
    dsp::SineTable table_;
    std::array<dsp::PercussionVoice, kTracks> voices_;
    std::array<BankPattern, kBanks> banks_{};
    std::array<dsp::Ramp, kTracks> mute_gain_;
    dsp::Ramp master_, drive_, load_gain_;
    std::array<dsp::DcBlock, 2> dc_;
    core::GrooveClock clock_;
    std::uint64_t step_events_{0};
    Bank active_bank_{Bank::A}, queued_bank_{Bank::A}, play_bank_{Bank::A};
    std::uint8_t preset_{0}, current_step_{0}, next_step_{0}, mute_mask_{0};
    int pending_preset_{-1};
    bool running_{false}, first_{true}, fill_{false};
};
} // namespace hydrapulse
