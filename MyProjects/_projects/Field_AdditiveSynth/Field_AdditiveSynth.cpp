// Field_AdditiveSynth.cpp
// 8-voice polyphonic additive synthesizer for Daisy Field
// HarmonicOscillator<16> + Adsr per voice
// Global: LFO (tutti vibrato/tremolo), Chorus, ReverbSc
// SW1/SW2 dual knob pages with pickup/catch
// A1-A8: spectral presets | B1-B8: LFO waveform + performance toggles
// MIDI: TRS hardware, last-note stealing

#include "daisy_field.h"
#include "daisysp.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

using namespace daisy;
using namespace daisysp;

// ─── Constants ───────────────────────────────────────────────────────────────
static constexpr int   NUM_VOICES   = 8;
static constexpr int   NUM_PARTIALS = 16;
static constexpr float NORM_TARGET  = 0.85f;   // amplitude sum target

// Knob LED indices on DaisyField
static const int knob_leds[8] = {
    DaisyField::LED_KNOB_1, DaisyField::LED_KNOB_2,
    DaisyField::LED_KNOB_3, DaisyField::LED_KNOB_4,
    DaisyField::LED_KNOB_5, DaisyField::LED_KNOB_6,
    DaisyField::LED_KNOB_7, DaisyField::LED_KNOB_8
};
static const int key_a_leds[8] = {
    DaisyField::LED_KEY_A1, DaisyField::LED_KEY_A2,
    DaisyField::LED_KEY_A3, DaisyField::LED_KEY_A4,
    DaisyField::LED_KEY_A5, DaisyField::LED_KEY_A6,
    DaisyField::LED_KEY_A7, DaisyField::LED_KEY_A8
};
static const int key_b_leds[8] = {
    DaisyField::LED_KEY_B1, DaisyField::LED_KEY_B2,
    DaisyField::LED_KEY_B3, DaisyField::LED_KEY_B4,
    DaisyField::LED_KEY_B5, DaisyField::LED_KEY_B6,
    DaisyField::LED_KEY_B7, DaisyField::LED_KEY_B8
};

// ─── Spectral Presets ─────────────────────────────────────────────────────────
struct SpectralPreset {
    float rolloff;
    float even;
    float odd;
    const char* name;
};

static const SpectralPreset PRESETS[8] = {
    { 5.0f, 0.0f, 0.0f, "Sine"  },
    { 2.0f, 0.0f, 0.7f, "Tri"   },
    { 1.0f, 0.8f, 0.8f, "Saw"   },
    { 1.0f, 0.0f, 0.8f, "Sqr"   },
    { 1.5f, 0.9f, 0.1f, "Hollow"},
    { 0.5f, 0.2f, 0.9f, "Bell"  },
    { 0.0f, 0.8f, 0.6f, "Organ" },
    { 0.1f, 0.8f, 0.8f, "Buzz"  },
};

// ─── Global state ────────────────────────────────────────────────────────────
DaisyField hw;

float g_sample_rate = 48000.f;

// ── Voice ──
struct AdditiveVoice {
    HarmonicOscillator<NUM_PARTIALS> osc;
    Adsr  env;
    uint8_t note     = 0;
    float   velocity = 0.f;
    bool    gate     = false;
    bool    active   = false;

    void Init(float sr) {
        osc.Init(sr);
        env.Init(sr);
    }

    float Process() {
        float e = env.Process(gate);
        if(e < 0.0001f && !gate) active = false;
        return osc.Process() * e * velocity;
    }

    void NoteOn(uint8_t n, float vel, const float* amps) {
        note     = n;
        velocity = vel / 127.f;
        osc.SetFreq(mtof(n));
        osc.SetAmplitudes(amps);
        gate = active = true;
    }

    void NoteOff() { gate = false; }

    void SetAmplitudes(const float* amps) {
        osc.SetAmplitudes(amps);
    }

    void SetEnv(float a, float d, float s, float r) {
        env.SetAttackTime(a);
        env.SetDecayTime(d);
        env.SetSustainLevel(s);
        env.SetReleaseTime(r);
    }
};

