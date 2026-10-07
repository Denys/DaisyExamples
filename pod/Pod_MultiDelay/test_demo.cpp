#include "Demo.h"
#include <cmath>
#include <cstdio>
#include <limits>
#include <vector>
using namespace poddemo;
int main() {
    int failures = 0, checks = 0;
    auto check = [&](bool ok, const char *name) {
        ++checks;
        if (!ok) {
            ++failures;
            std::printf("FAIL %s\n", name);
        }
    };
    std::vector<float> history(2 * kHistory + 2, 12345), freeze(2 * kFreeze + 2, 12345);
    Demo demo;
    Snapshot s;
    demo.Init(history.data() + 1, freeze.data() + 1, s);
    float in[48]{}, left[48]{}, right[48]{};
    for (int mode = 0; mode < 5; ++mode) {
        s.mode = mode;
        s.epoch++;
        s.slots[2] = 1;
        s.command = mode == 4 ? 1 : 0;
        double energy = 0;
        for (int block = 0; block < 2400; ++block) {
            for (int i = 0; i < 48; ++i)
                in[i] =
                    block < 1100 ? 0.05f * std::sin(static_cast<float>(block * 48 + i) * 0.04f) : 0;
            demo.Process(in, left, right, 48, s);
            demo.Service(s);
            for (int i = 0; i < 48; ++i) {
                if (!std::isfinite(left[i]) || left[i] != right[i])
                    ++failures;
                if (block > 1100 && block < 2200)
                    energy += left[i] * left[i];
            }
        }
        check(energy > 0.001, "mode emits audio");
    }
    s.mode = 0;
    s.epoch++;
    s.command = 0;
    s.slots[2] = 0.35f;
    for (int n = 0; n < 30; ++n) {
        demo.Process(in, left, right, 48, s);
        demo.Service(s);
    }
    s.bypass = true;
    s.trails = false;
    for (int n = 0; n < 300; ++n) {
        for (auto &x : in)
            x = 0.1f;
        demo.Process(in, left, right, 48, s);
        demo.Service(s);
    }
    check(std::abs(left[47] - 0.1f) < 1e-5f, "bypass reaches unity dry");
    s.epoch++;
    s.mode = 1;
    for (int n = 0; n < 6; ++n)
        demo.Process(in, left, right, 48, s);
    check(demo.Paused(), "disruptive edit requires acknowledged pause");
    demo.Process(in, left, right, 48, s);
    check(std::abs(left[20] - 0.1f) < 1e-6f, "paused path is dry");
    check(demo.Service(s), "main services only acknowledged pause");
    check(!demo.Service(s), "main cannot mutate running engine");
    in[0] = std::numeric_limits<float>::quiet_NaN();
    in[1] = std::numeric_limits<float>::infinity();
    demo.Process(in, left, right, 48, s);
    check(std::isfinite(left[0]) && std::isfinite(left[1]), "nonfinite input contained");
    check(history.front() == 12345 && history.back() == 12345 && freeze.front() == 12345 &&
              freeze.back() == 12345,
          "storage canaries preserved");

    s.mode = 4;
    s.command = 1;
    s.epoch++;
    s.bypass = false;
    s.slots[0] = 0.2f;
    s.slots[2] = 1;
    for (int b = 0; b < 1500; ++b) {
        for (int i = 0; i < 48; ++i)
            in[i] = 0.03f * std::sin((b * 48 + i) * 0.03f);
        demo.Process(in, left, right, 48, s);
        demo.Service(s);
    }
    check(demo.freezeStatus.load() == 2, "capture automatically enters hold");
    s.command = 3;
    s.epoch++;
    for (int b = 0; b < 20; ++b) {
        demo.Process(in, left, right, 48, s);
        demo.Service(s);
    }
    check(demo.freezeStatus.load() == 3, "accumulate state is reachable");
    s.command = 4;
    s.epoch++;
    for (int b = 0; b < 1500; ++b) {
        demo.Process(in, left, right, 48, s);
        demo.Service(s);
    }
    check(demo.freezeStatus.load() == 2, "replace recaptures then holds");
    s.command = 5;
    s.epoch++;
    for (int b = 0; b < 300; ++b) {
        for (auto &x : in)
            x = 0;
        demo.Process(in, left, right, 48, s);
        demo.Service(s);
    }
    check(demo.freezeStatus.load() == 0 && std::abs(left[47]) < 1e-6f,
          "clear removes captured loop");
    s.command = 0;
    s.epoch++;
    s.mode = 99;
    s.config = -99;
    s.slots[0] = std::numeric_limits<float>::quiet_NaN();
    for (int b = 0; b < 20; ++b) {
        demo.Process(in, left, right, 48, s);
        demo.Service(s);
    }
    check(std::isfinite(left[47]), "malformed snapshot remains finite");
    std::printf("DEMO: %d named checks, %d failures\n", checks, failures);
    return failures ? 1 : 0;
}
