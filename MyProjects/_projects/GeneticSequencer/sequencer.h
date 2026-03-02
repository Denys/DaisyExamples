#pragma once
#ifndef SEQUENCER_H
#define SEQUENCER_H

#include "genetic_algorithm.h"

namespace seq
{

// Sequencer state
enum class State
{
    Stopped,
    Playing,
    Paused
};

// Sequencer engine
class Sequencer
{
  public:
    Sequencer();

    void Init(float sampleRate);
    void Process(); // Call in audio callback

    // Transport
    void  Play();
    void  Stop();
    void  Pause();
    State GetState() const { return state_; }

    // Tempo
    void  SetTempo(float bpm);
    float GetTempo() const { return tempo_; }
    void  SetSwing(float swing); // 0.0 - 0.5

    // Sequence
    void    SetSequence(const ga::Individual& ind);
    uint8_t GetCurrentStep() const { return currentStep_; }

    // Note output (polled by main loop)
    bool    HasNewNote() const { return hasNewNote_; }
    uint8_t GetNoteNumber() const { return currentNote_; }
    uint8_t GetNoteVelocity() const { return currentVelocity_; }
    float   GetGateLength() const { return currentGate_; }
    void    ClearNewNote() { hasNewNote_ = false; }

    // Gate state
    bool IsGateOpen() const { return gateOpen_; }

  private:
    // Timing
    float    sampleRate_;
    float    tempo_;
    float    swing_;
    uint32_t samplesPerStep_;
    uint32_t sampleCounter_;

    // State
    State    state_;
    uint8_t  currentStep_;
    bool     gateOpen_;
    uint32_t gateCounter_;
    uint32_t gateSamples_;

    // Current sequence
    ga::Gene sequence_[ga::SEQUENCE_LENGTH];

    // Note output
    bool    hasNewNote_;
    uint8_t currentNote_;
    uint8_t currentVelocity_;
    float   currentGate_;

    // Helpers
    void     AdvanceStep();
    void     TriggerNote();
    uint32_t CalculateSamplesPerStep();
};

} // namespace seq

#endif // SEQUENCER_H
