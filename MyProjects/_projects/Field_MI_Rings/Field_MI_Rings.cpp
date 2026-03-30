#include "daisy_field.h"
#include "daisysp.h"
#include "../../foundation_examples/field_defaults.h"
#include "../../foundation_examples/field_instrument_ui.h"
#include "../../foundation_examples/field_parameter_banks.h"
#include <cmath>
#include <cstdio>

using namespace daisy;
using namespace daisysp;
using namespace FieldDefaults;
using namespace FieldInstrumentUI;
using namespace FieldParameterBanks;

namespace
{
constexpr int      kNumVoices                  = 2;
constexpr uint32_t kZoomMs                     = 1400;
constexpr float    kZoomDelta                  = 0.015f;
constexpr float    kCatchDelta                 = 0.02f;
constexpr float    kKnobLedCapturedBrightness  = 1.0f;
constexpr float    kKnobLedUncapturedBrightness = 0.35f;
constexpr float    kPanHalfPi                  = 1.57079632679f;

enum class ModelFamily : int
{
    Modal = 0,
    String,
    Sympathetic,
};

enum class UiPage : int
{
    Main = 0,
    Midi,
    Help,
};

enum class SpreadMode : int
{
    Narrow = 0,
    Wide,
    Split,
};

struct Params
{
    float structure   = 0.45f;
    float brightness  = 0.68f;
    float damping     = 0.52f;
    float position    = 0.35f;
    float spread      = 0.58f;
    float exciter     = 0.72f;
    float macro       = 0.42f;
    float interaction = 0.28f;

    bool        sustain_pedal = false;
    ModelFamily model         = ModelFamily::String;
    UiPage      page          = UiPage::Main;
    SpreadMode  spread_mode   = SpreadMode::Wide;
};

struct MidiState
{
    uint8_t  last_note      = 60;
    float    last_velocity  = 0.0f;
    uint32_t last_event_ms  = 0;
    bool     midi_rx_recent = false;
};

float Clamp01(float value)
{
    return fclamp(value, 0.0f, 1.0f);
}

float SpreadModeScale(SpreadMode mode)
{
    switch(mode)
    {
        case SpreadMode::Narrow: return 0.22f;
        case SpreadMode::Wide: return 0.60f;
        case SpreadMode::Split: return 1.0f;
        default: return 0.60f;
    }
}

const char* ModelName(ModelFamily model)
{
    switch(model)
    {
        case ModelFamily::Modal: return "MODAL";
        case ModelFamily::String: return "STRING";
        case ModelFamily::Sympathetic: return "SYMP";
        default: return "RINGS";
    }
}

const char* SpreadModeName(SpreadMode mode)
{
    switch(mode)
    {
        case SpreadMode::Narrow: return "NARROW";
        case SpreadMode::Wide: return "WIDE";
        case SpreadMode::Split: return "SPLIT";
        default: return "WIDE";
    }
}

float PanForVoice(int voice_index, const Params& params)
{
    const float width = Clamp01(params.spread) * SpreadModeScale(params.spread_mode);
    return voice_index == 0 ? 0.5f - width * 0.5f : 0.5f + width * 0.5f;
}

void PanSample(float in, float pan, float& left, float& right)
{
    const float angle = Clamp01(pan) * kPanHalfPi;
    left              = in * cosf(angle);
    right             = in * sinf(angle);
}

struct RingsVoice
{
    ModalVoice modal;
    StringVoice string_voice;
    Resonator resonator;
    WhiteNoise noise;
    AdEnv      exciter_env;

    float sample_rate   = kDefaultSampleRate;
    float base_position = 0.25f;
    float freq          = 261.63f;
    float velocity      = 0.8f;
    float level_estimate = 0.0f;
    float last_output   = 0.0f;

    uint8_t  midi_note  = 60;
    uint32_t note_age   = 0;

    bool active          = false;
    bool note_held       = false;
    bool pedal_latched   = false;
    bool trigger_pending = false;

