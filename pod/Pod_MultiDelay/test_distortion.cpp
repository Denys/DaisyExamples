#include "Demo.h"
#include <cstdio>
#include <limits>
#include <vector>
using namespace poddemo;
int main() {
    int checks = 0, failures = 0;
    auto check = [&](bool ok, const char *label) {
        ++checks;
        if (!ok) { ++failures; std::printf("FAIL %s\n", label); }
    };
    Distortion d;
    d.Init(.4f, .5f, false);
    bool clean = true;
    for (int i = 0; i < 10000; ++i) {
        float x = .3f * std::sin(i * .17f);
        clean &= d.Process(x) == x;
    }
    check(clean, "OFF is bit-exact clean at startup, no added latency");
    d.Configure(.4f, .5f, true);
    float first = d.Process(.1f);
    check(std::abs(first - .1f) < .005f, "enable begins with a bounded crossfade step");
    for (int i = 0; i < 2000; ++i) d.Process(.1f);
    check(d.Process(.1f) > .7f, "ON actually drives and clips the input");
    d.Configure(.4f, .5f, false);
    for (int i = 0; i < 242; ++i) d.Process(.1f);
    check(d.Process(.1f) == .1f, "disable settles to exact clean within 242 samples");
    bool finite = true;
    for (float bad : {std::numeric_limits<float>::quiet_NaN(),
                      std::numeric_limits<float>::infinity(),
                      -std::numeric_limits<float>::infinity(), 1e30f}) {
        d.Configure(bad, bad, true);
        for (int i = 0; i < 1000; ++i) {
            float y = d.Process(bad);
            finite &= std::isfinite(y) && std::abs(y) <= 1.000001f;
        }
    }
    check(finite, "nonfinite parameters/input and extremes stay finite and bounded");
    Distortion dark, bright;
    dark.Init(.7f, 0, true); bright.Init(.7f, 1, true);
    double ed = 0, eb = 0;
    for (int i = 0; i < 48000; ++i) {
        float x = .1f * std::sin(i * 6.28318530718f * 8000 / 48000);
        float a = dark.Process(x), b = bright.Process(x);
        if (i > 2400) { ed += a * a; eb += b * b; }
    }
    check(eb > 8 * ed, "Tone independently changes high-frequency energy");
    Distortion gentle, hard;
    gentle.Init(0, 1, true); hard.Init(1, 1, true);
    for (int i = 0; i < 10000; ++i) { gentle.Process(.05f); hard.Process(.05f); }
    check(hard.Process(.05f) > 10 * gentle.Process(.05f), "Drive has an independent audible effect");

    std::vector<float> history(2 * kHistory), freeze(2 * kFreeze);
    Demo demo;
    Snapshot s;
    s.bypass = true; s.trails = false; s.driveOn = true;
    demo.Init(history.data(), freeze.data(), s);
    float in[kBlock], l[kBlock], r[kBlock];
    for (auto &x : in) x = .1f;
    bool allModes = true;
    for (int mode = 0; mode < 5; ++mode) {
        s.mode = mode; ++s.epoch;
        for (int b = 0; b < 1000; ++b) {
            demo.Process(in, l, r, kBlock, s);
            demo.Service(s);
        }
        allModes &= l[kBlock - 1] > .7f && l[kBlock - 1] == r[kBlock - 1];
    }
    check(allModes, "PRE distortion survives delay bypass in all five modes");
    ++s.epoch; s.mode = 0;
    for (int b = 0; b < 6; ++b) demo.Process(in, l, r, kBlock, s);
    check(demo.Paused(), "mode change reaches pause with drive ON");
    demo.Process(in, l, r, kBlock, s);
    check(l[kBlock - 1] > .7f, "PRE distortion remains active during delay pause");
    demo.Service(s);
    s.driveOn = false;
    for (int b = 0; b < 1000; ++b) demo.Process(in, l, r, kBlock, s);
    check(std::abs(l[kBlock - 1] - .1f) < 1e-6f, "independent OFF restores clean delay bypass");

    // Independent clean-vs-driven demo runs: an impulse must enter the delay AFTER drive.
    auto echoEnergy = [&](bool enabled) {
        Demo echo;
        Snapshot t;
        t.driveOn = enabled; t.drive = .7f; t.tone = 1;
        t.slots[0] = DigiTimeSlot(20); t.slots[1] = 0; t.slots[2] = 1; t.slots[3] = 0;
        echo.Init(history.data(), freeze.data(), t);
        float x[kBlock]{}, a[kBlock], b[kBlock];
        for (int i = 0; i < 2000; ++i) echo.Process(x, a, b, kBlock, t);
        x[0] = .05f;
        echo.Process(x, a, b, kBlock, t); x[0] = 0;
        double energy = 0;
        for (int i = 1; i < 25; ++i) {
            echo.Process(x, a, b, kBlock, t);
            if (i >= 20) for (float v : a) energy += v * v;
        }
        return energy;
    };
    check(echoEnergy(true) > 20 * echoEnergy(false), "distorted input also feeds the delay repeats");
    std::printf("DISTORTION: %d checks, %d failures\n", checks, failures);
    return failures ? 1 : 0;
}
