/*
 *  Genotype for genetic step sequencer
 *  Brent Edstrom, 2020
 */

#ifndef __GENOTYPE
#define __GENOTYPE

#include "bit_tools.h"
#include "Chromosome.h"

struct MIDINote
{
    byte channel;
    byte note;
    byte velocity;

    MIDINote(): channel(1), note(60), velocity(100){};
};

struct Genotype
{
    enum{numChromosomes = 8};
    
    Chromosome chromosomes[numChromosomes];
    int activeStep;
        
    //Default to "sussy" palette of notes
    uint8_t noteArray[numChromosomes] = {55, 60, 62, 65, 67, 65, 69, 67};

    //Default constructor
    Genotype();

    MIDINote nextNote(bool ignore_note_off);
    void evaluateChromosomes(int &best_index, int &second_best_index, 
                             int &least_index);
    void applyFunction(int function, MIDINote &n);
};

#endif
