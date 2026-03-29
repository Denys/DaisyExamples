#include "daisy_field.h"
#include "daisysp.h"
#include "../../foundation_examples/field_defaults.h"
#include "../../foundation_examples/field_instrument_ui.h"
#include "plaits/dsp/dsp.h"
#include "plaits/dsp/voice.h"

using namespace daisy;
using namespace daisysp;
using namespace FieldDefaults;
using namespace FieldInstrumentUI;

namespace
{
constexpr bool     kEnableBootLog      = false;
constexpr uint32_t kZoomMs             = 1400;
constexpr float    kZoomDelta          = 0.015f;
constexpr uint32_t kHoldMs             = 300;
constexpr int      kFullRangeIndex     = 8;
constexpr int      kTriggerPulseBlocks = 2;
constexpr size_t   kSharedBufferSize   = 16384;

DaisyField       hw;
OneHotKeyLedBank key_leds;
ParamZoomState   zoom_state;

plaits::Voice       voice;
plaits::Patch       patch       = {};
plaits::Modulations modulations = {};
plaits::Voice::Frame audio_frames[plaits::kMaxBlockSize];
char                shared_buffer[kSharedBufferSize] = {};

const char* kEngineSlotNames[16] = {
    "PAIR",  "SHAPE", "FM",    "FORMANT",
    "HARM",  "WTBL",  "CHORD", "SPEECH",
    "CLOUD", "FILT",  "PART",  "STRING",
    "MODAL", "BD",    "SNARE", "HHAT",
};

const int kEngineSlotToEngine[16] = {
    0, -1, 1, 2,
    3, -1, -1, -1,
    4, -1, 5, -1,
    -1, 6, 7, 8,
};

const char* kParamNames[8] = {
    "Freq", "Harm", "Timbre", "Morph",
    "FM Amt", "Tim Amt", "Mor Amt", "Level",
};

const char* kRangeNames[9] = {
    "C0 +/-7", "C1 +/-7", "C2 +/-7",
    "C3 +/-7", "C4 +/-7", "C5 +/-7",
    "C6 +/-7", "C7 +/-7", "C0-C8",
};

struct Params
{
    float knob_values[8] = {0.50f, 0.40f, 0.50f, 0.50f, 0.50f, 0.50f, 0.50f, 0.85f};
    float frequency      = 0.50f;
    float harmonics      = 0.40f;
    float timbre         = 0.50f;
    float morph          = 0.50f;
    float fm_amount      = 0.50f;
    float timbre_amount  = 0.50f;
    float morph_amount   = 0.50f;
    float level          = 0.85f;
    float lpg_colour     = 0.50f;
    float decay          = 0.50f;
    int   engine_slot    = 0;
    int   freq_range     = kFullRangeIndex;
} params;

struct MidiState
{
    uint8_t  note          = 60;
    float    velocity      = 0.85f;
    bool     note_held     = false;
    bool     gate_open     = false;
    bool     sustain_pedal = false;
    uint32_t last_event_ms = 0;
    int      trigger_blocks = 0;
} midi;

struct HoldState
{
    uint32_t down_ms = 0;
    bool     active  = false;
} hold_states[2];

float level_current = params.level;

float Clamp01(float value) { return fclamp(value, 0.0f, 1.0f); }

float KnobToBipolar(float value) { return value * 2.0f - 1.0f; }

float CoarseNoteFromKnob()
{
    const float normalized = Clamp01(params.frequency);
    if(params.freq_range == kFullRangeIndex)
        return 12.0f + normalized * 96.0f;

    const float center = 12.0f + static_cast<float>(params.freq_range) * 12.0f;
    return fclamp(center - 7.0f + normalized * 14.0f, 0.0f, 120.0f);
}

void FormatSignedPercent(char* buffer, size_t size, float value)
{
    snprintf(buffer,
             size,
             "%+d%%",
             static_cast<int>(KnobToBipolar(Clamp01(value)) * 100.0f + (value >= 0.5f ? 0.5f : -0.5f)));
}

void FormatCoarseNote(char* buffer, size_t size)
{
    char note_name[8];
    const uint8_t coarse_note = static_cast<uint8_t>(CoarseNoteFromKnob() + 0.5f);
    FormatMidiNoteName(note_name, sizeof(note_name), coarse_note);
    snprintf(buffer, size, "%s", note_name);
}

void SetEngineLeds()
{
    if(params.engine_slot < 8)
    {
        key_leds.SetActiveA(params.engine_slot);
        key_leds.ClearB();
    }
    else
    {
        key_leds.ClearA();
        key_leds.SetActiveB(params.engine_slot - 8);
    }
}

void SetRangeLeds()
{
    if(params.freq_range < 8)
    {
        key_leds.SetActiveA(params.freq_range);
        key_leds.ClearB();
    }
    else
    {
        key_leds.ClearA();
        key_leds.SetActiveB(0);
    }
}

void RefreshLeds()
{
    if(hold_states[1].active)
        SetRangeLeds();
    else
        SetEngineLeds();
}

void SelectEngine(int engine)
{
    const int slot = (engine < 0) ? 0 : (engine > 15 ? 15 : engine);
    if(kEngineSlotToEngine[slot] < 0)
        return;

    params.engine_slot = slot;
    RefreshLeds();
}

void PanicVoice()
{
    midi.note_held      = false;
    midi.gate_open      = false;
    midi.sustain_pedal  = false;
    midi.trigger_blocks = 0;
}

void StartNote(uint8_t note, uint8_t velocity)
{
    midi.note          = note;
    midi.velocity      = fclamp(static_cast<float>(velocity) / 127.0f, 0.05f, 1.0f);
    midi.note_held     = true;
    midi.gate_open     = true;
    midi.trigger_blocks = kTriggerPulseBlocks;
    midi.last_event_ms = System::GetNow();
}

void StopNote(uint8_t note)
{
    if(note != midi.note)
        return;

    midi.note_held = false;
    if(!midi.sustain_pedal)
        midi.gate_open = false;

    midi.last_event_ms = System::GetNow();
}

void ReleaseSustain()
{
    if(!midi.note_held)
        midi.gate_open = false;
}

void HandleMidiMessage(MidiEvent msg)
{
    switch(msg.type)
    {
        case NoteOn:
        {
            const NoteOnEvent note = msg.AsNoteOn();
            if(note.velocity == 0)
                StopNote(note.note);
            else
                StartNote(note.note, note.velocity);
            break;
        }

        case NoteOff:
        {
            const NoteOffEvent note = msg.AsNoteOff();
            StopNote(note.note);
            break;
        }

        case ControlChange:
        {
            const ControlChangeEvent cc = msg.AsControlChange();
            if(cc.control_number == 64)
            {
                midi.sustain_pedal = cc.value >= 64;
                if(!midi.sustain_pedal)
                    ReleaseSustain();
                midi.last_event_ms = System::GetNow();
            }
            break;
        }

        default: break;
    }
}

void UpdateHoldState()
{
    const uint32_t now = System::GetNow();

    for(int i = 0; i < 2; ++i)
    {
        if(hw.sw[i].RisingEdge())
            hold_states[i].down_ms = now;

        hold_states[i].active
            = hw.sw[i].Pressed() && (now - hold_states[i].down_ms) >= kHoldMs;

        if(hw.sw[i].FallingEdge())
        {
            const bool was_hold = (now - hold_states[i].down_ms) >= kHoldMs;
            hold_states[i].active = false;

            if(i == 0 && !was_hold)
                PanicVoice();
            if(i == 1 && !was_hold)
                zoom_state.Clear();
        }
    }
}

void ReadKnobs(float raw[8])
{
    for(int i = 0; i < 8; ++i)
        raw[i] = hw.knob[i].Process();
}

void ProcessNormalKnobs(const float raw[8])
{
    for(int i = 0; i < 8; ++i)
        params.knob_values[i] = raw[i];

    params.frequency     = raw[0];
    params.harmonics     = raw[1];
    params.timbre        = raw[2];
    params.morph         = raw[3];
    params.fm_amount     = raw[4];
    params.timbre_amount = raw[5];
    params.morph_amount  = raw[6];
    params.level         = raw[7];

    zoom_state.Capture(params.knob_values, System::GetNow(), kZoomDelta);
}

void ProcessHiddenKnobs(const float raw[8])
{
    params.lpg_colour = raw[0];
    params.decay      = raw[1];
}

void ProcessKeys()
{
    if(hold_states[1].active)
    {
        for(int i = 0; i < 8; ++i)
        {
            if(hw.KeyboardRisingEdge(kKeyAIndices[i]))
            {
                params.freq_range = i;
                RefreshLeds();
                return;
            }
        }

        if(hw.KeyboardRisingEdge(kKeyBIndices[0]))
        {
            params.freq_range = kFullRangeIndex;
            RefreshLeds();
        }
        return;
    }

    for(int i = 0; i < 8; ++i)
    {
        if(hw.KeyboardRisingEdge(kKeyAIndices[i]))
        {
            SelectEngine(i);
            return;
        }

        if(hw.KeyboardRisingEdge(kKeyBIndices[i]))
        {
            SelectEngine(i + 8);
            return;
        }
    }
}

void UpdatePatchState()
{
    patch.note                        = CoarseNoteFromKnob();
    patch.harmonics                   = Clamp01(params.harmonics);
    patch.timbre                      = Clamp01(params.timbre);
    patch.morph                       = Clamp01(params.morph);
    patch.frequency_modulation_amount = KnobToBipolar(params.fm_amount);
    patch.timbre_modulation_amount    = KnobToBipolar(params.timbre_amount);
    patch.morph_modulation_amount     = KnobToBipolar(params.morph_amount);
    patch.engine                      = kEngineSlotToEngine[params.engine_slot];
    patch.decay                       = Clamp01(params.decay);
    patch.lpg_colour                  = Clamp01(params.lpg_colour);

    modulations.engine           = 0.0f;
    modulations.note             = midi.note_held || midi.gate_open
                                     ? static_cast<float>(midi.note) - 60.0f
                                     : 0.0f;
    modulations.frequency        = 0.0f;
    modulations.harmonics        = 0.0f;
    modulations.timbre           = 0.0f;
    modulations.morph            = 0.0f;
    modulations.trigger          = midi.trigger_blocks > 0 ? 5.0f : 0.0f;
    modulations.level            = midi.gate_open ? midi.velocity * level_current : 0.0f;
    modulations.frequency_patched = false;
    modulations.timbre_patched    = false;
    modulations.morph_patched     = false;
    modulations.trigger_patched   = true;
    modulations.level_patched     = midi.gate_open;
}

void FormatParamValue(int idx, char* buffer, size_t size)
{
    switch(idx)
    {
        case 0: FormatCoarseNote(buffer, size); break;
        case 1: FormatPercent(buffer, size, params.knob_values[idx]); break;
        case 2: FormatPercent(buffer, size, params.knob_values[idx]); break;
        case 3: FormatPercent(buffer, size, params.knob_values[idx]); break;
        case 4: FormatSignedPercent(buffer, size, params.knob_values[idx]); break;
        case 5: FormatSignedPercent(buffer, size, params.knob_values[idx]); break;
        case 6: FormatSignedPercent(buffer, size, params.knob_values[idx]); break;
        case 7: FormatPercent(buffer, size, params.knob_values[idx]); break;
        default: snprintf(buffer, size, "%.2f", params.knob_values[idx]); break;
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

void DrawHiddenKnobPage()
{
    char line[40];

    hw.display.Fill(false);
    hw.display.SetCursor(0, 0);
    hw.display.WriteString("PLAITS ALT KNOBS", Font_7x10, true);

    snprintf(line,
             sizeof(line),
             "K1 LPG Col : %d%%",
             static_cast<int>(params.lpg_colour * 100.0f + 0.5f));
    hw.display.SetCursor(0, 14);
    hw.display.WriteString(line, Font_6x8, true);

    snprintf(line,
             sizeof(line),
             "K2 Decay   : %d%%",
             static_cast<int>(params.decay * 100.0f + 0.5f));
    hw.display.SetCursor(0, 24);
    hw.display.WriteString(line, Font_6x8, true);

    hw.display.SetCursor(0, 38);
    hw.display.WriteString("Original hidden page", Font_6x8, true);
    hw.display.SetCursor(0, 48);
    hw.display.WriteString("Hold SW1 to edit", Font_6x8, true);
    hw.display.SetCursor(0, 58);
    hw.display.WriteString("Tap SW1 = Panic", Font_6x8, true);
    hw.display.Update();
}

void DrawRangePage()
{
    char line[40];
    char note_name[8];
    const uint8_t coarse_note = static_cast<uint8_t>(CoarseNoteFromKnob() + 0.5f);
    FormatMidiNoteName(note_name, sizeof(note_name), coarse_note);

    hw.display.Fill(false);
    hw.display.SetCursor(0, 0);
    hw.display.WriteString("PLAITS FREQ RANGE", Font_7x10, true);

    snprintf(line, sizeof(line), "Now : %s", kRangeNames[params.freq_range]);
    hw.display.SetCursor(0, 14);
    hw.display.WriteString(line, Font_6x8, true);

    snprintf(line, sizeof(line), "Base: %s", note_name);
    hw.display.SetCursor(0, 24);
    hw.display.WriteString(line, Font_6x8, true);

    hw.display.SetCursor(0, 38);
    hw.display.WriteString("A1-A8 = C0..C7 +/-7", Font_6x8, true);
    hw.display.SetCursor(0, 48);
    hw.display.WriteString("B1 = Full 8 oct", Font_6x8, true);
    hw.display.SetCursor(0, 58);
    hw.display.WriteString("Tap SW2 = Clr Zoom", Font_6x8, true);
    hw.display.Update();
}

void DrawOverview()
{
    char line[40];
    char note_name[8];
    const bool note_active = midi.note_held || midi.gate_open;
    FormatMidiNoteName(note_name, sizeof(note_name), midi.note);

    hw.display.Fill(false);

    hw.display.SetCursor(0, 0);
    hw.display.WriteString("FIELD MI PLAITS", Font_7x10, true);

    snprintf(line,
             sizeof(line),
             "Eng:%02d %s",
             params.engine_slot + 1,
             kEngineSlotNames[params.engine_slot]);
    hw.display.SetCursor(0, 10);
    hw.display.WriteString(line, Font_6x8, true);

    snprintf(line,
             sizeof(line),
             "Note:%s Range:%s",
             note_active ? note_name : "--",
             kRangeNames[params.freq_range]);
    hw.display.SetCursor(0, 18);
    hw.display.WriteString(line, Font_6x8, true);

    snprintf(line,
             sizeof(line),
             "H:%d T:%d M:%d",
             static_cast<int>(params.harmonics * 100.0f + 0.5f),
             static_cast<int>(params.timbre * 100.0f + 0.5f),
             static_cast<int>(params.morph * 100.0f + 0.5f));
    hw.display.SetCursor(0, 26);
    hw.display.WriteString(line, Font_6x8, true);

    snprintf(line,
             sizeof(line),
             "FM:%+d Ti:%+d Mo:%+d",
             static_cast<int>(KnobToBipolar(params.fm_amount) * 100.0f),
             static_cast<int>(KnobToBipolar(params.timbre_amount) * 100.0f),
             static_cast<int>(KnobToBipolar(params.morph_amount) * 100.0f));
    hw.display.SetCursor(0, 34);
    hw.display.WriteString(line, Font_6x8, true);

    snprintf(line,
             sizeof(line),
             "Lvl:%d Col:%d Dec:%d",
             static_cast<int>(params.level * 100.0f + 0.5f),
             static_cast<int>(params.lpg_colour * 100.0f + 0.5f),
             static_cast<int>(params.decay * 100.0f + 0.5f));
    hw.display.SetCursor(0, 42);
    hw.display.WriteString(line, Font_6x8, true);

    snprintf(line,
             sizeof(line),
             "Midi:%s Gate:%s",
             (System::GetNow() - midi.last_event_ms) < 1200 ? "RX" : "IDLE",
             midi.gate_open ? "ON" : "OFF");
    hw.display.SetCursor(0, 50);
    hw.display.WriteString(line, Font_6x8, true);

    hw.display.SetCursor(0, 58);
    hw.display.WriteString("A/B=Eng SW1/2 Hold", Font_6x8, true);

    hw.display.Update();
}

void UpdateDisplay()
{
    if(hold_states[0].active)
        DrawHiddenKnobPage();
    else if(hold_states[1].active)
        DrawRangePage();
    else if(zoom_state.IsActive(System::GetNow(), kZoomMs))
        DrawZoom();
    else
        DrawOverview();
}

void AudioCallback(AudioHandle::InputBuffer in,
                   AudioHandle::OutputBuffer out,
                   size_t size)
{
    fonepole(level_current, params.level, 0.0015f);

    UpdatePatchState();
    voice.Render(patch, modulations, audio_frames, size);

    for(size_t i = 0; i < size; ++i)
    {
        out[0][i] = static_cast<float>(audio_frames[i].out) / 32768.0f;
        out[1][i] = static_cast<float>(audio_frames[i].aux) / 32768.0f;
        (void)in;
    }

    if(midi.trigger_blocks > 0)
        --midi.trigger_blocks;
}

} // namespace

int main(void)
{
    hw.Init();
    hw.SetAudioBlockSize(plaits::kMaxBlockSize);
    hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

    if(kEnableBootLog)
    {
        hw.seed.StartLog(false);
        hw.seed.PrintLine("[BOOT] Field_MI_Plaits");
    }

    stmlib::BufferAllocator allocator(shared_buffer, sizeof(shared_buffer));
    voice.Init(&allocator);

    key_leds.Init(&hw);
    zoom_state.Init(params.frequency);
    SelectEngine(params.engine_slot);
    RefreshLeds();
    key_leds.Update();

    hw.midi.StartReceive();
    hw.StartAdc();
    hw.StartAudio(AudioCallback);

    while(1)
    {
        hw.midi.Listen();
        while(hw.midi.HasEvents())
            HandleMidiMessage(hw.midi.PopEvent());

        hw.ProcessAllControls();
        UpdateHoldState();

        float raw_knobs[8] = {};
        ReadKnobs(raw_knobs);

        if(hold_states[0].active)
            ProcessHiddenKnobs(raw_knobs);
        else
            ProcessNormalKnobs(raw_knobs);

        ProcessKeys();
        RefreshLeds();
        key_leds.Update();
        UpdateDisplay();

        System::Delay(1);
    }
}
