#pragma once
#ifndef FREEZE_BUFFER_H
#define FREEZE_BUFFER_H

#include <cstddef>
#include <cstdint>
#include <cmath>

// 2 seconds at 48kHz
#define FREEZE_BUFFER_SIZE 96000
static constexpr float FREEZE_BUFFER_SIZE_F = 96000.0f;

class FreezeBuffer
{
  public:
    void Init()
    {
        write_pos_ = 0;
        is_frozen_  = false;
        // Pre-fill with a low sawtooth so instrument sounds immediately
        float phase = 0.0f;
        float inc   = 110.0f / 48000.0f; // A2 at 48kHz
        for(size_t i = 0; i < FREEZE_BUFFER_SIZE; i++)
        {
            data_[i] = (phase * 2.0f - 1.0f) * 0.3f;
            phase += inc;
            if(phase >= 1.0f)
                phase -= 1.0f;
        }
    }

    void SetFreeze(bool state) { is_frozen_ = state; }
    bool IsFrozen() const { return is_frozen_; }

    // Conditional write — bypassed when frozen
    void Write(float sample)
    {
        if(!is_frozen_)
        {
            data_[write_pos_] = sample;
            write_pos_++;
            if(write_pos_ >= FREEZE_BUFFER_SIZE)
                write_pos_ = 0;
        }
    }

    // Fractional read with linear interpolation
    // delay_samples: how far back from write head to read
    float Read(float delay_samples) const
    {
        float read_f = static_cast<float>(write_pos_) - delay_samples;

        // Wrap into valid range (delay clamped to [1, SIZE-1] by callers,
        // so read_f in [-(SIZE-1), SIZE-2] — one add suffices, no upper wrap)
        if(read_f < 0.0f)
            read_f += FREEZE_BUFFER_SIZE_F;

        // Linear interpolation
        int32_t idx0 = static_cast<int32_t>(read_f);
        float   frac = read_f - static_cast<float>(idx0);
        int32_t idx1 = idx0 + 1;
        if(idx1 >= FREEZE_BUFFER_SIZE)
            idx1 = 0;

        return data_[idx0] + frac * (data_[idx1] - data_[idx0]);
    }

    // Absolute position read (0.0 - 1.0 maps to full buffer)
    float ReadAbsolute(float position_0to1) const
    {
        float delay = position_0to1 * FREEZE_BUFFER_SIZE_F;
        if(delay < 1.0f)
            delay = 1.0f;
        if(delay > FREEZE_BUFFER_SIZE - 1)
            delay = static_cast<float>(FREEZE_BUFFER_SIZE - 1);
        return Read(delay);
    }

    size_t GetWritePos() const { return write_pos_; }

  private:
    float  data_[FREEZE_BUFFER_SIZE];
    size_t write_pos_;
    bool   is_frozen_;
};

#endif // FREEZE_BUFFER_H
