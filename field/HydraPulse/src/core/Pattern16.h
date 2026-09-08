#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace hydrapulse::core
{

template <std::size_t Tracks> class Pattern16
{
  public:
    static_assert(Tracks > 0, "Pattern16 requires at least one track");
    static_assert(Tracks <= 32, "Pattern16 track masks are limited to 32 tracks");

    static constexpr std::size_t kSteps = 16;

    bool Set(std::size_t track, std::size_t step, bool active) noexcept
    {
        if (!Valid(track, step))
            return false;

        const auto bit = static_cast<std::uint16_t>(std::uint16_t{1u} << step);
        if (active)
            masks_[track] = static_cast<std::uint16_t>(masks_[track] | bit);
        else
            masks_[track] = static_cast<std::uint16_t>(masks_[track] & ~bit);
        return true;
    }

    bool Toggle(std::size_t track, std::size_t step) noexcept
    {
        if (!Valid(track, step))
            return false;

        const auto bit = static_cast<std::uint16_t>(std::uint16_t{1u} << step);
        masks_[track] = static_cast<std::uint16_t>(masks_[track] ^ bit);
        return true;
    }

    [[nodiscard]] bool IsActive(std::size_t track, std::size_t step) const noexcept
    {
        if (!Valid(track, step))
            return false;

        const auto bit = static_cast<std::uint16_t>(std::uint16_t{1u} << step);
        return (masks_[track] & bit) != 0u;
    }

    bool ClearTrack(std::size_t track) noexcept
    {
        if (track >= Tracks)
            return false;
        masks_[track] = 0u;
        return true;
    }

    void Clear() noexcept
    {
        masks_.fill(0u);
    }

    [[nodiscard]] std::uint16_t TrackMask(std::size_t track) const noexcept
    {
        return track < Tracks ? masks_[track] : 0u;
    }

  private:
    [[nodiscard]] static constexpr bool Valid(std::size_t track, std::size_t step) noexcept
    {
        return track < Tracks && step < kSteps;
    }

    std::array<std::uint16_t, Tracks> masks_{};
};

} // namespace hydrapulse::core
