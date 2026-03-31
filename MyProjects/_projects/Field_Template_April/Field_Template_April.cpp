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
constexpr float    kCatchThreshold               = 0.02f;
constexpr float    kCapturedLedBrightness        = 1.00f;
constexpr float    kUncapturedLedBrightness      = 0.30f;
constexpr uint32_t kZoomDurationMs               = 1400;
constexpr int      kWaveformCount                = 4;
constexpr int      kTransposeCount               = 4;
constexpr int      kTemplateKnobCount            = 8;

enum MainParamIndex
{
    MAIN_CUTOFF = 0,
    MAIN_RESONANCE,
    MAIN_ATTACK,
    MAIN_DECAY,
    MAIN_SUSTAIN,
    MAIN_RELEASE,
    MAIN_DRIVE,
    MAIN_COLOR,
};

enum AltParamIndex
{
    ALT_ENV_AMOUNT = 0,
    ALT_LFO_RATE,
    ALT_LFO_DEPTH,
    ALT_GLIDE,
    ALT_VELOCITY,
    ALT_KEYTRACK,
    ALT_NOISE,
    ALT_SUB,
};

enum VelocityMode
{
    VELOCITY_FIXED = 0,
    VELOCITY_SCALED,
    VELOCITY_PUNCH,
};

enum KeyTrackMode
{
    KEYTRACK_OFF = 0,
    KEYTRACK_HALF,
    KEYTRACK_FULL,
};

enum LfoTargetMode
{
    LFO_TARGET_OFF = 0,
    LFO_TARGET_PITCH,
    LFO_TARGET_FILTER,
};

enum GlideMode
{
    GLIDE_OFF = 0,
    GLIDE_LEGATO,
    GLIDE_ALWAYS,
};

constexpr float kMainDefaults[8] = {
    0.58f, 0.12f, 0.02f, 0.22f, 0.78f, 0.24f, 0.12f, 0.50f,
};

constexpr float kAltDefaults[8] = {
    0.42f, 0.18f, 0.14f, 0.10f, 0.70f, 0.50f, 0.00f, 0.18f,
};

constexpr float kOutputLevelDefault = 0.80f;

const char* kMainLabels[8] = {
    "Cutoff", "Reso", "Attack", "Decay",
    "Sustain", "Release", "Drive", "Color",
};

const char* kAltLabels[8] = {
    "EnvAmt", "LfoRt", "LfoDp", "Glide",
    "VelAmt", "KeyTrk", "Noise", "Sub",
};

const int kTransposeSemitones[kTransposeCount] = {-12, 0, 12, 24};

const char* kWaveformNames[kWaveformCount] = {"SINE", "TRI", "SAW", "SQR"};
const char* kVelocityNames[3]              = {"FIX", "SCL", "PCH"};
const char* kKeyTrackNames[3]              = {"OFF", "HALF", "FULL"};
const char* kLfoTargetNames[3]             = {"OFF", "PITCH", "FILT"};
const char* kGlideNames[3]                 = {"OFF", "LEG", "ON"};
const char* kTransposeNames[kTransposeCount] = {"-12", "0", "+12", "+24"};
const uint8_t kOscWaveforms[kWaveformCount] = {
    Oscillator::WAVE_SIN,
    Oscillator::WAVE_TRI,
    Oscillator::WAVE_SAW,
    Oscillator::WAVE_SQUARE,
};

struct MidiState
{
    uint8_t note          = 60;
    uint8_t velocity      = 100;
    bool    gate          = false;
    bool    held          = false;
    bool    sustain       = false;
    bool    note_active   = false;
};

struct FocusState
{
    char     label[16] = "";
    char     value[24] = "";
    uint32_t until_ms  = 0;
};

struct SynthState
{
    ParamBankSet banks;

    ParamBank active_bank = ParamBank::Main;

    float output_level     = kOutputLevelDefault;
    bool  output_captured  = false;
    bool  sw2_was_pressed  = false;

    int waveform_index     = 2;
    int transpose_index    = 1;
    int velocity_mode      = VELOCITY_SCALED;
    int keytrack_mode      = KEYTRACK_HALF;
    int lfo_target_mode    = LFO_TARGET_FILTER;
    int glide_mode         = GLIDE_LEGATO;

