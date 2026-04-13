#include "daisy_seed.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

namespace
{
constexpr uint8_t kShapeCc          = 14;
constexpr uint8_t kFormantFreqCc    = 15;
constexpr uint8_t kBleedCc          = 16;
constexpr float   kDefaultShape     = 0.35f;
constexpr float   kDefaultFormantHz = 1200.0f;
constexpr float   kDefaultBleed     = 0.25f;
constexpr float   kDefaultFreqHz    = 220.0f;
constexpr float   kOutputGain       = 0.3f;

DaisySeed          hw;
MidiUartHandler    midi;
GrainletOscillator grainlet;
Adsr               amp_env;

volatile float current_freq_hz    = kDefaultFreqHz;
volatile float current_shape      = kDefaultShape;
volatile float current_formant_hz = kDefaultFormantHz;
volatile float current_bleed      = kDefaultBleed;
volatile float current_level      = 0.0f;
volatile bool  gate               = false;

float MidiTo01(uint8_t value)
{
    return static_cast<float>(value) / 127.0f;
}

float MidiToFormantHz(uint8_t value)
{
    const float normalized = MidiTo01(value);
    return 100.0f + (normalized * normalized * 3900.0f);
}

const char* MidiTypeName(MidiMessageType type)
{
    switch(type)
    {
        case NoteOff: return "NoteOff";
        case NoteOn: return "NoteOn";
        case PolyphonicKeyPressure: return "PolyKeyPressure";
        case ControlChange: return "ControlChange";
        case ProgramChange: return "ProgramChange";
        case ChannelPressure: return "ChannelPressure";
        case PitchBend: return "PitchBend";
        case SystemCommon: return "SystemCommon";
        case SystemRealTime: return "SystemRealTime";
        case ChannelMode: return "ChannelMode";
        default: return "Unknown";
    }
}

void LogMidiEvent(MidiEvent msg)
{
    switch(msg.type)
    {
        case NoteOn:
        {
            const NoteOnEvent note = msg.AsNoteOn();
            hw.PrintLine("[MIDI RX] NoteOn ch=%d note=%d vel=%d",
                         note.channel,
                         note.note,
                         note.velocity);
            break;
        }
        case NoteOff:
        {
            const NoteOffEvent note = msg.AsNoteOff();
            hw.PrintLine("[MIDI RX] NoteOff ch=%d note=%d vel=%d",
                         note.channel,
                         note.note,
                         note.velocity);
            break;
        }
        case ControlChange:
        {
            const ControlChangeEvent control = msg.AsControlChange();
            hw.PrintLine("[MIDI RX] CC ch=%d cc=%d value=%d",
                         control.channel,
                         control.control_number,
                         control.value);
            break;
        }
        default:
            hw.PrintLine("[MIDI RX] %s ch=%d data0=%d data1=%d",
                         MidiTypeName(msg.type),
                         msg.channel,
                         msg.data[0],
                         msg.data[1]);
            break;
    }
}

void HandleMidiMessage(MidiEvent msg)
{
    switch(msg.type)
    {
        case NoteOn:
        {
            const NoteOnEvent note = msg.AsNoteOn();
            if(note.velocity == 0)
            {
                gate = false;
            }
            else
            {
                current_freq_hz = mtof(note.note);
                current_level   = MidiTo01(note.velocity);
                gate            = true;
                amp_env.Retrigger(false);
            }
            break;
        }
        case NoteOff:
        {
            gate = false;
            break;
        }
        case ControlChange:
        {
            const ControlChangeEvent control = msg.AsControlChange();
            switch(control.control_number)
            {
                case kShapeCc:
                    current_shape = MidiTo01(control.value);
                    break;
                case kFormantFreqCc:
                    current_formant_hz = MidiToFormantHz(control.value);
                    break;
                case kBleedCc:
                    current_bleed = MidiTo01(control.value);
                    break;
                default: break;
            }
            break;
        }
        default: break;
    }
}

static void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                          AudioHandle::InterleavingOutputBuffer out,
                          size_t                                size)
{
    (void)in;

    for(size_t i = 0; i < size; i += 2)
    {
        grainlet.SetFreq(current_freq_hz);
        grainlet.SetShape(current_shape);
        grainlet.SetFormantFreq(current_formant_hz);
        grainlet.SetBleed(current_bleed);

        const float env = amp_env.Process(gate);
        const float sig = grainlet.Process() * env * current_level * kOutputGain;

        out[i]     = sig;
        out[i + 1] = sig;
    }
}
} // namespace

int main(void)
{
    hw.Configure();
    hw.Init();
    hw.StartLog(false);
    hw.SetAudioBlockSize(4);

    hw.PrintLine("[BOOT] Seed_MIDI_Grainlet USB debug active");
    hw.PrintLine("[BOOT] Listening for UART MIDI on D14 / USART1_RX");

    const float sample_rate = hw.AudioSampleRate();

    grainlet.Init(sample_rate);
    grainlet.SetFreq(kDefaultFreqHz);
    grainlet.SetShape(kDefaultShape);
    grainlet.SetFormantFreq(kDefaultFormantHz);
    grainlet.SetBleed(kDefaultBleed);

    amp_env.Init(sample_rate);
    amp_env.SetAttackTime(0.005f);
    amp_env.SetDecayTime(0.02f);
    amp_env.SetSustainLevel(1.0f);
    amp_env.SetReleaseTime(0.08f);

    MidiUartHandler::Config midi_config;
    midi.Init(midi_config);
    midi.StartReceive();

    hw.StartAudio(AudioCallback);

    while(1)
    {
        midi.Listen();
        while(midi.HasEvents())
        {
            const MidiEvent msg = midi.PopEvent();
            LogMidiEvent(msg);
            HandleMidiMessage(msg);
        }
    }
}
