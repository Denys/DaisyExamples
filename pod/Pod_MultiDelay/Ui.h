#pragma once
// v0.5 visual navigation for a 128x64 SSD1306/SSD1315 OLED. Pure rendering into an
// SSD1306 page-layout frame; no I/O here, so the host tests render every screen.
#include "Controls.h"
#include <cstdint>
#include <cstring>
namespace poddemo
{
// ADAPTED-FROM: adafruit/Adafruit-GFX-Library glcdfont.c (BSD-2-Clause), classic 5x7
// glyph table, subset re-entered by hand (sha not pinned). Columns, bit 0 = top row.
struct Glyph
{
    char    c;
    uint8_t col[5];
};
constexpr Glyph kFont[] = {
    {' ', {0x00, 0x00, 0x00, 0x00, 0x00}},
    {'#', {0x14, 0x7F, 0x14, 0x7F, 0x14}},
    {'%', {0x23, 0x13, 0x08, 0x64, 0x62}},
    {'*', {0x14, 0x08, 0x3E, 0x08, 0x14}},
    {'+', {0x08, 0x08, 0x3E, 0x08, 0x08}},
    {'-', {0x08, 0x08, 0x08, 0x08, 0x08}},
    {'.', {0x00, 0x60, 0x60, 0x00, 0x00}},
    {'/', {0x20, 0x10, 0x08, 0x04, 0x02}},
    {'0', {0x3E, 0x51, 0x49, 0x45, 0x3E}},
    {'1', {0x00, 0x42, 0x7F, 0x40, 0x00}},
    {'2', {0x42, 0x61, 0x51, 0x49, 0x46}},
    {'3', {0x21, 0x41, 0x45, 0x4B, 0x31}},
    {'4', {0x18, 0x14, 0x12, 0x7F, 0x10}},
    {'5', {0x27, 0x45, 0x45, 0x45, 0x39}},
    {'6', {0x3C, 0x4A, 0x49, 0x49, 0x30}},
    {'7', {0x01, 0x71, 0x09, 0x05, 0x03}},
    {'8', {0x36, 0x49, 0x49, 0x49, 0x36}},
    {'9', {0x06, 0x49, 0x49, 0x29, 0x1E}},
    {':', {0x00, 0x36, 0x36, 0x00, 0x00}},
    {'<', {0x08, 0x14, 0x22, 0x41, 0x00}},
    {'=', {0x14, 0x14, 0x14, 0x14, 0x14}},
    {'>', {0x00, 0x41, 0x22, 0x14, 0x08}},
    {'A', {0x7E, 0x11, 0x11, 0x11, 0x7E}},
    {'B', {0x7F, 0x49, 0x49, 0x49, 0x36}},
    {'C', {0x3E, 0x41, 0x41, 0x41, 0x22}},
    {'D', {0x7F, 0x41, 0x41, 0x22, 0x1C}},
    {'E', {0x7F, 0x49, 0x49, 0x49, 0x41}},
    {'F', {0x7F, 0x09, 0x09, 0x09, 0x01}},
    {'G', {0x3E, 0x41, 0x49, 0x49, 0x7A}},
    {'H', {0x7F, 0x08, 0x08, 0x08, 0x7F}},
    {'I', {0x00, 0x41, 0x7F, 0x41, 0x00}},
    {'J', {0x20, 0x40, 0x41, 0x3F, 0x01}},
    {'K', {0x7F, 0x08, 0x14, 0x22, 0x41}},
    {'L', {0x7F, 0x40, 0x40, 0x40, 0x40}},
    {'M', {0x7F, 0x02, 0x0C, 0x02, 0x7F}},
    {'N', {0x7F, 0x04, 0x08, 0x10, 0x7F}},
    {'O', {0x3E, 0x41, 0x41, 0x41, 0x3E}},
    {'P', {0x7F, 0x09, 0x09, 0x09, 0x06}},
    {'Q', {0x3E, 0x41, 0x51, 0x21, 0x5E}},
    {'R', {0x7F, 0x09, 0x19, 0x29, 0x46}},
    {'S', {0x46, 0x49, 0x49, 0x49, 0x31}},
    {'T', {0x01, 0x01, 0x7F, 0x01, 0x01}},
    {'U', {0x3F, 0x40, 0x40, 0x40, 0x3F}},
    {'V', {0x1F, 0x20, 0x40, 0x20, 0x1F}},
    {'W', {0x3F, 0x40, 0x38, 0x40, 0x3F}},
    {'X', {0x63, 0x14, 0x08, 0x14, 0x63}},
    {'Y', {0x07, 0x08, 0x70, 0x08, 0x07}},
    {'Z', {0x61, 0x51, 0x49, 0x45, 0x43}},
    {'|', {0x00, 0x00, 0x7F, 0x00, 0x00}},
};

class Frame
{
  public:
    static constexpr int kW = 128, kH = 64, kPages = 8;
    uint8_t              px[kW * kPages]{};
    void                 Clear() { std::memset(px, 0, sizeof(px)); }
    void                 Set(int x, int y, bool on)
    {
        if(x < 0 || y < 0 || x >= kW || y >= kH)
            return;
        uint8_t &b = px[x + (y / 8) * kW];
        b          = on ? b | (1 << (y % 8)) : b & ~(1 << (y % 8));
    }
    bool Get(int x, int y) const
    {
        return x >= 0 && y >= 0 && x < kW && y < kH
               && (px[x + (y / 8) * kW] >> (y % 8) & 1);
    }
    void Rect(int x, int y, int w, int h, bool on, bool fill)
    {
        for(int j = 0; j < h; ++j)
            for(int i = 0; i < w; ++i)
                if(fill || i == 0 || j == 0 || i == w - 1 || j == h - 1)
                    Set(x + i, y + j, on);
    }
    static const Glyph *Find(char c)
    {
        if(c >= 'a' && c <= 'z')
            c = static_cast<char>(c - 'a' + 'A');
        for(const auto &g : kFont)
            if(g.c == c)
                return &g;
        return &kFont[0];
    }
    // Returns the x after the text; 6 px advance per glyph at scale 1.
    int Text(int x, int y, const char *s, bool on = true, int scale = 1)
    {
        for(; *s; ++s, x += 6 * scale)
        {
            const Glyph *g = Find(*s);
            for(int i = 0; i < 5; ++i)
                for(int j = 0; j < 7; ++j)
                    if(g->col[i] >> j & 1)
                        for(int a = 0; a < scale; ++a)
                            for(int b = 0; b < scale; ++b)
                                Set(x + i * scale + a, y + j * scale + b, on);
        }
        return x;
    }
    static int Width(const char *s, int scale = 1)
    {
        return static_cast<int>(std::strlen(s)) * 6 * scale - scale;
    }
    void TextRight(int right, int y, const char *s, bool on = true)
    {
        Text(right - Width(s), y, s, on);
    }
};

// Small integer formatting: the firmware avoids float printf (code size).
inline char *Cat(char *d, const char *s)
{
    while(*s)
        *d++ = *s++;
    *d = 0;
    return d;
}
inline char *Int(char *d, long v)
{
    char          t[12];
    int           n   = 0;
    bool          neg = v < 0;
    unsigned long u
        = neg ? static_cast<unsigned long>(-v) : static_cast<unsigned long>(v);
    do
        t[n++] = static_cast<char>('0' + u % 10);
    while(u /= 10);
    if(neg)
        *d++ = '-';
    while(n)
        *d++ = t[--n];
    *d = 0;
    return d;
}
inline long Round(float v)
{
    return static_cast<long>(v < 0 ? v - .5f : v + .5f);
}
// "740MS", "1.25S", "35%", "6.0KHZ", "-6DB".
inline void FormatMs(char *d, float ms)
{
    if(ms < 1000)
        Cat(Int(d, Round(ms)), "MS");
    else
    {
        const long c = Round(ms / 10);
        d            = Int(d, c / 100);
        *d++         = '.';
        *d++         = static_cast<char>('0' + c / 10 % 10);
        *d++         = static_cast<char>('0' + c % 10);
        Cat(d, "S");
    }
}
inline void FormatHz(char *d, float hz)
{
    if(hz < 1000)
        Cat(Int(d, Round(hz)), "HZ");
    else
    {
        const long t = Round(hz / 100);
        d            = Int(d, t / 10);
        *d++         = '.';
        *d++         = static_cast<char>('0' + t % 10);
        Cat(d, "KHZ");
    }
}
inline void FormatPct(char *d, float unit)
{
    Cat(Int(d, Round(100 * std::clamp(Finite(unit), 0.0f, 1.0f))), "%");
}
inline void FormatDb(char *d, float db)
{
    const long r = Round(db);
    if(r > 0)
        *d++ = '+';
    Cat(Int(d, r), "DB");
}

struct UiStatus
{
    uint32_t now = 0, freezeState = 0;
    bool     paused = false, shift = false;
};

constexpr const char *kModeNames[5] = {"DIGI", "TAPE", "MOD", "REV", "FREEZE"};
constexpr const char *kConfigNames[3] = {"SINGLE", "SERIES", "PARALLEL"};
constexpr const char *kFreezeNames[5]
    = {"IDLE", "CAPTURE", "HOLD", "ACCUM", "REPLACE"};
constexpr const char *kDriveNames[kDriveTypes]
    = {"OVERDRIVE", "DISTORTION", "FUZZ"};
constexpr const char *kDriveShort[kDriveTypes] = {"OD", "DIST", "FUZZ"};
constexpr const char *kRatioNames[7]
    = {"1/4", "1/3", "3/8", "1/2", "2/3", "3/4", "1/1"};

class Ui
{
    daisyhost::PedalDelayEngine mapper_;

