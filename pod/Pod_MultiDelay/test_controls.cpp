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
    e.now = 2000 + Controls::kChordMs + 1; // SHIFT held past the chord window
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
    f.now    = 100 + Controls::kChordMs + 1;
    f.b1Rise = true;
    d.Step(f);
    f.b1Rise = false;
    check2(d.s.feedbackE2 < 0 && !d.s.bypass,
           "SHIFT+B1 relinks E2 without bypass");
    f.b2     = false;
    f.b2Fall = true;
    f.now    = 300;
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
    // v0.5 DRIVE: a B1+B2 chord enters from any delay page and returns to it.
    Controls dr;
    Events   h;
    dr.Init();
    h.pots[0] = h.pots[1] = 0.1f;
    uint32_t t            = 10000;
    auto     step         = [&](uint32_t dt) {
        t += dt;
        h.now = t;
        dr.Step(h);
        h.b1Rise = h.b1Fall = h.b2Rise = h.b2Fall = h.encClick = false;
        h.turn                                                 = 0;
    };
    auto b1Down = [&](uint32_t dt) {
        h.b1 = h.b1Rise = true;
        step(dt);
    };
    auto b1Up = [&](uint32_t dt) {
        h.b1     = false;
        h.b1Fall = true;
        step(dt);
    };
    auto b2Down = [&](uint32_t dt) {
        h.b2 = h.b2Rise = true;
        step(dt);
    };
    auto b2Up = [&](uint32_t dt) {
        h.b2     = false;
        h.b2Fall = true;
        step(dt);
    };
    step(0);
    h.encClick = true;
    step(5);
    const auto ep = dr.s.epoch;
    const int  m0 = dr.s.mode;
    b1Down(100);
    b2Down(30);
    check2(dr.inDrive && dr.drivePage == 0 && !dr.s.bypass && dr.s.epoch == ep,
           "B1 then B2 within the window enters DRIVE without bypass");
    b1Up(50);
    b2Up(20);
    check2(!dr.s.bypass && dr.tapMs == 0 && !dr.s.drive.on,
           "chord release neither bypasses, taps nor enables DRIVE");
    h.turn = 1;
    step(10);
    check2(dr.s.drive.type == 2 && dr.s.mode == m0,
           "turn in DRIVE selects the type, not the delay mode");
    h.turn = 1;
    step(10);
    check2(dr.s.drive.type == 0, "type wraps FUZZ -> OVERDRIVE");
    b2Down(10);
    h.turn = 1;
    step(200);
    check2(dr.s.drive.type == 1 && dr.s.config == 0,
           "SHIFT+turn in DRIVE selects the type, not the DIGI config");
    b2Up(10);
    check2(dr.tapMs == 0, "SHIFT+turn consumes the B2 release");
    h.pots[0] = 0.9f;
    step(10);
    check2(dr.s.drive.gain == 0.9f, "K1 edits GAIN of the selected type");
    h.turn = 1;
    step(10);
    check2(dr.s.drive.gain == 0.6f, "each type keeps its own GAIN");
    h.turn = -1;
    step(10);
    check2(dr.s.drive.gain == 0.9f, "returning restores DISTORTION GAIN");
    h.encClick = true;
    step(10);
    check2(dr.drivePage == 1 && dr.inDrive, "click switches to LEVEL/BLEND");
    h.pots[1] = 0.5f;
    step(10);
    check2(dr.s.drive.blend == 0.5f && dr.s.drive.tone == 0.5f,
           "K2 on LEVEL/BLEND edits BLEND only");
    b1Down(10);
    check2(!dr.s.drive.on, "B1 waits for the chord window");
    step(Controls::kChordMs);
    check2(dr.s.drive.on && !dr.s.bypass,
           "B1 alone toggles DRIVE after the window");
    b1Up(10);
    check2(dr.s.drive.on, "B1 release does not toggle again");
    b1Down(10);
    b1Up(20);
    check2(!dr.s.drive.on, "a short B1 acts on release");
    b1Down(10);
    step(100);
    b1Up(10);
    b2Down(500);
    b1Down(40);
    check2(!dr.inDrive && dr.page == 1 && dr.s.drive.on,
           "B2 then B1 within the window exits to the previous page, DRIVE "
           "stays ON");
    b1Up(10);
    b2Up(10);
    check2(!dr.s.bypass && dr.tapMs == 0,
           "exit chord neither bypasses nor taps");
    h.encClick = true;
    step(10);
    h.turn = 1;
    step(10);
    check2(dr.s.drive.on && dr.s.mode == 1,
           "DRIVE stays ON across page and mode");
    h.turn = -1;
    step(10);
    b2Down(10);
    step(200);
    b1Down(10);
    check2(!dr.inDrive && !dr.s.bypass && dr.notice == Notice::Relink,
           "B2 held past the window then B1 is SHIFT+B1, not a chord");
    b1Up(10);
    b2Up(10);
    b1Down(10);
    step(Controls::kChordMs);
    b1Up(10);
    check2(dr.s.bypass && dr.s.drive.on, "B1 alone toggles delay bypass only");
    h.b1 = h.b1Rise = h.b2 = h.b2Rise = true;
    step(10);
    check2(dr.inDrive, "B1 and B2 in the same scan enter DRIVE");
    h.b1 = h.b2 = false;
    h.b1Fall = h.b2Fall = true;
    step(10);
    h.b1 = h.b1Rise = h.b2 = h.b2Rise = true;
    step(10);
    h.b1 = h.b2 = false;
    h.b1Fall = h.b2Fall = true;
    step(10);
    check2(!dr.inDrive, "same-scan chord also exits");
    h.turn = -1;
    step(10);
    const auto ep4 = dr.s.epoch;
    b2Down(10);
    b1Down(20);
    b1Up(10);
    b2Up(10);
    check2(dr.s.mode == 4 && dr.inDrive && dr.s.epoch == ep4,
           "chord in FREEZE neither captures nor reconfigures");
    b2Down(10);
    b1Down(20);
    b1Up(10);
    b2Up(10);
    bool neverDrive = true;
    for(int i = 0; i < 3; ++i)
    {
        h.encClick = true;
        step(10);
        neverDrive &= !dr.inDrive;
    }
    check2(neverDrive && dr.page == 0, "click cycles three delay pages only");
    std::printf("CONTROLS: %d checks, %d failures\n", checks, fails);
    return fails ? 1 : 0;
}