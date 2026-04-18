#pragma once

#include <cstddef>

#include "daisyhost/HostedAppCore.h"

namespace daisyhost
{
class MidiNotePreview
{
  public:
    void Prepare(double sampleRate);
    void HandleMidiEvent(const MidiMessageEvent& event);
    void RenderAdd(float* destination, std::size_t numSamples, float level);
    bool IsActive() const;

  private:
    double sampleRate_ = 48000.0;
    float  phase_      = 0.0f;
    int    midiNote_   = 69;
    float  currentGain_ = 0.0f;
    float  targetGain_  = 0.0f;
    float  attackStep_  = 0.0f;
    float  releaseStep_ = 0.0f;
    bool   active_     = false;
};
} // namespace daisyhost