  public:
    // Label and native value of a delay slot. DIGI uses the contract v1 laws.
    void Slot(int mode, int slot, float v, char *label, char *value) const
    {
        static constexpr const char *kNames[5]
            = {"TIME", "FEEDBACK", "MIX", "COLOR", "MOTION"};
        Cat(label, kNames[slot]);
        if(mode == 0)
        {
            switch(slot)
            {
                case 0: FormatMs(value, DigiTimeMs(v)); return;
                case 1: FormatPct(value, .95f * v); return;
                case 2: FormatPct(value, v); return;
                case 3:
                {
                    const auto p = DigiMap(.5f, 0, -1, v, 5, Config::Single);
                    if(p.highCutHz >= kGuardHz)
                        Cat(value, "OPEN");
                    else
                        FormatHz(value, p.highCutHz);
                    return;
                }
                default: Cat(value, "--"); return;
            }
        }
        const auto  m  = static_cast<daisyhost::PedalDelayMode>(mode);
        const auto  sl = static_cast<daisyhost::PedalSlot>(slot);
        const auto &d  = daisyhost::GetPedalSlotDescriptor(m, sl);
        const float n  = mapper_.NormalizedToNative(m, sl, v);
        if(std::strcmp(d.unit, "ms") == 0)
            FormatMs(value, n);
        else if(std::strcmp(d.unit, "Hz") == 0)
            FormatHz(value, n);
        else
            FormatPct(value, v);
    }

