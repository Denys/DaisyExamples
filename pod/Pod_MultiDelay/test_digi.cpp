#include "DigiMono.h"
#include <cmath>
#include <cstdio>
#include <limits>
#include <vector>
using namespace poddemo;
int main() {
    long long checks = 0;
    int failures = 0;
    for (auto cfg : {Config::Single, Config::Series, Config::Parallel})
        for (float g : {0.0f, 0.5f}) {
            constexpr int length = 8192;
            std::vector<float> storage(2 * 4096 + 2, 12345);
            DigiMono d;
            d.Attach(storage.data() + 1, 4096);
            d.Reset();
            d.Set(101, 149, g, 1, 0, cfg, true);
            std::vector<double> oracle(length, 0);
            for (int a = 1; a * 101 < length; ++a) {
                if (cfg == Config::Single || cfg == Config::Parallel)
                    oracle[a * 101] += (cfg == Config::Parallel ? 0.5 : 1.0) * std::pow(g, a - 1);
                if (cfg == Config::Series)
                    for (int b = 1; a * 101 + b * 149 < length; ++b)
                        oracle[a * 101 + b * 149] += std::pow(g, a + b - 2);
            }
            if (cfg == Config::Parallel)
                for (int b = 1; b * 149 < length; ++b)
                    oracle[b * 149] += 0.5 * std::pow(g, b - 1);
            for (int n = 0; n < length; ++n) {
                const float y = d.Process(n == 0 ? 1.0f : 0.0f);
                ++checks;
                if (std::abs(y - oracle[n]) > 2e-6)
                    ++failures;
            }
            ++checks;
            if (storage.front() != 12345 || storage.back() != 12345)
                ++failures;
        }
    for (auto cfg : {Config::Single, Config::Series, Config::Parallel}) {
        std::vector<float> storage(2 * 4096);
        DigiMono d;
        d.Attach(storage.data(), 4096);
        d.Reset();
        d.Set(960.25f, 1440.75f, 0.9f, 0.8f, 0, cfg, true);
        float tailPeak = 0, peak = 0;
        for (int n = 0; n < 60 * 48000; ++n) {
            const float x = n < 4800 ? 0.01f * std::sin(n * 0.121f) : 0;
            const float y = d.Process(x);
            ++checks;
            if (!std::isfinite(y) || std::abs(y) > 1)
                ++failures;
            peak = std::max(peak, std::abs(y));
            if (n > 59 * 48000)
                tailPeak = std::max(tailPeak, std::abs(y));
        }
        ++checks;
        if (tailPeak > 1e-6f || peak == 0)
            ++failures;
        d.Set(std::numeric_limits<float>::infinity(), -5, std::numeric_limits<float>::quiet_NaN(),
              1, 0, cfg);
        for (int n = 0; n < 8192; ++n) {
            ++checks;
            if (!std::isfinite(d.Process(n == 0 ? std::numeric_limits<float>::infinity() : 0)))
                ++failures;
        }
    }
    std::printf("DIGI: %lld sample/guard checks, %d failures; 3 x 60-second host soaks\n", checks,
                failures);
    return failures ? 1 : 0;
}