struct AdditiveVoiceManager {
    AdditiveVoice voices[NUM_VOICES];

    void Init(float sr) {
        for(auto& v : voices) v.Init(sr);
    }

    float Process() {
        float sum = 0.f;
        for(auto& v : voices)
            if(v.active) sum += v.Process();
        return sum * (1.f / NUM_VOICES);
    }

    AdditiveVoice* FindFree() {
        for(auto& v : voices) if(!v.active) return &v;
        for(auto& v : voices) if(!v.gate)   return &v;
        return &voices[0];  // steal oldest
    }

    void NoteOn(uint8_t note, uint8_t vel, const float* amps) {
        FindFree()->NoteOn(note, (float)vel, amps);
    }

    void NoteOff(uint8_t note, bool sustained) {
        if(sustained) return;
        for(auto& v : voices)
            if(v.active && v.note == note) v.NoteOff();
    }

    void SetAmplitudes(const float* amps) {
        for(auto& v : voices)
            if(v.active) v.SetAmplitudes(amps);
    }

    void SetEnvAll(float a, float d, float s, float r) {
        for(auto& v : voices) v.SetEnv(a, d, s, r);
    }

    // Collect up to 2 active note pitches for OLED
    int GetActiveNotes(uint8_t* buf, int maxn) {
        int cnt = 0;
        for(auto& v : voices)
            if(v.active && v.gate && cnt < maxn)
                buf[cnt++] = v.note;
        return cnt;
    }
} voice_mgr;

// ── Global effects ──
Oscillator lfo;
Chorus     chorus;
ReverbSc   reverb;

// ── Partial amplitude buffer (shared, normalized) ──
float partial_amps[NUM_PARTIALS];

// ── Tone params (updated from knobs or presets) ──
float rolloff    = 1.0f;
float even_level = 0.8f;
float odd_level  = 0.8f;

// ── ADSR params ──
float atk_s  = 0.01f;
float dec_s  = 0.1f;
float sus_lv = 0.7f;
float rel_s  = 0.3f;

// ── Reverb params ──
float reverb_mix  = 0.3f;
bool  reverb_bypass = false;

// ── LFO params ──
float lfo_rate   = 2.0f;
float lfo_depth  = 0.0f;
float lfo_target = 0.0f;  // 0=pitch, 1=amp
int   lfo_wave   = 0;     // 0=sin,1=tri,2=saw,3=S&H
bool  lfo_sync   = false;
volatile bool lfo_reset_req = false;

// ── Chorus params ──
float cho_rate    = 0.5f;
float cho_depth   = 0.5f;
float cho_delay   = 0.5f;
float cho_mix     = 0.3f;
bool  chorus_bypass = false;

// ── Master ──
float master_vol = 0.8f;

// ── Performance ──
bool sustain   = false;
int  preset_idx = -1;   // -1 = custom

// ─── Knob Pickup (Catch) System ──────────────────────────────────────────────
struct PageState {
    float stored[8];    // applied parameter values
    float prev_phys[8]; // previous physical reading
    bool  locked[8];    // true = waiting to catch
};

static PageState pages[2];
static int active_page = 0;

void InitPages() {
    // Sensible defaults — physical knob positions won't match immediately
    // Page A defaults
    pages[0].stored[0] = rolloff / 2.0f;     // K1 Rolloff: 0→2 → raw 0.5
    pages[0].stored[1] = even_level;          // K2 Even
    pages[0].stored[2] = odd_level;           // K3 Odd
    pages[0].stored[3] = 0.01f;              // K4 Atk
    pages[0].stored[4] = 0.1f;               // K5 Dec
    pages[0].stored[5] = sus_lv;             // K6 Sus
    pages[0].stored[6] = 0.3f;               // K7 Rel
    pages[0].stored[7] = reverb_mix;         // K8 Rev
    // Page B defaults
    pages[1].stored[0] = 0.1f;              // K1 LFO rate (log mapped)
    pages[1].stored[1] = lfo_depth;         // K2 LFO depth
    pages[1].stored[2] = lfo_target;        // K3 LFO target
    pages[1].stored[3] = 0.1f;              // K4 Chorus rate
    pages[1].stored[4] = cho_depth;         // K5 Chorus depth
    pages[1].stored[5] = cho_delay;         // K6 Chorus delay
    pages[1].stored[6] = cho_mix;           // K7 Chorus mix
    pages[1].stored[7] = master_vol;        // K8 Master vol

    for(int p = 0; p < 2; p++) {
        for(int k = 0; k < 8; k++) {
            pages[p].prev_phys[k] = hw.GetKnobValue(k);
            pages[p].locked[k]    = true;   // lock all until first catch
        }
    }
}

