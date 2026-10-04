#include "src/core/SampleAccurateStepClock.h"
#include "tests/native/test_support.h"

#include <cmath>
#include <cstdint>

int main()
{
    using hydrapulse::core::SampleAccurateStepClock;

    SampleAccurateStepClock clock(48'000u, 120.0, 4u);
    HPF_CHECK_EQ(clock.SamplesUntilNextStep(), std::uint64_t{6'000u});

    std::uint64_t samples = 0u;
    while (!clock.ProcessSample())
        ++samples;
    ++samples; // include the triggering sample
    HPF_CHECK_EQ(samples, std::uint64_t{6'000u});

    HPF_CHECK(clock.SetTempoBpm(123.4567));
    HPF_CHECK_NEAR(clock.TempoBpm(), 123.4567, 0.000001);

    constexpr std::uint64_t kEvents = 1024u;
    std::uint64_t event_count = 0u;
    std::uint64_t sample_index = 0u;

    const long double exact_interval = (48'000.0L * 60.0L) / (123.4567L * 4.0L);

    while (event_count < kEvents)
    {
        ++sample_index;
        if (clock.ProcessSample())
        {
            ++event_count;
            const long double ideal = exact_interval * static_cast<long double>(event_count);
            const long double error = std::fabs(static_cast<long double>(sample_index) - ideal);
            HPF_CHECK(error <= 1.0L);
        }
    }

    HPF_CHECK(!clock.SetTempoBpm(0.0));
    HPF_CHECK(!clock.SetTempoBpm(-120.0));
    HPF_CHECK(!clock.SetSampleRate(0u));

    return EXIT_SUCCESS;
}
