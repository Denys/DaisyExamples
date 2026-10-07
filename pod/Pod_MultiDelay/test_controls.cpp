#include "Controls.h"
#include <cmath>
#include <cstdio>
using namespace poddemo;
int main() {
    Controls c;
    Events e;
    c.Init();
    int fails = 0;
    auto check = [&](bool ok, const char *msg) {
        if (!ok) {
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
    e.b2 = true;
    e.now = 1000;
    c.Step(e);
    e.b2Rise = false;
    e.turn = 1;
    c.Step(e);
    e.turn = 0;
    check(c.s.config == 1, "shift turn selects dual series");
    const auto epoch = c.s.epoch;
    e.b2 = false;
    e.b2Fall = true;
    e.now = 1200;
    c.Step(e);
    e.b2Fall = false;
    check(c.s.epoch == epoch && c.s.slots[0] == 0.3f, "shift release does not also tap");
    e.turn = -1;
    c.Step(e);
    e.turn = 0;
    check(c.s.mode == 4, "mode wraps backwards");
    e.b2 = true;
    e.b2Rise = true;
    e.now = 2000;
    c.Step(e);
    e.b2Rise = false;
    e.b1Rise = true;
    c.Step(e);
    check(c.s.command == 5 && !c.s.bypass, "shift B1 clears freeze without bypass");
    e.b1Rise = false;
    e.b2 = false;
    e.b2Fall = true;
    e.now = 2100;
    c.Step(e);
    check(c.s.command == 5, "clear gesture consumes capture release");
    int checks = 9;
    auto check2 = [&](bool ok, const char *msg) {
        ++checks;
        check(ok, msg);
    };
    // DIGI contract v1 control law on the Pod (B2 = SHIFT).
    Controls d;
    Events f;
    d.Init();
    f.pots[0] = f.pots[1] = 0.5f;
    d.Step(f);
    check2(std::abs(DigiTimeMs(d.s.slots[0]) - 400) < 0.1f && d.s.ratio == kDigiDefaultRatio &&
               d.s.feedbackE2 < 0,
           "DIGI startup 400 ms, ratio 3/4, E2 linked");
    f.b2 = f.b2Rise = true;
    f.now = 100;
    d.Step(f);
    f.b2Rise = false;
    f.turn = 1;
    const auto epoch2 = d.s.epoch;
    d.Step(f);
    f.turn = 0;
    check2(d.s.config == 1 && d.s.epoch == epoch2, "DIGI config change needs no pause");
    const float time = d.s.slots[0], feedback = d.s.slots[1];
    f.pots[0] = 0.1f;
    f.pots[1] = 0.9f;
    d.Step(f);
    check2(d.s.ratio == 0 && std::abs(d.s.feedbackE2 - 0.9f) < 1e-6f, "SHIFT+K1 ratio, SHIFT+K2 E2");
    check2(d.s.slots[0] == time && d.s.slots[1] == feedback, "SHIFT pots leave TIME/FEEDBACK");
    f.b1Rise = true;
    d.Step(f);
    f.b1Rise = false;
    check2(d.s.feedbackE2 < 0 && !d.s.bypass, "SHIFT+B1 relinks E2 without bypass");
    f.b2 = false;
    f.b2Fall = true;
    f.now = 200;
    d.Step(f);
    f.b2Fall = false;
    check2(d.s.slots[0] == time && d.s.slots[1] == feedback, "SHIFT release rearms, no jump or tap");
    f.pots[0] = 0.12f; // moved from 0.1: arms the gate (>= 0.012)
    d.Step(f);
    const float armed = d.s.slots[0];
    f.pots[0] = 0.122f;
    d.Step(f);
    check2(armed == 0.12f && d.s.slots[0] == armed, "TIME deadband ignores ADC-size steps");
    f.pots[0] = 0.13f;
    d.Step(f);
    check2(d.s.slots[0] == 0.13f, "TIME follows real movement");
    for (int i = 0; i < 2; ++i) {
        f.encClick = true;
        d.Step(f);
        f.encClick = false;
    }
    f.pots[1] = 0.2f;
    d.Step(f);
    check2(d.page == 2 && d.s.ratio == 0, "page 3 no longer owns the ratio");
    f.b2 = f.b2Rise = true;
    f.now = 1000;
    d.Step(f);
    f.b2 = f.b2Rise = false;
    f.b2Fall = true;
    f.now = 1100;
    d.Step(f);
    f.b2Fall = false;
    f.b2 = f.b2Rise = true;
    f.now = 1500;
    d.Step(f);
    f.b2 = f.b2Rise = false;
    f.b2Fall = true;
    f.now = 1600;
    d.Step(f);
    check2(std::abs(DigiTimeMs(d.s.slots[0]) - 500) < 0.5f, "DIGI tap uses the contract TIME law");
    std::printf("CONTROLS: %d checks, %d failures\n", checks, fails);
    return fails ? 1 : 0;
}