void SwitchToPage(int p) {
    for(int k = 0; k < 8; k++) pages[p].locked[k] = true;
    active_page = p;
}

// ─── Harmonic Amplitude Calculation ──────────────────────────────────────────
void RecalcPartials() {
    float sum = 0.f;
    for(int i = 0; i < NUM_PARTIALS; i++) {
        float n   = (float)(i + 1);
        float ro  = (rolloff > 0.001f) ? powf(1.f / n, rolloff) : 1.0f;
        float par;
        if(i == 0)
            par = 1.0f;                     // fundamental always full
        else if((i + 1) % 2 == 0)
            par = even_level;               // 2nd, 4th, 6th ... (even harmonics)
        else
            par = odd_level;                // 3rd, 5th, 7th ... (odd harmonics)
        partial_amps[i] = ro * par;
        sum += partial_amps[i];
    }
    // Normalize so sum ≤ NORM_TARGET (HarmonicOscillator requires sum < 1)
    if(sum > 0.001f) {
        float scale = NORM_TARGET / sum;
        for(int i = 0; i < NUM_PARTIALS; i++) partial_amps[i] *= scale;
    }
    voice_mgr.SetAmplitudes(partial_amps);
}

// ─── Param application ───────────────────────────────────────────────────────
// Log time mapping: 1ms..maxMs exponential
static float LogTimeS(float raw, float maxMs) {
    return 0.001f * powf(maxMs, raw);  // raw=0→1ms, raw=1→maxMs
}

// Log Hz mapping: minHz..maxHz exponential
static float LogHz(float raw, float minHz, float maxHz) {
    return minHz * powf(maxHz / minHz, raw);
}

void ApplyPageA(int k, float v) {
    switch(k) {
        case 0:
            rolloff = v * 2.0f;
            RecalcPartials();
            break;
        case 1:
            even_level = v;
            RecalcPartials();
            break;
        case 2:
            odd_level = v;
            RecalcPartials();
            break;
        case 3: atk_s = LogTimeS(v, 2000.f); voice_mgr.SetEnvAll(atk_s, dec_s, sus_lv, rel_s); break;
        case 4: dec_s = LogTimeS(v, 2000.f); voice_mgr.SetEnvAll(atk_s, dec_s, sus_lv, rel_s); break;
        case 5: sus_lv = v;                   voice_mgr.SetEnvAll(atk_s, dec_s, sus_lv, rel_s); break;
        case 6: rel_s = LogTimeS(v, 4000.f); voice_mgr.SetEnvAll(atk_s, dec_s, sus_lv, rel_s); break;
        case 7: reverb_mix = v; break;
    }
}

void ApplyPageB(int k, float v) {
    switch(k) {
        case 0:
            lfo_rate = LogHz(v, 0.1f, 20.f);
            lfo.SetFreq(lfo_rate);
            break;
        case 1: lfo_depth  = v;                 break;
        case 2: lfo_target = v;                 break;
        case 3:
            cho_rate = LogHz(v, 0.1f, 5.f);
            chorus.SetLfoFreq(cho_rate);
            break;
        case 4: cho_depth = v; chorus.SetLfoDepth(cho_depth); break;
        case 5: cho_delay = v; chorus.SetDelay(cho_delay);    break;
        case 6: cho_mix   = v; break;
        case 7: master_vol = v; break;
    }
}

