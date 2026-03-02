#include "daisy_field.h"
#include "daisysp.h"
#include "field_ux.h"
#include <cmath>
#include <cstdint>
#include <cstdio>

using namespace daisy;
using namespace daisysp;
using namespace synth;

// Hardware + UX
DaisyField hw;
FieldUX    ux;

// =============================================================
// Synth Configuration
// =============================================================
constexpr size_t kNumVoices      = 8;
constexpr float  kMaxDetuneCents = 7.0f;
constexpr float  kCutoffMin      = 20.0f;
constexpr float  kCutoffMax      = 18000.0f;
constexpr float  kChorusMix      = 0.35f;

// -------------------------------------------------------------
// Parameters (mapped from knobs/CV/switches)
// -------------------------------------------------------------
struct Params
{
    float attack;
    float decay;
    float sustain;
    float release;
    float detune;
    float chorus_rate;
    float chorus_depth;
    float master_level;
    float cutoff;
    float resonance;
    bool  filter_enable;
    bool  chorus_enable;
};

static Params params;
static float  g_knobs[8] = {0};

// -------------------------------------------------------------
// Helpers
// -------------------------------------------------------------
static inline float ClampFloat(float v, float lo, float hi)
{
    return v < lo ? lo : (v > hi ? hi : v);
}

static inline float MidiNoteToFreq(float note)
{
    return mtof(note);
}

// -------------------------------------------------------------
// OscillatorBank Voice
// -------------------------------------------------------------
struct Voice
{
    OscillatorBank osc;
    Adsr           env;
    Svf            filt;
    float          note;
    float          velocity;
    float          base_freq;
    float          detune_ratio;
    bool           active;
    bool           gate;
    uint32_t       age;

    void Init(float sr)
    {
        osc.Init(sr);
        osc.SetGain(1.0f);
        env.Init(sr);
        env.SetSustainLevel(0.8f);
        env.SetTime(ADSR_SEG_ATTACK, 0.01f);
        env.SetTime(ADSR_SEG_DECAY, 0.1f);
        env.SetTime(ADSR_SEG_RELEASE, 0.3f);
        filt.Init(sr);
        filt.SetFreq(2000.0f);
        filt.SetRes(0.2f);
        note         = -1.0f;
        velocity     = 0.0f;
        base_freq    = 0.0f;
        detune_ratio = 1.0f;
        active       = false;
        gate         = false;
        age          = 0;
    }

    void SetDetune(float ratio) { detune_ratio = ratio; }

    void UpdateRegistration(const float* amps) { osc.SetAmplitudes(amps); }

    void UpdateEnv(float a, float d, float s, float r)
    {
        env.SetTime(ADSR_SEG_ATTACK, a);
        env.SetTime(ADSR_SEG_DECAY, d);
        env.SetSustainLevel(s);
        env.SetTime(ADSR_SEG_RELEASE, r);
    }

    void UpdateFilter(float cutoff, float res)
    {
        filt.SetFreq(cutoff);
        filt.SetRes(res);
    }

    void OnNoteOn(float note_in, float vel)
    {
        note      = note_in;
        velocity  = vel;
        base_freq = MidiNoteToFreq(note);
        active    = true;
        gate      = true;
        env.Retrigger(false);
    }

    void OnNoteOff() { gate = false; }

    float Process(bool filter_enable, float cutoff, float res)
    {
        if(!active)
            return 0.0f;

        osc.SetFreq(base_freq * detune_ratio);

        float amp = env.Process(gate);
        if(!env.IsRunning())
            active = false;

        float sig = osc.Process();

        if(filter_enable)
        {
            filt.SetFreq(cutoff);
            filt.SetRes(res);
            filt.Process(sig);
            sig = filt.Low();
        }

        return sig * amp * (velocity / 127.0f);
    }

    bool IsActive() const { return active; }
};

// -------------------------------------------------------------
// Voice Manager (8 voices)
// -------------------------------------------------------------
struct VoiceManager
{
    Voice    voices[kNumVoices];
    uint32_t age_counter;

    void Init(float sr)
    {
        age_counter = 0;
        for(size_t i = 0; i < kNumVoices; i++)
            voices[i].Init(sr);
    }

    Voice* FindVoice(float note)
    {
        for(size_t i = 0; i < kNumVoices; i++)
        {
            if(voices[i].IsActive() && voices[i].note == note)
                return &voices[i];
        }

        for(size_t i = 0; i < kNumVoices; i++)
        {
            if(!voices[i].IsActive())
                return &voices[i];
        }

        Voice* oldest = &voices[0];
        for(size_t i = 1; i < kNumVoices; i++)
        {
            if(voices[i].age < oldest->age)
                oldest = &voices[i];
        }
        return oldest;
    }

    void NoteOn(float note, float velocity)
    {
        Voice* v = FindVoice(note);
        if(v == nullptr)
            return;
        v->age = age_counter++;
        v->OnNoteOn(note, velocity);
    }

