#include "daisy_field.h"
#include "daisysp.h"
#include "../../foundation_examples/field_defaults.h"
#include "../../foundation_examples/field_parameter_banks.h"
#include "../../foundation_examples/field_instrument_ui.h"
#include "plaits/dsp/dsp.h"
#include "plaits/dsp/voice.h"

using namespace daisy;
using namespace daisysp;
using namespace FieldDefaults;
using namespace FieldParameterBanks;
using namespace FieldInstrumentUI;

static_assert(FieldParameterBanks::kNumKnobs == FieldDefaults::kNumKnobs,
              "Field parameter helper knob count mismatch");

namespace
{
constexpr bool     kEnableBootLog      = false;
constexpr uint32_t kZoomMs             = 1400;
constexpr float    kZoomDelta          = 0.015f;
constexpr float    kCatchDelta         = 0.015f;
constexpr uint32_t kHoldMs             = 300;
constexpr int      kFullRangeIndex     = 8;
constexpr int      kTriggerPulseBlocks = 2;
constexpr size_t   kSharedBufferSize   = 16384;
constexpr float    kKnobLedCapturedBrightness   = 1.0f;
constexpr float    kKnobLedUncapturedBrightness = 0.35f;

DaisyField       hw;
OneHotKeyLedBank key_leds;
ParamZoomState   zoom_state;

plaits::Voice       voice;
plaits::Patch       patch       = {};
plaits::Modulations modulations = {};
plaits::Voice::Frame audio_frames[plaits::kMaxBlockSize];
char                shared_buffer[kSharedBufferSize] = {};
ParamBankSet        param_banks;
ParamBank           active_knob_bank = ParamBank::Main;

const int kEngineSlotToEngine[16] = {
    0, -1, 1, 2,
    3, -1, -1, -1,
    4, -1, 5, -1,
    -1, 6, 7, 8,
};

const char* kMainParamNames[8] = {
    "Freq", "Harm", "Timb", "Morph",
    "FM", "TiM", "MoM", "Lvl",
};

const char* kAltParamNames[8] = {
    "LPG", "Dec", "Rsv", "Rsv",
    "Rsv", "Rsv", "Rsv", "Rsv",
};

constexpr float kMainBankDefaults[8] = {
    0.50f, 0.40f, 0.50f, 0.50f, 0.50f, 0.50f, 0.50f, 0.85f,
};

struct Params
{
    float lpg_colour  = 0.50f;
    float decay       = 0.50f;
    int   engine_slot = 0;
    int   freq_range  = kFullRangeIndex;
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

float level_current = kMainBankDefaults[7];

float Clamp01(float value) { return fclamp(value, 0.0f, 1.0f); }

float KnobToBipolar(float value) { return value * 2.0f - 1.0f; }

bool IsEditableBankKnob(ParamBank bank, int idx)
{
    if(idx < 0 || idx >= 8)
        return false;

    return bank == ParamBank::Alt ? idx < 2 : true;
}

void SetActiveKnobBank(ParamBank bank)
{
    if(active_knob_bank == bank)
    {
        param_banks.SetActiveBank(bank);
        return;
    }

    active_knob_bank = bank;
    param_banks.SetActiveBank(bank);
    float values[8] = {};
    for(int i = 0; i < 8; ++i)
        values[i] = param_banks.Read(bank, i);
    zoom_state.SetBaseline(values);
    zoom_state.Clear();
}

void ProcessBankKnobs(ParamBank bank, const float raw[8])
{
    for(int i = 0; i < 8; ++i)
    {
        if(!IsEditableBankKnob(bank, i))
            continue;

        if(!param_banks.IsCaptured(bank, i))
            param_banks.CatchIfClose(bank, i, raw[i], kCatchDelta);

        if(!param_banks.IsCaptured(bank, i))
            continue;

        param_banks.Write(bank, i, raw[i]);
        if(bank == ParamBank::Alt)
        {
            if(i == 0)
                params.lpg_colour = raw[i];
            else if(i == 1)
                params.decay = raw[i];
        }
    }

    float active_bank_values[8] = {};
    for(int i = 0; i < 8; ++i)
        active_bank_values[i] = param_banks.Read(bank, i);
    zoom_state.Capture(active_bank_values, System::GetNow(), kZoomDelta);
}

void UpdateKnobLeds(ParamBank bank)
{
    for(int i = 0; i < 8; ++i)
    {
        if(!IsEditableBankKnob(bank, i))
        {
            hw.led_driver.SetLed(kLedKnobs[i], 0.0f);
            continue;
        }

        const float stored_value = Clamp01(param_banks.Read(bank, i));
        const float brightness
            = param_banks.IsCaptured(bank, i) ? kKnobLedCapturedBrightness
                                              : kKnobLedUncapturedBrightness;
        hw.led_driver.SetLed(kLedKnobs[i], stored_value * brightness);
    }
}

void InitParamBanks()
{
    param_banks.Init(0.5f);

    for(int i = 0; i < 8; ++i)
        param_banks.Write(ParamBank::Main, i, kMainBankDefaults[i]);

    param_banks.Write(ParamBank::Alt, 0, params.lpg_colour);
    param_banks.Write(ParamBank::Alt, 1, params.decay);
    for(int i = 2; i < 8; ++i)
        param_banks.Write(ParamBank::Alt, i, 0.5f);

    active_knob_bank = ParamBank::Main;
    param_banks.SetActiveBank(ParamBank::Main);
    float values[8] = {};
    for(int i = 0; i < 8; ++i)
        values[i] = param_banks.Read(ParamBank::Main, i);
    zoom_state.SetBaseline(values);
    zoom_state.Clear();
}

float CoarseNoteFromValue(float normalized)
{
    if(params.freq_range == kFullRangeIndex)
        return 12.0f + normalized * 96.0f;

    const float center = 12.0f + static_cast<float>(params.freq_range) * 12.0f;
    return fclamp(center - 7.0f + normalized * 14.0f, 0.0f, 120.0f);
}

void FormatRangeName(char* buffer, size_t size)
{
    if(params.freq_range == kFullRangeIndex)
        snprintf(buffer, size, "C0-C8");
    else
        snprintf(buffer, size, "C%d+/-7", params.freq_range);
}

void FormatSignedPercent(char* buffer, size_t size, float value)
{
    snprintf(buffer,
             size,
             "%+d%%",
             static_cast<int>(KnobToBipolar(Clamp01(value)) * 100.0f + (value >= 0.5f ? 0.5f : -0.5f)));
}

void FormatCoarseNote(char* buffer, size_t size, float normalized)
{
    char note_name[8];
    const float    note_value  = Clamp01(normalized);
    const uint8_t coarse_note
        = static_cast<uint8_t>(CoarseNoteFromValue(note_value) + 0.5f);
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
    UpdateKnobLeds(active_knob_bank);

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
    ProcessBankKnobs(ParamBank::Main, raw);
}

void ProcessHiddenKnobs(const float raw[8])
{
    ProcessBankKnobs(ParamBank::Alt, raw);
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
                return;
            }
        }

