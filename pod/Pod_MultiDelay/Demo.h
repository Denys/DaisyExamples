#pragma once
#include "DigiMono.h"
#include "daisyhost/PedalDelayEngine.h"
#include <atomic>
#include <cstddef>
#include <cstdint>
namespace poddemo {
constexpr std::size_t kHistory = 120008, kFreeze = 96008, kBlock = 48;
struct Snapshot {
    int mode = 0;
    uint32_t epoch = 0;
    float slots[5] = {0.650515f, 0.368421f, 0.35f, 0.7815f, 0};
    int config = 0, ratio = 2, command = 0;
    bool bypass = false, trails = true;
};
class Demo {
    enum Phase : uint32_t { Running, FadingOut, PausedState, FadingIn };
    std::atomic<uint32_t> phase_{PausedState};
    daisyhost::PedalDelayEngine engine_;
    DigiMono digi_;
    int mode_ = -1, config_ = -1;
    uint32_t epoch_ = 0;
    float fade_ = 0, mix_ = 0.35f, active_ = 1;
    bool snapDigi_ = true;
    float send_[kBlock]{}, wet_[kBlock]{}, unused_[kBlock]{};

  public:
    std::atomic<uint32_t> freezeStatus{0};
    static_assert(std::atomic<uint32_t>::is_always_lock_free, "32-bit atomics must be lock-free");
    void Init(float *history, float *freeze, const Snapshot &s) {
        engine_.AttachStorage(history, kHistory, freeze, kFreeze);
        engine_.Prepare(48000, kBlock);
        digi_.Attach(history, kHistory);
        Service(s);
    }
    bool Paused() const { return phase_.load(std::memory_order_acquire) == PausedState; }
    // Main only. Audio has acknowledged that it no longer touches engine/buffers.
    bool Service(const Snapshot &s) {
        if (!Paused())
            return false;
        const int mode = std::clamp(s.mode, 0, 4), config = std::clamp(s.config, 0, 2);
        if (mode != mode_ || config != config_) {
            engine_.SetMode(static_cast<daisyhost::PedalDelayMode>(mode));
            for (int i = 0; i < 5; ++i)
                engine_.SetSlotNormalized(static_cast<daisyhost::PedalSlot>(i),
                                          i == 2 ? 1 : std::clamp(Finite(s.slots[i]), 0.0f, 1.0f));
            engine_.Reset();
            digi_.Reset();
            snapDigi_ = true;
        }
        if (mode == 4) {
            if (s.command >= 1 && s.command <= 4)
                engine_.SetFreezeState(static_cast<daisyhost::PedalFreezeState>(s.command));
            else if (s.command == 5)
                engine_.FreezeClear();
            else if (s.command == 6)
                engine_.SetFreezeState(daisyhost::PedalFreezeState::kIdle);
        }
        freezeStatus.store(static_cast<uint32_t>(engine_.GetFreezeState()),
                           std::memory_order_relaxed);
        mode_ = mode;
        config_ = config;
        epoch_ = s.epoch;
        phase_.store(FadingIn, std::memory_order_release);
        return true;
    }
    void Process(const float *input, float *left, float *right, std::size_t n, const Snapshot &s) {
        auto phase = phase_.load(std::memory_order_acquire);
        if (n > kBlock || phase == PausedState) {
            for (std::size_t i = 0; i < n; ++i)
                left[i] = right[i] = std::clamp(Finite(input[i]), -1.0f, 1.0f);
            return;
        }
        if (s.epoch != epoch_ && phase != FadingOut) {
            phase = FadingOut;
            phase_.store(phase, std::memory_order_relaxed);
        }
        float slots[5];
        for (int i = 0; i < 5; ++i)
            slots[i] = std::clamp(Finite(s.slots[i]), 0.0f, 1.0f);
        // A mode command waits for its pause; old-mode parameters remain stable meanwhile.
        if (phase != FadingOut) {
            for (int i = 0; i < 5; ++i)
                engine_.SetSlotNormalized(static_cast<daisyhost::PedalSlot>(i),
                                          i == 2 ? 1 : slots[i]);
            if (mode_ == 0) {
                constexpr float ratios[] = {0.5f, 2.0f / 3, 0.75f, 1, 4.0f / 3, 1.5f, 2};
                const float t1 = 20 * std::pow(100.0f, slots[0]);
                const float t2 = std::clamp(t1 * ratios[std::clamp(s.ratio, 0, 6)], 20.0f, 2000.0f);
                const float alpha =
                    1 - std::exp(-6.2831853f * (500 * std::pow(24.0f, slots[3])) / 48000);
                digi_.Set(t1 * 48, t2 * 48, std::min(slots[1] * 0.95f, 0.90f), alpha,
                          slots[4] * 240, static_cast<Config>(config_), snapDigi_);
                snapDigi_ = false;
            }
        }
        // Smooth send/bypass and mix. Read the coherent snapshot once per block.
        float mixes[kBlock], actives[kBlock];
        for (std::size_t i = 0; i < n; ++i) {
            mix_ += (slots[2] - mix_) * 0.00104112f;
            active_ += ((s.bypass ? 0.0f : 1.0f) - active_) * 0.004158f;
            mixes[i] = mix_;
            actives[i] = active_;
            send_[i] = std::clamp(Finite(input[i]), -1.0f, 1.0f) * active_;
        }
        if (mode_ == 0)
            for (std::size_t i = 0; i < n; ++i)
                wet_[i] = digi_.Process(send_[i]);
        else
            engine_.Process(send_, send_, wet_, unused_, n);
        freezeStatus.store(static_cast<uint32_t>(engine_.GetFreezeState()),
                           std::memory_order_relaxed);
        for (std::size_t i = 0; i < n; ++i) {
            if (phase == FadingOut)
                fade_ = std::max(0.0f, fade_ - 1.0f / 240);
            else if (phase == FadingIn)
                fade_ = std::min(1.0f, fade_ + 1.0f / 240);
            const float dry = std::clamp(Finite(input[i]), -1.0f, 1.0f);
            const float wetGain = mixes[i] * (s.trails ? 1 : actives[i]);
            const float effect = dry * (1 - actives[i] * mixes[i]) + Finite(wet_[i]) * wetGain;
            left[i] = right[i] = std::clamp(Finite(dry + fade_ * (effect - dry)), -1.0f, 1.0f);
        }
        if (phase == FadingOut && fade_ <= 0)
            phase_.store(PausedState, std::memory_order_release);
        else if (phase == FadingIn && fade_ >= 1)
            phase_.store(Running, std::memory_order_release);
    }
};
} // namespace poddemo
