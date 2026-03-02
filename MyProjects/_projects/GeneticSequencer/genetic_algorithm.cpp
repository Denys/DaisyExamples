#include "genetic_algorithm.h"
#include <cmath>
#include <algorithm>

namespace ga
{

GeneticAlgorithm::GeneticAlgorithm()
: historyIndex_(0), bestIndex_(0), generation_(0)
{
}

void GeneticAlgorithm::Init()
{
    RandomizePopulation();
}

void GeneticAlgorithm::RandomizePopulation()
{
    for(uint8_t i = 0; i < params_.populationSize; i++)
    {
        for(uint8_t s = 0; s < SEQUENCE_LENGTH; s++)
        {
            population_[i].sequence[s].note
                = params_.noteRangeLow
                  + RandomByte(params_.noteRangeHigh - params_.noteRangeLow);
            population_[i].sequence[s].velocity = 80 + RandomByte(48);
            population_[i].sequence[s].gate     = 30 + RandomByte(70);
            population_[i].sequence[s].active
                = RandomFloat() < params_.targetDensity;
        }
        population_[i].fitness = EvaluateFitness(population_[i]);
    }

    // Find best
    bestIndex_ = 0;
    for(uint8_t i = 1; i < params_.populationSize; i++)
    {
        if(population_[i].fitness > population_[bestIndex_].fitness)
        {
            bestIndex_ = i;
        }
    }

    generation_ = 0;
}

void GeneticAlgorithm::SeedFromMidi(const uint8_t* notes, uint8_t count)
{
    // Create seed individual from MIDI input
    Individual seed;
    for(uint8_t i = 0; i < SEQUENCE_LENGTH; i++)
    {
        if(i < count)
        {
            seed.sequence[i].note     = notes[i];
            seed.sequence[i].velocity = 100;
            seed.sequence[i].gate     = 50;
            seed.sequence[i].active   = true;
        }
        else
        {
            seed.sequence[i].active = false;
        }
    }
    seed.fitness = EvaluateFitness(seed);

    // Initialize population with variations of seed
    population_[0] = seed;
    for(uint8_t i = 1; i < params_.populationSize; i++)
    {
        population_[i] = seed;
        // Apply mutations to create variation
        for(int m = 0; m < 3; m++)
        {
            Mutate(population_[i]);
        }
        population_[i].fitness = EvaluateFitness(population_[i]);
    }

    bestIndex_  = 0;
    generation_ = 0;
}

void GeneticAlgorithm::Evolve()
{
    // Save history for undo
    for(uint8_t i = 0; i < params_.populationSize; i++)
    {
        history_[historyIndex_][i] = population_[i];
    }
    historyIndex_ = (historyIndex_ + 1) % HISTORY_SIZE;

    // Create next generation
    Individual nextGen[MAX_POPULATION];

    // Elitism: keep best 2
    nextGen[0]         = population_[bestIndex_];
    uint8_t secondBest = (bestIndex_ == 0) ? 1 : 0;
    for(uint8_t i = 0; i < params_.populationSize; i++)
    {
        if(i != bestIndex_
           && population_[i].fitness > population_[secondBest].fitness)
        {
            secondBest = i;
        }
    }
    nextGen[1] = population_[secondBest];

    // Fill rest with offspring
    for(uint8_t i = 2; i < params_.populationSize; i++)
    {
        uint8_t p1, p2;
        Selection(p1, p2);

        if(RandomFloat() < params_.crossoverRate)
        {
            Crossover(population_[p1], population_[p2], nextGen[i]);
        }
        else
        {
            nextGen[i]
                = (RandomFloat() < 0.5f) ? population_[p1] : population_[p2];
        }

        if(RandomFloat() < params_.mutationRate)
        {
            Mutate(nextGen[i]);
        }

        nextGen[i].fitness = EvaluateFitness(nextGen[i]);
    }

    // Copy next generation
    for(uint8_t i = 0; i < params_.populationSize; i++)
    {
        population_[i] = nextGen[i];
    }

    // Find new best
    bestIndex_ = 0;
    for(uint8_t i = 1; i < params_.populationSize; i++)
    {
        if(population_[i].fitness > population_[bestIndex_].fitness)
        {
            bestIndex_ = i;
        }
    }

    generation_++;
}

float GeneticAlgorithm::EvaluateFitness(const Individual& ind)
{
    float fitness = 0.0f;

    // 1. Note density score (target ~60% active steps)
    uint8_t activeCount = 0;
    for(uint8_t i = 0; i < SEQUENCE_LENGTH; i++)
    {
        if(ind.sequence[i].active)
            activeCount++;
    }
    float density      = static_cast<float>(activeCount) / SEQUENCE_LENGTH;
    float densityScore = 1.0f - fabsf(density - params_.targetDensity);
    fitness += densityScore * 0.3f;

    // 2. Melodic contour score (penalize large intervals)
    float   contourScore = 0.0f;
    uint8_t lastNote     = 0;
    bool    foundFirst   = false;
    for(uint8_t i = 0; i < SEQUENCE_LENGTH; i++)
    {
        if(ind.sequence[i].active)
        {
            if(foundFirst)
            {
                int interval
                    = abs(static_cast<int>(ind.sequence[i].note) - lastNote);
                // Stepwise motion (1-2 semitones) is best
                if(interval <= 2)
                    contourScore += 1.0f;
                else if(interval <= 5)
                    contourScore += 0.5f;
                else if(interval <= 7)
                    contourScore += 0.2f;
                // Large jumps penalized
            }
            lastNote   = ind.sequence[i].note;
            foundFirst = true;
        }
    }
    if(activeCount > 1)
    {
        contourScore /= (activeCount - 1);
    }
    fitness += contourScore * 0.4f;

    // 3. Note range score (stay within specified octaves)
    float rangeScore = 1.0f;
    for(uint8_t i = 0; i < SEQUENCE_LENGTH; i++)
    {
        if(ind.sequence[i].active)
        {
            if(ind.sequence[i].note < params_.noteRangeLow
               || ind.sequence[i].note > params_.noteRangeHigh)
            {
                rangeScore -= 0.1f;
            }
        }
    }
    if(rangeScore < 0)
        rangeScore = 0;
    fitness += rangeScore * 0.3f;

    return fitness;
}

void GeneticAlgorithm::Selection(uint8_t& parent1, uint8_t& parent2)
{
    // Tournament selection (k=3)
    auto tournament = [this]() -> uint8_t
    {
        uint8_t best = RandomByte(params_.populationSize);
        for(int i = 0; i < 2; i++)
        {
            uint8_t candidate = RandomByte(params_.populationSize);
            if(population_[candidate].fitness > population_[best].fitness)
            {
                best = candidate;
            }
        }
        return best;
    };

    parent1 = tournament();
    do
    {
        parent2 = tournament();
    } while(parent2 == parent1 && params_.populationSize > 1);
}

void GeneticAlgorithm::Crossover(const Individual& p1,
                                 const Individual& p2,
                                 Individual&       child)
{
    // Single-point crossover
    uint8_t crossPoint = 1 + RandomByte(SEQUENCE_LENGTH - 2);

    for(uint8_t i = 0; i < SEQUENCE_LENGTH; i++)
    {
        if(i < crossPoint)
        {
            child.sequence[i] = p1.sequence[i];
        }
        else
        {
            child.sequence[i] = p2.sequence[i];
        }
    }
}

void GeneticAlgorithm::Mutate(Individual& ind)
{
    // Mutate a random step
    uint8_t step = RandomByte(SEQUENCE_LENGTH);

    switch(RandomByte(4))
    {
        case 0: // Change note
            ind.sequence[step].note
                = params_.noteRangeLow
                  + RandomByte(params_.noteRangeHigh - params_.noteRangeLow);
            break;
        case 1: // Toggle active
            ind.sequence[step].active = !ind.sequence[step].active;
            break;
        case 2: // Change velocity
            ind.sequence[step].velocity = 60 + RandomByte(68);
            break;
        case 3: // Change gate
            ind.sequence[step].gate = 20 + RandomByte(80);
            break;
    }
}

void GeneticAlgorithm::UndoGeneration()
{
    if(generation_ == 0)
        return;

    uint8_t prevIndex
        = (historyIndex_ == 0) ? HISTORY_SIZE - 1 : historyIndex_ - 1;
    for(uint8_t i = 0; i < params_.populationSize; i++)
    {
        population_[i] = history_[prevIndex][i];
    }
    historyIndex_ = prevIndex;
    generation_--;

    // Recalculate best
    bestIndex_ = 0;
    for(uint8_t i = 1; i < params_.populationSize; i++)
    {
        if(population_[i].fitness > population_[bestIndex_].fitness)
        {
            bestIndex_ = i;
        }
    }
}

const Individual& GeneticAlgorithm::GetIndividual(uint8_t index) const
{
    if(index >= params_.populationSize)
        index = 0;
    return population_[index];
}

} // namespace ga
