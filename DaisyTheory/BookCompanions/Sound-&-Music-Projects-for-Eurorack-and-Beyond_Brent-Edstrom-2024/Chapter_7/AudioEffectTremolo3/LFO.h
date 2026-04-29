/******************************************************************* 
   lfo class
   Brent Edstrom, 2020
   
   FILE: lfo.h
   
   DESCRIPTION: A simple fixed-point LFO
   
*******************************************************************/

#ifndef __OSCILLATORBASE
#define __OSCILLATORBASE

#include <Arduino.h>

const uint32_t max_32 = 4294967295L;

class LFO
{    
    protected:
    //MEMBER VARIABLES:
    uint16_t m_sampleRate;               //The sample rate (e.g. 16384)
    float m_frequency;                   //Frequency of the oscillator
    uint16_t m_maxAmplitude;             //Max amplitude 8 bit = 255 16 bit = 65535
    volatile uint32_t m_accumulator;     //32-bit accumulator (produces a ramp wave)
    volatile uint32_t m_triAccumulator;  //Triangle output based on ramp wave
    volatile uint32_t m_increment;       //Value used to increment the accumulator
        
    public:
    LFO();
    
    //GETTERS AND SETTERS
    virtual void setFrequency(float freq);    //Sets current frequency
    void setSampleRate(uint16_t sample_rate); //Sets sample rate
     
    //HELPER METHODS
    void init();                        //Initializes member variables
        
    //Calculates the time "slice" of the frequency control word based on the 
    //values of m_frequency,  m_sampleRate, and the register length of m_increment
    inline
    uint32_t calculateIncrement()
    {   
        return (uint32_t)((m_frequency * max_32 + m_sampleRate/2)/m_sampleRate);
    }

  
    /*Call this method each time the timer fires. Declared "inline" to 
     optimize speed. Returns 8-bit real-number portion of accumulator value. 
     Override in subclasses.
     */
    inline virtual uint8_t tick()
    {
         m_accumulator += m_increment;
        
         //Use left shift to compare the accumulator against half
         //the full amplitude (in 32 bits)
         if(m_accumulator < max_32 >>1 )
         {
             //Use left shift to multiply the accumulator by two if we
             //are less than 1/2 the maximum amplitude 
             m_triAccumulator = m_accumulator <<1;
         }else{
             //If we are over the halfway point, subract the accumulator
             //from maximum amplitude and use left shift to multiply by two. 
             m_triAccumulator = (max_32 - m_accumulator) <<1;
         }
         return m_triAccumulator >>24;
    }
};

#endif
