#include "fm_voice.h"

using namespace daisysp;

void FmVoice::Init(float sample_rate)
{
    next_voice_    = 0;
    target_index_  = 1.0f;
    target_ratio_  = 2.0f;
    current_index_ = 1.0f;
    current_ratio_ = 2.0f;

    for(int i = 0; i < kNumFmVoices; i++)
    {
        osc_[i].Init(sample_rate);
        osc_[i].SetRatio(2.0f);
        osc_[i].SetIndex(1.0f);

        env_[i].Init(sample_rate);
        env_[i].SetTime(ADSR_SEG_ATTACK, 0.005f);
        env_[i].SetTime(ADSR_SEG_DECAY, 0.15f);
        env_[i].SetSustainLevel(0.7f);
        env_[i].SetTime(ADSR_SEG_RELEASE, 0.4f);

        note_[i] = 0;
        vel_[i]  = 0.0f;
        gate_[i] = false;
    }
}

void FmVoice::NoteOn(uint8_t note, uint8_t velocity)
{
    if(velocity == 0)
    {
        NoteOff(note);
        return;
    }

    int v = FindVoice(note);
    if(v < 0)
        v = AllocVoice();

    note_[v] = note;
    vel_[v]  = velocity / 127.0f;
    gate_[v] = true;
    osc_[v].SetFrequency(mtof(note));
}

void FmVoice::NoteOff(uint8_t note)
{
    int v = FindVoice(note);
    if(v >= 0)
        gate_[v] = false;
}

float FmVoice::Process()
{
    // Smooth parameters per-sample (prevents zipper noise)
    fonepole(current_index_, target_index_, 0.002f);
    fonepole(current_ratio_, target_ratio_, 0.002f);

    float mix = 0.0f;
    for(int i = 0; i < kNumFmVoices; i++)
    {
        float env_out = env_[i].Process(gate_[i]);
        if(env_out > 0.001f)
        {
            osc_[i].SetIndex(current_index_);
            osc_[i].SetRatio(current_ratio_);
            mix += osc_[i].Process() * env_out * vel_[i];
        }
    }
    return mix * 0.25f; // Normalize for 4 voices
}

void FmVoice::SetParam(int param_id, float value)
{
    switch(param_id)
    {
        case 0: target_index_ = value * 10.0f; break;       // Mod index: 0-10
        case 1: target_ratio_ = 0.5f + value * 7.5f; break; // FM ratio: 0.5-8.0
    }
}

int FmVoice::GetActiveCount() const
{
    int count = 0;
    for(int i = 0; i < kNumFmVoices; i++)
    {
        if(gate_[i])
            count++;
    }
    return count;
}

int FmVoice::FindVoice(uint8_t note)
{
    for(int i = 0; i < kNumFmVoices; i++)
    {
        if(note_[i] == note && gate_[i])
            return i;
    }
    return -1;
}

int FmVoice::AllocVoice()
{
    // Prefer free voices first
    for(int i = 0; i < kNumFmVoices; i++)
    {
        int idx = (next_voice_ + i) % kNumFmVoices;
        if(!gate_[idx])
        {
            next_voice_ = (idx + 1) % kNumFmVoices;
            return idx;
        }
    }
    // All voices active: steal oldest (round-robin)
    int v      = next_voice_;
    next_voice_ = (next_voice_ + 1) % kNumFmVoices;
    return v;
}
