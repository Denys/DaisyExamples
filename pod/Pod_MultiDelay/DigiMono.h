#pragma once
// Mono-first DIGI, contract v1 (custom-pedals delay/runs/2026-09-24_mono_first_digi_contract_v1.md,
// accepted 2026-09-25). The per-engine law mirrors DAFX DigitalDelayNode::Process @ 73976da in mono:
//   tap = h[D] (read before write); cond = LP(HP(tap)); h <- send*in + f*cond.
// Routing (contract 4): in1 = x; in2 = 0 | tap1 | x; wet = g1*tap1 + g2*tap2.
#include <algorithm>
#include <cmath>
#include <cstddef>
namespace poddemo {
enum class Config { Single, Series, Parallel };
inline float Finite(float x) { return std::isfinite(x) ? x : 0.0f; }

constexpr float kFs = 48000, kGuardHz = kFs * 0.49f;
constexpr float kDigiRatios[7] = {0.25f, 1.0f / 3, 0.375f, 0.5f, 2.0f / 3, 0.75f, 1};
constexpr int kDigiDefaultRatio = 5; // 3/4

// Contract 5: TIME 20 ms ... 2.5 s, log taper.
inline float DigiTimeMs(float t) { return 20 * std::pow(125.0f, std::clamp(Finite(t), 0.0f, 1.0f)); }
inline float DigiTimeSlot(float ms) {
    return std::clamp(std::log(std::max(Finite(ms), 20.0f) / 20) / std::log(125.0f), 0.0f, 1.0f);
}

struct DigiParams {
    float delay[2] = {19200, 14400}; // samples, rounded half up at use
    float feedback[2] = {0, 0};
    float lowCutHz = 40, highCutHz = kGuardHz; // 0 disables HP; >= guard disables LP
    Config config = Config::Single;
};

// Contract 5-6: front-panel slots in [0,1] -> engine parameters (mdd_controls_to_params.m).
// feedbackE2 < 0 means linked to FEEDBACK; SHIFT+FEEDBACK sets it and unlinks.
inline DigiParams DigiMap(float time, float feedback, float feedbackE2, float color, int ratio,
                          Config config) {
    auto unit = [](float v) { return std::clamp(Finite(v), 0.0f, 1.0f); };
    DigiParams p;
    p.config = config;
    const float d1 = std::round(DigiTimeMs(time) * kFs / 1000);
    p.delay[0] = d1;
    p.delay[1] = std::round(kDigiRatios[std::clamp(ratio, 0, 6)] * d1);
    const float k1 = 0.95f * unit(feedback);
    const float k2 = feedbackE2 < 0 ? k1 : 0.95f * unit(feedbackE2);
    const bool series = config == Config::Series;
    p.feedback[0] = series ? 1 - std::sqrt(1 - k1) : k1;
    p.feedback[1] = series ? 1 - std::sqrt(1 - k2) : k2;
    const float c = unit(color);
    p.highCutHz = c == 0 ? kGuardHz : kGuardHz * std::pow(2000 / kGuardHz, c);
    return p;
}

class DigiMono {
    static constexpr float kPi = 3.14159265358979323846f;
    float *data_ = nullptr;
    std::size_t size_ = 0, index_ = 0, delay_[2] = {1, 1};
    float feedback_[2] = {0, 0}, hpPole_ = 0, lpAlpha_ = 1;
    float hpIn_[2] = {0, 0}, hpOut_[2] = {0, 0}, lpOut_[2] = {0, 0};
    bool hpOn_ = false, lpOn_ = false;
    Config config_ = Config::Single;

  public:
    // Two histories of `size` floats each; maximum delay is size - 1 samples.
    void Attach(float *data, std::size_t size) {
        data_ = data;
        size_ = size;
    }
    void Reset() {
        if (data_)
            std::fill(data_, data_ + 2 * size_, 0.0f);
        index_ = 0;
        for (int e = 0; e < 2; ++e)
            hpIn_[e] = hpOut_[e] = lpOut_[e] = 0;
    }
    // Once per block (contract 7-8): a config change applies here and never clears histories.
    void Set(const DigiParams &p) {
        if (!data_ || size_ < 2)
            return;
        for (int e = 0; e < 2; ++e) {
            float d = Finite(p.delay[e]);
            d = std::clamp(d, 1.0f, static_cast<float>(size_ - 1));
            delay_[e] = static_cast<std::size_t>(d + 0.5f);
            delay_[e] = std::min(delay_[e], size_ - 1);
            feedback_[e] = std::clamp(Finite(p.feedback[e]), -0.999f, 0.999f);
        }
        const float lo = std::clamp(Finite(p.lowCutHz), 0.0f, kGuardHz);
        const float hi = std::isfinite(p.highCutHz) ? std::clamp(p.highCutHz, 0.0f, kGuardHz) : kGuardHz;
        hpOn_ = lo > 0;
        lpOn_ = hi < kGuardHz;
        if (hpOn_)
            hpPole_ = std::exp(-2.0f * kPi * lo / kFs);
        if (lpOn_)
            lpAlpha_ = 1.0f - std::exp(-2.0f * kPi * hi / kFs);
        config_ = p.config;
    }
    // Returns the wet sum g1*tap1 + g2*tap2; the caller applies Dry = 1, Wet = MIX.
    float Process(float x) {
        if (!data_ || size_ < 2)
            return 0;
        x = Finite(x);
        float tap[2];
        for (int e = 0; e < 2; ++e) {
            const float in = e == 0                      ? x
                             : config_ == Config::Single ? 0.0f
                             : config_ == Config::Series ? tap[0]
                                                         : x;
            float *h = data_ + e * size_;
            const float t = Finite(h[(index_ + size_ - delay_[e]) % size_]);
            float c = t;
            if (hpOn_) {
                const float o = hpPole_ * (hpOut_[e] + c - hpIn_[e]);
                hpIn_[e] = c;
                hpOut_[e] = o;
                c = o;
            }
            if (lpOn_) {
                lpOut_[e] += lpAlpha_ * (c - lpOut_[e]);
                c = lpOut_[e];
            }
            h[index_] = Finite(1.0f * in + feedback_[e] * c);
            tap[e] = t;
        }
        index_ = index_ + 1 == size_ ? 0 : index_ + 1;
        switch (config_) {
        case Config::Single: return tap[0];
        case Config::Series: return tap[0] + tap[1];
        default: return 0.5f * (tap[0] + tap[1]);
        }
    }
};
} // namespace poddemo