void ApplyParam(int page, int k, float v) {
    if(page == 0) ApplyPageA(k, v);
    else          ApplyPageB(k, v);
}

// ─── OLED Display ────────────────────────────────────────────────────────────
static int    zoom_param    = -1;
static uint32_t zoom_start  = 0;
static float  prev_knob[8]  = {};

static const char* NOTE_NAMES[12] = {
    "C","C#","D","D#","E","F","F#","G","G#","A","A#","B"
};

void DrawNormalView() {
    hw.display.Fill(false);

    // Row 0: active notes + page
    uint8_t notes[2];
    int cnt = voice_mgr.GetActiveNotes(notes, 2);
    char top[32] = "";
    for(int i = 0; i < cnt; i++) {
        char nb[8];
        int oct = (int)(notes[i] / 12) - 1;
        snprintf(nb, sizeof(nb), "%s%d ", NOTE_NAMES[notes[i] % 12], oct);
        strncat(top, nb, sizeof(top) - strlen(top) - 1);
    }
    char pg[4];
    snprintf(pg, sizeof(pg), "[%c]", active_page == 0 ? 'A' : 'B');
    strncat(top, pg, sizeof(top) - strlen(top) - 1);
    hw.display.SetCursor(0, 0);
    hw.display.WriteString(top, Font_7x10, true);

    // Row 1 (y=16): 16 partial bars (8px wide each)
    float maxAmp = 0.001f;
    for(int i = 0; i < NUM_PARTIALS; i++)
        if(partial_amps[i] > maxAmp) maxAmp = partial_amps[i];

    for(int i = 0; i < NUM_PARTIALS; i++) {
        int barH = (int)((partial_amps[i] / maxAmp) * 30.f);
        int x = i * 8;
        hw.display.DrawRect(x, 46 - barH, x + 6, 46, true, true);
    }

    // Row 2 (y=54): preset + bypass flags
    char bot[32] = "";
    if(preset_idx >= 0) {
        snprintf(bot, sizeof(bot), "[%s]", PRESETS[preset_idx].name);
    }
    if(reverb_bypass)  strncat(bot, "[R]", sizeof(bot) - strlen(bot) - 1);
    if(chorus_bypass)  strncat(bot, "[C]", sizeof(bot) - strlen(bot) - 1);
    hw.display.SetCursor(0, 54);
    hw.display.WriteString(bot, Font_6x8, true);

    hw.display.Update();
}

