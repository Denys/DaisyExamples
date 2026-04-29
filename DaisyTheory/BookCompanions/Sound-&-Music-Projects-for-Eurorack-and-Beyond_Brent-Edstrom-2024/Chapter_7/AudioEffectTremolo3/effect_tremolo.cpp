/*
 * effect_tremolo.cpp
 * Brent Edstrom, 2020
 */

#include <Arduino.h>
#include "effect_tremolo.h"
  
void AudioEffectTremolo::passthrough(boolean p)
{
     pass = p;
}

void AudioEffectTremolo::setFrequency(float freq)
{
     lfo.setFrequency(freq);
}

void AudioEffectTremolo::setAmplitude(float amp)
{
    //Scale amplitude to an 8-bit value
    amplitude = amp * 255.0;
}

void AudioEffectTremolo::update()
{
  audio_block_t *block;

  //Handle passthrough
  if(pass)
  {
     block = receiveReadOnly();
     if (!block) return;
     transmit(block);
     release(block);
     return;
  }

  //Retrieve sample block
  block = receiveWritable();
  if (!block)return;

  //Loop through sample block
  for(int i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
  { 
   //Retrieve values from the LFO & sample block
      uint8_t scale = lfo.tick();
      uint32_t sample = block->data[i];

      //Alter level by multiplying the sample by scale and amplitude
      uint32_t processed_sample = sample * scale * amplitude;

      //Divide by 65536 to scale back down to 16-bit range
      block->data[i] = (processed_sample >> 16);
  }


   transmit(block, 0);
   release(block);
}
