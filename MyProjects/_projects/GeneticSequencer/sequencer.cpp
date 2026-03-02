#include "sequencer.h"

namespace seq
{

Sequencer::Sequencer()
: sampleRate_(48000.0f),
  tempo_(120.0f),
  swing_(0.0f),
  samplesPerStep_(0),
  sampleCounter_(0),
  state_(State::Stopped),
  currentStep_(0),
  gateOpen_(false),
  gateCounter_(0),
  gateSamples_(0),
  hasNewNote_(false),
  currentNote_(60),
  currentVelocity_(100),
  currentGate_(0.5f)
{
}

void Sequencer::Init(float sampleRate)
{
    sampleRate_     = sampleRate;
    samplesPerStep_ = CalculateSamplesPerStep();
    sampleCounter_  = 0;
    currentStep_    = 0;
    gateOpen_       = false;

    // Initialize with empty sequence
    for(uint8_t i = 0; i < ga::SEQUENCE_LENGTH; i++)
    {
        sequence_[i] = ga::Gene();
    }
}

void Sequencer::Process()
{
    if(state_ != State::Playing)
        return;

    // Gate timing
    if(gateOpen_)
    {
        gateCounter_++;
        if(gateCounter_ >= gateSamples_)
        {
            gateOpen_ = false;
        }
    }

    // Step timing
    sampleCounter_++;
    if(sampleCounter_ >= samplesPerStep_)
    {
        sampleCounter_ = 0;
        AdvanceStep();
    }
}

void Sequencer::Play()
{
    if(state_ == State::Stopped)
    {
        currentStep_   = 0;
        sampleCounter_ = 0;
        TriggerNote();
    }
    state_ = State::Playing;
}

void Sequencer::Stop()
{
    state_         = State::Stopped;
    currentStep_   = 0;
    sampleCounter_ = 0;
    gateOpen_      = false;
}

void Sequencer::Pause()
{
    if(state_ == State::Playing)
    {
        state_ = State::Paused;
    }
    else if(state_ == State::Paused)
    {
        state_ = State::Playing;
    }
}

void Sequencer::SetTempo(float bpm)
{
    if(bpm < 20.0f)
        bpm = 20.0f;
    if(bpm > 300.0f)
        bpm = 300.0f;
    tempo_          = bpm;
    samplesPerStep_ = CalculateSamplesPerStep();
}

void Sequencer::SetSwing(float swing)
{
    if(swing < 0.0f)
        swing = 0.0f;
    if(swing > 0.5f)
        swing = 0.5f;
    swing_ = swing;
}

void Sequencer::SetSequence(const ga::Individual& ind)
{
    for(uint8_t i = 0; i < ga::SEQUENCE_LENGTH; i++)
    {
        sequence_[i] = ind.sequence[i];
    }
}

void Sequencer::AdvanceStep()
{
    currentStep_ = (currentStep_ + 1) % ga::SEQUENCE_LENGTH;
    TriggerNote();
}

void Sequencer::TriggerNote()
{
    const ga::Gene& gene = sequence_[currentStep_];

    if(gene.active)
    {
        currentNote_     = gene.note;
        currentVelocity_ = gene.velocity;
        currentGate_     = static_cast<float>(gene.gate) / 100.0f;
        hasNewNote_      = true;

        // Open gate
        gateOpen_    = true;
        gateCounter_ = 0;
        gateSamples_ = static_cast<uint32_t>(samplesPerStep_ * currentGate_);
    }
}

uint32_t Sequencer::CalculateSamplesPerStep()
{
    // 16th notes at given tempo
    // BPM = beats/minute, 1 beat = 4 steps (16th notes)
    // steps/second = BPM * 4 / 60
    float stepsPerSecond = tempo_ * 4.0f / 60.0f;
    return static_cast<uint32_t>(sampleRate_ / stepsPerSecond);
}

} // namespace seq
