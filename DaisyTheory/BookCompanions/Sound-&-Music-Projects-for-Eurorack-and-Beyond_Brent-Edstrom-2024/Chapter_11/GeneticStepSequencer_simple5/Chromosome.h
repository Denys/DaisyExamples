/*
 *  Chromosome.h
 *  Brent Edstrom, 2020
 *  
 *  Chromosome: 
 *    3-bits: note index
 *    5-bits: function ~68% chance of no change, 3% chance of permanent mutation
 *    4-bits: dynamic (16 dynamic levels)
 *    1-bit:  on/off
 *    3-bits: transposition
 */

#ifndef __CHROMOSOME
#define __CHROMOSOME

#include <Arduino.h>

struct Chromosome
{
    uint16_t chromosome;

    //Function enumeration
    enum{octave_up, octave_down, fifth_up, fourth_down, 
         upper_neighbor, lower_neighbor, transpose_up, 
         transpose_down, mutate, permanent_mutation};
    
    //Default constructor
    Chromosome();

    //Setters
    void setNoteIndex(const uint16_t index);
    void setFunction(uint16_t f);
    void setDynamic(uint16_t dynamic);
    void setNoteOnStatus(bool on); 

    //Getters
    bool noteOn();
    int getTransposition();
    uint16_t getNoteIndex();
    uint16_t getFunction();
    uint16_t getDynamic();  
    int getFitness();
    
};

#endif
