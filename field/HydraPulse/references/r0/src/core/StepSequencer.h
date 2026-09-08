#pragma once

#include "src/core/Pattern16.h"

#include <cstddef>
#include <cstdint>
#include <utility>

namespace hydrapulse::core
{

template <std::size_t Tracks>
class StepSequencer
{
  public:
    using Pattern = Pattern16<Tracks>;

    struct Tick
    {
        bool          emitted{false};
        std::uint8_t  step{0};
        std::uint32_t track_mask{0};
    };

    explicit StepSequencer(const Pattern& pattern) noexcept : pattern_(pattern) {}

    void Start() noexcept
    {
        running_      = true;
        current_step_ = 0u;
    }

    void Stop() noexcept { running_ = false; }

    [[nodiscard]] bool Running() const noexcept { return running_; }

    [[nodiscard]] std::uint8_t CurrentStep() const noexcept { return current_step_; }

    [[nodiscard]] Tick EmitAndAdvance() noexcept
    {
        if(!running_)
            return {};

        Tick result;
        result.emitted    = true;
        result.step       = current_step_;
        result.track_mask = ActiveTrackMask(current_step_);

        current_step_ = static_cast<std::uint8_t>((current_step_ + 1u) % Pattern::kSteps);
        return result;
    }

    template <typename Fn>
    static void ForEachTriggeredTrack(const Tick& tick, Fn&& fn)
    {
        if(!tick.emitted)
            return;

        for(std::size_t track = 0; track < Tracks; ++track)
        {
            const auto bit = std::uint32_t{1u} << track;
            if((tick.track_mask & bit) != 0u)
                std::forward<Fn>(fn)(track);
        }
    }

  private:
    [[nodiscard]] std::uint32_t ActiveTrackMask(std::uint8_t step) const noexcept
    {
        std::uint32_t mask = 0u;
        for(std::size_t track = 0; track < Tracks; ++track)
        {
            if(pattern_.IsActive(track, step))
                mask |= std::uint32_t{1u} << track;
        }
        return mask;
    }

    const Pattern& pattern_;
    bool           running_{false};
    std::uint8_t   current_step_{0};
};

} // namespace hydrapulse::core