    void NoteOff(float note)
    {
        for(size_t i = 0; i < kNumVoices; i++)
        {
            if(voices[i].IsActive() && voices[i].note == note)
                voices[i].OnNoteOff();
        }
    }

    void UpdateRegistration(const float* amps)
    {
        for(size_t i = 0; i < kNumVoices; i++)
            voices[i].UpdateRegistration(amps);
    }

    void UpdateDetune(float spread)
    {
        const float center = (kNumVoices - 1) * 0.5f;
        for(size_t i = 0; i < kNumVoices; i++)
        {
            float offset = static_cast<float>(i) - center;
            float cents  = offset * spread * kMaxDetuneCents;
            float ratio  = powf(2.0f, cents / 1200.0f);
            voices[i].SetDetune(ratio);
        }
    }

    void UpdateEnv(float a, float d, float s, float r)
    {
        for(size_t i = 0; i < kNumVoices; i++)
            voices[i].UpdateEnv(a, d, s, r);
    }

    void UpdateFilter(float cutoff, float res)
    {
        for(size_t i = 0; i < kNumVoices; i++)
            voices[i].UpdateFilter(cutoff, res);
    }

    float Process(bool filter_enable, float cutoff, float res)
    {
        float sum = 0.0f;
        for(size_t i = 0; i < kNumVoices; i++)
            sum += voices[i].Process(filter_enable, cutoff, res);
        return sum;
    }
};

static VoiceManager g_voice_mgr;
static Chorus       g_chorus;

// -------------------------------------------------------------
// OscillatorBank registration (7 harmonics)
// -------------------------------------------------------------
static float registration[7] = {0.25f, 0.2f, 0.2f, 0.15f, 0.1f, 0.06f, 0.04f};

// -------------------------------------------------------------
// MIDI
// -------------------------------------------------------------
static void HandleMidiMessage(MidiEvent m)
{
    switch(m.type)
    {
        case NoteOn:
        {
            NoteOnEvent p = m.AsNoteOn();
            if(p.velocity == 0)
                g_voice_mgr.NoteOff(p.note);
            else
                g_voice_mgr.NoteOn(p.note, p.velocity);
        }
        break;
        case NoteOff:
        {
            NoteOnEvent p = m.AsNoteOn();
            g_voice_mgr.NoteOff(p.note);
        }
        break;
        default: break;
    }
}

// -------------------------------------------------------------
// Controls
// -------------------------------------------------------------
static void UpdateParamsFromControls()
{
    ux.ProcessKnobs(g_knobs);

    const float cv_attack = hw.GetCvValue(hw.CV_1) * 0.5f;
    const float cv_decay  = hw.GetCvValue(hw.CV_2) * 0.5f;
    const float cv_detune = hw.GetCvValue(hw.CV_3) * 0.5f;
    const float cv_master = hw.GetCvValue(hw.CV_4) * 0.5f;

    const float attack_norm = ClampFloat(g_knobs[0] + cv_attack, 0.0f, 1.0f);
    const float decay_norm  = ClampFloat(g_knobs[1] + cv_decay, 0.0f, 1.0f);
    const float detune_norm = ClampFloat(g_knobs[4] + cv_detune, 0.0f, 1.0f);
    const float master_norm = ClampFloat(g_knobs[7] + cv_master, 0.0f, 1.0f);

    params.attack       = 0.001f + attack_norm * 2.0f;
    params.decay        = 0.001f + decay_norm * 2.0f;
    params.sustain      = g_knobs[2];
    params.release      = 0.001f + g_knobs[3] * 3.0f;
    params.detune       = detune_norm;
    params.chorus_rate  = 0.1f + g_knobs[5] * 5.0f;
    params.chorus_depth = ClampFloat(g_knobs[6], 0.0f, 1.0f);
    params.master_level = master_norm;

    params.cutoff    = 2000.0f;
    params.resonance = 0.3f;

    // Switches
    if(hw.GetSwitch(hw.SW_1)->RisingEdge())
        params.filter_enable = !params.filter_enable;
    if(hw.GetSwitch(hw.SW_2)->RisingEdge())
        params.chorus_enable = !params.chorus_enable;

    if(hw.KeyboardRisingEdge(0))
        params.detune = ClampFloat(params.detune + 0.02f, 0.0f, 1.0f);
    if(hw.KeyboardRisingEdge(1))
        params.detune = ClampFloat(params.detune - 0.02f, 0.0f, 1.0f);
    if(hw.KeyboardRisingEdge(2))
        params.chorus_rate = ClampFloat(params.chorus_rate + 0.1f, 0.1f, 6.0f);
    if(hw.KeyboardRisingEdge(3))
        params.chorus_rate = ClampFloat(params.chorus_rate - 0.1f, 0.1f, 6.0f);
    if(hw.KeyboardRisingEdge(4))
        params.chorus_depth
            = ClampFloat(params.chorus_depth + 0.05f, 0.0f, 1.0f);
    if(hw.KeyboardRisingEdge(5))
        params.chorus_depth
            = ClampFloat(params.chorus_depth - 0.05f, 0.0f, 1.0f);
    if(hw.KeyboardRisingEdge(8))
        params.master_level
            = ClampFloat(params.master_level + 0.05f, 0.0f, 1.0f);
    if(hw.KeyboardRisingEdge(9))
        params.master_level
            = ClampFloat(params.master_level - 0.05f, 0.0f, 1.0f);

    g_voice_mgr.UpdateEnv(
        params.attack, params.decay, params.sustain, params.release);
    g_voice_mgr.UpdateDetune(params.detune);
    g_voice_mgr.UpdateFilter(params.cutoff, params.resonance);
    g_chorus.SetLfoFreq(params.chorus_rate);
    g_chorus.SetLfoDepth(params.chorus_depth);
}

