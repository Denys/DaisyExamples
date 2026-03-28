#include "daisy_field.h"
#include "daisysp.h"
#include "../../foundation_examples/field_defaults.h"

using namespace daisy;
using namespace daisysp;
using namespace FieldDefaults;

DaisyField         hw;
FieldKeyboardLEDs  key_leds;
FieldOLEDDisplay   display;

struct Voice
{
    Oscillator osc;
    Adsr       env;
    bool       gate     = false;
    bool       active   = false;
    uint8_t    midi_note = 60;

    void Init(float sample_rate)
    {
        osc.Init(sample_rate);
        osc.SetWaveform(Oscillator::WAVE_SAW);
        osc.SetAmp(0.5f);

        env.Init(sample_rate);
        env.SetAttackTime(0.005f);
        env.SetDecayTime(0.15f);
        env.SetSustainLevel(0.75f);
        env.SetReleaseTime(0.25f);
    }

    void NoteOn(uint8_t note)
    {
        midi_note = note;
        osc.SetFreq(mtof(note));
        gate   = true;
        active = true;
    }

    void NoteOff() { gate = false; }

    float Process()
    {
        float env_out = env.Process(gate);
        if(!gate && env_out < 0.0001f)
            active = false;
        return active ? osc.Process() * env_out : 0.0f;
    }
};

Voice voice;
Svf   filter;

struct Params
{
    float cutoff       = 0.45f;
    float resonance    = 0.15f;
    float attack       = 0.05f;
    float decay        = 0.25f;
    float sustain      = 0.80f;
    float release      = 0.30f;
    float drive        = 0.10f;
    float output_level = 0.75f;

    bool  sustain_pedal = false;
    int   waveform      = Oscillator::WAVE_SAW;
} params;

void ApplyParams()
{
    const float cutoff_hz = 50.0f + params.cutoff * 16000.0f;
    filter.SetFreq(cutoff_hz);
    filter.SetRes(params.resonance);

    voice.env.SetAttackTime(0.001f + params.attack * 2.0f);
    voice.env.SetDecayTime(0.001f + params.decay * 2.0f);
    voice.env.SetSustainLevel(params.sustain);
    voice.env.SetReleaseTime(0.001f + params.release * 3.0f);

    voice.osc.SetWaveform(params.waveform);
}

void UpdateDisplayValues()
{
    display.SetValue(0, params.cutoff);
    display.SetValue(1, params.resonance);
    display.SetValue(2, params.attack);
    display.SetValue(3, params.decay);
    display.SetValue(4, params.sustain);
    display.SetValue(5, params.release);
    display.SetValue(6, params.drive);
    display.SetValue(7, params.output_level);
}

void ProcessKnobs()
{
    params.cutoff       = hw.knob[0].Process();
    params.resonance    = hw.knob[1].Process();
    params.attack       = hw.knob[2].Process();
    params.decay        = hw.knob[3].Process();
    params.sustain      = hw.knob[4].Process();
    params.release      = hw.knob[5].Process();
    params.drive        = hw.knob[6].Process();
    params.output_level = hw.knob[7].Process();

    ApplyParams();
    UpdateDisplayValues();
}

void HandleMidiMessage(const MidiEvent& msg)
{
    switch(msg.type)
    {
        case NoteOn:
        {
            NoteOnEvent note = msg.AsNoteOn();
            if(note.velocity == 0)
                voice.NoteOff();
            else
                voice.NoteOn(note.note);
        }
        break;

        case NoteOff:
            voice.NoteOff();
            break;

        case ControlChange:
        {
            ControlChangeEvent cc = msg.AsControlChange();
            if(cc.control_number == 64)
            {
                params.sustain_pedal = cc.value >= 64;
                if(!params.sustain_pedal && !voice.gate)
                    voice.NoteOff();
            }
        }
        break;

        default: break;
    }
}

void HandleKeyboardShortcuts()
{
    for(int i = 0; i < 8; i++)
    {
        if(hw.KeyboardRisingEdge(kKeyAIndices[i]))
        {
            key_leds.ToggleA(i);
            params.waveform = (i % 4 == 0)
                                  ? Oscillator::WAVE_SIN
                                  : (i % 4 == 1) ? Oscillator::WAVE_TRI
                                                 : (i % 4 == 2) ? Oscillator::WAVE_SAW
                                                                : Oscillator::WAVE_SQUARE;
            voice.osc.SetWaveform(params.waveform);
            display.SetActiveParam(0);
        }

        if(hw.KeyboardRisingEdge(kKeyBIndices[i]))
            key_leds.ToggleB(i);
    }

    // SW1: panic (all notes off)
    if(hw.sw[0].RisingEdge())
    {
        voice.NoteOff();
        params.sustain_pedal = false;
    }

    // SW2: reset display focus
    if(hw.sw[1].RisingEdge())
        display.SetActiveParam(-1);
}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    for(size_t i = 0; i < size; i++)
    {
        float sig = voice.Process();

        filter.Process(sig);
        sig = filter.Low();

        // Soft-drive example for quick template coloration
        sig = tanhf(sig * (1.0f + params.drive * 6.0f));

        sig *= params.output_level;

        out[0][i] = sig + in[0][i] * 0.0f;
        out[1][i] = sig + in[1][i] * 0.0f;
    }
}

int main(void)
{
    hw.Init();
    hw.SetAudioBlockSize(kRecommendedBlockSize);
    hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

    const float sample_rate = hw.AudioSampleRate();

    voice.Init(sample_rate);
    filter.Init(sample_rate);
    ApplyParams();

    key_leds.Init(&hw);

    display.Init(&hw);
    display.SetTitle("FIELD TEMPLATE");
    display.SetLabel(0, "Cutoff");
    display.SetLabel(1, "Res");
    display.SetLabel(2, "Attack");
    display.SetLabel(3, "Decay");
    display.SetLabel(4, "Sustain");
    display.SetLabel(5, "Release");
    display.SetLabel(6, "Drive");
    display.SetLabel(7, "Volume");
    UpdateDisplayValues();

    // External MIDI keyboard via Daisy Field TRS MIDI input
    hw.midi.StartReceive();

    hw.StartAdc();
    hw.StartAudio(AudioCallback);

    while(1)
    {
        hw.midi.Listen();
        while(hw.midi.HasEvents())
            HandleMidiMessage(hw.midi.PopEvent());

        hw.ProcessAllControls();
        ProcessKnobs();
        HandleKeyboardShortcuts();

        key_leds.Update();
        display.UpdateCompact();

        System::Delay(1);
    }
}
