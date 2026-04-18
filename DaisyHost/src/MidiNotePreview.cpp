#include "daisyhost/MidiNotePreview.h"

#include <algorithm>
#include <cmath>

namespace daisyhost
{
namespace
{
constexpr double kTwoPi = 6.28318530717958647692;

float MidiNoteToFrequency(int midiNote)
{
    return 440.0f * std::pow(2.0f, (static_cast<float>(midiNote) - 69.0f) / 12.0f);
}
} // namespace

void MidiNotePreview::Prepare(double sampleRate)
{
    sampleRate_ = sampleRate > 1000.0 ? sampleRate : 48000.0;
    phase_      = 0.0f;
    currentGain_ = 0.0f;
    targetGain_  = 0.0f;
    attackStep_  = 1.0f / static_cast<float>(sampleRate_ * 0.005);
    releaseStep_ = 1.0f / static_cast<float>(sampleRate_ * 0.02);
}

void MidiNotePreview::HandleMidiEvent(const MidiMessageEvent& event)
{
    const std::uint8_t status = static_cast<std::uint8_t>(event.status & 0xF0);
    const int          note   = static_cast<int>(event.data1);
    const float        velocity = std::clamp(
        static_cast<float>(event.data2) / 127.0f, 0.0f, 1.0f);

    if(status == 0x90 && event.data2 > 0)
    {
        midiNote_   = note;
        targetGain_ = velocity;
        active_     = true;
        return;
    }

    if(status == 0x80 || (status == 0x90 && event.data2 == 0))
    {
        if(note == midiNote_)
        {
            targetGain_ = 0.0f;
        }
    }
}

void MidiNotePreview::RenderAdd(float* destination,
                                std::size_t numSamples,
                                float       level)
{
    if(destination == nullptr || !active_)
    {
        return;
    }

    const float frequency      = MidiNoteToFrequency(midiNote_);
    const float phaseIncrement = static_cast<float>(
        (kTwoPi * frequency) / sampleRate_);
    const float levelClamped = std::clamp(level, 0.0f, 1.0f);

    for(std::size_t sample = 0; sample < numSamples; ++sample)
    {
        if(currentGain_ < targetGain_)
        {
            currentGain_ = std::min(targetGain_, currentGain_ + attackStep_);
        }
        else if(currentGain_ > targetGain_)
        {
            currentGain_ = std::max(targetGain_, currentGain_ - releaseStep_);
        }

        destination[sample] += std::sin(phase_) * currentGain_ * levelClamped;
        phase_ += phaseIncrement;
        if(phase_ >= static_cast<float>(kTwoPi))
        {
            phase_ -= static_cast<float>(kTwoPi);
        }
    }

    if(targetGain_ <= 0.0f && currentGain_ <= 0.0f)
    {
        active_ = false;
    }
}

bool MidiNotePreview::IsActive() const
{
    return active_;
}
} // namespace daisyhost
