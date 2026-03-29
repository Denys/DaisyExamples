#include "daisy_field.h"
#include "daisysp.h"
#include "../../foundation_examples/field_defaults.h"
#include "../../foundation_examples/field_instrument_ui.h"

using namespace daisy;
using namespace daisysp;
using namespace FieldDefaults;
using namespace FieldInstrumentUI;

namespace
{
constexpr bool     kEnableBootLog  = false;
constexpr uint32_t kZoomMs         = 1400;
constexpr float    kZoomDelta      = 0.015f;
constexpr int      kWaveformKeys   = 4;

DaisyField      hw;
OneHotKeyLedBank key_leds;
ParamZoomState   zoom_state;

struct Voice
{
    Oscillator osc;
    Adsr       env;
    bool       gate          = false;
    bool       active        = false;
    bool       note_held     = false;
    uint8_t    midi_note     = 60;
    float      attack_time_s = 0.101f;
    float      decay_time_s  = 0.501f;
    float      sustain_level = 0.80f;
    float      release_time_s = 0.901f;

    void Init(float sample_rate)
    {
        osc.Init(sample_rate);
        osc.SetWaveform(Oscillator::WAVE_SAW);
        osc.SetAmp(0.5f);

        env.Init(sample_rate);
        ApplyEnvelope();
    }

    void ApplyEnvelope()
    {
        env.SetAttackTime(attack_time_s);
        env.SetDecayTime(decay_time_s);
        env.SetSustainLevel(sustain_level);
        env.SetReleaseTime(release_time_s);
    }

    void NoteOn(uint8_t note)
    {
        midi_note = note;
        osc.SetFreq(mtof(note));
        note_held = true;
        gate      = true;
        active    = true;
    }

    void NoteOff(bool sustain_pedal)
    {
        note_held = false;
        if(!sustain_pedal)
            gate = false;
    }

    void ReleaseSustain()
    {
        if(!note_held)
            gate = false;
    }

    void Panic()
    {
        note_held = false;
        gate      = false;
    }

