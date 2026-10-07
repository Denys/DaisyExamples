#pragma once
#include <algorithm>
#include <cmath>
#include <cstddef>
namespace poddemo {
enum class Config { Single, Series, Parallel };
inline float Finite(float x) { return std::isfinite(x) ? x : 0.0f; }
class DigiMono {
    float *data_ = nullptr;
    std::size_t size_ = 0, index_ = 0;
    float time_[2] = {101, 149}, target_[2] = {101, 149}, lp_[2] = {0, 0};
    float feedback_ = 0, alpha_ = 1, motion_ = 0, phase_ = 0;
    Config config_ = Config::Single;
    float Read(unsigned ch, float delay) const {
        delay = std::clamp(delay, 1.0f, static_cast<float>(size_ - 2));
        const auto whole = static_cast<std::size_t>(delay);
        const float frac = delay - static_cast<float>(whole);
        const auto a = (index_ + size_ - whole) % size_, b = (a + size_ - 1) % size_;
        const float *base = data_ + ch * size_;
        return base[a] + (base[b] - base[a]) * frac;
    }

  public:
    void Attach(float *data, std::size_t size) {
        data_ = data;
        size_ = size;
    }
    void Reset() {
        if (data_)
            std::fill(data_, data_ + 2 * size_, 0.0f);
        index_ = 0;
        lp_[0] = lp_[1] = phase_ = 0;
    }
    void Set(float t1, float t2, float feedback, float alpha, float motion, Config config,
             bool snap = false) {
        target_[0] = std::clamp(Finite(t1), 1.0f, static_cast<float>(size_ - 2));
        target_[1] = std::clamp(Finite(t2), 1.0f, static_cast<float>(size_ - 2));
        feedback_ = std::clamp(Finite(feedback), 0.0f, 0.90f);
        alpha_ = std::clamp(Finite(alpha), 0.0f, 1.0f);
        motion_ = std::clamp(Finite(motion), 0.0f, 240.0f);
        config_ = config;
        if (snap) {
            time_[0] = target_[0];
            time_[1] = target_[1];
        }
    }
    float Process(float input) {
        if (!data_ || size_ < 4)
            return 0;
        input = Finite(input);
        time_[0] += (target_[0] - time_[0]) * 0.00034716f;
        time_[1] += (target_[1] - time_[1]) * 0.00034716f;
        const float drift = motion_ == 0 ? 0 : motion_ * std::sin(phase_);
        phase_ += 0.00001309f;
        if (phase_ > 6.2831853f)
            phase_ -= 6.2831853f;
        const float a = Read(0, time_[0] + drift),
                    b = config_ == Config::Single ? 0 : Read(1, time_[1] + drift);
        lp_[0] += alpha_ * (a - lp_[0]);
        lp_[1] += alpha_ * (b - lp_[1]);
        data_[index_] = std::clamp(Finite(input + feedback_ * lp_[0]), -16.0f, 16.0f);
        if (config_ != Config::Single)
            data_[size_ + index_] =
                std::clamp(Finite((config_ == Config::Series ? a : input) + feedback_ * lp_[1]),
                           -16.0f, 16.0f);
        index_ = (index_ + 1) % size_;
        return config_ == Config::Single ? a : config_ == Config::Series ? b : 0.5f * (a + b);
    }
};
} // namespace poddemo
