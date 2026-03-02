#pragma once
#include "voice.h"
#include "daisysp.h"

static const int kNumFmVoices = 4;

class FmVoice : public Voice
{
  public:
    void  Init(float sample_rate) override;
    void  NoteOn(uint8_t note, uint8_t velocity) override;
    void  NoteOff(uint8_t note) override;
    float Process() override;
    void  SetParam(int param_id, float value) override;
    int   GetActiveCount() const override;

  private:
    daisysp::Fm2  osc_[kNumFmVoices];
    daisysp::Adsr env_[kNumFmVoices];

    uint8_t note_[kNumFmVoices];
    float   vel_[kNumFmVoices];
    bool    gate_[kNumFmVoices];
    int     next_voice_;

    // Parameter targets (set from knobs)
    float target_index_;
    float target_ratio_;

    // Smoothed values (updated per-sample via fonepole)
    float current_index_;
    float current_ratio_;

    int FindVoice(uint8_t note);
    int AllocVoice();
};