void DrawZoomView() {
    hw.display.Fill(false);

    static const char* pageA_names[8] = {
        "Rolloff","Even","Odd","Attack","Decay","Sustain","Release","Rev Mix"
    };
    static const char* pageB_names[8] = {
        "LFO Rate","LFO Dep","LFO Tgt","Ch.Rate","Ch.Dep","Ch.Dly","Ch.Mix","Vol"
    };

    const char** names = (active_page == 0) ? pageA_names : pageB_names;
    bool locked = pages[active_page].locked[zoom_param];
    float v     = pages[active_page].stored[zoom_param];

    // Row 0: name (with lock indicator)
    char title[32];
    snprintf(title, sizeof(title), "%s%s", locked ? "(L) " : "", names[zoom_param]);
    hw.display.SetCursor(0, 0);
    hw.display.WriteString(title, Font_7x10, true);

    // Row 1: value + unit (Font_11x18)
    char val[32];
    if(active_page == 0) {
        switch(zoom_param) {
            case 0: snprintf(val, sizeof(val), "%.2f", v * 2.f); break;
            case 1: snprintf(val, sizeof(val), "%d%%", (int)(v * 100)); break;
            case 2: snprintf(val, sizeof(val), "%d%%", (int)(v * 100)); break;
            case 3: snprintf(val, sizeof(val), "%.0f ms", LogTimeS(v, 2000.f) * 1000.f); break;
            case 4: snprintf(val, sizeof(val), "%.0f ms", LogTimeS(v, 2000.f) * 1000.f); break;
            case 5: snprintf(val, sizeof(val), "%d%%", (int)(v * 100)); break;
            case 6: snprintf(val, sizeof(val), "%.0f ms", LogTimeS(v, 4000.f) * 1000.f); break;
            case 7: snprintf(val, sizeof(val), "%d%%", (int)(v * 100)); break;
            default: snprintf(val, sizeof(val), "%.2f", v); break;
        }
    } else {
        switch(zoom_param) {
            case 0: snprintf(val, sizeof(val), "%.2f Hz", LogHz(v, 0.1f, 20.f)); break;
            case 1: {
                float semis = v * 2.0f;
                snprintf(val, sizeof(val), "%d%% +/-%.1fst", (int)(v*100), semis);
                break;
            }
            case 2: snprintf(val, sizeof(val), "%s", v < 0.5f ? "Pitch" : "Amp"); break;
            case 3: snprintf(val, sizeof(val), "%.2f Hz", LogHz(v, 0.1f, 5.f)); break;
            case 4: snprintf(val, sizeof(val), "%d%%", (int)(v * 100)); break;
            case 5: snprintf(val, sizeof(val), "%d%%", (int)(v * 100)); break;
            case 6: snprintf(val, sizeof(val), "%d%%", (int)(v * 100)); break;
            case 7: snprintf(val, sizeof(val), "%d%%", (int)(v * 100)); break;
            default: snprintf(val, sizeof(val), "%.2f", v); break;
        }
    }
    hw.display.SetCursor(0, 16);
    hw.display.WriteString(val, Font_11x18, true);

    // Row 2: progress bar
    int barW = (int)(v * 127.f);
    hw.display.DrawRect(0, 52, barW, 60, true, true);

    hw.display.Update();
}

void UpdateDisplay() {
    if(zoom_param >= 0 && (System::GetNow() - zoom_start) < 1500)
        DrawZoomView();
    else {
        zoom_param = -1;
        DrawNormalView();
    }
}

// ─── LED Management ───────────────────────────────────────────────────────────
void UpdateLeds() {
    // Knob LEDs: dim if locked on active page
    for(int k = 0; k < 8; k++) {
        float br = pages[active_page].locked[k] ? 0.05f : 1.0f;
        hw.led_driver.SetLed(knob_leds[k], br);
    }

    // SW LEDs: bright = active page
    hw.led_driver.SetLed(DaisyField::LED_SW_1, active_page == 0 ? 1.0f : 0.1f);
    hw.led_driver.SetLed(DaisyField::LED_SW_2, active_page == 1 ? 1.0f : 0.1f);

    // A row: radio for preset
    for(int i = 0; i < 8; i++)
        hw.led_driver.SetLed(key_a_leds[i], (preset_idx == i) ? 1.0f : 0.0f);

    // B row
    for(int i = 0; i < 4; i++)   // B1-B4 LFO wave radio
        hw.led_driver.SetLed(key_b_leds[i], (lfo_wave == i) ? 1.0f : 0.0f);
    hw.led_driver.SetLed(key_b_leds[4], lfo_sync       ? 1.0f : 0.0f);
    hw.led_driver.SetLed(key_b_leds[5], reverb_bypass  ? 1.0f : 0.0f);
    hw.led_driver.SetLed(key_b_leds[6], chorus_bypass  ? 1.0f : 0.0f);
    hw.led_driver.SetLed(key_b_leds[7], sustain        ? 1.0f : 0.0f);

    hw.led_driver.SwapBuffersAndTransmit();
}

