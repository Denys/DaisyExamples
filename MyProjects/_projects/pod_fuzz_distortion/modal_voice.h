#pragma once
#include "voice.h"
#include "daisysp.h"

static const int kNumModalVoices = 4;

class ModalSynth : public Voice
{
  public:
    void  Init(float sample_rate) override;
    void  NoteOn(uint8_t note, uint8_t velocity) override;
    void  NoteOff(uint8_t note) override;
    float Process() override;
    void  SetParam(int param_id, float value) override;
    int   GetActiveCount() const override;

  private:
    daisysp::ModalVoice voice_[kNumModalVoices];

    uint8_t note_[kNumModalVoices];
    bool    active_[kNumModalVoices];
    int     next_voice_;

    // Parameter targets (set from knobs)
    float target_structure_;
    float target_brightness_;

    // Smoothed values (updated per-sample via fonepole)
    float current_structure_;
    float current_brightness_;

    // Direct-set (no smoothing needed)
    float damping_;

    int FindVoice(uint8_t note);
    int AllocVoice();
};
