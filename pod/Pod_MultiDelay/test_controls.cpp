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
    std::printf("CONTROLS: 9 checks, %d failures\n", fails);
    return fails ? 1 : 0;
}