// -------------------------------------------------------------
// Audio Callback
// -------------------------------------------------------------
void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    hw.ProcessAllControls();
    UpdateParamsFromControls();

    for(size_t i = 0; i < size; i++)
    {
        float sig = g_voice_mgr.Process(
            params.filter_enable, params.cutoff, params.resonance);
        sig *= params.master_level * 0.2f;

        if(params.chorus_enable)
        {
            g_chorus.Process(sig);
            const float wet_l = g_chorus.GetLeft();
            const float wet_r = g_chorus.GetRight();
            out[0][i] = (sig * (1.0f - kChorusMix)) + (wet_l * kChorusMix);
            out[1][i] = (sig * (1.0f - kChorusMix)) + (wet_r * kChorusMix);
        }
        else
        {
            out[0][i] = sig;
            out[1][i] = sig;
        }
    }
}

// -------------------------------------------------------------
// Display + LEDs
// -------------------------------------------------------------
static void UpdateDisplay()
{
    hw.display.Fill(false);
    hw.display.SetCursor(0, 0);
    hw.display.WriteString("String Machine Poly", Font_7x10, true);

    char line[32];
    hw.display.SetCursor(0, 12);
    sprintf(line, "A:%.2f D:%.2f", params.attack, params.decay);
    hw.display.WriteString(line, Font_6x8, true);

    hw.display.SetCursor(0, 22);
    sprintf(line, "S:%.2f R:%.2f", params.sustain, params.release);
    hw.display.WriteString(line, Font_6x8, true);

    hw.display.SetCursor(0, 32);
    sprintf(line, "Det:%.2f Vol:%.2f", params.detune, params.master_level);
    hw.display.WriteString(line, Font_6x8, true);

    hw.display.SetCursor(0, 42);
    sprintf(line,
            "Ch:%s Rt:%.1f D:%.2f",
            params.chorus_enable ? "On" : "Off",
            params.chorus_rate,
            params.chorus_depth);
    hw.display.WriteString(line, Font_6x8, true);

    hw.display.Update();
}

static void UpdateLeds(float* knob_values)
{
    float note_leds[4] = {
        params.filter_enable ? 1.0f : 0.0f,
        params.chorus_enable ? 1.0f : 0.0f,
        params.detune,
        params.master_level,
    };
    ux.UpdateLeds(
        0, note_leds, params.filter_enable, params.chorus_enable, knob_values);
}

// -------------------------------------------------------------
// Main
// -------------------------------------------------------------
int main(void)
{
    hw.Init();
    hw.SetAudioBlockSize(32);
    hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

    ux.Init(&hw);

    params.attack        = 0.01f;
    params.decay         = 0.1f;
    params.sustain       = 0.8f;
    params.release       = 0.3f;
    params.detune        = 0.2f;
    params.chorus_rate   = 0.8f;
    params.chorus_depth  = 0.4f;
    params.master_level  = 0.6f;
    params.cutoff        = 2000.0f;
    params.resonance     = 0.3f;
    params.filter_enable = true;
    params.chorus_enable = true;

    const float sr = hw.AudioSampleRate();
    g_voice_mgr.Init(sr);
    g_voice_mgr.UpdateRegistration(registration);
    g_voice_mgr.UpdateDetune(params.detune);
    g_voice_mgr.UpdateEnv(
        params.attack, params.decay, params.sustain, params.release);
    g_voice_mgr.UpdateFilter(params.cutoff, params.resonance);

    g_chorus.Init(sr);
    g_chorus.SetLfoFreq(params.chorus_rate);
    g_chorus.SetLfoDepth(params.chorus_depth);

    hw.midi.StartReceive();
    hw.StartAdc();
    hw.StartAudio(AudioCallback);

    float    knob_values[8] = {0};
    uint32_t last_ui_ms     = 0;

    while(1)
    {
        hw.midi.Listen();
        while(hw.midi.HasEvents())
            HandleMidiMessage(hw.midi.PopEvent());

        uint32_t now = System::GetNow();
        if(now - last_ui_ms > 40)
        {
            ux.ProcessKnobs(g_knobs);
            UpdateLeds(g_knobs);
            UpdateDisplay();
            last_ui_ms = now;
        }
        System::Delay(1);
    }
}