    float Process()
    {
        const float env_out = env.Process(gate);
        if(!gate && env_out < 0.0001f)
            active = false;
        return active ? osc.Process() * env_out : 0.0f;
    }
};

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

Voice voice;
Svf   filter;

float knob_values[8] = {
    params.cutoff,
    params.resonance,
    params.attack,
    params.decay,
    params.sustain,
    params.release,
    params.drive,
    params.output_level,
};

float cutoff_current    = params.cutoff;
float resonance_current = params.resonance;
float drive_current     = params.drive;
float output_current    = params.output_level;

uint32_t last_midi_event_ms = 0;

const char* kParamNames[8] = {
    "Cutoff",
    "Resonance",
    "Attack",
    "Decay",
    "Sustain",
    "Release",
    "Drive",
    "Output",
};

float AttackSeconds() { return 0.001f + params.attack * 2.0f; }
float DecaySeconds() { return 0.001f + params.decay * 2.0f; }
float ReleaseSeconds() { return 0.001f + params.release * 3.0f; }
float CutoffHertz(float normalized) { return 50.0f + normalized * 16000.0f; }

int WaveformSlotFromValue(int waveform)
{
    switch(waveform)
    {
        case Oscillator::WAVE_SIN: return 0;
        case Oscillator::WAVE_TRI: return 1;
        case Oscillator::WAVE_SAW: return 2;
        case Oscillator::WAVE_SQUARE: return 3;
        default: return 2;
    }
}

int WaveformFromSlot(int slot)
{
    switch(slot)
    {
        case 0: return Oscillator::WAVE_SIN;
        case 1: return Oscillator::WAVE_TRI;
        case 2: return Oscillator::WAVE_SAW;
        case 3: return Oscillator::WAVE_SQUARE;
        default: return Oscillator::WAVE_SAW;
    }
}

void ApplyControlParams()
{
    voice.attack_time_s = AttackSeconds();
    voice.decay_time_s = DecaySeconds();
    voice.sustain_level = params.sustain;
    voice.release_time_s = ReleaseSeconds();
    voice.ApplyEnvelope();
    voice.osc.SetWaveform(params.waveform);
}

void SelectWaveformSlot(int slot)
{
    params.waveform = WaveformFromSlot(slot);
    voice.osc.SetWaveform(params.waveform);
    key_leds.SetActiveA(slot);
}

void ProcessKnobs()
{
    knob_values[0] = params.cutoff = hw.knob[0].Process();
    knob_values[1] = params.resonance = hw.knob[1].Process();
    knob_values[2] = params.attack = hw.knob[2].Process();
    knob_values[3] = params.decay = hw.knob[3].Process();
    knob_values[4] = params.sustain = hw.knob[4].Process();
    knob_values[5] = params.release = hw.knob[5].Process();
    knob_values[6] = params.drive = hw.knob[6].Process();
    knob_values[7] = params.output_level = hw.knob[7].Process();

    zoom_state.Capture(knob_values, System::GetNow(), kZoomDelta);
    ApplyControlParams();
}

void HandleMidiMessage(MidiEvent msg)
{
    last_midi_event_ms = System::GetNow();

    switch(msg.type)
    {
        case NoteOn:
        {
            const NoteOnEvent note = msg.AsNoteOn();
            if(note.velocity == 0)
                voice.NoteOff(params.sustain_pedal);
            else
                voice.NoteOn(note.note);
            break;
        }

        case NoteOff:
        {
            const NoteOffEvent note = msg.AsNoteOff();
            if(note.note == voice.midi_note)
                voice.NoteOff(params.sustain_pedal);
            break;
        }

        case ControlChange:
        {
            const ControlChangeEvent cc = msg.AsControlChange();
            if(cc.control_number == 64)
            {
                params.sustain_pedal = cc.value >= 64;
                if(!params.sustain_pedal)
                    voice.ReleaseSustain();
            }
            break;
        }

        default: break;
    }
}

void HandleKeyboardShortcuts()
{
    for(int i = 0; i < kWaveformKeys; ++i)
    {
        if(hw.KeyboardRisingEdge(kKeyAIndices[i]))
        {
            SelectWaveformSlot(i);
            break;
        }
    }

    if(hw.sw[0].RisingEdge())
    {
        voice.Panic();
        params.sustain_pedal = false;
    }

    if(hw.sw[1].RisingEdge())
        zoom_state.Clear();
}

void FormatParamValue(int idx, char* buffer, size_t size)
{
    switch(idx)
    {
        case 0: FormatHertz(buffer, size, CutoffHertz(knob_values[idx])); break;
        case 1: FormatPercent(buffer, size, knob_values[idx]); break;
        case 2: FormatMilliseconds(buffer, size, AttackSeconds()); break;
        case 3: FormatMilliseconds(buffer, size, DecaySeconds()); break;
        case 4: FormatPercent(buffer, size, knob_values[idx]); break;
        case 5: FormatMilliseconds(buffer, size, ReleaseSeconds()); break;
        case 6: FormatPercent(buffer, size, knob_values[idx]); break;
        case 7: FormatPercent(buffer, size, knob_values[idx]); break;
        default: snprintf(buffer, size, "%.2f", knob_values[idx]); break;
    }
}

void DrawZoom()
{
    const int idx = zoom_state.ActiveIndex();
    if(idx < 0 || idx >= 8)
        return;

    char value[32];
    FormatParamValue(idx, value, sizeof(value));

    hw.display.Fill(false);
    hw.display.SetCursor(0, 0);
    hw.display.WriteString(kParamNames[idx], Font_7x10, true);
    hw.display.SetCursor(0, 18);
    hw.display.WriteString(value, Font_11x18, true);

    const int bar_width = static_cast<int>(zoom_state.ActiveValue() * 127.0f);
    hw.display.DrawRect(0, 54, 127, 62, true, false);
    if(bar_width > 0)
        hw.display.DrawRect(0, 54, bar_width, 62, true, true);

    hw.display.Update();
}

void DrawOverview()
{
    char line[40];
    char note_name[8];
    FormatMidiNoteName(note_name, sizeof(note_name), voice.midi_note);

    hw.display.Fill(false);

    hw.display.SetCursor(0, 0);
    hw.display.WriteString("FIELD TEMPLATE 2", Font_7x10, true);

    snprintf(line,
             sizeof(line),
             "Wave:%s Gate:%s",
             WaveformName(params.waveform),
             voice.gate ? "ON" : "OFF");
    hw.display.SetCursor(0, 10);
    hw.display.WriteString(line, Font_6x8, true);

    snprintf(line,
             sizeof(line),
             "Cut:%d Res:%d",
             static_cast<int>(params.cutoff * 100.0f + 0.5f),
             static_cast<int>(params.resonance * 100.0f + 0.5f));
    hw.display.SetCursor(0, 18);
    hw.display.WriteString(line, Font_6x8, true);

    snprintf(line,
             sizeof(line),
             "A:%d D:%d S:%d",
             static_cast<int>(params.attack * 100.0f + 0.5f),
             static_cast<int>(params.decay * 100.0f + 0.5f),
             static_cast<int>(params.sustain * 100.0f + 0.5f));
    hw.display.SetCursor(0, 26);
    hw.display.WriteString(line, Font_6x8, true);

    snprintf(line,
             sizeof(line),
             "R:%d Drv:%d Vol:%d",
             static_cast<int>(params.release * 100.0f + 0.5f),
             static_cast<int>(params.drive * 100.0f + 0.5f),
             static_cast<int>(params.output_level * 100.0f + 0.5f));
    hw.display.SetCursor(0, 34);
    hw.display.WriteString(line, Font_6x8, true);

    snprintf(line,
             sizeof(line),
             "Midi:%s %s",
             (System::GetNow() - last_midi_event_ms) < 1200 ? "RX" : "IDLE",
             note_name);
    hw.display.SetCursor(0, 42);
    hw.display.WriteString(line, Font_6x8, true);

    hw.display.SetCursor(0, 50);
    hw.display.WriteString("A1-4 Wave  SW1 Panic", Font_6x8, true);
    hw.display.SetCursor(0, 58);
    hw.display.WriteString("SW2 Clear  TRS MIDI", Font_6x8, true);

    hw.display.Update();
}

void UpdateDisplay()
{
    if(zoom_state.IsActive(System::GetNow(), kZoomMs))
        DrawZoom();
    else
        DrawOverview();
}

void AudioCallback(AudioHandle::InputBuffer in,
                   AudioHandle::OutputBuffer out,
                   size_t size)
{
    for(size_t i = 0; i < size; ++i)
    {
        fonepole(cutoff_current, params.cutoff, 0.0010f);
        fonepole(resonance_current, params.resonance, 0.0010f);
        fonepole(drive_current, params.drive, 0.0015f);
        fonepole(output_current, params.output_level, 0.0015f);

        filter.SetFreq(CutoffHertz(cutoff_current));
        filter.SetRes(resonance_current);

        float sig = voice.Process();
        filter.Process(sig);
        sig = filter.Low();

        sig = tanhf(sig * (1.0f + drive_current * 6.0f));
        sig *= output_current;

        out[0][i] = sig + in[0][i] * 0.0f;
        out[1][i] = sig + in[1][i] * 0.0f;
    }
}

} // namespace

int main(void)
{
    hw.Init();
    hw.SetAudioBlockSize(kRecommendedBlockSize);
    hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

    if(kEnableBootLog)
    {
        hw.seed.StartLog(false);
        hw.seed.PrintLine("[BOOT] Field_Template_Std_2");
    }

    const float sample_rate = hw.AudioSampleRate();

    voice.Init(sample_rate);
    filter.Init(sample_rate);
    ApplyControlParams();

    key_leds.Init(&hw);
    SelectWaveformSlot(WaveformSlotFromValue(params.waveform));
    key_leds.Update();

    zoom_state.Init(params.cutoff);
    last_midi_event_ms = System::GetNow();

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
        UpdateDisplay();

        System::Delay(1);
    }
}
