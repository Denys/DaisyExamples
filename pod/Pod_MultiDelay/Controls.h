#pragma once
#include "Demo.h"
namespace poddemo
{
struct Events
{
    float pots[2]  = {0, 0};
    int   turn     = 0;
    bool  encClick = false, b1Rise = false, b1Fall = false, b1 = false,
         b2Rise = false, b2Fall = false, b2 = false;
    uint32_t now = 0, freezeState = 0;
};
// Transient notices for the display (UI only; never read by audio).
enum class Notice : int
{
    None,
    Trails,
    Tap,
    Relink,
    FreezeClear,
    DriveToggle,
    Bypass,
    DriveEnter,
    DriveExit
};
class Controls
{
    struct DriveBank
    {
        float gain, tone, level, blend;
    };
    float     bank_[5][5]{}, entry_[2]{};
    DriveBank drive_[kDriveTypes] = {{.5f, .55f, .8f, 1},  // OD, 0 dB
                                     {.4f, .5f, .6f, 1},   // DIST, -6 dB
                                     {.6f, .45f, .5f, 1}}; // FUZZ, -9 dB
    bool     touched_[2]{}, armed_ = false, consumed_ = false, haveTap_ = false;
    bool     b1Pending_ = false;
    uint32_t pressedAt_ = 0, lastTap_ = 0, b1At_ = 0;
    daisyhost::PedalDelayEngine mapper_;
    static int Wrap(int n, int count) { return (n % count + count) % count; }
    void       Arm(const Events &e)
    {
        for(int i = 0; i < 2; ++i)
        {
            entry_[i]   = e.pots[i];
            touched_[i] = false;
        }
        armed_ = true;
    }
    void Command(int command)
    {
        s.command = command;
        ++s.epoch;
    }
    void Note(Notice n, uint32_t now)
    {
        notice   = n;
        noticeAt = now;
    }
    void LoadDrive()
    {
        const auto &b = drive_[DriveTypeIndex(s.drive.type)];
        s.drive.gain  = b.gain;
        s.drive.tone  = b.tone;
        s.drive.level = b.level;
        s.drive.blend = b.blend;
    }
    // B1 alone: acts once the chord window has passed (or on an earlier release).
    void B1Action(const Events &e)
    {
        b1Pending_ = false;
        if(inDrive)
        {
            s.drive.on = !s.drive.on;
            Note(Notice::DriveToggle, e.now);
        }
        else
        {
            s.bypass = !s.bypass;
            Note(Notice::Bypass, e.now);
        }
    }
    void Chord(const Events &e)
    {
        b1Pending_ = false;
        consumed_  = true; // the B2 release must not also tap or capture
        if(!inDrive)
        {
            previousPage = page;
            inDrive      = true;
            drivePage    = 0;
            Note(Notice::DriveEnter, e.now);
        }
        else
        {
            inDrive = false;
            page    = previousPage;
            Note(Notice::DriveExit, e.now);
        }
        Arm(e);
    }
    void Shift1(const Events &e)
    {
        consumed_ = true;
        if(s.mode == 4)
        {
            Command(5);
            Note(Notice::FreezeClear, e.now);
        }
        else if(s.mode == 0)
        {
            s.feedbackE2 = -1; // relink E2 feedback to FEEDBACK
            Arm(e); // a still-held SHIFT+K2 must move again to unlink
            Note(Notice::Relink, e.now);
        }
    }
    void Pot(int k, float value, const Events &e)
    {
        if(inDrive)
        {
            auto & b = drive_[DriveTypeIndex(s.drive.type)];
            float &v = drivePage == 0 ? (k == 0 ? b.gain : b.tone)
                                      : (k == 0 ? b.level : b.blend);
            v = value;
            LoadDrive();
            if(e.b2)
                consumed_ = true;
            return;
        }
        if(e.b2 && s.mode == 0)
        {
            // Contract 5/8: SHIFT+TIME = E2 ratio, SHIFT+FEEDBACK = E2 feedback (unlinks).
            consumed_ = true;
            if(k == 0)
                s.ratio = std::min(6, static_cast<int>(value * 7));
            else
                s.feedbackE2 = value;
            return;
        }
        const int slot = page * 2 + k;
        if(slot >= 5)
            return;
        // ponytail: fixed deadband so ADC noise cannot step the integer-sample DIGI delay
        // (each step is an audible jump); replace with a measured threshold after listening.
        if(s.mode == 0 && slot == 0 && std::abs(value - s.slots[0]) < 0.004f)
            return;
        s.slots[slot]       = value;
        bank_[s.mode][slot] = value;
    }

