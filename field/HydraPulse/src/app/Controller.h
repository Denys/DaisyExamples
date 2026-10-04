#pragma once
#include "src/app/Engine.h"
#include "src/core/ControlPickup.h"
#include <array>
#include <cstdint>

namespace hydrapulse
{
// Tick at 1 kHz with debounced states. The adapter and engine share the audio owner.
class Controller
{
  public:
    void Init(Engine &engine, const InputFrame &frame) noexcept;
    void Tick(Engine &engine, const InputFrame &frame) noexcept;
    UiSnapshot Snapshot(const Engine &engine) const noexcept;
    bool SuppressMidi() const noexcept
    {
        return panic_latched_ || (previous_.play && previous_.shift);
    }

  private:
    void CaptureTargets(const Engine &engine, const InputFrame &frame, bool globals) noexcept;
    void ShiftPress(Engine &engine, std::size_t key, const InputFrame &frame) noexcept;
    std::array<core::ControlPickup, 8> pickup_{};
    std::array<std::uint16_t, 16> held_ms_{};
    InputFrame previous_{};
    std::uint32_t revision_{0};
    std::uint16_t consumed_{0}, long_done_{0}, panic_ms_{0}, clear_ms_{0}, preset_age_{0};
    std::uint8_t selected_{0}, pending_preset_{0};
    Bank edit_{Bank::A};
    bool initialized_{false}, play_armed_{false}, panic_latched_{false}, clear_done_{false},
        preset_armed_{false};
};
} // namespace hydrapulse