// ─── Controls (main loop) ─────────────────────────────────────────────────────
void ProcessKnobs() {
    for(int k = 0; k < 8; k++) {
        float phys = hw.GetKnobValue(k);
        PageState& pg = pages[active_page];

        if(pg.locked[k]) {
            // Check if physical knob crossed stored value
            bool crossed =
                (pg.prev_phys[k] <= pg.stored[k] && phys >= pg.stored[k]) ||
                (pg.prev_phys[k] >= pg.stored[k] && phys <= pg.stored[k]) ||
                (fabsf(phys - pg.stored[k]) < 0.02f);
            if(crossed) pg.locked[k] = false;
        }

        pg.prev_phys[k] = phys;

        if(!pg.locked[k]) {
            // Detect movement for OLED zoom (threshold 0.01)
            if(fabsf(phys - prev_knob[k]) > 0.01f) {
                zoom_param = k;
                zoom_start = System::GetNow();
                prev_knob[k] = phys;
            }
            if(pg.stored[k] != phys) {
                pg.stored[k] = phys;
                ApplyParam(active_page, k, phys);
            }
        }
    }
}

void ProcessSwitches() {
    if(hw.GetSwitch(DaisyField::SW_1)->RisingEdge()) {
        SwitchToPage(0);
    }
    if(hw.GetSwitch(DaisyField::SW_2)->RisingEdge()) {
        SwitchToPage(1);
    }
}

void LoadPreset(int idx) {
    preset_idx = idx;
    rolloff    = PRESETS[idx].rolloff;
    even_level = PRESETS[idx].even;
    odd_level  = PRESETS[idx].odd;
    RecalcPartials();
    // Lock K1-K3 on Page A and sync stored values
    pages[0].stored[0] = rolloff / 2.f;
    pages[0].stored[1] = even_level;
    pages[0].stored[2] = odd_level;
    pages[0].locked[0] = true;
    pages[0].locked[1] = true;
    pages[0].locked[2] = true;
}

void ProcessKeys() {
    // A row: spectral presets (keys 0-7)
    for(int i = 0; i < 8; i++) {
        if(hw.KeyboardRisingEdge(i)) {
            LoadPreset(i);
        }
    }

    // B row: keys 8-15
    for(int i = 0; i < 4; i++) {
        if(hw.KeyboardRisingEdge(8 + i)) {
            lfo_wave = i;
            static const int waves[4] = {
                Oscillator::WAVE_SIN,
                Oscillator::WAVE_TRI,
                Oscillator::WAVE_SAW,
                Oscillator::WAVE_SQUARE
            };
            lfo.SetWaveform(waves[i]);
        }
    }
    // B5: LFO sync toggle
    if(hw.KeyboardRisingEdge(12)) lfo_sync = !lfo_sync;
    // B6: Reverb bypass toggle
    if(hw.KeyboardRisingEdge(13)) reverb_bypass = !reverb_bypass;
    // B7: Chorus bypass toggle
    if(hw.KeyboardRisingEdge(14)) chorus_bypass = !chorus_bypass;
    // B8: Sustain toggle
    if(hw.KeyboardRisingEdge(15)) sustain = !sustain;

    // Detect K1-K3 physical movement while preset locked → release preset
    if(preset_idx >= 0) {
        for(int k = 0; k < 3; k++) {
            if(!pages[0].locked[k]) {
                preset_idx = -1;  // custom mode
                break;
            }
        }
    }
}

void HandleMidiMessage(MidiEvent m) {
    if(m.type == NoteOn) {
        NoteOnEvent p = m.AsNoteOn();
        if(p.velocity == 0) {
            // Velocity 0 = NoteOff
            voice_mgr.NoteOff(p.note, sustain);
        } else {
            if(lfo_sync) lfo_reset_req = true;
            voice_mgr.NoteOn(p.note, p.velocity, partial_amps);
        }
    } else if(m.type == NoteOff) {
        NoteOffEvent p = m.AsNoteOff();
        voice_mgr.NoteOff(p.note, sustain);
    }
}

