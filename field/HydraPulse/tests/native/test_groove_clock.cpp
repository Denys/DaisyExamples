#include "src/core/GrooveClock.h"
#include "tests/native/test_support.h"
#include <cmath>
#include <cstdint>
int main()
{
    using hydrapulse::core::GrooveClock;
    for (const auto rate : {44100u, 48000u, 96000u})
    {
        for (const float swing : {0.0f, 0.5f, 1.0f})
        {
            GrooveClock c;
            c.Init(rate);
            HPF_CHECK(c.SetTempo(123.4567));
            HPF_CHECK(c.SetSwing(swing));
            c.Reset();
            std::uint64_t n = 0, event = 0;
            while (event < 512)
            {
                ++n;
                HPF_CHECK(c.SetTempo(123.4567)); // emulate frequent control updates
                if (!c.ProcessSample())
                    continue;
                ++event;
                const long double intervals = static_cast<long double>(event) +
                                              ((event & 1u) ? static_cast<long double>(swing) * 0.4L : 0.0L);
                const long double expected =
                    static_cast<long double>(rate) * 60.0L * intervals / (123.4567L * 4.0L);
                HPF_CHECK(std::fabs(static_cast<long double>(n) - expected) < 1.00001L);
            }
        }
    }
    GrooveClock c;
    HPF_CHECK(!c.SetTempo(0));
    HPF_CHECK(!c.SetSwing(-1));
    return EXIT_SUCCESS;
}
