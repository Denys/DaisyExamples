/*
 * Simple clipper distortion effect
 * Brent Edstrom, 2020
 */

 #ifndef __CLIPPER
 #define __CLIPPER

#include <AudioStream.h>

class AudioEffectClipper: public AudioStream
{
  bool pass;
  int16_t clip_point; 

  public: 
  AudioEffectClipper() : AudioStream(1, inputQueueArray) 
  {
     pass = false;
     setClipPoint(0.90);
  }

  void passthrough(boolean p)
  {
      pass = p;
  }
  
  void setClipPoint(float cp)
  {
      //Clip point represents a fractional value of 
      //maximum absolute 16-bit dynamic level
      clip_point = cp * 32767.0;
  }
  
  virtual void update();
  private:
  audio_block_t *inputQueueArray[1];
  
};

 #endif