// ─── Audio Callback ───────────────────────────────────────────────────────────
void AudioCallback(AudioHandle::InputBuffer  /*in*/,
                   AudioHandle::OutputBuffer out,
                   size_t size) {
    // Consume LFO sync request
    if(lfo_reset_req) {
        lfo_reset_req = false;
        lfo.Init(g_sample_rate);
        lfo.SetFreq(lfo_rate);
        lfo.SetAmp(1.0f);
        static const int waves[4] = {
            Oscillator::WAVE_SIN, Oscillator::WAVE_TRI,
            Oscillator::WAVE_SAW, Oscillator::WAVE_SQUARE
        };
        lfo.SetWaveform(waves[lfo_wave]);
    }

    for(size_t i = 0; i < size; i++) {
        float lfo_val = lfo.Process();  // -1..+1

        // Pitch modulation (tutti vibrato): ±2 semitones max
        float pitch_scale = 1.0f;
        if(lfo_target < 0.5f && lfo_depth > 0.001f) {
            pitch_scale = powf(2.f, lfo_val * lfo_depth * 2.f / 12.f);
        }
        if(pitch_scale != 1.0f) {
            for(auto& v : voice_mgr.voices) {
                if(v.active) v.osc.SetFreq(mtof(v.note) * pitch_scale);
            }
        }

        float sig = voice_mgr.Process();

        // Amplitude modulation (tremolo) when lfo_target → amp
        if(lfo_target >= 0.5f) {
            float mod = 1.f + lfo_val * lfo_depth * 0.5f;  // ±50%
            sig *= mod;
        }

        sig *= master_vol;

        // Chorus (stereo)
        float l = sig, r = sig;
        if(!chorus_bypass && cho_mix > 0.001f) {
            float cho_l = chorus.Process(sig);
            float cho_r = chorus.GetRight();
            l = sig * (1.f - cho_mix) + cho_l * cho_mix;
            r = sig * (1.f - cho_mix) + cho_r * cho_mix;
        }

        // ReverbSc (stereo, LGPL)
        if(!reverb_bypass && reverb_mix > 0.001f) {
            float wl, wr;
            reverb.Process(l, r, &wl, &wr);
            l = l * (1.f - reverb_mix) + wl * reverb_mix;
            r = r * (1.f - reverb_mix) + wr * reverb_mix;
        }

        // Soft clamp
        out[0][i] = fclamp(l, -1.f, 1.f);
        out[1][i] = fclamp(r, -1.f, 1.f);
    }
}

// ─── Main ─────────────────────────────────────────────────────────────────────
int main(void) {
    hw.Init();
    hw.SetAudioBlockSize(48);
    hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
    g_sample_rate = hw.AudioSampleRate();

    // Init DSP
    voice_mgr.Init(g_sample_rate);
    chorus.Init(g_sample_rate);
    reverb.Init(g_sample_rate);
    reverb.SetFeedback(0.85f);
    reverb.SetLpFreq(8000.f);

    lfo.Init(g_sample_rate);
    lfo.SetFreq(lfo_rate);
    lfo.SetAmp(1.0f);
    lfo.SetWaveform(Oscillator::WAVE_SIN);

    chorus.SetLfoFreq(cho_rate);
    chorus.SetLfoDepth(cho_depth);
    chorus.SetDelay(cho_delay);

    // Compute initial partials and apply ADSR to all voices
    RecalcPartials();
    voice_mgr.SetEnvAll(atk_s, dec_s, sus_lv, rel_s);

    // OLED
    hw.display.Fill(false);
    hw.display.SetCursor(0, 0);
    hw.display.WriteString("AdditiveSynth", Font_7x10, true);
    hw.display.SetCursor(0, 16);
    hw.display.WriteString("8-voice poly", Font_6x8, true);
    hw.display.Update();

    // MIDI
    hw.midi.StartReceive();

    // Start ADC before audio!
    hw.StartAdc();

    // Init knob pickup with current physical positions
    InitPages();

    // Brief splash
    System::Delay(600);

    hw.StartAudio(AudioCallback);

    for(;;) {
        // MIDI
        hw.midi.Listen();
        while(hw.midi.HasEvents()) {
            HandleMidiMessage(hw.midi.PopEvent());
        }

        // Controls
        hw.ProcessAllControls();
        ProcessSwitches();
        ProcessKnobs();
        ProcessKeys();

        // Display & LEDs
        UpdateDisplay();
        UpdateLeds();

        System::Delay(1);
    }
}