    float target_freq_hz   = 261.63f;
    float current_freq_hz  = 261.63f;
    bool  legato_playing   = false;
};

DaisyField           hw;
FieldTriStateKeyLEDs key_leds;
SynthState           state;
MidiState            midi_state;
FocusState           focus;

Oscillator osc;
Oscillator sub;
Oscillator lfo;
Adsr       env;
Svf        filter;
WhiteNoise noise;

float sample_rate_hz = 48000.0f;

float Clamp01(float value)
{
    return fclamp(value, 0.0f, 1.0f);
}

void SetFocus(const char* label, const char* value_text)
{
    snprintf(focus.label, sizeof(focus.label), "%s", label);
    snprintf(focus.value, sizeof(focus.value), "%s", value_text);
    focus.until_ms = System::GetNow() + kZoomDurationMs;
}

void FormatPercentText(char* buffer, size_t size, float value)
{
    snprintf(buffer, size, "%d%%", static_cast<int>(Clamp01(value) * 100.0f + 0.5f));
}

void FormatMainValue(int idx, float value, char* buffer, size_t size)
{
    switch(idx)
    {
        case MAIN_CUTOFF:
        {
            const float hz = 40.0f + Clamp01(value) * 14000.0f;
            FormatHertz(buffer, size, hz);
            break;
        }
        case MAIN_ATTACK:
        case MAIN_DECAY:
        case MAIN_RELEASE:
        {
            const float seconds = (idx == MAIN_ATTACK)
                                      ? (0.001f + Clamp01(value) * 1.5f)
                                      : (0.001f + Clamp01(value) * 2.5f);
            FormatMilliseconds(buffer, size, seconds);
            break;
        }
        default: FormatPercentText(buffer, size, value); break;
    }
}

void FormatAltValue(int idx, float value, char* buffer, size_t size)
{
    switch(idx)
    {
        case ALT_LFO_RATE:
        {
            const float hz = 0.05f + Clamp01(value) * 12.0f;
            FormatHertz(buffer, size, hz);
            break;
        }
        case ALT_GLIDE:
        {
            const float seconds = Clamp01(value) * 0.8f;
            FormatMilliseconds(buffer, size, seconds);
            break;
        }
        default: FormatPercentText(buffer, size, value); break;
    }
}

void FormatParamValue(ParamBank bank, int idx, float value, char* buffer, size_t size)
{
    if(bank == ParamBank::Main)
        FormatMainValue(idx, value, buffer, size);
    else
        FormatAltValue(idx, value, buffer, size);
}

const char* LabelFor(ParamBank bank, int idx)
{
    return bank == ParamBank::Main ? kMainLabels[idx] : kAltLabels[idx];
}

void ResetBank(ParamBank bank)
{
    const float* defaults = bank == ParamBank::Main ? kMainDefaults : kAltDefaults;
    for(int i = 0; i < kTemplateKnobCount; ++i)
    {
        state.banks.Write(bank, i, defaults[i]);
        state.banks.SetCaptured(bank, i, false);
    }
}

void ResetAllState()
{
    ResetBank(ParamBank::Main);
    ResetBank(ParamBank::Alt);
    state.output_level    = kOutputLevelDefault;
    state.output_captured = false;
    state.waveform_index  = 2;
    state.transpose_index = 1;
    state.velocity_mode   = VELOCITY_SCALED;
    state.keytrack_mode   = KEYTRACK_HALF;
    state.lfo_target_mode = LFO_TARGET_FILTER;
    state.glide_mode      = GLIDE_LEGATO;
    midi_state.sustain    = false;
}

void InitDefaults()
{
    state.banks.Init(0.5f);
    ResetAllState();
}

void SetActiveBank(ParamBank bank)
{
    state.active_bank = bank;
    state.banks.SetActiveBank(bank);
}

float MainValue(int idx)
{
    return state.banks.Read(ParamBank::Main, idx);
}

float AltValue(int idx)
{
    return state.banks.Read(ParamBank::Alt, idx);
}

float VelocityScale()
{
    const float midi_velocity = fclamp(static_cast<float>(midi_state.velocity) / 127.0f, 0.0f, 1.0f);
    switch(state.velocity_mode)
    {
        case VELOCITY_FIXED: return 1.0f;
        case VELOCITY_PUNCH: return 0.65f + midi_velocity * 0.7f;
        case VELOCITY_SCALED:
        default: return 0.35f + midi_velocity * 0.65f;
    }
}

