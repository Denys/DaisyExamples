#include "daisy_field.h"
#include "daisysp.h"
#include "../../foundation_examples/field_defaults.h"

#include <cstdio>

using namespace daisy;
using namespace daisysp;
using namespace FieldDefaults;

namespace
{
constexpr size_t kNumVoices        = 8;
constexpr int    kMidiChannelHuman = 1; // OXI test channel: 1-16
constexpr float  kMinCutoffHz      = 80.0f;
constexpr float  kMaxCutoffHz      = 12000.0f;

DaisyField        hw;
FieldKeyboardLEDs key_leds;
FieldOLEDDisplay  display;

struct MidiDiagnostics
{
    uint32_t note_on            = 0;
    uint32_t note_off           = 0;
    uint32_t control_change     = 0;
    uint32_t pitch_bend         = 0;
    uint32_t program_change     = 0;
    uint32_t system_realtime    = 0;
    uint32_t timing_clock       = 0;
    uint32_t transport_start    = 0;
    uint32_t transport_continue = 0;
    uint32_t transport_stop     = 0;
    uint8_t  last_note          = 0;
    uint8_t  last_velocity      = 0;
    uint8_t  last_cc            = 0;
    uint8_t  last_cc_value      = 0;
    uint8_t  last_program       = 0;

    void Reset() { *this = MidiDiagnostics(); }
} g_diag;

struct Params
{
    float cutoff       = 0.45f;
    float resonance    = 0.20f;
    float attack       = 0.03f;
    float decay        = 0.15f;
    float sustain      = 0.78f;
    float release      = 0.25f;
    float bend_range   = 0.15f;
    float output_level = 0.35f;

    int waveform = Oscillator::WAVE_POLYBLEP_SAW;
} params;

float g_pitch_bend_norm     = 0.0f;
float g_pitch_bend_semitone = 0.0f;
int   g_display_page        = 0;

float MapCutoff(float normalized)
{
    return kMinCutoffHz + normalized * (kMaxCutoffHz - kMinCutoffHz);
}

float MapResonance(float normalized)
{
    return 0.05f + normalized * 0.90f;
}

float MapAttack(float normalized)
{
    return 0.001f + normalized * 1.50f;
}

float MapDecay(float normalized)
{
    return 0.005f + normalized * 1.50f;
}

float MapRelease(float normalized)
{
    return 0.005f + normalized * 2.50f;
}

float MapBendRange(float normalized)
{
    return 1.0f + normalized * 11.0f;
}

void UpdatePitchBend()
{
    g_pitch_bend_semitone = g_pitch_bend_norm * MapBendRange(params.bend_range);
}

struct Voice
{
    Oscillator osc;
    Svf        filter;
    Adsr       env;
    uint8_t    midi_note  = 60;
    uint8_t    velocity   = 0;
    bool       gate       = false;
    bool       active     = false;
    uint32_t   age_stamp  = 0;

    void Init(float sample_rate)
    {
        osc.Init(sample_rate);
        osc.SetWaveform(params.waveform);
        osc.SetAmp(0.6f);

        filter.Init(sample_rate);
        filter.SetDrive(0.7f);

        env.Init(sample_rate);
        ApplyParams();
    }

    void ApplyParams()
    {
        osc.SetWaveform(params.waveform);
        filter.SetFreq(MapCutoff(params.cutoff));
        filter.SetRes(MapResonance(params.resonance));
        env.SetAttackTime(MapAttack(params.attack));
        env.SetDecayTime(MapDecay(params.decay));
        env.SetSustainLevel(fclamp(params.sustain, 0.0f, 1.0f));
        env.SetReleaseTime(MapRelease(params.release));
    }

    void UpdatePitch(float bend_semitones)
    {
        if(active)
            osc.SetFreq(mtof(static_cast<float>(midi_note) + bend_semitones));
    }

    void NoteOn(uint8_t note, uint8_t vel, float bend_semitones, uint32_t stamp)
    {
        midi_note = note;
        velocity  = vel;
        gate      = true;
        active    = true;
        age_stamp = stamp;

        ApplyParams();
        UpdatePitch(bend_semitones);
    }

    void NoteOff() { gate = false; }

    float Process()
    {
        if(!active)
            return 0.0f;

        const float env_out = env.Process(gate);
        if(!gate && env_out < 0.0001f)
        {
            active = false;
            return 0.0f;
        }

        filter.Process(osc.Process());
        return filter.Low() * env_out * (static_cast<float>(velocity) / 127.0f);
    }
};

struct VoiceManager
{
    Voice    voices[kNumVoices];
    uint32_t allocation_counter = 1;
    uint32_t steals             = 0;
    uint32_t dropped_notes      = 0;