    void Init(float sr, float resonator_position)
    {
        sample_rate   = sr;
        base_position = resonator_position;

        modal.Init(sr);
        string_voice.Init(sr);
        resonator.Init(base_position, 24, sr);
        noise.Init();
        noise.SetAmp(1.0f);

        exciter_env.Init(sr);
        exciter_env.SetTime(ADENV_SEG_ATTACK, 0.001f);
        exciter_env.SetTime(ADENV_SEG_DECAY, 0.12f);
        exciter_env.SetMin(0.0f);
        exciter_env.SetMax(1.0f);
    }

    bool Matches(uint8_t note) const { return active && midi_note == note; }
    bool IsHeld() const { return note_held || pedal_latched; }

    void NoteOn(uint8_t note, float vel, uint32_t age)
    {
        midi_note       = note;
        freq            = mtof(note);
        velocity        = Clamp01(vel);
        note_age        = age;
        active          = true;
        note_held       = true;
        pedal_latched   = false;
        trigger_pending = true;
        level_estimate  = 1.0f;
        last_output     = 0.0f;

        modal.SetFreq(freq);
        string_voice.Reset();
        string_voice.SetFreq(freq);
        resonator.Init(base_position, 24, sample_rate);
        resonator.SetFreq(freq);
    }

    void NoteOff(bool sustain_pedal)
    {
        note_held     = false;
        pedal_latched = sustain_pedal;

        if(!sustain_pedal)
        {
            modal.SetSustain(false);
            string_voice.SetSustain(false);
        }
    }

    void ReleasePedal()
    {
        pedal_latched = false;
        modal.SetSustain(false);
        string_voice.SetSustain(false);
    }

    void Panic()
    {
        active          = false;
        note_held       = false;
        pedal_latched   = false;
        trigger_pending = false;
        level_estimate  = 0.0f;
        last_output     = 0.0f;
        modal.SetSustain(false);
        string_voice.SetSustain(false);
        string_voice.Reset();
        resonator.Init(base_position, 24, sample_rate);
    }

    void Prepare(const Params& params, ModelFamily model, int voice_index)
    {
        const float accent = Clamp01(0.15f + params.exciter * 0.45f + velocity * 0.55f);
        const float bright = Clamp01(params.brightness * (0.75f + velocity * 0.35f));
        const float damp   = Clamp01(0.10f + params.damping * 0.85f);
        const float skew   = 1.0f + (params.macro - 0.5f) * 0.03f * (voice_index == 0 ? -1.0f : 1.0f);

        switch(model)
        {
            case ModelFamily::Modal:
                modal.SetFreq(freq * skew);
                modal.SetAccent(accent);
                modal.SetStructure(Clamp01(params.structure * 0.70f + params.macro * 0.30f));
                modal.SetBrightness(Clamp01(bright + params.position * 0.10f));
                modal.SetDamping(damp);
                modal.SetSustain(pedal_latched);
                if(trigger_pending)
                    modal.Trig();
                break;

            case ModelFamily::String:
                string_voice.SetFreq(freq * skew);
                string_voice.SetAccent(accent);
                string_voice.SetStructure(Clamp01(params.structure * 0.55f + params.macro * 0.45f));
                string_voice.SetBrightness(bright);
                string_voice.SetDamping(damp);
                string_voice.SetSustain(pedal_latched);
                if(trigger_pending)
                    string_voice.Trig();
                break;

            case ModelFamily::Sympathetic:
                resonator.SetFreq(freq * skew);
                resonator.SetStructure(Clamp01(params.structure * 0.40f + params.macro * 0.60f));
                resonator.SetBrightness(Clamp01(bright + params.position * 0.15f));
                resonator.SetDamping(Clamp01(damp * (0.85f + params.macro * 0.20f)));
                exciter_env.SetTime(ADENV_SEG_ATTACK, 0.001f);
                exciter_env.SetTime(ADENV_SEG_DECAY, 0.018f + (1.0f - params.position) * 0.14f);
                if(trigger_pending)
                    exciter_env.Trigger();
                break;
        }

        trigger_pending = false;
    }