  public:
    // PROPOSED: two presses within this window are one B1+B2 chord.
    static constexpr uint32_t kChordMs = 80;
    static constexpr int kPages = 3; // delay pages; DRIVE is entered by chord
    Snapshot             s;
    int                  page = 0, previousPage = 0, drivePage = 0;
    bool                 inDrive  = false;
    Notice               notice   = Notice::None;
    uint32_t             noticeAt = 0, tapMs = 0;
    bool                 Touched(int k) const { return touched_[k & 1]; }
    float                Entry(int k) const { return entry_[k & 1]; }
    void                 Init()
    {
        for(int m = 0; m < 5; ++m)
            for(int i = 0; i < 5; ++i)
            {
                const auto mode = static_cast<daisyhost::PedalDelayMode>(m);
                const auto slot = static_cast<daisyhost::PedalSlot>(i);
                bank_[m][i]     = mapper_.NativeToNormalized(
                    mode,
                    slot,
                    daisyhost::GetPedalSlotDescriptor(mode, slot).defaultValue);
            }
        // DIGI uses the contract v1 control law, not the reference engine's descriptors.
        for(int i = 0; i < 5; ++i)
            s.slots[i] = bank_[0][i] = Snapshot{}.slots[i];
        LoadDrive();
    }
    void Step(const Events &e)
    {
        if(!armed_)
            Arm(e);
        if(e.b2Rise)
        {
            pressedAt_ = e.now;
            consumed_  = false;
            if(b1Pending_)
                Chord(e); // B1 first, B2 inside the window
        }
        // Pots change owner when SHIFT changes, so neither owner jumps to the other's position.
        if(e.b2Rise || e.b2Fall)
            Arm(e);
        if(e.b1Rise)
        {
            b1At_ = e.now;
            if(e.b2 && !consumed_ && e.now - pressedAt_ <= kChordMs)
                Chord(e); // B2 first, B1 inside the window
            else if(e.b2)
                Shift1(e);
            else
                b1Pending_ = true;
        }
        if(b1Pending_ && (e.b1Fall || e.now - b1At_ >= kChordMs))
            B1Action(e);
        if(e.turn)
        {
            if(inDrive)
            {
                // Inside DRIVE, turn and SHIFT+turn both select the type.
                if(e.b2)
                    consumed_ = true;
                s.drive.type = Wrap(s.drive.type + e.turn, kDriveTypes);
                LoadDrive();
            }
            else if(e.b2)
            {
                consumed_ = true;
                if(s.mode == 0)
                {
                    // Applied at the next block boundary without a pause (contract 7).
                    s.config = Wrap(s.config + e.turn, 3);
                }
                else if(s.mode == 4)
                {
                    const int current
                        = std::clamp(static_cast<int>(e.freezeState), 1, 4);
                    Command(1 + Wrap(current - 1 + e.turn, 4));
                }
            }
            else
            {
                s.mode = Wrap(s.mode + e.turn, 5);
                for(int i = 0; i < 5; ++i)
                    s.slots[i] = bank_[s.mode][i];
                page     = 0;
                haveTap_ = false;
                Command(0);
            }
            Arm(e);
        }
        if(e.encClick)
        {
            if(e.b2)
            {
                consumed_ = true;
                s.trails  = !s.trails;
                Note(Notice::Trails, e.now);
            }
            else if(inDrive)
                drivePage ^= 1;
            else
                page = (page + 1) % kPages;
            Arm(e);
        }
        if(e.b2Fall && !consumed_ && e.now - pressedAt_ < 500)
        {
            if(s.mode == 4)
                Command(e.freezeState == 0 ? 1 : 6);
            else
            {
                const uint32_t dt = e.now - lastTap_;
                if(haveTap_ && dt >= 100 && dt <= 2000)
                {
                    s.slots[0] = s.mode == 0
                                     ? DigiTimeSlot(static_cast<float>(dt))
                                     : mapper_.NativeToNormalized(
                                         static_cast<daisyhost::PedalDelayMode>(
                                             s.mode),
                                         daisyhost::PedalSlot::kTime,
                                         static_cast<float>(dt));
                    bank_[s.mode][0] = s.slots[0];
                    tapMs            = dt;
                    Note(Notice::Tap, e.now);
                    Arm(e);
                }
                lastTap_ = e.now;
                haveTap_ = true;
            }
        }
        for(int k = 0; k < 2; ++k)
        {
            if(!std::isfinite(e.pots[k]))
                continue;
            const float value = std::clamp(e.pots[k], 0.0f, 1.0f);
            if(!touched_[k] && std::abs(value - entry_[k]) >= 0.012f)
                touched_[k] = true;
            if(touched_[k])
                Pot(k, value, e);
        }
    }
};
} // namespace poddemo