    void Init(float sample_rate)
    {
        for(size_t i = 0; i < kNumVoices; ++i)
            voices[i].Init(sample_rate);
    }

    Voice* FindVoice()
    {
        for(size_t i = 0; i < kNumVoices; ++i)
        {
            if(!voices[i].active)
                return &voices[i];
        }

        for(size_t i = 0; i < kNumVoices; ++i)
        {
            if(!voices[i].gate)
                return &voices[i];
        }

        Voice* oldest = &voices[0];
        for(size_t i = 1; i < kNumVoices; ++i)
        {
            if(voices[i].age_stamp < oldest->age_stamp)
                oldest = &voices[i];
        }
        return oldest;
    }

    void NoteOn(uint8_t note, uint8_t velocity, float bend_semitones)
    {
        Voice* target = FindVoice();
        if(target == nullptr)
        {
            dropped_notes++;
            return;
        }

        if(target->active && target->gate)
            steals++;

        target->NoteOn(note, velocity, bend_semitones, allocation_counter++);
    }

    void NoteOff(uint8_t note)
    {
        for(size_t i = 0; i < kNumVoices; ++i)
        {
            if(voices[i].active && voices[i].midi_note == note)
                voices[i].NoteOff();
        }
    }

    void AllNotesOff()
    {
        for(size_t i = 0; i < kNumVoices; ++i)
            voices[i].NoteOff();
    }

    void Refresh(float bend_semitones)
    {
        for(size_t i = 0; i < kNumVoices; ++i)
        {
            voices[i].ApplyParams();
            voices[i].UpdatePitch(bend_semitones);
        }
    }

    int ActiveCount() const
    {
        int active_count = 0;
        for(size_t i = 0; i < kNumVoices; ++i)
        {
            if(voices[i].active)
                active_count++;
        }
        return active_count;
    }