    float ProcessSample(const Params& params, ModelFamily model, float coupling)
    {
        if(!active)
            return 0.0f;

        float sample = 0.0f;
        switch(model)
        {
            case ModelFamily::Modal:
            {
                const float body = modal.Process();
                const float aux  = modal.GetAux();
                const float mix  = 0.08f + params.position * 0.30f;
                sample           = body * (1.0f - mix) + aux * mix;
                break;
            }

            case ModelFamily::String:
            {
                const float body = string_voice.Process();
                const float aux  = string_voice.GetAux();
                const float mix  = 0.10f + params.position * 0.35f;
                sample           = body * (1.0f - mix) + aux * mix;
                break;
            }

            case ModelFamily::Sympathetic:
            {
                float exciter = exciter_env.Process() * (0.45f + velocity * 0.55f);
                exciter *= 0.35f + params.exciter * 0.65f;
                exciter *= 0.35f + fabsf(noise.Process()) * 0.65f;
                if(pedal_latched)
                    exciter += noise.Process() * (0.008f + params.exciter * 0.025f);
                sample = resonator.Process(exciter + coupling * 0.20f);
                sample = sample + exciter * params.position * 0.10f;
                break;
            }
        }

        sample += coupling * (0.03f + params.interaction * 0.12f);
        sample = fclamp(sample, -1.0f, 1.0f);

        level_estimate = level_estimate * 0.995f + fabsf(sample) * 0.005f;
        if(!IsHeld() && level_estimate < 0.0002f)
            active = false;

        last_output = sample;
        return sample;
    }
};

DaisyField           hw;
FieldOLEDDisplay     display;
FieldTriStateKeyLEDs key_leds;
ParamZoomState       zoom_state;
ParamBankSet         param_banks;
ParamBank            active_bank = ParamBank::Main;

Params    params;
MidiState midi;

RingsVoice voices[kNumVoices];
uint32_t   note_age_counter = 0;
float      knob_values[8]   = {
    params.structure,
    params.brightness,
    params.damping,
    params.position,
    params.spread,
    params.exciter,
    params.macro,
    params.interaction,
};

const char* kParamNames[8] = {
    "Struct",
    "Bright",
    "Damp",
    "Pos",
    "Spread",
    "Excite",
    "Macro",
    "Inter",
};

void SyncParamsFromBank()
{
    for(int i = 0; i < 8; ++i)
        knob_values[i] = Clamp01(param_banks.Read(ParamBank::Main, i));

    params.structure   = knob_values[0];
    params.brightness  = knob_values[1];
    params.damping     = knob_values[2];
    params.position    = knob_values[3];
    params.spread      = knob_values[4];
    params.exciter     = knob_values[5];
    params.macro       = knob_values[6];
    params.interaction = knob_values[7];
}

void InitParamBank()
{
    param_banks.Init(0.5f);
    param_banks.SetActiveBank(active_bank);

    for(int i = 0; i < 8; ++i)
    {
        param_banks.Write(ParamBank::Main, i, knob_values[i]);
        param_banks.SetCaptured(ParamBank::Main, i, true);
    }

    zoom_state.SetBaseline(knob_values);
    SyncParamsFromBank();
}

void MarkExternalParamChange(int knob_idx, float value)
{
    param_banks.Write(ParamBank::Main, knob_idx, Clamp01(value));
    param_banks.SetCaptured(ParamBank::Main, knob_idx, false);
    SyncParamsFromBank();
}

void UpdateKnobLeds()
{
    for(int i = 0; i < 8; ++i)
    {
        const float brightness = param_banks.IsCaptured(ParamBank::Main, i)
                                     ? kKnobLedCapturedBrightness
                                     : kKnobLedUncapturedBrightness;
        hw.led_driver.SetLed(kLedKnobs[i], Clamp01(knob_values[i]) * brightness);
    }
}

int FindVoiceForNote(uint8_t note)
{
    for(int i = 0; i < kNumVoices; ++i)
        if(voices[i].Matches(note))
            return i;

    return -1;
}

int AllocateVoice()
{
    for(int i = 0; i < kNumVoices; ++i)
        if(!voices[i].active)
            return i;

    for(int i = 0; i < kNumVoices; ++i)
        if(!voices[i].IsHeld())
            return i;

    return voices[0].note_age <= voices[1].note_age ? 0 : 1;
}

void PanicAllVoices()
{
    params.sustain_pedal = false;
    for(int i = 0; i < kNumVoices; ++i)
        voices[i].Panic();
}

void StartMidiNote(uint8_t note, float velocity)
{
    const int voice_index = FindVoiceForNote(note) >= 0 ? FindVoiceForNote(note) : AllocateVoice();
    voices[voice_index].NoteOn(note, velocity, ++note_age_counter);

    midi.last_note      = note;
    midi.last_velocity  = velocity;
    midi.last_event_ms  = System::GetNow();
    midi.midi_rx_recent = true;
}

void ReleaseMidiNote(uint8_t note)
{
    for(int i = 0; i < kNumVoices; ++i)
    {
        if(voices[i].Matches(note))
            voices[i].NoteOff(params.sustain_pedal);
    }

    midi.last_event_ms  = System::GetNow();
    midi.midi_rx_recent = true;
}

void HandleMidiMessage(MidiEvent msg)
{
    switch(msg.type)
    {
        case NoteOn:
        {
            const NoteOnEvent note = msg.AsNoteOn();
            if(note.velocity == 0)
                ReleaseMidiNote(note.note);
            else
                StartMidiNote(note.note, note.velocity / 127.0f);
            break;
        }

        case NoteOff:
        {
            const NoteOffEvent note = msg.AsNoteOff();
            ReleaseMidiNote(note.note);
            break;
        }

        case ControlChange:
        {
            const ControlChangeEvent cc    = msg.AsControlChange();
            const float              value = cc.value / 127.0f;

            if(cc.control_number == 64)
            {
                params.sustain_pedal = cc.value >= 64;
                if(!params.sustain_pedal)
                {
                    for(int i = 0; i < kNumVoices; ++i)
                        voices[i].ReleasePedal();
                }
            }
            else if(cc.control_number == 16)
            {
                MarkExternalParamChange(0, value);
            }
            else if(cc.control_number == 74)
            {
                MarkExternalParamChange(1, value);
            }
            else if(cc.control_number == 71)
            {
                MarkExternalParamChange(2, value);
            }
            else if(cc.control_number == 17)
            {
                MarkExternalParamChange(3, value);
            }

            midi.last_event_ms  = System::GetNow();
            midi.midi_rx_recent = true;
            break;
        }

        default: break;
    }
}

void ProcessKnobs()
{
    float raw[8] = {};
    for(int i = 0; i < 8; ++i)
        raw[i] = hw.knob[i].Process();

    for(int i = 0; i < 8; ++i)
    {
        if(!param_banks.IsCaptured(ParamBank::Main, i))
            param_banks.CatchIfClose(ParamBank::Main, i, raw[i], kCatchDelta);

        if(param_banks.IsCaptured(ParamBank::Main, i))
            param_banks.Write(ParamBank::Main, i, raw[i]);
    }

    SyncParamsFromBank();
    zoom_state.Capture(knob_values, System::GetNow(), kZoomDelta);
}

void ApplyModelAndPageKeyLeds()
{
    key_leds.Clear();

    key_leds.SetA(0, params.model == ModelFamily::Modal ? KeyLedState::On : KeyLedState::Off);
    key_leds.SetA(1, params.model == ModelFamily::String ? KeyLedState::On : KeyLedState::Off);
    key_leds.SetA(2, params.model == ModelFamily::Sympathetic ? KeyLedState::On : KeyLedState::Off);

    KeyLedState spread_state = KeyLedState::Off;
    if(params.spread_mode == SpreadMode::Wide)
        spread_state = KeyLedState::Blink;
    else if(params.spread_mode == SpreadMode::Split)
        spread_state = KeyLedState::On;
    key_leds.SetA(3, spread_state);

    key_leds.SetB(0, params.page == UiPage::Main ? KeyLedState::On : KeyLedState::Off);
    key_leds.SetB(1, params.page == UiPage::Midi ? KeyLedState::On : KeyLedState::Off);
    key_leds.SetB(2, params.page == UiPage::Help ? KeyLedState::On : KeyLedState::Off);
    key_leds.SetB(6,
                  (System::GetNow() - midi.last_event_ms) < 1200 ? KeyLedState::Blink
                                                                 : KeyLedState::Off);
    key_leds.SetB(7, params.sustain_pedal ? KeyLedState::On : KeyLedState::Off);
}

void HandleKeyboardShortcuts()
{
    if(hw.KeyboardRisingEdge(kKeyAIndices[0]))
        params.model = ModelFamily::Modal;
    if(hw.KeyboardRisingEdge(kKeyAIndices[1]))
        params.model = ModelFamily::String;
    if(hw.KeyboardRisingEdge(kKeyAIndices[2]))
        params.model = ModelFamily::Sympathetic;

    if(hw.KeyboardRisingEdge(kKeyAIndices[3]))
    {
        const int next = (static_cast<int>(params.spread_mode) + 1) % 3;
        params.spread_mode = static_cast<SpreadMode>(next);
    }

    if(hw.KeyboardRisingEdge(kKeyBIndices[0]))
        params.page = UiPage::Main;
    if(hw.KeyboardRisingEdge(kKeyBIndices[1]))
        params.page = UiPage::Midi;
    if(hw.KeyboardRisingEdge(kKeyBIndices[2]))
        params.page = UiPage::Help;
    if(hw.KeyboardRisingEdge(kKeyBIndices[7]))
        PanicAllVoices();

    if(hw.sw[0].RisingEdge())
        params.page = static_cast<UiPage>((static_cast<int>(params.page) + 1) % 3);

    if(hw.sw[1].RisingEdge())
        PanicAllVoices();
}

int ActiveVoiceCount()
{
    int count = 0;
    for(int i = 0; i < kNumVoices; ++i)
        count += voices[i].active ? 1 : 0;
    return count;
}

void UpdateMainPage()
{
    char title[20];
    snprintf(title, sizeof(title), "RINGS %s", ModelName(params.model));
    display.SetTitle(title);
    for(int i = 0; i < 8; ++i)
    {
        display.SetLabel(i, kParamNames[i]);
        display.SetValue(i, knob_values[i], 0.0f);
    }

    if(zoom_state.IsActive(System::GetNow(), kZoomMs))
    {
        display.SetActiveParam(zoom_state.ActiveIndex());
        display.Update();
    }
    else
    {
        display.SetActiveParam(-1);
        display.UpdateCompact();
    }
}

void UpdateMidiPage()
{
    char note_name[8];
    char velocity_text[16];
    FormatMidiNoteName(note_name, sizeof(note_name), midi.last_note);
    FormatPercent(velocity_text, sizeof(velocity_text), midi.last_velocity);

    hw.display.Fill(false);
    hw.display.SetCursor(0, 0);
    hw.display.WriteString("RINGS MIDI", Font_7x10, true);

    hw.display.SetCursor(0, 14);
    hw.display.WriteString("MODEL:", Font_6x8, true);
    hw.display.SetCursor(42, 14);
    hw.display.WriteString(ModelName(params.model), Font_6x8, true);

    hw.display.SetCursor(0, 24);
    hw.display.WriteString("NOTE:", Font_6x8, true);
    hw.display.SetCursor(42, 24);
    hw.display.WriteString(note_name, Font_6x8, true);

    hw.display.SetCursor(68, 24);
    hw.display.WriteString(velocity_text, Font_6x8, true);

    hw.display.SetCursor(0, 34);
    hw.display.WriteString("CC16 ST 74 BR", Font_6x8, true);
    hw.display.SetCursor(0, 44);
    hw.display.WriteString("CC71 DP 17 PO", Font_6x8, true);
    hw.display.SetCursor(0, 54);
    hw.display.WriteString(params.sustain_pedal ? "PEDAL ON" : "PEDAL OFF", Font_6x8, true);
    hw.display.SetCursor(68, 54);
    hw.display.WriteString((System::GetNow() - midi.last_event_ms) < 1200 ? "RX" : "IDLE",
                           Font_6x8,
                           true);
    char voices_text[10];
    snprintf(voices_text, sizeof(voices_text), "V:%d", ActiveVoiceCount());
    hw.display.SetCursor(96, 14);
    hw.display.WriteString(voices_text, Font_6x8, true);
    hw.display.Update();
}

void UpdateHelpPage()
{
    hw.display.Fill(false);
    hw.display.SetCursor(0, 0);
    hw.display.WriteString("RINGS KEYS", Font_7x10, true);
    hw.display.SetCursor(0, 14);
    hw.display.WriteString("A1 MOD A2 STR", Font_6x8, true);
    hw.display.SetCursor(0, 24);
    hw.display.WriteString("A3 SYM A4 SPR", Font_6x8, true);
    hw.display.SetCursor(0, 34);
    hw.display.WriteString("B1 MAIN B2 MIDI", Font_6x8, true);
    hw.display.SetCursor(0, 44);
    hw.display.WriteString("B3 HELP B8 PANIC", Font_6x8, true);
    hw.display.SetCursor(0, 54);
    hw.display.WriteString(SpreadModeName(params.spread_mode), Font_6x8, true);
    hw.display.Update();
}

void UpdateDisplay()
{
    switch(params.page)
    {
        case UiPage::Main: UpdateMainPage(); break;
        case UiPage::Midi: UpdateMidiPage(); break;
        case UiPage::Help: UpdateHelpPage(); break;
        default: UpdateMainPage(); break;
    }
}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    (void)in;