float KeyTracking()
{
    switch(state.keytrack_mode)
    {
        case KEYTRACK_HALF: return 0.5f;
        case KEYTRACK_FULL: return 1.0f;
        case KEYTRACK_OFF:
        default: return 0.0f;
    }
}

float GlideSeconds()
{
    return Clamp01(AltValue(ALT_GLIDE)) * 0.8f;
}

float ComputeGlideAlpha()
{
    const float glide_seconds = GlideSeconds();
    if(glide_seconds <= 0.0005f)
        return 1.0f;

    return 1.0f - expf(-1.0f / (glide_seconds * sample_rate_hz));
}

void UpdateTargetFrequency()
{
    const int note = static_cast<int>(midi_state.note) + kTransposeSemitones[state.transpose_index];
    state.target_freq_hz = mtof(static_cast<float>(fclamp(note, 0, 127)));

    if(!state.legato_playing || state.glide_mode == GLIDE_OFF)
        state.current_freq_hz = state.target_freq_hz;
}

void NoteOn(uint8_t note, uint8_t velocity)
{
    const bool was_playing = midi_state.note_active;
    midi_state.note        = note;
    midi_state.velocity    = velocity;
    midi_state.gate        = true;
    midi_state.held        = true;
    midi_state.note_active = true;

    const bool keep_glide = state.glide_mode == GLIDE_ALWAYS
                            || (state.glide_mode == GLIDE_LEGATO && was_playing);

    state.legato_playing = keep_glide;
    UpdateTargetFrequency();
    if(!keep_glide)
        state.current_freq_hz = state.target_freq_hz;
}

void NoteOff(uint8_t note)
{
    if(note != midi_state.note)
        return;

    midi_state.held = false;
    if(!midi_state.sustain)
        midi_state.gate = false;
}

void Panic()
{
    midi_state.gate        = false;
    midi_state.held        = false;
    midi_state.sustain     = false;
    midi_state.note_active = false;
    state.legato_playing   = false;
}

void HandleMidiMessage(MidiEvent msg)
{
    switch(msg.type)
    {
        case daisy::NoteOn:
        {
            const NoteOnEvent note_on = msg.AsNoteOn();
            if(note_on.velocity == 0)
                NoteOff(note_on.note);
            else
                NoteOn(note_on.note, note_on.velocity);
            break;
        }
        case daisy::NoteOff:
        {
            const NoteOffEvent note_off = msg.AsNoteOff();
            NoteOff(note_off.note);
            break;
        }
        case daisy::ControlChange:
        {
            const ControlChangeEvent cc = msg.AsControlChange();
            if(cc.control_number == 64)
            {
                midi_state.sustain = cc.value >= 64;
                if(!midi_state.sustain && !midi_state.held)
                    midi_state.gate = false;
            }
            break;
        }
        default: break;
    }
}

void ApplyVoiceSetup()
{
    osc.SetWaveform(kOscWaveforms[state.waveform_index]);
    sub.SetWaveform(kOscWaveforms[state.waveform_index] == Oscillator::WAVE_SQUARE
                        ? Oscillator::WAVE_SQUARE
                        : Oscillator::WAVE_SAW);

    const float color = MainValue(MAIN_COLOR);
    osc.SetPw(0.08f + color * 0.84f);
    sub.SetPw(0.50f);
    osc.SetAmp(0.70f);
    sub.SetAmp(0.45f * Clamp01(AltValue(ALT_SUB)));

    env.SetAttackTime(0.001f + MainValue(MAIN_ATTACK) * 1.5f);
    env.SetDecayTime(0.001f + MainValue(MAIN_DECAY) * 2.0f);
    env.SetSustainLevel(Clamp01(MainValue(MAIN_SUSTAIN)));
    env.SetReleaseTime(0.001f + MainValue(MAIN_RELEASE) * 2.5f);

    lfo.SetFreq(0.05f + AltValue(ALT_LFO_RATE) * 12.0f);
}

