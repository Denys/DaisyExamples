/*
 * Simple tremolo effect
 * Brent Edstrom, 2020
 */

#ifndef __TREMOLO
#define __TREMOLO

#include <Arduino.h>
#include <AudioStream.h>
#include "lfo.h"

class AudioEffectTremolo: public AudioStream
{
  bool pass;
  LFO lfo;
  uint8_t amplitude;
  
  public: 
  AudioEffectTremolo() : AudioStream(1, inputQueueArray) 
  {
     pass = false;
     lfo.setSampleRate(AUDIO_SAMPLE_RATE);
     amplitude = 255;
  }

  void passthrough(boolean p);
  void setFrequency(float freq);
  void setAmplitude(float amp);
  
  virtual void update();
  private:
  audio_block_t *inputQueueArray[1];
  
};

#endif
