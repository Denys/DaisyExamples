#include "src/app/Engine.h"
#include "src/dsp/Voices.h"
#include "tests/native/test_support.h"
#include <algorithm>
#include <cmath>
#include <limits>
int main()
{
    using namespace hydrapulse;
    dsp::SineTable table;
    table.Init();
    for (unsigned id = 0; id < 4; ++id)
    {
        for (unsigned corner = 0; corner < 8; ++corner)
        {
            dsp::PercussionVoice v;
            v.Init(static_cast<dsp::VoiceId>(id), 48000, table);
            v.SetParameters({float(corner & 1u), float((corner >> 1u) & 1u), float((corner >> 2u) & 1u), 1},
                            true);
            v.Trigger(1);
            float peak = 0, tail = 0;
            for (int n = 0; n < 48000 * 6; ++n)
            {
                const float s = v.Process();
                HPF_CHECK(std::isfinite(s));
                HPF_CHECK(std::fabs(s) <= 1.00001f);
                peak = std::max(peak, std::fabs(s));
                if (n > 48000 * 5)
                    tail = std::max(tail, std::fabs(s));
            }
            HPF_CHECK(peak > 0.0001f);
            HPF_CHECK(tail < 1e-5f);
        }
        dsp::PercussionVoice v;
        v.Init(static_cast<dsp::VoiceId>(id), 48000, table);
        v.Trigger(1);
        for (int n = 0; n < 500; ++n)
            (void)v.Process();
        v.Release();
        for (int n = 0; n < 300; ++n)
            (void)v.Process();
        HPF_CHECK_EQ(v.Process(), 0.0f);
    }
    for (unsigned preset = 0; preset < 8; ++preset)
    {
        Engine a, b;
        a.Init();
        b.Init();
        a.RequestPreset(static_cast<std::uint8_t>(preset));
        b.RequestPreset(static_cast<std::uint8_t>(preset));
        for (int n = 0; n < 600; ++n)
        {
            (void)a.Process();
            (void)b.Process();
        }
        a.Start();
        b.Start();
        for (int n = 0; n < 48000 * 3; ++n)
        {
            if (n % 1103 == 0)
            {
                a.Trigger(unsigned(n) % 4u);
                b.Trigger(unsigned(n) % 4u);
            }
            auto x = a.Process(), y = b.Process();
            HPF_CHECK(std::isfinite(x.left) && std::isfinite(x.right));
            HPF_CHECK(std::fabs(x.left) < 0.737f && std::fabs(x.right) < 0.737f);
            HPF_CHECK_EQ(x.left, y.left);
            HPF_CHECK_EQ(x.right, y.right);
        }
        HPF_CHECK_EQ(a.Faults(), 0u);
    }
    dsp::Ramp ramp;
    ramp.Reset(1);
    ramp.Set(0, 240);
    float previous = 1;
    for (int n = 0; n < 240; ++n)
    {
        const float x = ramp.Process();
        HPF_CHECK(std::fabs(x - previous) <= 1.0f / 240.0f + 1e-6f);
        previous = x;
    }
    HPF_CHECK_EQ(ramp.Value(), 0.0f);
    return EXIT_SUCCESS;
}
