// Bit-exact parity of DigiMono against DAFX DigitalDelayNode @ 73976da (contract 3: per-engine law
// unchanged from main). E1/E2 are two nodes; routing follows contract 4. verify.ps1 extracts the
// pinned headers into build/dafx with `git show 73976da:src/pedal_harness/...`.
#include "DigiMono.h"
#include "pedal_harness/digital_delay_node.hpp"
#include <cstdint>
#include <cstdio>
#include <vector>
using namespace poddemo;
namespace
{
struct Node
{
    std::vector<unsigned char> arena;
    phh::StaticArena           a;
    phh::DigitalDelayNode      node;
    phh::DiagnosticsCounters   diag;
    explicit Node(std::size_t maxDelay)
    : arena(2 * (maxDelay + 1) * sizeof(float) + 64),
      a(arena.data(), arena.size()),
      node(maxDelay)
    {
        phh::PrepareSpec spec;
        spec.max_block_frames = 48;
        ok                    = node.Prepare(spec, a);
    }
    bool ok = false;
    void
    Run(const float *in, float *out, std::size_t n, const DigiParams &p, int e)
    {
        const float values[]
            = {p.delay[e], p.feedback[e], 1, 0, 1, p.lowCutHz, p.highCutHz};
        phh::ParameterSnapshot s{values, 7, 0};
        std::vector<float>     dummy(n);
        phh::AudioBlock        b;
        b.in[0]  = in;
        b.in[1]  = in;
        b.out[0] = out;
        b.out[1] = dummy.data();
        b.frames = n;
        node.Process(b, s, diag);
    }
};
} // namespace
int main()
{
    constexpr std::size_t kMax     = 120000;
    int                   failures = 0;
    long long             samples  = 0;
    uint32_t              seed     = 99;
    for(auto c : {Config::Single, Config::Series, Config::Parallel})
    {
        std::vector<float> storage(2 * (kMax + 1));
        DigiMono           d;
        d.Attach(storage.data(), kMax + 1);
        d.Reset();
        Node e1(kMax), e2(kMax);
        if(!e1.ok || !e2.ok)
        {
            std::puts("FAIL node prepare");
            return 1;
        }
        float x[48], t1[48], t2[48], zero[48]{};
        for(int block = 0; block < 10 * 1000; ++block)
        { // 10 s with a parameter change per 0.25 s
            const int        step = block / 250;
            const DigiParams p    = DigiMap(0.05f * (step % 20),
                                         0.1f * (step % 11),
                                         step % 3 ? -1 : 0.7f,
                                         0.1f * (step % 11),
                                         step % 7,
                                         c);
            d.Set(p);
            for(auto &v : x)
            {
                seed = seed * 1664525u + 1013904223u;
                v    = block % 500 < 300
                        ? static_cast<float>(seed >> 8) / 8388608.0f - 1.0f
                        : 0;
            }
            e1.Run(x, t1, 48, p, 0);
            e2.Run(c == Config::Single ? zero : c == Config::Series ? t1 : x,
                   t2,
                   48,
                   p,
                   1);
            for(int i = 0; i < 48; ++i)
            {
                const float ref = c == Config::Single
                                      ? t1[i]
                                      : c == Config::Series
                                            ? t1[i] + t2[i]
                                            : 0.5f * (t1[i] + t2[i]);
                failures += d.Process(x[i]) != ref;
                ++samples;
            }
        }
    }
    std::printf(
        "PARITY vs DigitalDelayNode@73976da: %lld samples, %d mismatches "
        "(bit-exact)\n",
        samples,
        failures);
    return failures ? 1 : 0;
}