void UpdateFocusForParam(ParamBank bank, int idx, float value)
{
    char value_text[24];
    FormatParamValue(bank, idx, value, value_text, sizeof(value_text));
    SetFocus(LabelFor(bank, idx), value_text);
}

void ProcessOutputLevel(const float raw_knob)
{
    if(!state.output_captured)
    {
        if(fabsf(raw_knob - state.output_level) <= kCatchThreshold)
            state.output_captured = true;
        else
            return;
    }

    if(fabsf(raw_knob - state.output_level) > 0.0005f)
    {
        state.output_level = Clamp01(raw_knob);
        char value_text[24];
        FormatPercentText(value_text, sizeof(value_text), state.output_level);
        SetFocus("Level", value_text);
    }
}

void ProcessBankKnobs(ParamBank bank, const float raw_knobs[8], bool override_level)
{
    for(int i = 0; i < kTemplateKnobCount; ++i)
    {
        if(override_level && i == 7)
            continue;

        if(!state.banks.IsCaptured(bank, i))
            state.banks.CatchIfClose(bank, i, raw_knobs[i], kCatchThreshold);

        if(!state.banks.IsCaptured(bank, i))
            continue;

        const float current = state.banks.Read(bank, i);
        const float updated = Clamp01(raw_knobs[i]);
        if(fabsf(updated - current) > 0.0005f)
        {
            state.banks.Write(bank, i, updated);
            UpdateFocusForParam(bank, i, updated);
        }
    }
}

void UpdateKnobLeds(bool sw2_pressed)
{
    for(int i = 0; i < kTemplateKnobCount; ++i)
    {
        float value      = state.banks.Read(state.active_bank, i);
        bool  captured   = state.banks.IsCaptured(state.active_bank, i);

        if(sw2_pressed && i == 7)
        {
            value    = state.output_level;
            captured = state.output_captured;
        }

        const float brightness = captured ? kCapturedLedBrightness
                                          : kUncapturedLedBrightness;
        hw.led_driver.SetLed(kLedKnobs[i], Clamp01(value) * brightness);
    }

    hw.led_driver.SetLed(kLedSwitches[0], state.active_bank == ParamBank::Alt ? 1.0f : 0.12f);
    hw.led_driver.SetLed(kLedSwitches[1], sw2_pressed ? 1.0f : 0.12f);
}

void UpdateKeyLeds()
{
    key_leds.Clear();

    for(int i = 0; i < 4; ++i)
        key_leds.SetA(i, i == state.waveform_index ? KeyLedState::On : KeyLedState::Off);

    key_leds.SetA(4, static_cast<KeyLedState>(state.velocity_mode));
    key_leds.SetA(5, static_cast<KeyLedState>(state.keytrack_mode));
    key_leds.SetA(6, static_cast<KeyLedState>(state.lfo_target_mode));
    key_leds.SetA(7, static_cast<KeyLedState>(state.glide_mode));

    for(int i = 0; i < 4; ++i)
        key_leds.SetB(i, i == state.transpose_index ? KeyLedState::On : KeyLedState::Off);

    key_leds.SetB(4, midi_state.sustain ? KeyLedState::Blink
                                        : (midi_state.gate ? KeyLedState::On : KeyLedState::Off));
    key_leds.SetB(5, KeyLedState::Blink);
    key_leds.SetB(6, KeyLedState::Blink);
    key_leds.SetB(7, KeyLedState::On);
}