    for(int i = 0; i < kNumVoices; ++i)
        voices[i].Prepare(params, params.model, i);

    for(size_t i = 0; i < size; ++i)
    {
        float sample_l = 0.0f;
        float sample_r = 0.0f;

        const float coupling = params.interaction * 0.22f;
        const float cross_0  = voices[1].last_output * coupling;
        const float cross_1  = voices[0].last_output * coupling;

        for(int v = 0; v < kNumVoices; ++v)
        {
            const float voice_sample = voices[v].ProcessSample(params,
                                                               params.model,
                                                               v == 0 ? cross_0
                                                                      : cross_1);
            float left  = 0.0f;
            float right = 0.0f;
            PanSample(voice_sample, PanForVoice(v, params), left, right);
            sample_l += left;
            sample_r += right;
        }

        out[0][i] = fclamp(sample_l * 0.32f, -1.0f, 1.0f);
        out[1][i] = fclamp(sample_r * 0.32f, -1.0f, 1.0f);
    }
}

} // namespace

int main(void)
{
    hw.Init();
    hw.SetAudioBlockSize(kRecommendedBlockSize);
    hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

    display.Init(&hw);
    key_leds.Init(&hw);
    zoom_state.Init();
    InitParamBank();

    for(int i = 0; i < 8; ++i)
        display.SetLabel(i, kParamNames[i]);

    const float sample_rate = hw.AudioSampleRate();
    for(int i = 0; i < kNumVoices; ++i)
    {
        const float resonator_position = i == 0 ? 0.20f : 0.80f;
        voices[i].Init(sample_rate, resonator_position);
    }

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
        UpdateKnobLeds();
        ApplyModelAndPageKeyLeds();
        UpdateDisplay();

        key_leds.Update(System::GetNow(), 1.0f, 0.8f, 220);
        System::Delay(1);
    }
}
