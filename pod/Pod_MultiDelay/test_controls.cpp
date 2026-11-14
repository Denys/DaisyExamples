#include "Controls.h"
#include <cmath>
#include <cstdio>
using namespace poddemo;
int main()
{
    Controls c;
    Events   e;
    c.Init();
    int  fails = 0;
    auto check = [&](bool ok, const char *msg) {
        if(!ok)
        {
            ++fails;
            std::printf("FAIL %s\n", msg);
        }
    };
    e.pots[0] = e.pots[1] = 0.1f;
    c.Step(e);
    const float initial = c.s.slots[0];
    c.Step(e);
    check(c.s.slots[0] == initial, "startup keeps preset until touched");
    e.pots[0] = 0.3f;
    c.Step(e);
    check(std::abs(c.s.slots[0] - 0.3f) < 1e-6, "knob moves active slot");
    e.encClick = true;
    c.Step(e);
    e.encClick = false;
    check(c.page == 1, "click selects next page");
    const float mix = c.s.slots[2];
    c.Step(e);
    check(c.s.slots[2] == mix, "page change rearms movement gate");
    e.b2Rise = true;
    e.b2     = true;
    e.now    = 1000;
    c.Step(e);
    e.b2Rise = false;
    e.turn   = 1;
    c.Step(e);
    e.turn = 0;
    check(c.s.config == 1, "shift turn selects dual series");
    const auto epoch = c.s.epoch;
    e.b2             = false;
    e.b2Fall         = true;
    e.now            = 1200;
    c.Step(e);
    e.b2Fall = false;
    check(c.s.epoch == epoch && c.s.slots[0] == 0.3f,
          "shift release does not also tap");
    e.turn = -1;
    c.Step(e);
    e.turn = 0;
    check(c.s.mode == 4, "mode wraps backwards");
    e.b2     = true;
    e.b2Rise = true;
    e.now    = 2000;
    c.Step(e);
    e.b2Rise = false;
    e.b1Rise = true;
    c.Step(e);
    check(c.s.command == 5 && !c.s.bypass,
          "shift B1 clears freeze without bypass");
    e.b1Rise = false;
    e.b2     = false;
    e.b2Fall = true;
    e.now    = 2100;
    c.Step(e);
    check(c.s.command == 5, "clear gesture consumes capture release");
    int  checks = 9;
    auto check2 = [&](bool ok, const char *msg) {
        ++checks;
        check(ok, msg);
    };
    // DIGI contract v1 control law on the Pod (B2 = SHIFT).
    Controls d;
    Events   f;
    d.Init();
    f.pots[0] = f.pots[1] = 0.5f;
    d.Step(f);
    check2(std::abs(DigiTimeMs(d.s.slots[0]) - 400) < 0.1f
               && d.s.ratio == kDigiDefaultRatio && d.s.feedbackE2 < 0,
           "DIGI startup 400 ms, ratio 3/4, E2 linked");
    f.b2 = f.b2Rise = true;
    f.now           = 100;
    d.Step(f);
    f.b2Rise          = false;
    f.turn            = 1;
    const auto epoch2 = d.s.epoch;
    d.Step(f);
    f.turn = 0;
    check2(d.s.config == 1 && d.s.epoch == epoch2,
           "DIGI config change needs no pause");
    const float time = d.s.slots[0], feedback = d.s.slots[1];
    f.pots[0] = 0.1f;
    f.pots[1] = 0.9f;
    d.Step(f);
    check2(d.s.ratio == 0 && std::abs(d.s.feedbackE2 - 0.9f) < 1e-6f,
           "SHIFT+K1 ratio, SHIFT+K2 E2");
    check2(d.s.slots[0] == time && d.s.slots[1] == feedback,
           "SHIFT pots leave TIME/FEEDBACK");
    f.b1Rise = true;
    d.Step(f);
    f.b1Rise = false;
    check2(d.s.feedbackE2 < 0 && !d.s.bypass,
           "SHIFT+B1 relinks E2 without bypass");
    f.b2     = false;
    f.b2Fall = true;
    f.now    = 200;
    d.Step(f);
    f.b2Fall = false;
    check2(d.s.slots[0] == time && d.s.slots[1] == feedback,
           "SHIFT release rearms, no jump or tap");
    f.pots[0] = 0.12f; // moved from 0.1: arms the gate (>= 0.012)
    d.Step(f);
    const float armed = d.s.slots[0];
    f.pots[0]         = 0.122f;
    d.Step(f);
    check2(armed == 0.12f && d.s.slots[0] == armed,
           "TIME deadband ignores ADC-size steps");
    f.pots[0] = 0.13f;
    d.Step(f);
    check2(d.s.slots[0] == 0.13f, "TIME follows real movement");
    for(int i = 0; i < 2; ++i)
    {
        f.encClick = true;
        d.Step(f);
        f.encClick = false;
    }
    f.pots[1] = 0.2f;
    d.Step(f);
    check2(d.page == 2 && d.s.ratio == 0, "page 3 no longer owns the ratio");
    f.b2 = f.b2Rise = true;
    f.now           = 1000;
    d.Step(f);
    f.b2 = f.b2Rise = false;
    f.b2Fall        = true;
    f.now           = 1100;
    d.Step(f);
    f.b2Fall = false;
    f.b2 = f.b2Rise = true;
    f.now           = 1500;
    d.Step(f);
    f.b2 = f.b2Rise = false;
    f.b2Fall        = true;
    f.now           = 1600;
    d.Step(f);
    check2(std::abs(DigiTimeMs(d.s.slots[0]) - 500) < 0.5f,
           "DIGI tap uses the contract TIME law");
    // Navigation must never serve as an effect enable command.
    Controls dist;
    Events   g;
    dist.Init();
    g.pots[0] = g.pots[1] = 0.1f;
    dist.Step(g);
    const auto distEpoch = dist.s.epoch;
    for(int i = 0; i < 3; ++i)
    {
        g.encClick = true;
        dist.Step(g);
    }
    g.encClick = false;
    check2(dist.page == 3, "encoder selects dedicated DIST page");
    check2(dist.s.mode == 0 && dist.s.epoch == distEpoch && !dist.s.bypass,
           "entering DIST preserves running delay and tails");
    check2(!dist.s.driveOn, "entering DIST does not enable distortion");
    const auto distTime = dist.s.slots[0], distFeedback = dist.s.slots[1];
    g.pots[0] = 0.8f;
    g.pots[1] = 0.7f;
    dist.Step(g);
    check2(dist.s.drive == 0.8f && dist.s.tone == 0.7f && !dist.s.driveOn,
           "DIST pots edit drive and tone without enabling");
    check2(dist.s.slots[0] == distTime && dist.s.slots[1] == distFeedback,
           "DIST pots do not change delay parameters");
    g.b1Rise = true;
    dist.Step(g);
    g.b1Rise = false;
    check2(dist.s.driveOn && !dist.s.bypass && dist.s.epoch == distEpoch,
           "B1 enables distortion without bypass or reconfiguration");
    dist.Step(g);
    check2(dist.s.driveOn, "held button without new edge cannot retrigger");
    g.encClick = true;
    dist.Step(g);
    g.encClick = false;
    check2(dist.page == 0 && dist.s.driveOn,
           "leaving DIST retains distortion ON");
    dist.Step(g);
    check2(dist.s.slots[0] == distTime, "leaving DIST rearms pot ownership");
    g.b1Rise = true;
    dist.Step(g);
    g.b1Rise = false;
    check2(dist.s.bypass && dist.s.driveOn,
           "delay bypass leaves distortion ON");
    g.turn = 1;
    dist.Step(g);
    g.turn = 0;
    check2(dist.s.mode == 1 && dist.s.driveOn && dist.s.drive == 0.8f,
           "mode change preserves distortion enable and settings");
    for(int i = 0; i < 3; ++i)
    {
        g.encClick = true;
        dist.Step(g);
    }
    g.encClick = false;
    g.b1Rise   = true;
    dist.Step(g);
    g.b1Rise = false;
    check2(!dist.s.driveOn && dist.s.bypass,
           "B1 on DIST disables only distortion");
    // SHIFT ownership in DIGI is distinct from the DIST editing page.
    g.turn = -1;
    dist.Step(g);
    g.turn = 0;
    for(int i = 0; i < 3; ++i)
    {
        g.encClick = true;
        dist.Step(g);
    }
    g.encClick = false;
    g.b2 = g.b2Rise = true;
    g.now           = 3000;
    dist.Step(g);
    g.b2Rise  = false;
    g.pots[0] = 0.2f;
    g.pots[1] = 0.3f;
    dist.Step(g);
    check2(dist.s.drive == 0.2f && dist.s.tone == 0.3f
               && dist.s.ratio == kDigiDefaultRatio && dist.s.feedbackE2 < 0,
           "DIST pots keep ownership under SHIFT in DIGI");
    // Restore the unshifted page cycle, still without any implicit enable.
    g.b2     = false;
    g.b2Fall = true;
    g.now    = 3600;
    dist.Step(g);
    g.b2Fall = false;
    for(int i = 0; i < 8; ++i)
    {
        g.encClick = true;
        dist.Step(g);
        check2(!dist.s.driveOn, "page cycle cannot enable distortion");
    }
    std::printf("CONTROLS: %d checks, %d failures\n", checks, fails);
    return fails ? 1 : 0;
}
