#include "src/app/Controller.h"
#include <algorithm>
#include <cmath>

namespace hydrapulse
{
void Controller::CaptureTargets(const Engine &engine, const InputFrame &frame, bool globals) noexcept
{
    const auto p = engine.Voice(selected_);
    const float values[8] = {
        p.tune,         p.decay,        p.character,    p.level, float((engine.Tempo() - 40.0) / 200.0),
        engine.Swing(), engine.Drive(), engine.Master()};
    for (std::size_t i = 0; i < (globals ? 8u : 4u); ++i)
        pickup_[i].Reset(values[i], frame.knobs[i]);
}
void Controller::Init(Engine &engine, const InputFrame &frame) noexcept
{
    selected_ = 0;
    edit_ = Bank::A;
    previous_ = frame;
    revision_ = engine.PresetRevision();
    pending_preset_ = engine.PresetId();
    consumed_ = frame.keys;
    long_done_ = 0;
    held_ms_.fill(0);
    panic_ms_ = clear_ms_ = preset_age_ = 0;
    initialized_ = true;
    play_armed_ = panic_latched_ = clear_done_ = preset_armed_ = false;
    CaptureTargets(engine, frame, true);
}
void Controller::ShiftPress(Engine &engine, std::size_t key, const InputFrame &frame) noexcept
{
    if (key < 4)
    {
        selected_ = static_cast<std::uint8_t>(key);
        CaptureTargets(engine, frame, false);
        preset_armed_ = false;
        if (!engine.Running())
            engine.Trigger(key);
    }
    else if (key == 4 || key == 5)
    {
        edit_ = (key == 4) ? Bank::A : Bank::B;
        engine.QueueBank(edit_);
        preset_armed_ = false;
    }
    else if (key == 6)
    {
        edit_ = Bank::Fill;
        preset_armed_ = false;
    }
    else if (key >= 8 && key <= 11)
        engine.ToggleMute(key - 8);
    else if ((key == 13 || key == 14) && !engine.Running() && !engine.Loading())
    {
        if (!preset_armed_)
            pending_preset_ = engine.PresetId();
        pending_preset_ = static_cast<std::uint8_t>((pending_preset_ + (key == 13 ? 7u : 1u)) % 8u);
        preset_armed_ = true;
        preset_age_ = 0;
    }
    else if (key == 15 && preset_armed_ && !engine.Running())
    {
        (void)engine.RequestPreset(pending_preset_);
        preset_armed_ = false;
    }
}
void Controller::Tick(Engine &engine, const InputFrame &frame) noexcept
{
    if (!initialized_)
    {
        Init(engine, frame);
        return;
    }
    if (revision_ != engine.PresetRevision())
    {
        revision_ = engine.PresetRevision();
        edit_ = Bank::A;
        pending_preset_ = engine.PresetId();
        consumed_ |= frame.keys;
        long_done_ = 0;
        CaptureTargets(engine, frame, true);
    }
    const auto pressed = std::uint16_t(frame.keys & ~previous_.keys);
    const auto released = std::uint16_t(previous_.keys & ~frame.keys);
    if (frame.shift)
    {
        consumed_ |= frame.keys;
        play_armed_ = false;
    }
    if (frame.play && frame.shift)
    {
        if (panic_ms_ < 500)
            ++panic_ms_;
        if (panic_ms_ == 500 && !panic_latched_)
        {
            engine.Panic();
            panic_latched_ = true;
            preset_armed_ = false;
        }
    }
    else
        panic_ms_ = 0;
    if (panic_latched_)
    {
        consumed_ |= frame.keys;
        play_armed_ = false;
        engine.SetFill(false);
        previous_ = frame;
        if (!frame.play && !frame.shift)
            panic_latched_ = false;
        return;
    }
    if (frame.play && !previous_.play && !frame.shift)
        play_armed_ = true;
    if (frame.shift || previous_.shift)
        play_armed_ = false;
    if (!frame.play && previous_.play && play_armed_)
    {
        if (engine.Running())
            engine.Stop();
        else
            engine.Start();
        play_armed_ = false;
        preset_armed_ = false;
    }
    if (!(frame.play && frame.shift))
    {
        for (std::size_t key = 0; key < 16; ++key)
        {
            const auto bit = std::uint16_t(1u << key);
            if (pressed & bit)
            {
                held_ms_[key] = 0;
                long_done_ &= std::uint16_t(~bit);
                if (frame.shift)
                    ShiftPress(engine, key, frame);
            }
            if ((frame.keys & bit) && !frame.shift && !(consumed_ & bit))
            {
                if (held_ms_[key] < 400)
                    ++held_ms_[key];
                if (held_ms_[key] == 400 && !(long_done_ & bit))
                {
                    engine.ToggleAccent(edit_, selected_, key);
                    long_done_ |= bit;
                }
            }
            if (released & bit)
            {
                if (!(consumed_ & bit) && !(long_done_ & bit) && !frame.shift)
                    engine.ToggleStep(edit_, selected_, key);
                consumed_ &= std::uint16_t(~bit);
                long_done_ &= std::uint16_t(~bit);
            }
        }
    }
    engine.SetFill(frame.shift && ((frame.keys & (1u << 7u)) != 0));
    const bool clear = frame.shift && ((frame.keys & (1u << 12u)) != 0) && !engine.Running();
    if (clear)
    {
        if (clear_ms_ < 1000)
            ++clear_ms_;
        if (clear_ms_ == 1000 && !clear_done_)
        {
            engine.ClearTrack(edit_, selected_);
            clear_done_ = true;
        }
    }
    else
    {
        clear_ms_ = 0;
        clear_done_ = false;
    }
    if (preset_armed_ && ++preset_age_ >= 5000)
        preset_armed_ = false;

    auto params = engine.Voice(selected_);
    float *fields[4] = {&params.tune, &params.decay, &params.character, &params.level};
    bool changed = false;
    for (std::size_t i = 0; i < 8; ++i)
    {
        if (!pickup_[i].Update(frame.knobs[i]))
            continue;
        const float value = pickup_[i].Value();
        if (i < 4)
        {
            if (std::fabs(*fields[i] - value) > 0.002f)
            {
                *fields[i] = value;
                changed = true;
            }
        }
        else if (i == 4)
        {
            const double bpm = std::round((40.0 + 200.0 * double(value)) * 10.0) / 10.0;
            if (std::fabs(engine.Tempo() - bpm) >= 0.099)
                (void)engine.SetTempo(bpm);
        }
        else if (i == 5)
        {
            if (std::fabs(value - engine.Swing()) > 0.002f)
                (void)engine.SetSwing(value);
        }
        else if (i == 6)
        {
            if (std::fabs(value - engine.Drive()) > 0.002f)
                engine.SetDrive(value);
        }
        else if (i == 7)
        {
            if (std::fabs(value - engine.Master()) > 0.002f)
                engine.SetMaster(value);
        }
    }
    if (changed)
        engine.SetVoice(selected_, params);
    previous_ = frame;
}
UiSnapshot Controller::Snapshot(const Engine &engine) const noexcept
{
    UiSnapshot s{};
    const auto p = engine.Voice(selected_);
    s.params = {p.tune, p.decay, p.character, p.level};
    s.notes = engine.Notes(edit_, selected_);
    s.accents = engine.Accents(edit_, selected_);
    for (std::size_t i = 0; i < 8; ++i)
        if (!pickup_[i].Captured())
            s.pickup_mask |= std::uint16_t(1u << i);
    s.selected = selected_;
    s.step = engine.CurrentStep();
    s.edit_bank = static_cast<std::uint8_t>(edit_);
    s.play_bank = static_cast<std::uint8_t>(engine.PlayingBank());
    s.queued_bank = static_cast<std::uint8_t>(engine.QueuedBank());
    s.preset = engine.PresetId();
    s.pending_preset = pending_preset_;
    s.mutes = engine.Mutes();
    s.running = engine.Running();
    s.shift = previous_.shift;
    s.fill = engine.Fill();
    s.preset_armed = preset_armed_;
    s.loading = engine.Loading();
    s.bpm = float(engine.Tempo());
    s.swing = engine.Swing();
    s.drive = engine.Drive();
    s.master = engine.Master();
    s.faults = engine.Faults();
    return s;
}
} // namespace hydrapulse
