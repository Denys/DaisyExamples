#include "Demo.h"
#include <cstdio>
#include <limits>
#include <vector>
using namespace poddemo;
namespace
{
constexpr float kPi2 = 6.28318530718f;
DriveParams     P(int type, float gain, bool on = true)
{
    DriveParams p;
    p.type = type;
    p.gain = gain;
    p.on   = on;
    return p;
}
double Rms(const std::vector<float> &v, std::size_t from)
{
    double e = 0;
    for(std::size_t i = from; i < v.size(); ++i)
        e += double(v[i]) * v[i];
    return std::sqrt(e / double(v.size() - from));
}
std::vector<float> Run(Drive &d, float amp, float hz, int n)
{
    std::vector<float> y(n);
    for(int i = 0; i < n; ++i)
        y[i] = d.Process(amp * std::sin(kPi2 * hz * i / kFs));
    return y;
}
// Alias-to-harmonic power ratio (dB) of a clipper driven by a bin-centred sine.
double AliasDb(int type, double gain, bool adaa)
{
    constexpr int N = 1024, bin = 37;
    Adaa          a;
    a.Reset(type);
    std::vector<double> y(N);
    for(int i = -256; i < N; ++i)
    {
        const double u = gain * .5 * std::sin(6.283185307179586 * bin * i / N);
        const double v = adaa ? a.Process(u) : a.Naive(u);
        if(i >= 0)
            y[i] = v;
    }
    double harm = 0, alias = 0;
    for(int k = 1; k < N / 2; ++k)
    {
        double re = 0, im = 0;
        for(int i = 0; i < N; ++i)
        {
            re += y[i] * std::cos(6.283185307179586 * k * i / N);
            im -= y[i] * std::sin(6.283185307179586 * k * i / N);
        }
        const double pw = re * re + im * im;
        bool         h  = false;
        for(int m = 1; m * bin < N / 2; ++m)
            h |= k == m * bin;
        (h ? harm : alias) += pw;
    }
    return 10 * std::log10(alias / harm);
}
} // namespace
int main()
{
    int  checks = 0, failures = 0;
    auto check = [&](bool ok, const char *label) {
        ++checks;
        if(!ok)
        {
            ++failures;
            std::printf("FAIL %s\n", label);
        }
    };
    Drive d;
    d.Init(P(1, .4f, false));
    bool clean = true;
    for(int i = 0; i < 10000; ++i)
    {
        float x = .3f * std::sin(i * .17f);
        clean &= d.Process(x) == x;
    }
    check(clean, "OFF is bit-exact clean at startup, no added latency");
    d.Configure(P(1, .4f, true));
    float first = d.Process(.1f);
    check(std::abs(first - .1f) < .01f,
          "enable begins with a bounded crossfade step");
    d.Configure(P(1, .4f, false));
    for(int i = 0; i < 242; ++i)
        d.Process(.1f);
    check(d.Process(.1f) == .1f,
          "disable settles to exact clean within 242 samples");

    // Harder: every type is louder than v0.4 DIST at its own maximum on a quiet note.
    check(DriveGainDb(1, 1) >= 50 && DriveGainDb(2, 1) >= 66
              && DriveGainDb(0, 1) >= 29.5f,
          "max gain: OD >= v0.4 (+29.5 dB), DIST +50 dB, FUZZ +66 dB");
    for(int t = 0; t < kDriveTypes; ++t)
    {
        Drive lo, hi;
        lo.Init(P(t, 0));
        hi.Init(P(t, 1));
        auto a = Run(lo, .01f, 440, 24000), b = Run(hi, .01f, 440, 24000);
        check(Rms(b, 4800) > 4 * Rms(a, 4800),
              "GAIN has an independent effect");
    }
    {
        Drive dist;
        dist.Init(P(1, 1));
        auto y = Run(dist, .003f, 440, 24000);
        check(Rms(y, 4800) > .3,
              "DIST max saturates a -50 dBFS note (v0.4 could not)");
    }
    bool finite = true;
    for(int t = 0; t < kDriveTypes; ++t)
        for(float bad : {std::numeric_limits<float>::quiet_NaN(),
                         std::numeric_limits<float>::infinity(),
                         -std::numeric_limits<float>::infinity(),
                         1e30f})
        {
            Drive       x;
            DriveParams p = P(t, bad);
            p.tone = p.level = p.blend = bad;
            x.Init(p);
            for(int i = 0; i < 1000; ++i)
            {
                float y = x.Process(bad);
                finite &= std::isfinite(y) && std::abs(y) <= 1.000001f;
            }
        }
    check(finite,
          "nonfinite parameters/input and extremes stay finite and bounded");
    {
        DriveParams dk = P(1, .7f), br = P(1, .7f);
        dk.tone = 0;
        br.tone = 1;
        Drive dark, bright;
        dark.Init(dk);
        bright.Init(br);
        auto a = Run(dark, .1f, 8000, 48000), b = Run(bright, .1f, 8000, 48000);
        check(Rms(b, 2400) > 2.8 * Rms(a, 2400),
              "TONE changes high-frequency energy");
    }
    {
        DriveParams q = P(0, 0), r = P(0, 0);
        q.level = 0;   // -24 dB
        r.level = .8f; // 0 dB
        Drive a, b;
        a.Init(q);
        b.Init(r);
        auto ya = Run(a, .05f, 1000, 24000), yb = Run(b, .05f, 1000, 24000);
        const double db = 20 * std::log10(Rms(yb, 4800) / Rms(ya, 4800));
        check(db > 23 && db < 25, "LEVEL spans 24 dB between 0 and 80%");
    }
    {
        DriveParams p = P(2, 1);
        p.blend       = 0;
        Drive b;
        b.Init(p);
        float worst = 0;
        for(int i = 0; i < 48000; ++i)
        {
            const float x = .2f * std::sin(kPi2 * 220 * i / kFs);
            const float y = b.Process(x);
            if(i > 24000)
                worst = std::max(worst, std::abs(y - x));
        }
        check(worst < 1e-4f, "BLEND 0 returns the clean signal");
    }
    {
        Drive fz;
        fz.Init(P(2, 1));
        std::vector<float> y(48000);
        unsigned           seed  = 1;
        auto               noise = [&] {
            seed = seed * 1664525u + 1013904223u;
            return (seed >> 8) / 8388608.0f - 1;
        };
        for(auto &v : y)
            v = fz.Process(3e-4f * noise());
        const double quiet = Rms(y, 24000);
        for(auto &v : y)
            v = fz.Process(.01f * noise());
        const double loud = Rms(y, 24000);
        check(quiet < .01 && loud > .1,
              "FUZZ gate closes on -70 dBFS hiss, opens at -40 dBFS");
        double mean = 0;
        auto   s    = Run(fz, .3f, 110, 48000);
        for(std::size_t i = 24000; i < s.size(); ++i)
            mean += s[i];
        check(std::abs(mean / 24000) < .01, "FUZZ asymmetry leaves no DC");
    }
    {
        // Type change: mute to the exact clean signal, then switch, then fade in.
        Drive sw;
        sw.Init(P(0, .5f));
        Run(sw, .1f, 100, 4800);
        DriveParams p = P(2, .5f);
        sw.Configure(p);
        bool sawClean = false, early = false;
        for(int i = 0; i < 400; ++i)
        {
            const float x = .1f * std::sin(kPi2 * 100 * i / kFs);
            const float y = sw.Process(x);
            sawClean |= y == x; // the switching sample itself is clean
            if(sw.Type() == 2 && !sawClean)
                early = true;
            if(i % 48 == 47)
                sw.Configure(p);
        }
        check(sawClean && !early && sw.Type() == 2 && !sw.Switching(),
              "type change passes through exact clean before the new type");
    }
    {
        const double dn = AliasDb(1, 316, false), da = AliasDb(1, 316, true);
        const double fn = AliasDb(2, 2000, false), fa = AliasDb(2, 2000, true);
        const double on = AliasDb(0, 31.6, false), oa = AliasDb(0, 31.6, true);
        std::printf(
            "ALIAS dB (naive -> ADAA): OD %.1f -> %.1f, DIST %.1f -> %.1f, "
            "FUZZ %.1f -> %.1f\n",
            on,
            oa,
            dn,
            da,
            fn,
            fa);
        check(da < dn - 6 && fa < fn - 6,
              "ADAA cuts aliasing by >= 6 dB at DIST and FUZZ maximum gain");
    }

    std::vector<float> history(2 * kHistory), freeze(2 * kFreeze);
    Demo               demo;
    Snapshot           s;
    s.bypass = true;
    s.trails = false;
    s.drive  = P(1, .4f);
    demo.Init(history.data(), freeze.data(), s);
    float in[kBlock], l[kBlock], r[kBlock];
    for(auto &x : in)
        x = .1f;
    bool allModes = true;
    for(int mode = 0; mode < 5; ++mode)
    {
        s.mode = mode;
        ++s.epoch;
        for(int b = 0; b < 1000; ++b)
        {
            for(int i = 0; i < int(kBlock); ++i)
                in[i] = .1f * std::sin(kPi2 * 220 * (b * kBlock + i) / kFs);
            demo.Process(in, l, r, kBlock, s);
            demo.Service(s);
        }
        float peak = 0;
        for(int i = 0; i < int(kBlock); ++i)
            peak = std::max(peak, std::abs(l[i]));
        allModes &= peak > .3f && l[kBlock - 1] == r[kBlock - 1];
    }
    check(allModes, "PRE drive survives delay bypass in all five modes");
    ++s.epoch;
    s.mode = 0;
    for(int b = 0; b < 6; ++b)
        demo.Process(in, l, r, kBlock, s);
    check(demo.Paused(), "mode change reaches pause with drive ON");
    float driven = 0;
    for(int b = 0; b < 3; ++b)
    {
        for(int i = 0; i < int(kBlock); ++i)
            in[i] = .1f * std::sin(kPi2 * 220 * (b * kBlock + i) / kFs);
        demo.Process(in, l, r, kBlock, s);
        for(int i = 0; i < int(kBlock); ++i)
            driven = std::max(driven, std::abs(l[i] - in[i]));
    }
    check(demo.Paused() && driven > .05f,
          "PRE drive remains active during delay pause");
    demo.Service(s);
    for(int i = 0; i < int(kBlock); ++i)
        in[i] = .1f;
    s.drive.on = false;
    for(int b = 0; b < 1000; ++b)
        demo.Process(in, l, r, kBlock, s);
    check(std::abs(l[kBlock - 1] - .1f) < 1e-6f,
          "independent OFF restores clean delay bypass");

    auto echoEnergy = [&](bool enabled) {
        Demo     echo;
        Snapshot t;
        t.drive      = P(1, .7f, enabled);
        t.drive.tone = 1;
        t.slots[0]   = DigiTimeSlot(20);
        t.slots[1]   = 0;
        t.slots[2]   = 1;
        t.slots[3]   = 0;
        echo.Init(history.data(), freeze.data(), t);
        float x[kBlock]{}, a[kBlock], b[kBlock];
        for(int i = 0; i < 2000; ++i)
            echo.Process(x, a, b, kBlock, t);
        x[0] = .05f;
        echo.Process(x, a, b, kBlock, t);
        x[0]          = 0;
        double energy = 0;
        for(int i = 1; i < 25; ++i)
        {
            echo.Process(x, a, b, kBlock, t);
            if(i >= 20)
                for(float v : a)
                    energy += v * v;
        }
        return energy;
    };
    check(echoEnergy(true) > 20 * echoEnergy(false),
          "driven input also feeds the delay repeats");
    std::printf("DRIVE: %d checks, %d failures\n", checks, failures);
    return failures ? 1 : 0;
}
