#include "src/core/StepSequencer.h"
#include "tests/native/test_support.h"

#include <array>
#include <cstddef>
#include <cstdint>

int main()
{
    using Pattern   = hydrapulse::core::Pattern16<4>;
    using Sequencer = hydrapulse::core::StepSequencer<4>;

    Pattern pattern;
    HPF_CHECK(pattern.Set(0, 0, true));
    HPF_CHECK(pattern.Set(2, 0, true));
    HPF_CHECK(pattern.Set(1, 15, true));

    Sequencer sequencer(pattern);
    HPF_CHECK(!sequencer.EmitAndAdvance().emitted);

    sequencer.Start();
    HPF_CHECK(sequencer.Running());

    auto first = sequencer.EmitAndAdvance();
    HPF_CHECK(first.emitted);
    HPF_CHECK_EQ(static_cast<int>(first.step), 0);
    HPF_CHECK_EQ(first.track_mask, std::uint32_t{0b0101u});

    std::array<std::size_t, 4> order{};
    std::size_t count = 0;
    Sequencer::ForEachTriggeredTrack(first, [&](std::size_t track) { order[count++] = track; });
    HPF_CHECK_EQ(count, std::size_t{2});
    HPF_CHECK_EQ(order[0], std::size_t{0});
    HPF_CHECK_EQ(order[1], std::size_t{2});

    // Steps 1..15; verify final step and wrap to zero.
    Sequencer::Tick tick{};
    for(int expected = 1; expected <= 15; ++expected)
    {
        tick = sequencer.EmitAndAdvance();
        HPF_CHECK_EQ(static_cast<int>(tick.step), expected);
    }
    HPF_CHECK_EQ(tick.track_mask, std::uint32_t{0b0010u});

    auto wrapped = sequencer.EmitAndAdvance();
    HPF_CHECK_EQ(static_cast<int>(wrapped.step), 0);
    HPF_CHECK_EQ(wrapped.track_mask, std::uint32_t{0b0101u});

    sequencer.Stop();
    HPF_CHECK(!sequencer.Running());
    HPF_CHECK(!sequencer.EmitAndAdvance().emitted);

    // Start is intentionally deterministic and restarts at step zero.
    sequencer.Start();
    HPF_CHECK_EQ(static_cast<int>(sequencer.EmitAndAdvance().step), 0);

    return EXIT_SUCCESS;
}
