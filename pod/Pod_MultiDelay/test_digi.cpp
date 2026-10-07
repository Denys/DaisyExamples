// DIGI contract v1 oracles (contract sections 4-7 and 10).
#include "DigiMono.h"
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <vector>
using namespace poddemo;
namespace {
int failures = 0;
long long checks = 0;
void Check(bool ok, const char *name) {
    ++checks;
    if (!ok) {
        ++failures;
        std::printf("FAIL %s\n", name);
    }
}
DigiParams Raw(float d1, float d2, float f1, float f2, Config c, bool filters) {
    DigiParams p;
    p.delay[0] = d1;
    p.delay[1] = d2;
    p.feedback[0] = f1;
    p.feedback[1] = f2;
    p.lowCutHz = filters ? 40 : 0;
    p.highCutHz = filters ? 6000 : kGuardHz;
    p.config = c;
    return p;
}
struct Rng { // deterministic, platform-independent
    uint32_t s = 0x1234567u;
    float Next() {
        s = s * 1664525u + 1013904223u;
        return static_cast<float>(s >> 8) / 8388608.0f - 1.0f;
    }
};
} // namespace

int main() {
    // 10. Impulse markers: D1 19200, r 3/4, f 0.5 on both engines, filters off, Dry 0, Wet 1.
    const struct {
        Config c;
        int n[7];
        double a[7];
    } markers[] = {
        {Config::Single,
         {19200, 38400, 57600, 76800, 96000, 115200, 134400},
         {1, 0.5, 0.25, 0.125, 0.0625, 0.03125, 0.015625}},
        {Config::Series,
         {19200, 33600, 38400, 48000, 52800, 57600, 62400},
         {1, 1, 0.5, 0.5, 0.5, 0.25, 0.25}},
        {Config::Parallel,
         {14400, 19200, 28800, 38400, 43200, 57600, 72000},
         {0.5, 0.5, 0.25, 0.25, 0.125, 0.1875, 0.03125}},
    };
    for (const auto &m : markers) {
        std::vector<float> storage(2 * 140000 + 2, 12345);
        DigiMono d;
        d.Attach(storage.data() + 1, 140000);
        d.Reset();
        d.Set(Raw(19200, 14400, 0.5f, 0.5f, m.c, false));
        int found = 0;
        bool exact = true;
        for (int n = 0; n <= m.n[6]; ++n) {
            const float y = d.Process(n == 0 ? 1.0f : 0.0f);
            if (y == 0)
                continue;
            if (found < 7 && (n != m.n[found] || std::abs(y - m.a[found]) > 1e-6))
                exact = false;
            ++found;
        }
        Check(exact && found == 7, "contract 10 impulse markers");
        Check(storage.front() == 12345 && storage.back() == 12345, "storage canaries");
    }

    // 6. Control law: SERIES compensation f = 1 - sqrt(1 - k), k = 0.95 * FEEDBACK.
    for (float fb : {0.0f, 0.5f, 1.0f}) {
        const float k = 0.95f * fb;
        const auto s = DigiMap(0.5f, fb, -1, 0, kDigiDefaultRatio, Config::Series);
        const auto p = DigiMap(0.5f, fb, -1, 0, kDigiDefaultRatio, Config::Parallel);
        Check(std::abs(s.feedback[0] - (1 - std::sqrt(1 - k))) < 1e-7f && s.feedback[0] == s.feedback[1],
              "series compensation");
        Check(p.feedback[0] == k && p.feedback[1] == k, "parallel and single use f = k");
    }
    {
        const auto lo = DigiMap(0, 0, -1, 0, 6, Config::Single);
        const auto hi = DigiMap(1, 0, 0.2f, 1, 0, Config::Single);
        Check(lo.delay[0] == 960 && lo.delay[1] == 960, "TIME minimum 20 ms, ratio 1");
        Check(hi.delay[0] == 120000 && hi.delay[1] == 30000, "TIME maximum 2.5 s, ratio 1/4");
        Check(lo.highCutHz == kGuardHz && std::abs(hi.highCutHz - 2000) < 0.5f, "COLOR off .. 2 kHz");
        Check(std::abs(hi.feedback[1] - 0.95f * 0.2f) < 1e-7f && hi.feedback[0] == 0, "E2 unlinked");
        Check(std::abs(DigiTimeMs(DigiTimeSlot(400)) - 400) < 0.01f, "tap mapping inverts TIME");
    }

    // 6. Build-up at the loop resonance (T1 = T2 = 480, -20 dBFS sine at 100 Hz, 6 s, filters off).
    {
        auto peak = [](Config c, float f) {
            std::vector<float> storage(2 * 1024);
            DigiMono d;
            d.Attach(storage.data(), 1024);
            d.Reset();
            d.Set(Raw(480, 480, f, f, c, false));
            float p = 0;
            for (int n = 0; n < 6 * 48000; ++n) {
                const float y = d.Process(0.1f * static_cast<float>(std::sin(6.283185307179586 * 100 * n / 48000.0)));
                if (n > 5 * 48000)
                    p = std::max(p, std::abs(y));
            }
            return p;
        };
        const float fc = 1 - std::sqrt(1 - 0.95f);
        const float single = peak(Config::Single, 0.95f), compensated = peak(Config::Series, fc),
                    uncompensated = peak(Config::Series, 0.95f);
        // DERIVED: tap1 + tap2 = 0.1 * (H1 + H1*H2) at resonance.
        const float hc = 1 / (1 - fc);
        Check(std::abs(single - 2.0f) < 0.06f, "single build-up 20x");
        Check(std::abs(compensated - 0.1f * (hc + hc * hc)) < 0.08f, "compensated series ~ single");
        Check(std::abs(uncompensated - 42.0f) < 1.3f, "uncompensated series 400x tail");
        std::printf("BUILD-UP peaks: single %.3f, series compensated %.3f, uncompensated %.3f\n",
                    single, compensated, uncompensated);
    }

    // 7. A config change applies at the next Set and keeps the histories.
    {
        std::vector<float> storage(2 * 40000);
        DigiMono d;
        d.Attach(storage.data(), 40000);
        d.Reset();
        d.Set(Raw(19200, 14400, 0.5f, 0.5f, Config::Single, false));
        float y = 0;
        for (int n = 0; n <= 38400; ++n) {
            if (n == 20000)
                d.Set(Raw(19200, 14400, 0.5f, 0.5f, Config::Parallel, false));
            y = d.Process(n == 0 ? 1.0f : 0.0f);
        }
        Check(std::abs(y - 0.25f) < 1e-7f, "tail survives SINGLE -> PARALLEL");
    }

    // 10. 60-second soak: 50 s seeded noise at -20 dBFS RMS, 10 s silence; k in {0, 0.5, 0.95},
    // COLOR at both ends, every config, contract control law and default TIME/ratio.
    for (auto c : {Config::Single, Config::Series, Config::Parallel})
        for (float fb : {0.0f, 0.5f / 0.95f, 1.0f})
            for (float color : {0.0f, 1.0f}) {
                std::vector<float> storage(2 * 120008);
                DigiMono d;
                d.Attach(storage.data(), 120008);
                d.Reset();
                d.Set(DigiMap(DigiTimeSlot(400), fb, -1, color, kDigiDefaultRatio, c));
                Rng rng;
                bool finite = true;
                double first = 0, last = 0;
                float pk = 0;
                long clipped = 0;
                for (int n = 0; n < 60 * 48000; ++n) {
                    const float x = n < 50 * 48000 ? 0.1f * 1.7320508f * rng.Next() : 0;
                    const float y = x + d.Process(x); // Dry 1, Wet 1
                    finite = finite && std::isfinite(y);
                    pk = std::max(pk, std::abs(y));
                    clipped += std::abs(y) > 1;
                    if (n >= 50 * 48000 && n < 51 * 48000)
                        first += double(y) * y;
                    if (n >= 59 * 48000)
                        last += double(y) * y;
                }
                Check(finite, "soak output finite");
                Check(last < first, "soak tail decays");
                std::printf("SOAK cfg %d k %.2f color %.0f: peak %.3f clipped %ld\n",
                            static_cast<int>(c), 0.95f * fb, color, pk, clipped);
            }

    // Malformed parameters and input stay finite.
    for (auto c : {Config::Single, Config::Series, Config::Parallel}) {
        std::vector<float> storage(2 * 4096);
        DigiMono d;
        d.Attach(storage.data(), 4096);
        d.Reset();
        const float inf = std::numeric_limits<float>::infinity(),
                    nan = std::numeric_limits<float>::quiet_NaN();
        DigiParams p = Raw(inf, -5, nan, 4, c, true);
        p.highCutHz = nan;
        d.Set(p);
        bool ok = true;
        for (int n = 0; n < 8192; ++n)
            ok = ok && std::isfinite(d.Process(n == 0 ? inf : n == 1 ? nan : 0));
        Check(ok, "nonfinite parameters and input contained");
        Check(std::isfinite(DigiMap(nan, nan, nan, nan, 99, c).delay[1]), "nonfinite controls mapped");
    }
    std::printf("DIGI: %lld contract checks, %d failures\n", checks, failures);
    return failures ? 1 : 0;
}
