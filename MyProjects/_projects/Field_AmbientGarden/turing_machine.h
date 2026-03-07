#pragma once
#include <cstdint>

/**
 * TuringMachine - 16-bit shift register with probabilistic feedback.
 *
 * On each clock tick, the MSB feeds back to LSB with a configurable
 * probability of being flipped. Output is the lower 8 bits (0-255).
 *
 * Probability behavior:
 *   0.0 or 1.0 = locked repeating pattern (deterministic)
 *   0.5         = maximum randomness (chaotic)
 */
class TuringMachine
{
  public:
    void Init()
    {
        register_ = 0xACE1; // Non-zero seed
        probability_ = 0.5f;
        length_ = 16;
    }

    /** Process one clock tick. Returns 8-bit output (0-255). */
    uint8_t Process()
    {
        uint16_t msb = (register_ >> (length_ - 1)) & 1;

        static uint32_t s = 0xACE10001u;
        s ^= s << 13; s ^= s >> 17; s ^= s << 5;
        float r = static_cast<float>(s) * 2.3283064e-10f;
        if(r < probability_)
        {
            msb ^= 1;
        }

        register_ = ((register_ << 1) | msb) & ((1 << length_) - 1);

        return register_ & 0xFF;
    }

    /** Set flip probability (0.0-1.0). 0.5 = max chaos, 0/1 = locked loop. */
    void SetProbability(float p)
    {
        probability_ = (p < 0.0f) ? 0.0f : (p > 1.0f) ? 1.0f : p;
    }

    float GetProbability() const { return probability_; }

    /** Set register length (8 or 16 bits). */
    void SetLength(uint8_t len) { length_ = (len <= 8) ? 8 : 16; }

    /** Get current register state (for display visualization). */
    uint16_t GetRegister() const { return register_; }

  private:
    uint16_t register_;
    float    probability_;
    uint8_t  length_;
};