void HandleKeybedControls()
{
    for(int i = 0; i < 4; ++i)
    {
        if(hw.KeyboardRisingEdge(kKeyAIndices[i]))
        {
            state.waveform_index = i;
            SetFocus("Wave", kWaveformNames[state.waveform_index]);
        }

        if(hw.KeyboardRisingEdge(kKeyBIndices[i]))
        {
            state.transpose_index = i;
            SetFocus("Trans", kTransposeNames[state.transpose_index]);
            UpdateTargetFrequency();
        }
    }

    if(hw.KeyboardRisingEdge(kKeyAIndices[4]))
    {
        state.velocity_mode = (state.velocity_mode + 1) % 3;
        SetFocus("VelMode", kVelocityNames[state.velocity_mode]);
    }

    if(hw.KeyboardRisingEdge(kKeyAIndices[5]))
    {
        state.keytrack_mode = (state.keytrack_mode + 1) % 3;
        SetFocus("KeyTrk", kKeyTrackNames[state.keytrack_mode]);
    }

    if(hw.KeyboardRisingEdge(kKeyAIndices[6]))
    {
        state.lfo_target_mode = (state.lfo_target_mode + 1) % 3;
        SetFocus("LfoTgt", kLfoTargetNames[state.lfo_target_mode]);
    }

    if(hw.KeyboardRisingEdge(kKeyAIndices[7]))
    {
        state.glide_mode = (state.glide_mode + 1) % 3;
        SetFocus("Glide", kGlideNames[state.glide_mode]);
    }

    if(hw.KeyboardRisingEdge(kKeyBIndices[4]))
    {
        Panic();
        SetFocus("Panic", "All notes off");
    }

    if(hw.KeyboardRisingEdge(kKeyBIndices[5]))
    {
        ResetBank(ParamBank::Main);
        SetFocus("Reset", "Main bank");
    }

    if(hw.KeyboardRisingEdge(kKeyBIndices[6]))
    {
        ResetBank(ParamBank::Alt);
        SetFocus("Reset", "Alt bank");
    }

    if(hw.KeyboardRisingEdge(kKeyBIndices[7]))
    {
        ResetAllState();
        SetFocus("Reset", "All defaults");
    }
}

void HandleSwitchActions(bool sw2_pressed)
{
    if(hw.sw[1].RisingEdge() && !sw2_pressed)
    {
        Panic();
        SetFocus("Panic", "SW2");
    }
}

void RenderOverview()
{
    hw.display.Fill(false);

    char line[32];
    snprintf(line,
             sizeof(line),
             "APRIL %s",
             state.active_bank == ParamBank::Main ? "MAIN" : "ALT");
    hw.display.SetCursor(0, 0);
    hw.display.WriteString(line, Font_7x10, true);

    snprintf(line,
             sizeof(line),
             "%s  TR:%s",
             kWaveformNames[state.waveform_index],
             kTransposeNames[state.transpose_index]);
    hw.display.SetCursor(0, 12);
    hw.display.WriteString(line, Font_6x8, true);

    snprintf(line,
             sizeof(line),
             "A5:%s A6:%s",
             kVelocityNames[state.velocity_mode],
             kKeyTrackNames[state.keytrack_mode]);
    hw.display.SetCursor(0, 22);
    hw.display.WriteString(line, Font_6x8, true);

    snprintf(line,
             sizeof(line),
             "A7:%s A8:%s",
             kLfoTargetNames[state.lfo_target_mode],
             kGlideNames[state.glide_mode]);
    hw.display.SetCursor(0, 30);
    hw.display.WriteString(line, Font_6x8, true);

    for(int i = 0; i < 8; ++i)
    {
        char value_text[20];
        const float value = state.banks.Read(state.active_bank, i);
        FormatParamValue(state.active_bank, i, value, value_text, sizeof(value_text));

        const int x = (i < 4) ? 0 : 64;
        const int y = 40 + ((i % 4) * 6);
        snprintf(line, sizeof(line), "%s:%s", LabelFor(state.active_bank, i), value_text);

        hw.display.SetCursor(x, y);
        hw.display.WriteString(line, Font_6x8, true);
    }

    hw.display.Update();
}

void RenderZoom()
{
    hw.display.Fill(false);

    char header[24];
    snprintf(header,
             sizeof(header),
             "%s",
             state.active_bank == ParamBank::Main ? "EDIT MAIN" : "EDIT ALT");
    hw.display.SetCursor(0, 0);
    hw.display.WriteString(header, Font_7x10, true);

    hw.display.SetCursor(0, 16);
    hw.display.WriteString(focus.label, Font_7x10, true);

    hw.display.SetCursor(0, 30);
    hw.display.WriteString(focus.value, Font_11x18, true);

    hw.display.SetCursor(0, 54);
    hw.display.WriteString("SW1=Alt  SW2+K8=Level", Font_6x8, true);
    hw.display.Update();
}

void RenderDisplay()
{
    const bool zoom_active = focus.until_ms > System::GetNow();
    if(zoom_active)
        RenderZoom();
    else
        RenderOverview();
}

