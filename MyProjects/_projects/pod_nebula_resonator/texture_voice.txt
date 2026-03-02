#pragma once
#ifndef TEXTURE_VOICE_H
#define TEXTURE_VOICE_H

#include "daisysp.h"
#include "freeze_buffer.h"

#define NUM_GRAINS 4

class TextureVoice
{
  public:
    void Init(float sample_rate)
    {
        for(int i = 0; i < NUM_GRAINS; i++)
        {
            jitter_[i].Init(sample_rate);
            jitter_[i].SetCpsMin(0.5f);  // Slow Brownian drift
            jitter_[i].SetCpsMax(4.0f);  // Fast twitch
            jitter_[i].SetAmp(500.0f);   // +/- 500 samples range
        }
        // Base spacing to prevent phase cancellation
        tap_offsets_[0] = 0.0f;
        tap_offsets_[1] = 300.0f;
        tap_offsets_[2] = 600.0f;
        tap_offsets_[3] = 900.0f;
    }

    // Process one sample frame
    // scan_pos: 0.0-1.0 position in buffer
    // texture: 0.0-1.0 jitter intensity
    // outL, outR: stereo output pointers
    void Process(const FreezeBuffer& buf,
                 float               scan_pos,
                 float               texture,
                 float*              outL,
                 float*              outR)
    {
        float base_delay = scan_pos * FREEZE_BUFFER_SIZE_F;
        float accL = 0.0f;
        float accR = 0.0f;

        for(int i = 0; i < NUM_GRAINS; i++)
        {
            // Jittered delay time
            float jit_val     = jitter_[i].Process() * texture;
            float final_delay = base_delay + tap_offsets_[i] + jit_val;

            // Clamp to valid buffer range
            if(final_delay < 1.0f)
                final_delay = 1.0f;
            if(final_delay > FREEZE_BUFFER_SIZE_F - 1.0f)
                final_delay = FREEZE_BUFFER_SIZE_F - 1.0f;

            float sig = buf.Read(final_delay);

            // Stereo: evens -> Left, odds -> Right
            if(i % 2 == 0)
                accL += sig;
            else
                accR += sig;
        }

        // Normalize (2 taps per channel)
        *outL = accL * 0.5f;
        *outR = accR * 0.5f;
    }

  private:
    daisysp::Jitter jitter_[NUM_GRAINS];
    float           tap_offsets_[NUM_GRAINS];
};

#endif // TEXTURE_VOICE_H