    float Process()
    {
        float sum = 0.0f;
        for(size_t i = 0; i < kNumVoices; ++i)
            sum += voices[i].Process();
        return sum * (1.0f / static_cast<float>(kNumVoices));
    }
} g_voice_mgr;

bool AcceptChannel(int zero_based_channel)
{
    return zero_based_channel == (kMidiChannelHuman - 1);
}

void UpdateDisplayValues()
{
    display.SetValue(0, params.cutoff);
    display.SetValue(1, params.resonance);
    display.SetValue(2, params.attack);
    display.SetValue(3, params.decay);
    display.SetValue(4, params.sustain);
    display.SetValue(5, params.release);
    display.SetValue(6, params.bend_range);
    display.SetValue(7, params.output_level);
}

void DrawDiagnosticsPage()
{
    char line[32];

    hw.display.Fill(false);
    hw.display.SetCursor(0, 0);
    hw.display.WriteString("OXI DIAG", Font_7x10, true);

    hw.display.SetCursor(0, 12);
    std::snprintf(line,
                  sizeof(line),
                  "Ch:%d Vo:%d St:%lu",
                  kMidiChannelHuman,
                  g_voice_mgr.ActiveCount(),
                  static_cast<unsigned long>(g_voice_mgr.steals));
    hw.display.WriteString(line, Font_6x8, true);

    hw.display.SetCursor(0, 22);
    std::snprintf(line,
                  sizeof(line),
                  "On:%lu Off:%lu Dr:%lu",
                  static_cast<unsigned long>(g_diag.note_on),
                  static_cast<unsigned long>(g_diag.note_off),
                  static_cast<unsigned long>(g_voice_mgr.dropped_notes));
    hw.display.WriteString(line, Font_6x8, true);

    hw.display.SetCursor(0, 32);
    std::snprintf(line,
                  sizeof(line),
                  "CC:%lu PB:%lu PC:%lu",
                  static_cast<unsigned long>(g_diag.control_change),
                  static_cast<unsigned long>(g_diag.pitch_bend),
                  static_cast<unsigned long>(g_diag.program_change));
    hw.display.WriteString(line, Font_6x8, true);

    hw.display.SetCursor(0, 42);
    std::snprintf(line,
                  sizeof(line),
                  "RT:%lu CLK:%lu ST:%lu",
                  static_cast<unsigned long>(g_diag.system_realtime),
                  static_cast<unsigned long>(g_diag.timing_clock),
                  static_cast<unsigned long>(g_diag.transport_start));
    hw.display.WriteString(line, Font_6x8, true);

    hw.display.SetCursor(0, 52);
    std::snprintf(line,
                  sizeof(line),
                  "N:%u V:%u CC%u=%u",
                  g_diag.last_note,
                  g_diag.last_velocity,
                  g_diag.last_cc,
                  g_diag.last_cc_value);
    hw.display.WriteString(line, Font_6x8, true);
    hw.display.Update();
}

void UpdateDisplayPage()
{
    UpdateDisplayValues();
    if(g_display_page == 0)
        display.UpdateCompact();
    else
        DrawDiagnosticsPage();
}

void ApplyParams()
{
    UpdatePitchBend();
    g_voice_mgr.Refresh(g_pitch_bend_semitone);
}

void ProcessKnobs()
{
    params.cutoff       = hw.knob[0].Process();
    params.resonance    = hw.knob[1].Process();
    params.attack       = hw.knob[2].Process();
    params.decay        = hw.knob[3].Process();
    params.sustain      = hw.knob[4].Process();
    params.release      = hw.knob[5].Process();
    params.bend_range   = hw.knob[6].Process();
    params.output_level = hw.knob[7].Process();

    ApplyParams();
    UpdateDisplayValues();
}

void SetWaveform(int waveform)
{
    params.waveform = waveform;
    ApplyParams();
}

void ResetDiagnostics()
{
    g_diag.Reset();
    g_voice_mgr.steals        = 0;
    g_voice_mgr.dropped_notes = 0;
}

void HandleKeyboardShortcuts()
{
    if(hw.KeyboardRisingEdge(kKeyAIndices[0]))
        SetWaveform(Oscillator::WAVE_SIN);
    if(hw.KeyboardRisingEdge(kKeyAIndices[1]))
        SetWaveform(Oscillator::WAVE_TRI);
    if(hw.KeyboardRisingEdge(kKeyAIndices[2]))
        SetWaveform(Oscillator::WAVE_POLYBLEP_SAW);
    if(hw.KeyboardRisingEdge(kKeyAIndices[3]))
        SetWaveform(Oscillator::WAVE_SQUARE);

    if(hw.KeyboardRisingEdge(kKeyAIndices[4]))
        g_display_page = 0;
    if(hw.KeyboardRisingEdge(kKeyAIndices[5]))
        g_display_page = 1;
    if(hw.KeyboardRisingEdge(kKeyAIndices[6]))
        ResetDiagnostics();
    if(hw.KeyboardRisingEdge(kKeyAIndices[7]))
        g_voice_mgr.AllNotesOff();

    if(hw.sw[0].RisingEdge())
        g_voice_mgr.AllNotesOff();

    if(hw.sw[1].RisingEdge())
        g_display_page = (g_display_page + 1) % 2;
}

void UpdateStatusLeds()
{
    for(int i = 0; i < 8; ++i)
        hw.led_driver.SetLed(kLedKnobs[i], fclamp(display.GetValue(i), 0.0f, 1.0f));

    key_leds.Clear();
    key_leds.SetA(0, params.waveform == Oscillator::WAVE_SIN);
    key_leds.SetA(1, params.waveform == Oscillator::WAVE_TRI);
    key_leds.SetA(2, params.waveform == Oscillator::WAVE_POLYBLEP_SAW);
    key_leds.SetA(3, params.waveform == Oscillator::WAVE_SQUARE);
    key_leds.SetA(4, g_display_page == 0);
    key_leds.SetA(5, g_display_page == 1);
    key_leds.SetA(6, g_diag.control_change > 0 || g_diag.pitch_bend > 0
                         || g_diag.program_change > 0);
    key_leds.SetA(7, g_voice_mgr.steals > 0 || g_voice_mgr.dropped_notes > 0);

    for(int i = 0; i < 8; ++i)
        key_leds.SetB(i, i < g_voice_mgr.ActiveCount());

    hw.led_driver.SetLed(kLedSwitches[0], g_voice_mgr.ActiveCount() > 0 ? 1.0f : 0.0f);
    hw.led_driver.SetLed(kLedSwitches[1], g_display_page == 1 ? 1.0f : 0.15f);
    key_leds.Update(0.8f);
}

void MaybeLogStatus()
{
    static uint32_t last_log_ms = 0;
    const uint32_t  now         = System::GetNow();
    if(now - last_log_ms < 500)
        return;

    last_log_ms = now;
    hw.seed.PrintLine("[OXI] ch=%d active=%d on=%lu off=%lu cc=%lu pb=%lu pc=%lu rt=%lu steals=%lu dropped=%lu",
                      kMidiChannelHuman,
                      g_voice_mgr.ActiveCount(),
                      static_cast<unsigned long>(g_diag.note_on),
                      static_cast<unsigned long>(g_diag.note_off),
                      static_cast<unsigned long>(g_diag.control_change),
                      static_cast<unsigned long>(g_diag.pitch_bend),
                      static_cast<unsigned long>(g_diag.program_change),
                      static_cast<unsigned long>(g_diag.system_realtime),
                      static_cast<unsigned long>(g_voice_mgr.steals),
                      static_cast<unsigned long>(g_voice_mgr.dropped_notes));
}

void HandleMidiMessage(MidiEvent msg)
{
    switch(msg.type)
    {
        case NoteOn:
        {
            if(!AcceptChannel(msg.channel))
                return;

            const NoteOnEvent note = msg.AsNoteOn();
            g_diag.last_note       = note.note;
            g_diag.last_velocity   = note.velocity;

            if(note.velocity == 0)
            {
                g_diag.note_off++;
                g_voice_mgr.NoteOff(note.note);
            }
            else
            {
                g_diag.note_on++;
                g_voice_mgr.NoteOn(note.note, note.velocity, g_pitch_bend_semitone);
            }
        }
        break;

        case NoteOff:
        {
            if(!AcceptChannel(msg.channel))
                return;

            const NoteOffEvent note = msg.AsNoteOff();
            g_diag.note_off++;
            g_diag.last_note     = note.note;
            g_diag.last_velocity = note.velocity;
            g_voice_mgr.NoteOff(note.note);
        }
        break;

        case ControlChange:
        {
            if(!AcceptChannel(msg.channel))
                return;

            const ControlChangeEvent cc = msg.AsControlChange();
            g_diag.control_change++;
            g_diag.last_cc       = cc.control_number;
            g_diag.last_cc_value = cc.value;

            if(cc.control_number == 74)
            {
                params.cutoff = static_cast<float>(cc.value) / 127.0f;
                display.SetActiveParam(0);
                ApplyParams();
            }
            else if(cc.control_number == 71)
            {
                params.resonance = static_cast<float>(cc.value) / 127.0f;
                display.SetActiveParam(1);
                ApplyParams();
            }
            else if(cc.control_number == 123 || cc.control_number == 120)
            {
                g_voice_mgr.AllNotesOff();
            }
        }
        break;

        case PitchBend:
        {
            if(!AcceptChannel(msg.channel))
                return;

            const PitchBendEvent bend = msg.AsPitchBend();
            g_diag.pitch_bend++;
            g_pitch_bend_norm = fclamp(static_cast<float>(bend.value) / 8192.0f,
                                       -1.0f,
                                       1.0f);
            UpdatePitchBend();
            g_voice_mgr.Refresh(g_pitch_bend_semitone);
        }
        break;

        case ProgramChange:
        {
            if(!AcceptChannel(msg.channel))
                return;

            const ProgramChangeEvent program = msg.AsProgramChange();
            g_diag.program_change++;
            g_diag.last_program = program.program;
        }
        break;

        case SystemRealTime:
        {
            g_diag.system_realtime++;
            switch(msg.srt_type)
            {
                case TimingClock: g_diag.timing_clock++; break;
                case Start: g_diag.transport_start++; break;
                case Continue: g_diag.transport_continue++; break;
                case Stop: g_diag.transport_stop++; break;
                default: break;
            }
        }
        break;

        default: break;
    }
}

void AudioCallback(AudioHandle::InputBuffer in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    for(size_t i = 0; i < size; ++i)
    {
        float sig = g_voice_mgr.Process();
        sig       = tanhf(sig * 1.5f) * params.output_level;

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

    const float sample_rate = hw.AudioSampleRate();
    g_voice_mgr.Init(sample_rate);
    ApplyParams();

    key_leds.Init(&hw);

    display.Init(&hw);
    display.SetTitle("FIELD MIDIOXI");
    display.SetLabel(0, "Cutoff");
    display.SetLabel(1, "Res");
    display.SetLabel(2, "Attack");
    display.SetLabel(3, "Decay");
    display.SetLabel(4, "Sustain");
    display.SetLabel(5, "Release");
    display.SetLabel(6, "Bend");
    display.SetLabel(7, "Volume");
    UpdateDisplayValues();

    hw.seed.StartLog(false);
    hw.seed.PrintLine("[BOOT] Field_MidiOXI");
    hw.seed.PrintLine("[BOOT] Expect OXI on MIDI channel %d", kMidiChannelHuman);

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
        UpdateDisplayPage();
        UpdateStatusLeds();
        MaybeLogStatus();

        System::Delay(1);
    }
}