void ProcessUi()
{
    float raw_knobs[8];
    for(int i = 0; i < 8; ++i)
        raw_knobs[i] = Clamp01(hw.knob[i].Process());

    const bool sw1_pressed = hw.sw[0].Pressed();
    const bool sw2_pressed = hw.sw[1].Pressed();

    SetActiveBank(sw1_pressed ? ParamBank::Alt : ParamBank::Main);

    if(sw2_pressed && !state.sw2_was_pressed)
        state.output_captured = false;
    state.sw2_was_pressed = sw2_pressed;

    ProcessBankKnobs(state.active_bank, raw_knobs, sw2_pressed);

    if(sw2_pressed)
        ProcessOutputLevel(raw_knobs[7]);

    HandleKeybedControls();
    HandleSwitchActions(sw2_pressed);
    ApplyVoiceSetup();

    UpdateKnobLeds(sw2_pressed);
    UpdateKeyLeds();
    key_leds.Update(System::GetNow(), 1.0f, 1.0f, 250);
    RenderDisplay();
}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    const float glide_alpha = ComputeGlideAlpha();
    const float env_amount  = AltValue(ALT_ENV_AMOUNT);
    const float lfo_depth   = AltValue(ALT_LFO_DEPTH);
    const float noise_mix   = AltValue(ALT_NOISE) * 0.30f;
    const float sub_mix     = AltValue(ALT_SUB);
    const float drive_gain  = 1.0f + MainValue(MAIN_DRIVE) * 7.0f;
    const float color       = MainValue(MAIN_COLOR);

    for(size_t i = 0; i < size; ++i)
    {
        state.current_freq_hz += (state.target_freq_hz - state.current_freq_hz) * glide_alpha;

        const float lfo_out = lfo.Process();
        float       osc_freq = state.current_freq_hz;
        if(state.lfo_target_mode == LFO_TARGET_PITCH)
            osc_freq *= 1.0f + (lfo_out * lfo_depth * 0.03f);

        osc.SetFreq(osc_freq);
        sub.SetFreq(osc_freq * 0.5f);

        const float envelope = env.Process(midi_state.gate);
        const float velocity = VelocityScale();

        const float keytrack_note = fclamp(static_cast<float>(midi_state.note) - 36.0f, 0.0f, 72.0f) / 72.0f;
        float cutoff = 50.0f + MainValue(MAIN_CUTOFF) * 12000.0f;
        cutoff += envelope * env_amount * 7000.0f;
        cutoff += keytrack_note * KeyTracking() * 2500.0f;
        if(state.lfo_target_mode == LFO_TARGET_FILTER)
            cutoff += lfo_out * lfo_depth * 3500.0f;
        cutoff = fclamp(cutoff, 40.0f, 18000.0f);

        filter.SetFreq(cutoff);
        filter.SetRes(0.15f + MainValue(MAIN_RESONANCE) * 0.80f);

        float source = osc.Process();
        source += sub.Process() * sub_mix;
        source += noise.Process() * noise_mix;
        source *= 0.6f + color * 0.4f;

        filter.Process(source * envelope * velocity);
        const float shaped = tanhf(filter.Low() * drive_gain) * state.output_level;

        out[0][i] = shaped + in[0][i] * 0.0f;
        out[1][i] = shaped + in[1][i] * 0.0f;
    }
}

} // namespace

int main(void)
{
    hw.Init();
    hw.SetAudioBlockSize(kRecommendedBlockSize);
    hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
    sample_rate_hz = hw.AudioSampleRate();

    key_leds.Init(&hw);
    InitDefaults();

    osc.Init(sample_rate_hz);
    sub.Init(sample_rate_hz);
    lfo.Init(sample_rate_hz);
    env.Init(sample_rate_hz);
    filter.Init(sample_rate_hz);
    noise.Init();

    lfo.SetWaveform(Oscillator::WAVE_SIN);
    lfo.SetAmp(1.0f);
    ApplyVoiceSetup();

    hw.midi.StartReceive();
    hw.StartAdc();
    hw.StartAudio(AudioCallback);

    while(1)
    {
        hw.midi.Listen();
        while(hw.midi.HasEvents())
            HandleMidiMessage(hw.midi.PopEvent());

        hw.ProcessAllControls();
        ProcessUi();
        System::Delay(1);
    }
}
