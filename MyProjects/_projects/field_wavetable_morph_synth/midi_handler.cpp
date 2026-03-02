#include "midi_handler.h"

namespace synth
{

void SynthMidiHandler::Init(daisy::DaisyField* hw)
{
    hw_        = hw;
    voice_     = nullptr;
    base_freq_ = 440.0f;
}

void SynthMidiHandler::ProcessMidi()
{
    hw_->midi.Listen();

    while(hw_->midi.HasEvents())
    {
        auto msg = hw_->midi.PopEvent();

        switch(msg.type)
        {
            case daisy::MidiMessageType::NoteOn:
                HandleNoteOn(msg.channel, msg.data[0], msg.data[1]);
                break;

            case daisy::MidiMessageType::NoteOff:
                HandleNoteOff(msg.channel, msg.data[0], msg.data[1]);
                break;

            case daisy::MidiMessageType::PitchBend:
                HandlePitchBend(msg.channel, (msg.data[1] << 7) | msg.data[0]);
                break;

            case daisy::MidiMessageType::ControlChange:
                HandleControlChange(msg.channel, msg.data[0], msg.data[1]);
                break;

            default: break;
        }
    }
}

void SynthMidiHandler::HandleNoteOn(uint8_t channel,
                                    uint8_t note,
                                    uint8_t velocity)
{
    if(!voice_)
        return;

    // Convert MIDI note to frequency
    float freq = 440.0f * powf(2.0f, (note - 69) / 12.0f);
    base_freq_ = freq; // Store base frequency for pitch bend
    voice_->SetFrequency(freq);
    voice_->NoteOn(velocity / 127.0f);
}

void SynthMidiHandler::HandleNoteOff(uint8_t channel,
                                     uint8_t note,
                                     uint8_t velocity)
{
    if(!voice_)
        return;
    voice_->NoteOff();
}

void SynthMidiHandler::HandlePitchBend(uint8_t channel, int bend)
{
    if(!voice_)
        return;

    // Pitch bend range of +/- 2 semitones
    float bend_ratio = powf(2.0f, (bend - 8192) / 8192.0f * 2.0f / 12.0f);
    float bent_freq  = base_freq_ * bend_ratio;
    voice_->SetFrequency(bent_freq);
}

void SynthMidiHandler::HandleControlChange(uint8_t channel,
                                           uint8_t control,
                                           uint8_t value)
{
    if(!voice_)
        return;

    // Map CC messages to synth parameters
    switch(control)
    {
        case 1: // Mod Wheel - Morph Position
            voice_->SetPosition(value / 127.0f);
            break;

        case 2: // Breath Controller - Morph Speed
            voice_->SetMorphSpeed(value / 12.7f); // 0-10 Hz
            break;

        case 74:                                             // Filter Cutoff
            voice_->SetFilterCutoff(20.0f + value * 158.0f); // 20Hz - 20kHz
            break;

        case 71: // Filter Resonance
            voice_->SetFilterResonance(value / 127.0f);
            break;

        case 73: // Attack
            voice_->SetAttack(value / 25.4f); // 0-5s
            break;

        case 75: // Decay
            voice_->SetDecay(value / 25.4f); // 0-5s
            break;

        case 76: // Sustain
            voice_->SetSustain(value / 127.0f); // 0-1
            break;

        case 77: // Release
            voice_->SetRelease(value / 12.7f); // 0-10s
            break;

        case 78: // FX Amount
            voice_->SetFxAmount(value / 127.0f);
            break;

        case 79: // Morph Curve
            voice_->SetMorphCurve(static_cast<MorphCurve>(value % MORPH_COUNT));
            break;

        case 80: // LFO Enable
            voice_->SetLfoEnabled(value >= 64);
            break;

        default: break;
    }
}

} // namespace synth