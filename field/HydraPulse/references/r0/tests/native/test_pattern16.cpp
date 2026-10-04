#include "src/core/Pattern16.h"
#include "tests/native/test_support.h"

#include <cstdint>
#include <random>

int main()
{
    using hydrapulse::core::Pattern16;
    Pattern16<8> pattern;

    for(std::size_t track = 0; track < 8; ++track)
        for(std::size_t step = 0; step < 16; ++step)
            HPF_CHECK(!pattern.IsActive(track, step));

    HPF_CHECK(pattern.Set(3, 7, true));
    HPF_CHECK(pattern.IsActive(3, 7));
    HPF_CHECK(pattern.Toggle(3, 7));
    HPF_CHECK(!pattern.IsActive(3, 7));

    HPF_CHECK(!pattern.Set(8, 0, true));
    HPF_CHECK(!pattern.Set(0, 16, true));
    HPF_CHECK(!pattern.Toggle(99, 99));
    HPF_CHECK(!pattern.IsActive(99, 99));

    for(std::size_t step = 0; step < 16; ++step)
        HPF_CHECK(pattern.Set(2, step, true));
    HPF_CHECK_EQ(pattern.TrackMask(2), static_cast<std::uint16_t>(0xFFFFu));
    HPF_CHECK(pattern.ClearTrack(2));
    HPF_CHECK_EQ(pattern.TrackMask(2), static_cast<std::uint16_t>(0u));

    // Deterministic property stress: mirror the bit operations in a reference model.
    std::mt19937 rng(0x48504631u);
    std::uint16_t reference[8]{};
    for(int i = 0; i < 20'000; ++i)
    {
        const std::size_t track = rng() % 8u;
        const std::size_t step  = rng() % 16u;
        const bool active       = (rng() & 1u) != 0u;
        const auto bit = static_cast<std::uint16_t>(std::uint16_t{1u} << step);

        if((rng() & 1u) != 0u)
        {
            HPF_CHECK(pattern.Set(track, step, active));
            if(active)
                reference[track] = static_cast<std::uint16_t>(reference[track] | bit);
            else
                reference[track] = static_cast<std::uint16_t>(reference[track] & ~bit);
        }
        else
        {
            HPF_CHECK(pattern.Toggle(track, step));
            reference[track] = static_cast<std::uint16_t>(reference[track] ^ bit);
        }

        HPF_CHECK_EQ(pattern.TrackMask(track), reference[track]);
    }

    pattern.Clear();
    for(std::size_t track = 0; track < 8; ++track)
        HPF_CHECK_EQ(pattern.TrackMask(track), static_cast<std::uint16_t>(0u));

    return EXIT_SUCCESS;
}
