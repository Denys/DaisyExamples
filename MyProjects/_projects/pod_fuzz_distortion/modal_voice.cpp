#include "modal_voice.h"

using namespace daisysp;

void ModalSynth::Init(float sample_rate)
{
    next_voice_          = 0;
    target_structure_    = 0.5f;
    target_brightness_   = 0.5f;
    current_structure_   = 0.5f;
    current_brightness_  = 0.5f;
    damping_             = 0.5f;

    for(int i = 0; i < kNumModalVoices; i++)
    {
        voice_[i].Init(sample_rate);
        voice_[i].SetFreq(440.0f);
        voice_[i].SetStructure(0.5f);
        voice_[i].SetBrightness(0.5f);
        voice_[i].SetDamping(0.5f);
        voice_[i].SetAccent(0.5f);

        note_[i]   = 0;
        active_[i] = false;
    }
}

void ModalSynth::NoteOn(uint8_t note, uint8_t velocity)
{
    if(velocity == 0)
    {
        NoteOff(note);
        return;
    }

    int v = FindVoice(note);
    if(v < 0)
        v = AllocVoice();

    note_[v]   = note;
    active_[v] = true;
    voice_[v].SetFreq(mtof(note));
    voice_[v].SetAccent(velocity / 127.0f);
    voice_[v].Trig();
}

void ModalSynth::NoteOff(uint8_t note)
{
    int v = FindVoice(note);
    if(v >= 0)
        active_[v] = false; // Natural decay continues
}

float ModalSynth::Process()
{
    // Smooth parameters per-sample (prevents zipper noise)
    fonepole(current_structure_, target_structure_, 0.002f);
    fonepole(current_brightness_, target_brightness_, 0.002f);

    float mix = 0.0f;
    for(int i = 0; i < kNumModalVoices; i++)
    {
        voice_[i].SetStructure(current_structure_);
        voice_[i].SetBrightness(current_brightness_);
        voice_[i].SetDamping(damping_);
        mix += voice_[i].Process();
    }
    return mix * 0.25f; // Normalize for 4 voices
}

void ModalSynth::SetParam(int param_id, float value)
{
    switch(param_id)
    {
        case 0: target_structure_  = value; break; // Structure: 0-1
        case 1: target_brightness_ = value; break; // Brightness: 0-1
        case 2: damping_           = value; break;  // Damping: 0-1
    }
}

int ModalSynth::GetActiveCount() const
{
    int count = 0;
    for(int i = 0; i < kNumModalVoices; i++)
    {
        if(active_[i])
            count++;
    }
    return count;
}

int ModalSynth::FindVoice(uint8_t note)
{
    for(int i = 0; i < kNumModalVoices; i++)
    {
        if(note_[i] == note && active_[i])
            return i;
    }
    return -1;
}

int ModalSynth::AllocVoice()
{
    // Prefer free voices first
    for(int i = 0; i < kNumModalVoices; i++)
    {
        int idx = (next_voice_ + i) % kNumModalVoices;
        if(!active_[idx])
        {
            next_voice_ = (idx + 1) % kNumModalVoices;
            return idx;
        }
    }
    // All voices active: steal oldest (round-robin)
    int v       = next_voice_;
    next_voice_ = (next_voice_ + 1) % kNumModalVoices;
    return v;
}