  private:
    // Value bar with a pickup tick: hollow tick = knob position until it moves.
    static void Bar(Frame &f, int y, float value, bool touched, float knob)
    {
        f.Rect(0, y, 128, 6, true, false);
        const int w
            = static_cast<int>(124 * std::clamp(Finite(value), 0.0f, 1.0f));
        f.Rect(2, y + 2, w, 2, true, true);
        if(!touched)
        {
            const int x = 2
                          + static_cast<int>(
                              124 * std::clamp(Finite(knob), 0.0f, 1.0f));
            for(int j = -1; j <= 6; ++j)
                f.Set(x, y + j, j < 1 || j > 4 || !f.Get(x, y + j));
        }
    }
    static void Header(Frame &f, const char *left, const char *right)
    {
        f.Rect(0, 0, 128, 9, true, true);
        f.Text(2, 1, left, false);
        f.TextRight(126, 1, right, false);
    }
    static void Param(Frame &     f,
                      int         y,
                      const char *knob,
                      const char *label,
                      const char *value,
                      float       unit,
                      bool        touched,
                      float       pos)
    {
        char t[24];
        Cat(Cat(Cat(t, knob), " "), label);
        f.Text(0, y, t);
        f.TextRight(127, y, value);
        Bar(f, y + 9, unit, touched, pos);
    }
    static const char *NoticeText(const Controls &c)
    {
        switch(c.notice)
        {
            case Notice::Trails: return c.s.trails ? "TRAILS ON" : "TRAILS OFF";
            case Notice::Tap: return "TAP TEMPO";
            case Notice::Relink: return "E2 LINKED";
            case Notice::FreezeClear: return "FREEZE CLEAR";
            case Notice::DriveToggle:
                return c.s.drive.on ? "DRIVE ON" : "DRIVE OFF";
            case Notice::Bypass: return c.s.bypass ? "BYPASS" : "DELAY ON";
            case Notice::DriveEnter: return "DRIVE";
            case Notice::DriveExit: return "DELAY";
            default: return nullptr;
        }
    }

