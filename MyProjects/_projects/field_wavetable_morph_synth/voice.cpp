#include "voice.h"

namespace synth
{

void Voice::Init(float        sample_rate,
                 const float* wavetable_data,
                 int          num_tables,
                 int          table_size)
{
    sample_rate_ = sample_rate;

    // Initialize components
    osc_.Init(sample_rate, wavetable_data, num_tables, table_size);
    morph_.Init(sample_rate);

    filter_.Init(sample_rate);
    env_.Init(sample_rate);
    fx_.Init(sample_rate);

    fx_amount_    = 0.0f;
    output_level_ = 0.5f; // Default to 50% volume
    note_on_      = false;

    // Set default parameters
    filter_.SetFreq(1000.0f);
    filter_.SetRes(0.5f);
    filter_.SetDrive(0.0f);

    env_.SetTime(daisysp::ADSR_SEG_ATTACK, 0.1f);
    env_.SetTime(daisysp::ADSR_SEG_DECAY, 0.1f);
    env_.SetTime(daisysp::ADSR_SEG_RELEASE, 0.2f);
    env_.SetSustainLevel(0.8f);

    fx_.SetLfoFreq(0.5f);
    fx_.SetLfoDepth(0.3f);
    fx_.SetDelay(0.5f, 0.6f);
    fx_.SetFeedback(0.2f);
}

void Voice::SetFrequency(float freq)
{
    osc_.SetFrequency(freq);
}

void Voice::SetWavetable(const float* wavetable_data)
{
    osc_.SetWavetable(wavetable_data);
}

void Voice::SetPosition(float pos)
{
    float modulated_pos = morph_.Process(pos);
    osc_.SetPosition(modulated_pos);
}

void Voice::SetMorphCurve(MorphCurve curve)
{
    morph_.SetCurve(curve);
}

void Voice::SetMorphSpeed(float speed)
{
    morph_.SetSpeed(speed);
}

void Voice::SetLfoEnabled(bool enabled)
{
    morph_.SetLfoEnabled(enabled);
}

void Voice::SetFilterCutoff(float cutoff)
{
    filter_.SetFreq(cutoff);
}

void Voice::SetFilterResonance(float res)
{
    filter_.SetRes(res);
}

void Voice::SetAdsr(float attack, float decay, float sustain, float release)
{
    env_.SetTime(daisysp::ADSR_SEG_ATTACK, attack);
    env_.SetTime(daisysp::ADSR_SEG_DECAY, decay);
    env_.SetTime(daisysp::ADSR_SEG_RELEASE, release);
    env_.SetSustainLevel(sustain);
}

void Voice::SetAttack(float attack)
{
    env_.SetTime(daisysp::ADSR_SEG_ATTACK, attack);
}

void Voice::SetDecay(float decay)
{
    env_.SetTime(daisysp::ADSR_SEG_DECAY, decay);
}

void Voice::SetSustain(float sustain)
{
    env_.SetSustainLevel(sustain);
}

void Voice::SetRelease(float release)
{
    env_.SetTime(daisysp::ADSR_SEG_RELEASE, release);
}

void Voice::SetFxAmount(float amount)
{
    fx_amount_ = amount;
}

void Voice::SetOutputLevel(float level)
{
    output_level_ = level;
}

void Voice::NoteOn(float velocity)
{
    note_on_ = true;
    env_.Retrigger(false);
}

void Voice::NoteOff()
{
    note_on_ = false;
}

float Voice::Process()
{
    // Generate oscillator output
    float osc_out = osc_.Process();

    // Apply envelope
    float env_out = env_.Process(note_on_);

    // Apply filter
    filter_.Process(osc_out * env_out);
    float filtered = filter_.Low();

    // Apply FX
    float fx_out = fx_.Process(filtered);
    float wet    = filtered * (1.0f - fx_amount_) + fx_out * fx_amount_;

    return wet * output_level_;
}

} // namespace synth