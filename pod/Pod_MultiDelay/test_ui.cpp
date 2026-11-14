#include "Ui.h"
#include <cstdio>
#include <string>
using namespace poddemo;
namespace
{
// Writes a 4x-scaled PBM so the screens can be inspected on the host.
void Save(const Frame &f, const std::string &name)
{
    const std::string path = "build/ui/" + name + ".pbm";
    if(FILE *o = std::fopen(path.c_str(), "wb"))
    {
        std::fprintf(o, "P1\n%d %d\n", Frame::kW * 4, Frame::kH * 4);
        for(int y = 0; y < Frame::kH * 4; ++y)
        {
            for(int x = 0; x < Frame::kW * 4; ++x)
                std::fputs(f.Get(x / 4, y / 4) ? "1 " : "0 ", o);
            std::fputc('\n', o);
        }
        std::fclose(o);
    }
}
int Lit(const Frame &f)
{
    int n = 0;
    for(int y = 0; y < Frame::kH; ++y)
        for(int x = 0; x < Frame::kW; ++x)
            n += f.Get(x, y);
    return n;
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
    Ui       ui;
    Frame    f;
    UiStatus st;
    Controls c;
    c.Init();
    Events e;
    e.pots[0] = e.pots[1] = .3f;
    c.Step(e);

    // Every label/value pair of every mode fits one 128 px line at both extremes.
    bool fits = true;
    for(int m = 0; m < 5; ++m)
        for(int sl = 0; sl < 5; ++sl)
            for(float v : {0.0f, .5f, 1.0f})
            {
                char l[16]{}, val[16]{}, k[24]{};
                ui.Slot(m, sl, v, l, val);
                Cat(Cat(k, "K1 "), l);
                fits &= Frame::Width(k) + 6 + Frame::Width(val) <= 128;
            }
    check(fits, "every delay label and value fits one line");
    for(int t = 0; t < kDriveTypes; ++t)
        check(Frame::Width(kDriveNames[t], 2) <= 128, "drive name fits at 2x");

    st.now = 100000;
    ui.Render(f, c, st);
    Save(f, "01_digi_page1");
    check(f.Get(1, 1) && Lit(f) > 300,
          "delay screen has inverted header and content");
    check(f.Get(0, 22) && f.Get(127, 22), "K1 value bar spans the screen");

    c.s.mode       = 4;
    st.freezeState = 2;
    ui.Render(f, c, st);
    Save(f, "02_freeze_hold");
    c.s.mode = 0;
    st.shift = true;
    ui.Render(f, c, st);
    Save(f, "03_digi_shift");
    st.shift = false;

    // Enter DRIVE by chord, then render both DRIVE pages and a notice.
    e.now = 200000;
    e.b1 = e.b1Rise = e.b2 = e.b2Rise = true;
    c.Step(e);
    e.b1Rise = e.b2Rise = false;
    check(c.inDrive, "chord enters DRIVE");
    st.now = e.now + 100;
    ui.Render(f, c, st);
    Save(f, "04_drive_enter_notice");
    Frame noted = f;
    st.now      = e.now + Ui::kNoticeMs + 1;
    ui.Render(f, c, st);
    Save(f, "05_drive_gain_tone");
    check(std::memcmp(noted.px, f.px, sizeof f.px) != 0,
          "notice disappears after its time");
    // Pickup: untouched knob shows its own position as a tick on the bar.
    const int tickX = 2 + static_cast<int>(124 * c.Entry(0));
    check(f.Get(tickX, 34) && f.Get(tickX, 41),
          "untouched knob shows a pickup tick");
    e.b1 = e.b2 = false;
    e.b1Fall = e.b2Fall = true;
    c.Step(e);
    e.b1Fall = e.b2Fall = false;
    e.encClick          = true;
    c.Step(e);
    e.encClick = false;
    e.turn     = 1;
    c.Step(e);
    e.turn = 0;
    ui.Render(f, c, st);
    Save(f, "06_drive_level_blend_fuzz");
    check(c.drivePage == 1 && c.s.drive.type == 2,
          "LEVEL/BLEND page of FUZZ rendered");

    // Transport bookkeeping: only changed pages are re-sent.
    PageDiff diff;
    int      dirty = 0;
    for(int p = 0; p < Frame::kPages; ++p)
    {
        dirty += diff.Dirty(f, p);
        diff.MarkSent(f, p);
    }
    check(dirty == 8, "first frame sends all eight pages");
    dirty = 0;
    for(int p = 0; p < Frame::kPages; ++p)
        dirty += diff.Dirty(f, p);
    check(dirty == 0, "an unchanged frame sends nothing");
    f.Set(10, 50, !f.Get(10, 50));
    dirty = 0;
    for(int p = 0; p < Frame::kPages; ++p)
        dirty += diff.Dirty(f, p);
    check(dirty == 1 && diff.Dirty(f, 6), "one changed pixel sends one page");
    diff.Invalidate();
    check(diff.Dirty(f, 0), "invalidate forces a full resend");

    char buf[16];
    FormatMs(buf, 400);
    check(std::string(buf) == "400MS", "format 400 ms");
    FormatMs(buf, 1250);
    check(std::string(buf) == "1.25S", "format 1.25 s");
    FormatHz(buf, 6000);
    check(std::string(buf) == "6.0KHZ", "format 6 kHz");
    FormatDb(buf, -6);
    check(std::string(buf) == "-6DB", "format -6 dB");
    FormatDb(buf, 23.6f);
    check(std::string(buf) == "+24DB", "format +24 dB");
    std::printf("UI: %d checks, %d failures\n", checks, failures);
    return failures ? 1 : 0;
}
