/*
 * Chromosome.cpp
 * Brent Edstrom, 2020
 */

#include "bit_tools.h"
#include "Chromosome.h"

Chromosome::Chromosome()
{
    //Randomize on construction
    chromosome = random(65535);
}

bool Chromosome::noteOn()
{
    if(isBitSet(chromosome, 12))
    {
        return true;
    }
    return false;
}

int Chromosome::getTransposition()
{
    return getValue(chromosome, 13, 15);
}

void Chromosome::setNoteIndex(const uint16_t index)
{
    for(int i = 0; i <= 2; i++)
    {
        if(isBitSet(index, i))
        {
            setBit(chromosome, i);
        }else{
            clearBit(chromosome, i);
        }
    }
}

void Chromosome::setNoteOnStatus(bool on)
{
    if(on)
    {
      setBit(chromosome, 12);
    }else{
      clearBit(chromosome, 12);
    }
}

uint16_t Chromosome::getNoteIndex()
{
    return getValue(chromosome, 0, 2);  
}

uint16_t Chromosome::getFunction()
{
    return getValue(chromosome, 3, 7);
}

void Chromosome::setFunction(uint16_t f)
{
    for(int i = 3; i <= 7; i++)
    {
        if(isBitSet(f, i))
        {
            setBit(chromosome, i);
        }else{
            clearBit(chromosome, i);
        }
    }
}

void Chromosome::setDynamic(uint16_t dynamic)
{
    for(int i = 8; i <= 11; i++)
    {
        if(isBitSet(dynamic, i))
        {
            setBit(chromosome, i);
        }else{
            clearBit(chromosome, i);
        }
    }
}

uint16_t Chromosome::getDynamic()
{
    uint16_t dynamic = getValue(chromosome, 8, 11);
    //Use an offset so MIDI velocity is at least 64 (~mezzo piano)
    return 64 + (dynamic * 4);
}


int Chromosome::getFitness()
{
    return getDynamic();
}
