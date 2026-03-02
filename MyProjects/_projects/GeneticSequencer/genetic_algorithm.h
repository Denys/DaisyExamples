#pragma once
#ifndef GENETIC_ALGORITHM_H
#define GENETIC_ALGORITHM_H

#include <cstdint>
#include <cstdlib>

namespace ga
{

// Configuration
constexpr uint8_t SEQUENCE_LENGTH = 16;
constexpr uint8_t MAX_POPULATION  = 50;
constexpr uint8_t HISTORY_SIZE    = 10;

// Gene: Single step in a sequence
struct Gene
{
    uint8_t note;     // MIDI note (0-127)
    uint8_t velocity; // 0-127
    uint8_t gate;     // Gate length % (0-100)
    bool    active;   // Step on/off

    Gene() : note(60), velocity(100), gate(50), active(true) {}
};

// Individual: A complete sequence
struct Individual
{
    Gene  sequence[SEQUENCE_LENGTH];
    float fitness;

    Individual() : fitness(0.0f) {}
};

// GA Parameters (controllable via knobs)
struct Parameters
{
    float   mutationRate;   // 0.0 - 0.5 (K1)
    float   crossoverRate;  // 0.5 - 1.0 (K2)
    uint8_t populationSize; // 10-50 (K3)
    float   contourBias;    // -1.0 to +1.0 (K4)
    float   targetDensity;  // 0.25 - 1.0 (K5)
    uint8_t noteRangeLow;   // MIDI note
    uint8_t noteRangeHigh;  // MIDI note

    Parameters()
    : mutationRate(0.15f),
      crossoverRate(0.8f),
      populationSize(20),
      contourBias(0.0f),
      targetDensity(0.6f),
      noteRangeLow(48),
      noteRangeHigh(84)
    {
    }
};

// Genetic Algorithm Engine
class GeneticAlgorithm
{
  public:
    GeneticAlgorithm();

    void Init();
    void RandomizePopulation();
    void Evolve();
    void SeedFromMidi(const uint8_t* notes, uint8_t count);

    // Accessors
    const Individual& GetBest() const { return population_[bestIndex_]; }
    const Individual& GetIndividual(uint8_t index) const;
    uint32_t          GetGeneration() const { return generation_; }
    float GetBestFitness() const { return population_[bestIndex_].fitness; }

    // Parameters
    Parameters& GetParams() { return params_; }
    void        SetParams(const Parameters& p) { params_ = p; }

    // History
    void UndoGeneration();

  private:
    // Core GA operations
    float EvaluateFitness(const Individual& ind);
    void  Selection(uint8_t& parent1, uint8_t& parent2);
    void
    Crossover(const Individual& p1, const Individual& p2, Individual& child);
    void Mutate(Individual& ind);

    // Population
    Individual population_[MAX_POPULATION];
    Individual history_[HISTORY_SIZE][MAX_POPULATION];
    uint8_t    historyIndex_;
    uint8_t    bestIndex_;
    uint32_t   generation_;

    // Parameters
    Parameters params_;

    // Random helpers
    float   RandomFloat() { return static_cast<float>(rand()) / RAND_MAX; }
    uint8_t RandomByte(uint8_t max) { return rand() % max; }
};

} // namespace ga

#endif // GENETIC_ALGORITHM_H
