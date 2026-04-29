
#include "Genotype.h"

Genotype::Genotype(): activeStep(0)
{
    for(int i = 0; i < numChromosomes; i++)
    {
        //Assign each chromosome an index to the lookup table
        chromosomes[i].setNoteIndex(i);

        //Randomize note-On status
        if(random(0, 2) == 1)
        {
            chromosomes[i].setNoteOnStatus(true);
        }else{
            chromosomes[i].setNoteOnStatus(false);
        }

        //Randomize dynamics and function
        chromosomes[i].setDynamic(random(1, 15));
        chromosomes[i].setFunction(random(0, 32));
    }
}

MIDINote Genotype::nextNote(bool ignore_note_off)
{
    static int counter = 0;
    MIDINote n;

    //Prepare a MIDI note based on the active chromosome
    int index = chromosomes[activeStep].getNoteIndex();
    n.note = noteArray[index];
    n.channel = 1;
    n.velocity = chromosomes[activeStep].getDynamic();

    //Get chromosome function
    int function = chromosomes[activeStep].getFunction();

    //Apply function to MIDI note
    applyFunction(function, n);
    
    //Handle velocity when notes are off
    if(chromosomes[activeStep].noteOn() == false && ignore_note_off ==false)
    {
        n.velocity = 0;
    }

    //Move to next step and check range
    activeStep++;
    if(activeStep >=8)
    {
        activeStep = 0;
    }

    int best_index;
    int second_best_index;
    int least_index;

    //Evaluate chromosomes and get indices to most- and least-fit individuals
    evaluateChromosomes(best_index, second_best_index, least_index);

    //Determine crossover point
    int crossover_point = random(1, 16);

    //Do the crossover
    crossover(chromosomes[best_index].chromosome, 
                          chromosomes[second_best_index].chromosome, 
                          crossover_point);

    //Mutate the function of the best chromosome
    chromosomes[best_index].setFunction(random(0, 32));


    //Force a more extreme randomized mutation every 50 iterations
    counter++;
    if(counter >= 50)
    {
        counter = 0;
        chromosomes[activeStep].setFunction(random(0, 32));
        toggleBit(chromosomes[best_index].chromosome, random(0, 16));
        invert(chromosomes[activeStep].chromosome);
    }

    //Return the MIDI note
    return n;
}

void Genotype::evaluateChromosomes(int &best_index, int &second_best_index, 
                                   int &least_index)
{
    int best = 0;
    int second_best = 0;
    int least = 0;
    best_index = 0;
    second_best_index = 0;
    least_index = 100;
    
    //Find top two chromosomes and least-fit chromosome
    for(int i = 0; i < numChromosomes; i++)
    {
        int fitness = chromosomes[i].getFitness();
        if(fitness > best)
        {
            best = fitness;
            best_index = i;
        }else if(fitness > second_best)
        {
            second_best = fitness;
            second_best_index = i;
        }
        if(fitness < least)
        {
            least = fitness;
            least_index = i;
        }
    }
}

void Genotype::applyFunction(int function, MIDINote &n)
{
    if(function == Chromosome::octave_up)
    {
        n.note += 12;
        
    }else if(function == Chromosome::octave_down)
    {
        n.note -= 12;
        
    }else if(function == Chromosome::upper_neighbor)
    {
        n.note +=2;
        
    }
    else if(function == Chromosome::lower_neighbor)
    {
        n.note -= 2;
        
    }else if(function == Chromosome::fifth_up)
    {
        n.note +=7;
        
    }else if(function == Chromosome::fourth_down)
    {
        n.note -= 5;
        
    }else if(function == Chromosome::transpose_up)
    {
        n.note += chromosomes[activeStep].getTransposition();
           
    }else if(function == Chromosome::transpose_down)
    {
        n.note -= chromosomes[activeStep].getTransposition();
        
    }else if(function == Chromosome::mutate)
    {
        toggleBit(chromosomes[activeStep].chromosome, random(0, 16));
        n.note = noteArray[activeStep];
    }else if(function == Chromosome::permanent_mutation)
    {
        noteArray[activeStep] = noteArray[activeStep] + 5;
        n.note = noteArray[activeStep];
    }
}
