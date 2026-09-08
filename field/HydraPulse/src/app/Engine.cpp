#include "src/app/Engine.h"
#include "src/app/Presets.generated.h"
#include <algorithm>
#include <cmath>

namespace hydrapulse
{
void Engine::Init(std::uint32_t sample_rate) noexcept
{
    sample_rate_ = (sample_rate >= 8000 && sample_rate <= 192000) ? sample_rate : 48000;
    ramp_samples_ = std::max<std::uint32_t>(1u, sample_rate_ / 200u);
    table_.Init();
    clock_.Init(sample_rate_);
    master_.Reset(0.25f);
    drive_.Reset(0);
    load_gain_.Reset(1);
    for (std::size_t i = 0; i < kTracks; ++i)
    {
        voices_[i].Init(static_cast<dsp::VoiceId>(i), float(sample_rate_), table_);
        mute_gain_[i].Reset(1);
    }
    for (auto &dc : dc_)
        dc.Init(float(sample_rate_));
    pending_preset_ = -1;
    faults_ = 0;
    step_events_ = 0;
    ApplyPreset(0);
}
const char *Engine::PresetName() const noexcept
{
    return kTutorialPresets[preset_].name;
}
void Engine::ApplyPreset(std::uint8_t id) noexcept
{
    const auto &preset = kTutorialPresets[id];
    preset_ = id;
    ++preset_revision_;
    running_ = false;
    first_ = true;
    fill_ = false;
    active_bank_ = queued_bank_ = play_bank_ = Bank::A;
    current_step_ = next_step_ = mute_mask_ = 0;
    (void)clock_.SetTempo(preset.bpm);
    (void)clock_.SetSwing(preset.swing);
    clock_.Reset();
    drive_.Reset(preset.drive);
    for (std::size_t t = 0; t < kTracks; ++t)
    {
        voices_[t].Reset();
        voices_[t].SetParameters(preset.voices[t], true);
        mute_gain_[t].Reset(1);
    }
    for (std::size_t b = 0; b < kBanks; ++b)
        for (std::size_t t = 0; t < kTracks; ++t)
            for (std::size_t s = 0; s < kSteps; ++s)
            {
                const auto bit = std::uint16_t(1u << s);
                banks_[b].notes.Set(t, s, (preset.notes[b][t] & bit) != 0);
                banks_[b].accents.Set(t, s, (preset.accents[b][t] & bit) != 0);
            }
    for (auto &dc : dc_)
        dc.Reset();
}
bool Engine::RequestPreset(std::uint8_t id) noexcept
{
    if (id >= kPresets || running_ || Loading())
        return false;
    pending_preset_ = int(id);
    load_gain_.Set(0, ramp_samples_);
    return true;
}
void Engine::Start() noexcept
{
    if (Loading() || running_)
        return;
    current_step_ = next_step_ = 0;
    first_ = true;
    running_ = true;
    clock_.Reset();
}
void Engine::Stop() noexcept
{
    running_ = false;
    fill_ = false;
    for (auto &voice : voices_)
        voice.Release();
}
void Engine::Panic() noexcept
{
    Stop();
    // Existing finite releases reach exact zero. No phase/state reset while audible.
    pending_preset_ = -1;
    load_gain_.Set(1, ramp_samples_);
}
void Engine::Trigger(std::size_t track, float velocity) noexcept
{
    if (track >= kTracks || Loading() || (mute_mask_ & (1u << track)))
        return;
    voices_[track].Trigger(velocity);
}
void Engine::HandleMidi(const MidiCommand &command) noexcept
{
    switch (command.type)
    {
    case MidiCommandType::Trigger:
        Trigger(command.track, float(command.velocity) / 127.0f);
        break;
    case MidiCommandType::Start:
        Start();
        break;
    case MidiCommandType::Stop:
        Stop();
        break;
    case MidiCommandType::Panic:
        Panic();
        break;
    }
}
void Engine::SetMaster(float level) noexcept
{
    master_.Set(dsp::Unit(level, master_.Target()), ramp_samples_);
}
void Engine::SetDrive(float drive) noexcept
{
    drive_.Set(dsp::Unit(drive, drive_.Target()), ramp_samples_);
}
void Engine::SetVoice(std::size_t track, const dsp::VoiceParameters &parameters) noexcept
{
    if (track < kTracks)
        voices_[track].SetParameters(parameters);
}
dsp::VoiceParameters Engine::Voice(std::size_t track) const noexcept
{
    return track < kTracks ? voices_[track].Parameters() : dsp::VoiceParameters{};
}
bool Engine::ToggleStep(Bank bank, std::size_t track, std::size_t step) noexcept
{
    if (!ValidBank(bank) || track >= kTracks || step >= kSteps)
        return false;
    auto &b = banks_[static_cast<std::size_t>(bank)];
    b.notes.Toggle(track, step);
    if (!b.notes.IsActive(track, step))
        b.accents.Set(track, step, false);
    return true;
}
bool Engine::ToggleAccent(Bank bank, std::size_t track, std::size_t step) noexcept
{
    if (!ValidBank(bank) || track >= kTracks || step >= kSteps)
        return false;
    auto &b = banks_[static_cast<std::size_t>(bank)];
    b.notes.Set(track, step, true);
    b.accents.Toggle(track, step);
    return true;
}
bool Engine::ClearTrack(Bank bank, std::size_t track) noexcept
{
    if (!ValidBank(bank) || track >= kTracks || running_)
        return false;
    auto &b = banks_[static_cast<std::size_t>(bank)];
    b.notes.ClearTrack(track);
    b.accents.ClearTrack(track);
    return true;
}
std::uint16_t Engine::Notes(Bank bank, std::size_t track) const noexcept
{
    return ValidBank(bank) ? banks_[static_cast<std::size_t>(bank)].notes.TrackMask(track) : 0;
}
std::uint16_t Engine::Accents(Bank bank, std::size_t track) const noexcept
{
    return ValidBank(bank) ? banks_[static_cast<std::size_t>(bank)].accents.TrackMask(track) : 0;
}
void Engine::QueueBank(Bank bank) noexcept
{
    if (bank != Bank::A && bank != Bank::B)
        return;
    queued_bank_ = bank;
    if (!running_)
        active_bank_ = play_bank_ = bank;
}
void Engine::ToggleMute(std::size_t track) noexcept
{
    if (track >= kTracks)
        return;
    mute_mask_ ^= static_cast<std::uint8_t>(1u << track);
    mute_gain_[track].Set((mute_mask_ & (1u << track)) ? 0.0f : 1.0f, ramp_samples_);
}
void Engine::EmitStep() noexcept
{
    if (next_step_ == 0)
        active_bank_ = queued_bank_;
    play_bank_ = fill_ ? Bank::Fill : active_bank_;
    current_step_ = next_step_;
    const auto &bank = banks_[static_cast<std::size_t>(play_bank_)];
    for (std::size_t t = 0; t < kTracks; ++t)
        if (bank.notes.IsActive(t, current_step_))
            Trigger(t, bank.accents.IsActive(t, current_step_) ? 1.0f : 0.72f);
    next_step_ = static_cast<std::uint8_t>((next_step_ + 1u) % 16u);
    ++step_events_;
}
StereoFrame Engine::Process() noexcept
{
    float loading_gain = load_gain_.Process();
    if (Loading() && load_gain_.Settled() && loading_gain == 0)
    {
        const auto id = static_cast<std::uint8_t>(pending_preset_);
        ApplyPreset(id);
        pending_preset_ = -1;
        load_gain_.Set(1, ramp_samples_);
    }
    if (running_)
    {
        if (first_)
        {
            first_ = false;
            EmitStep();
        }
        else if (clock_.ProcessSample())
            EmitStep();
    }
    // Fixed linear pan law: each voice's L+R contribution is constant.
    constexpr float pans[4] = {-0.08f, 0.1f, 0.4f, -0.35f};
    float left = 0, right = 0;
    for (std::size_t t = 0; t < kTracks; ++t)
    {
        const float sample = voices_[t].Process() * mute_gain_[t].Process();
        left += sample * (1.0f - pans[t]) * 0.5f;
        right += sample * (1.0f + pans[t]) * 0.5f;
    }
    const float master = master_.Process() * 0.8f * loading_gain;
    const float drive = drive_.Process();
    const float gain = 0.32f * (1.0f + 5.0f * drive);
    left = dc_[0].Process(left * gain);
    right = dc_[1].Process(right * gain);
    if (!std::isfinite(left) || !std::isfinite(right))
    {
        ++faults_;
        Panic();
        for (auto &dc : dc_)
            dc.Reset();
        return {};
    }
    return {0.92f * dsp::Saturate(left) * master, 0.92f * dsp::Saturate(right) * master};
}
} // namespace hydrapulse
