/*
 * effect_clipper.cpp
 * Brent Edstrom, 2020
 */

#include <Arduino.h>
#include "effect_clipper.h"

  
void AudioEffectClipper::update()
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

  //Handle active mode
  block = receiveWritable();
  if (!block) return;

   for(int i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
   { 
       int32_t sample = block->data[i];
       
       //Clip samples if necessary
       if(sample > clip_point)
       {
          block->data[i] = clip_point;
       }else if (sample < -clip_point)
       {
          block->data[i] = -clip_point;
       }
       
   }

   transmit(block, 0);
   release(block);
}