  public:
    static constexpr uint32_t kNoticeMs = 900;
    void Render(Frame &f, const Controls &c, const UiStatus &st) const
    {
        f.Clear();
        char        a[24]{}, b[24]{}, l0[16]{}, v0[16]{}, l1[16]{}, v1[16]{};
        const auto &s = c.s;
        if(c.inDrive)
        {
            const int t = DriveTypeIndex(s.drive.type);
            Header(f,
                   c.drivePage ? "DRIVE  LEVEL" : "DRIVE  GAIN",
                   s.drive.on ? "ON" : "OFF");
            const char *name = kDriveNames[t];
            f.Text((128 - Frame::Width(name, 2)) / 2, 11, name, true, 2);
            if(c.drivePage == 0)
            {
                FormatDb(v0, DriveGainDb(t, s.drive.gain));
                FormatHz(v1, DriveToneHz(s.drive.tone));
                Param(f,
                      26,
                      "K1",
                      "GAIN",
                      v0,
                      s.drive.gain,
                      c.Touched(0),
                      c.Entry(0));
                Param(f,
                      42,
                      "K2",
                      "TONE",
                      v1,
                      s.drive.tone,
                      c.Touched(1),
                      c.Entry(1));
            }
            else
            {
                FormatDb(v0, DriveLevelDb(s.drive.level));
                FormatPct(v1, s.drive.blend);
                Param(f,
                      26,
                      "K1",
                      "LEVEL",
                      v0,
                      s.drive.level,
                      c.Touched(0),
                      c.Entry(0));
                Param(f,
                      42,
                      "K2",
                      "BLEND",
                      v1,
                      s.drive.blend,
                      c.Touched(1),
                      c.Entry(1));
            }
        }
        else
        {
            const int mode = std::clamp(s.mode, 0, 4);
            char *    p    = Cat(a, kModeNames[mode]);
            if(mode == 0)
                Cat(Cat(p, " "), kConfigNames[std::clamp(s.config, 0, 2)]);
            else if(mode == 4)
                Cat(Cat(p, " "),
                    kFreezeNames[std::min<uint32_t>(st.freezeState, 4)]);
            Cat(Int(Cat(b, "P"), c.page + 1), "/3");
            Header(f, a, b);
            const int s0 = c.page * 2, s1 = s0 + 1;
            if(st.shift && mode == 0)
            {
                // SHIFT in DIGI: K1 = E2 ratio, K2 = E2 feedback (contract 5).
                Cat(v0, kRatioNames[std::clamp(s.ratio, 0, 6)]);
                if(s.feedbackE2 < 0)
                    Cat(v1, "LINKED");
                else
                    FormatPct(v1, .95f * s.feedbackE2);
                Param(f,
                      12,
                      "B2+K1",
                      "RATIO",
                      v0,
                      s.ratio / 6.0f,
                      c.Touched(0),
                      c.Entry(0));
                Param(f,
                      29,
                      "B2+K2",
                      "E2 FDBK",
                      v1,
                      s.feedbackE2 < 0 ? s.slots[1] : s.feedbackE2,
                      c.Touched(1),
                      c.Entry(1));
            }
            else
            {
                Slot(mode, s0, s.slots[s0], l0, v0);
                Param(
                    f, 12, "K1", l0, v0, s.slots[s0], c.Touched(0), c.Entry(0));
                if(s1 < 5)
                {
                    Slot(mode, s1, s.slots[s1], l1, v1);
                    Param(f,
                          29,
                          "K2",
                          l1,
                          v1,
                          s.slots[s1],
                          c.Touched(1),
                          c.Entry(1));
                }
                else if(mode == 0)
                {
                    Cat(l1, "RATIO E2");
                    Cat(v1, kRatioNames[std::clamp(s.ratio, 0, 6)]);
                    f.Text(0, 29, "B2+K1");
                    f.Text(36, 29, l1);
                    f.TextRight(127, 29, v1);
                }
            }
            // Status line: delay state, drive state, trails.
            char *q = Cat(a, s.bypass ? "BYPASS " : "");
            q       = Cat(q, kDriveShort[DriveTypeIndex(s.drive.type)]);
            Cat(q, s.drive.on ? " ON" : " OFF");
            f.Text(0, 47, a);
            f.TextRight(127, 47, s.trails ? "TRAILS" : "CUT");
        }
        f.Text(0, 56, c.inDrive ? "TURN TYPE  B1+B2 EXIT" : "B1+B2 DRIVE");
        if(st.paused)
            f.TextRight(127, 56, "...");
        if(const char *n = NoticeText(c))
            if(st.now - c.noticeAt < kNoticeMs)
            {
                const int w = Frame::Width(n) + 10;
                f.Rect((128 - w) / 2, 22, w, 15, false, true);
                f.Rect((128 - w) / 2, 22, w, 15, true, false);
                f.Text((128 - w) / 2 + 5, 26, n);
            }
    }
};

// Dirty-page tracking for the transport: only changed 128-byte pages are sent.
class PageDiff
{
    uint8_t sent_[Frame::kW * Frame::kPages]{};
    uint8_t force_ = 0xFF; // pages never sent since the last invalidate

  public:
    void Invalidate() { force_ = 0xFF; }
    bool Dirty(const Frame &f, int page) const
    {
        return (force_ >> page & 1)
               || std::memcmp(f.px + page * Frame::kW,
                              sent_ + page * Frame::kW,
                              Frame::kW)
                      != 0;
    }
    // Records the bytes actually transmitted for a page (frozen at page start).
    void MarkSent(int page, const uint8_t *bytes)
    {
        std::memcpy(sent_ + page * Frame::kW, bytes, Frame::kW);
        force_ = static_cast<uint8_t>(force_ & ~(1 << page));
    }
    void MarkSent(const Frame &f, int page)
    {
        MarkSent(page, f.px + page * Frame::kW);
    }
};
} // namespace poddemo