        if(hw.KeyboardRisingEdge(kKeyBIndices[0]))
        {
            params.freq_range = kFullRangeIndex;
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
    const float frequency      = Clamp01(param_banks.Read(ParamBank::Main, 0));
    const float harmonics      = Clamp01(param_banks.Read(ParamBank::Main, 1));
    const float timbre         = Clamp01(param_banks.Read(ParamBank::Main, 2));
    const float morph          = Clamp01(param_banks.Read(ParamBank::Main, 3));
    const float fm_amount      = Clamp01(param_banks.Read(ParamBank::Main, 4));
    const float timbre_amount  = Clamp01(param_banks.Read(ParamBank::Main, 5));
    const float morph_amount   = Clamp01(param_banks.Read(ParamBank::Main, 6));
    const float level          = Clamp01(param_banks.Read(ParamBank::Main, 7));
    const float lpg_colour     = Clamp01(param_banks.Read(ParamBank::Alt, 0));
    const float decay          = Clamp01(param_banks.Read(ParamBank::Alt, 1));

    patch.note                        = CoarseNoteFromValue(frequency);
    patch.harmonics                   = harmonics;
    patch.timbre                      = timbre;
    patch.morph                       = morph;
    patch.frequency_modulation_amount = KnobToBipolar(fm_amount);
    patch.timbre_modulation_amount    = KnobToBipolar(timbre_amount);
    patch.morph_modulation_amount     = KnobToBipolar(morph_amount);
    patch.engine                      = kEngineSlotToEngine[params.engine_slot];
    patch.decay                       = decay;
    patch.lpg_colour                  = lpg_colour;

    modulations.engine           = 0.0f;
    modulations.note             = midi.note_held || midi.gate_open
                                     ? static_cast<float>(midi.note) - 60.0f
                                     : 0.0f;
    modulations.frequency        = 0.0f;
    modulations.harmonics        = 0.0f;
    modulations.timbre           = 0.0f;
    modulations.morph            = 0.0f;
    modulations.trigger          = midi.trigger_blocks > 0 ? 5.0f : 0.0f;
    modulations.level            = midi.gate_open ? midi.velocity * level : 0.0f;
    modulations.frequency_patched = false;
    modulations.timbre_patched    = false;
    modulations.morph_patched     = false;
    modulations.trigger_patched   = true;
    modulations.level_patched     = midi.gate_open;
}

void FormatParamValue(ParamBank bank, int idx, char* buffer, size_t size)
{
    const float value = Clamp01(param_banks.Read(bank, idx));

    if(bank == ParamBank::Alt)
    {
        if(idx >= 2)
        {
            snprintf(buffer, size, "Reserved");
            return;
        }

        FormatPercent(buffer, size, value);
        return;
    }

    switch(idx)
    {
        case 0: FormatCoarseNote(buffer, size, value); break;
        case 1: FormatPercent(buffer, size, value); break;
        case 2: FormatPercent(buffer, size, value); break;
        case 3: FormatPercent(buffer, size, value); break;
        case 4: FormatSignedPercent(buffer, size, value); break;
        case 5: FormatSignedPercent(buffer, size, value); break;
        case 6: FormatSignedPercent(buffer, size, value); break;
        case 7: FormatPercent(buffer, size, value); break;
        default: snprintf(buffer, size, "%.2f", value); break;
    }
}

void DrawZoom()
{
    const int idx = zoom_state.ActiveIndex();
    if(idx < 0 || idx >= 8)
        return;

    char value[32];
    FormatParamValue(active_knob_bank, idx, value, sizeof(value));

    hw.display.Fill(false);
    hw.display.SetCursor(0, 0);
    hw.display.WriteString(active_knob_bank == ParamBank::Alt ? kAltParamNames[idx]
                                                              : kMainParamNames[idx],
                           Font_7x10,
                           true);
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
    hw.display.WriteString("ALT BANK", Font_7x10, true);

    snprintf(line,
             sizeof(line),
             "K1 LPG:%d%%",
             static_cast<int>(param_banks.Read(ParamBank::Alt, 0) * 100.0f + 0.5f));
    hw.display.SetCursor(0, 14);
    hw.display.WriteString(line, Font_6x8, true);

    snprintf(line,
             sizeof(line),
             "K2 Dec:%d%%",
             static_cast<int>(param_banks.Read(ParamBank::Alt, 1) * 100.0f + 0.5f));
    hw.display.SetCursor(0, 24);
    hw.display.WriteString(line, Font_6x8, true);

    hw.display.SetCursor(0, 38);
    hw.display.WriteString("K3-K8 Rsv", Font_6x8, true);
    hw.display.SetCursor(0, 48);
    hw.display.WriteString("K1/K2 store", Font_6x8, true);
    hw.display.SetCursor(0, 58);
    hw.display.WriteString("Tap=panic", Font_6x8, true);
    hw.display.Update();
}

void DrawRangePage()
{
    char line[40];
    char note_name[8];
    const uint8_t coarse_note
        = static_cast<uint8_t>(CoarseNoteFromValue(param_banks.Read(ParamBank::Main, 0)) + 0.5f);
    FormatMidiNoteName(note_name, sizeof(note_name), coarse_note);

    hw.display.Fill(false);
    hw.display.SetCursor(0, 0);
    hw.display.WriteString("RANGE", Font_7x10, true);

    char range_name[16];
    FormatRangeName(range_name, sizeof(range_name));
    snprintf(line, sizeof(line), "Rng:%s", range_name);
    hw.display.SetCursor(0, 14);
    hw.display.WriteString(line, Font_6x8, true);

    snprintf(line, sizeof(line), "Base %s", note_name);
    hw.display.SetCursor(0, 24);
    hw.display.WriteString(line, Font_6x8, true);

    hw.display.SetCursor(0, 38);
    hw.display.WriteString("A1-8 C0-7", Font_6x8, true);
    hw.display.SetCursor(0, 48);
    hw.display.WriteString("B1 full", Font_6x8, true);
    hw.display.SetCursor(0, 58);
    hw.display.WriteString("Tap clr z", Font_6x8, true);
    hw.display.Update();
}

void DrawOverview()
{
    char line[40];
    char note_name[8];
    const bool note_active = midi.note_held || midi.gate_open;
    FormatMidiNoteName(note_name, sizeof(note_name), midi.note);
    const float main_harmonics = param_banks.Read(ParamBank::Main, 1);
    const float main_timbre    = param_banks.Read(ParamBank::Main, 2);
    const float main_morph     = param_banks.Read(ParamBank::Main, 3);
    const float main_fm_amt    = param_banks.Read(ParamBank::Main, 4);
    const float main_timb_amt  = param_banks.Read(ParamBank::Main, 5);
    const float main_morph_amt = param_banks.Read(ParamBank::Main, 6);
    const float main_level     = param_banks.Read(ParamBank::Main, 7);

    hw.display.Fill(false);

    hw.display.SetCursor(0, 0);
    hw.display.WriteString("MI PLAITS", Font_7x10, true);

    snprintf(line,
             sizeof(line),
             "Eng:%02d",
             params.engine_slot + 1);
    hw.display.SetCursor(0, 10);
    hw.display.WriteString(line, Font_6x8, true);

    char range_name[16];
    FormatRangeName(range_name, sizeof(range_name));
    snprintf(line,
             sizeof(line),
             "N:%s R:%s",
             note_active ? note_name : "--",
             range_name);
    hw.display.SetCursor(0, 18);
    hw.display.WriteString(line, Font_6x8, true);

    snprintf(line,
             sizeof(line),
             "H:%d T:%d M:%d",
             static_cast<int>(main_harmonics * 100.0f + 0.5f),
             static_cast<int>(main_timbre * 100.0f + 0.5f),
             static_cast<int>(main_morph * 100.0f + 0.5f));
    hw.display.SetCursor(0, 26);
    hw.display.WriteString(line, Font_6x8, true);

    snprintf(line,
             sizeof(line),
             "FM:%+d Ti:%+d Mo:%+d",
             static_cast<int>(KnobToBipolar(main_fm_amt) * 100.0f),
             static_cast<int>(KnobToBipolar(main_timb_amt) * 100.0f),
             static_cast<int>(KnobToBipolar(main_morph_amt) * 100.0f));
    hw.display.SetCursor(0, 34);
    hw.display.WriteString(line, Font_6x8, true);

    snprintf(line,
             sizeof(line),
             "L:%d C:%d D:%d",
             static_cast<int>(main_level * 100.0f + 0.5f),
             static_cast<int>(params.lpg_colour * 100.0f + 0.5f),
             static_cast<int>(params.decay * 100.0f + 0.5f));
    hw.display.SetCursor(0, 42);
    hw.display.WriteString(line, Font_6x8, true);

    snprintf(line,
             sizeof(line),
             "M:%s G:%s",
             (System::GetNow() - midi.last_event_ms) < 1200 ? "RX" : "IDLE",
             midi.gate_open ? "ON" : "OFF");
    hw.display.SetCursor(0, 50);
    hw.display.WriteString(line, Font_6x8, true);

    hw.display.SetCursor(0, 58);
    snprintf(line, sizeof(line), "Bank:%s", active_knob_bank == ParamBank::Alt ? "ALT" : "MAIN");
    hw.display.WriteString(line, Font_6x8, true);

    hw.display.Update();
}

void UpdateDisplay()
{
    if(hold_states[1].active)
        DrawRangePage();
    else if(hold_states[0].active)
        DrawHiddenKnobPage();
    else if(zoom_state.IsActive(System::GetNow(), kZoomMs))
        DrawZoom();
    else
        DrawOverview();
}

void AudioCallback(AudioHandle::InputBuffer in,
                   AudioHandle::OutputBuffer out,
                   size_t size)
{
    fonepole(level_current, Clamp01(param_banks.Read(ParamBank::Main, 7)), 0.0015f);

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
    zoom_state.Init(0.0f);
    InitParamBanks();
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

        const ParamBank bank = hold_states[0].active ? ParamBank::Alt : ParamBank::Main;
        SetActiveKnobBank(bank);

        float raw_knobs[8] = {};
        ReadKnobs(raw_knobs);

        if(bank == ParamBank::Alt)
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
