#pragma once
#include "Demo.h"
namespace poddemo {
struct Events {
    float pots[2] = {0, 0};
    int turn = 0;
    bool encClick = false, b1Rise = false, b2Rise = false, b2Fall = false, b2 = false;
    uint32_t now = 0, freezeState = 0;
};
class Controls {
    float bank_[5][5]{}, entry_[2]{};
    bool touched_[2]{}, armed_ = false, consumed_ = false, haveTap_ = false;
    uint32_t pressedAt_ = 0, lastTap_ = 0;
    daisyhost::PedalDelayEngine mapper_;
    static int Wrap(int n, int count) { return (n % count + count) % count; }
    void Arm(const Events &e) {
        for (int i = 0; i < 2; ++i) {
            entry_[i] = e.pots[i];
            touched_[i] = false;
        }
        armed_ = true;
    }
    void Command(int command) {
        s.command = command;
        ++s.epoch;
    }

  public:
    Snapshot s;
    int page = 0;
    void Init() {
        for (int m = 0; m < 5; ++m)
            for (int i = 0; i < 5; ++i) {
                const auto mode = static_cast<daisyhost::PedalDelayMode>(m);
                const auto slot = static_cast<daisyhost::PedalSlot>(i);
                bank_[m][i] = mapper_.NativeToNormalized(
                    mode, slot, daisyhost::GetPedalSlotDescriptor(mode, slot).defaultValue);
            }
        for (int i = 0; i < 5; ++i)
            s.slots[i] = bank_[0][i];
    }
    void Step(const Events &e) {
        if (!armed_)
            Arm(e);
        if (e.b2Rise) {
            pressedAt_ = e.now;
            consumed_ = false;
        }
        if (e.turn) {
            if (e.b2) {
                consumed_ = true;
                if (s.mode == 0) {
                    s.config = Wrap(s.config + e.turn, 3);
                    Command(0);
                } else if (s.mode == 4) {
                    const int current = std::clamp(static_cast<int>(e.freezeState), 1, 4);
                    Command(1 + Wrap(current - 1 + e.turn, 4));
                }
            } else {
                s.mode = Wrap(s.mode + e.turn, 5);
                for (int i = 0; i < 5; ++i)
                    s.slots[i] = bank_[s.mode][i];
                page = 0;
                haveTap_ = false;
                Command(0);
            }
            Arm(e);
        }
        if (e.encClick) {
            if (e.b2) {
                consumed_ = true;
                s.trails = !s.trails;
            } else {
                page = (page + 1) % 3;
            }
            Arm(e);
        }
        if (e.b1Rise) {
            if (e.b2) {
                consumed_ = true;
                if (s.mode == 4)
                    Command(5);
            } else
                s.bypass = !s.bypass;
        }
        if (e.b2Fall && !consumed_ && e.now - pressedAt_ < 500) {
            if (s.mode == 4)
                Command(e.freezeState == 0 ? 1 : 6);
            else {
                const uint32_t dt = e.now - lastTap_;
                if (haveTap_ && dt >= 100 && dt <= 2000) {
                    s.slots[0] = mapper_.NativeToNormalized(
                        static_cast<daisyhost::PedalDelayMode>(s.mode), daisyhost::PedalSlot::kTime,
                        static_cast<float>(dt));
                    bank_[s.mode][0] = s.slots[0];
                    Arm(e);
                }
                lastTap_ = e.now;
                haveTap_ = true;
            }
        }
        for (int k = 0; k < 2; ++k) {
            if (!std::isfinite(e.pots[k]))
                continue;
            const float value = std::clamp(e.pots[k], 0.0f, 1.0f);
            if (!touched_[k] && std::abs(value - entry_[k]) >= 0.012f)
                touched_[k] = true;
            if (!touched_[k])
                continue;
            const int slot = page * 2 + k;
            if (slot < 5) {
                s.slots[slot] = value;
                bank_[s.mode][slot] = value;
            } else if (s.mode == 0)
                s.ratio = std::min(6, static_cast<int>(value * 7));
        }
    }
};
} // namespace poddemo
