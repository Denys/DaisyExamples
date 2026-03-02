#pragma once
#include <cstdint>

// Abstract voice interface for modular synth voices.
//
// To add a new voice:
//   1. Create new_voice.h/.cpp implementing this interface
//   2. #include "new_voice.h" in pod_fuzz_distortion.cpp
//   3. Change the voice type instantiation (FmVoice -> NewVoice)
//   4. Add new_voice.cpp to Makefile CPP_SOURCES

class Voice
{
  public:
    virtual ~Voice() {}
    virtual void  Init(float sample_rate)                    = 0;
    virtual void  NoteOn(uint8_t note, uint8_t velocity)     = 0;
    virtual void  NoteOff(uint8_t note)                      = 0;
    virtual float Process()                                  = 0;
    // param_id: 0 = Knob1 mapped param, 1 = Knob2 mapped param (0.0-1.0)
    virtual void  SetParam(int param_id, float value)        = 0;
    virtual int   GetActiveCount() const                     = 0;
};
