#include "lfo.h"

//Default constructor
LFO::LFO()
{
   //Initialize variables to reasonable defaults
   init();
}
    
//Initialize variables  
void LFO::init()
{
    m_sampleRate = AUDIO_SAMPLE_RATE;
    m_frequency = 1; 
    m_maxAmplitude = 255;
    m_increment = 1;
    m_accumulator = 0;
}

//Set the sample rate
void LFO::setSampleRate(uint16_t sample_rate)
{
     m_sampleRate = sample_rate; 
}


//Set the frequency of the oscillator
void LFO::setFrequency(float freq)
{
     m_frequency = freq; 
     if(m_frequency == 0)
     {
         m_frequency = 1;  //Avoid divide by zero
     }
     
     //Calculate increment
     m_increment = calculateIncrement();
     if(m_increment == 0)
     {
        m_increment = 1; 
     } 
}
