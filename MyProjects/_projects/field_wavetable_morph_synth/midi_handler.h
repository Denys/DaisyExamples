#pragma once
#include "daisy_field.h"
#include "voice.h"

namespace synth
{

class SynthMidiHandler
{
  public:
    SynthMidiHandler() {}
    ~SynthMidiHandler() {}

    void Init(daisy::DaisyField* hw);
    void ProcessMidi();
    void SetVoice(Voice* voice) { voice_ = voice; }

  private:
    daisy::DaisyField* hw_;
    Voice*             voice_;
    float              base_freq_; // For pitch bend

    void HandleNoteOn(uint8_t channel, uint8_t note, uint8_t velocity);
    void HandleNoteOff(uint8_t channel, uint8_t note, uint8_t velocity);
    void HandlePitchBend(uint8_t channel, int bend);
    void HandleControlChange(uint8_t channel, uint8_t control, uint8_t value);
};

} // namespace synth