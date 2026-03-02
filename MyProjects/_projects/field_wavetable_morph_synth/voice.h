#pragma once
#include "wavetable_osc.h"
#include "morph_processor.h"
#include "daisysp.h"

namespace synth
{

class Voice
{
  public:
    Voice() {}
    ~Voice() {}

    void  Init(float        sample_rate,
               const float* wavetable_data,
               int          num_tables,
               int          table_size);
    void  SetFrequency(float freq);
    void  SetWavetable(const float* wavetable_data);
    void  SetPosition(float pos);
    void  SetMorphCurve(MorphCurve curve);
    void  SetMorphSpeed(float speed);
    void  SetLfoEnabled(bool enabled);
    void  SetFilterCutoff(float cutoff);
    void  SetFilterResonance(float res);
    void  SetAdsr(float attack, float decay, float sustain, float release);
    void  SetAttack(float attack);
    void  SetDecay(float decay);
    void  SetSustain(float sustain);
    void  SetRelease(float release);
    void  SetFxAmount(float amount);
    void  SetOutputLevel(float level); // [NEW] Output Level Control
    void  NoteOn(float velocity);
    void  NoteOff();
    float Process();

  private:
    float           sample_rate_;
    WavetableOsc    osc_;
    MorphProcessor  morph_;
    daisysp::Svf    filter_;
    daisysp::Adsr   env_;
    daisysp::Chorus fx_; // Placeholder, could be reverb too

    float fx_amount_;
    float output_level_; // [NEW] Master Volume
    bool  note_on_;
};

} // namespace synth