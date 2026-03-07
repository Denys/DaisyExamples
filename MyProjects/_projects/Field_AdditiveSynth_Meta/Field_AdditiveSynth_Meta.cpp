// Field_AdditiveSynth_Meta.cpp
// 8-voice polyphonic additive synthesizer for Daisy Field — with Meta-Controller
// HarmonicOscillator<16> + Adsr per voice
// Global: LFO (tutti vibrato/tremolo), Chorus, ReverbSc
// SW1/SW2 dual knob pages with pickup/catch
// A1-A8: spectral presets | B1-B8: LFO waveform + performance toggles
// MIDI: TRS hardware, last-note stealing
// Meta-Controller: SW1+SW2 chord → Superknob morph across 4 anchor scenes (Yee-King 2024, Ch.13)

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

// ─── Meta-Controller ──────────────────────────────────────────────────────────
// Superknob: one meta-position morphs all params across 4 anchor scenes.
// Based on Chapter 13 (Superknob) of Yee-King 2024, extended to 4 scenes.
struct MetaScene {
    bool  stored     = false;
    // Page A — stored as actual DSP values (not raw 0-1 knob reading)
    float rolloff    = 1.0f;
    float even_lv    = 0.8f;
    float odd_lv     = 0.8f;
    float atk_s      = 0.01f;
    float dec_s      = 0.1f;
    float sus_lv     = 0.7f;
    float rel_s      = 0.3f;
    float rev_mix    = 0.3f;
    // Page B — actual DSP values
    float lfo_rate   = 2.0f;
    float lfo_depth  = 0.0f;
    float lfo_target = 0.0f;
    float cho_rate   = 0.5f;
    float cho_depth  = 0.5f;
    float cho_delay  = 0.5f;
    float cho_mix    = 0.3f;
    float mst_vol    = 0.8f;
};

static const int kNumMetaScenes = 4;
static MetaScene meta_scenes[kNumMetaScenes];
static int   meta_active_scene = 0;   // which scene B1-B4 selects (0-3)
static float meta_position     = 0.0f; // K1 in meta-mode [0,1]
static bool  meta_mode         = false;

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

// ─── Meta-Controller Core ─────────────────────────────────────────────────────
void RecalcPartials();  // forward declaration

void InitMetaScenes() {
    // Scene 0: Sine-like (pure fundamental, airy pad)
    meta_scenes[0].stored    = true;
    meta_scenes[0].rolloff   = 2.0f;    // max knob-reachable rolloff
    meta_scenes[0].even_lv   = 0.0f;
    meta_scenes[0].odd_lv    = 0.0f;
    meta_scenes[0].atk_s     = 0.05f;
    meta_scenes[0].dec_s     = 0.2f;
    meta_scenes[0].sus_lv    = 0.8f;
    meta_scenes[0].rel_s     = 0.5f;
    meta_scenes[0].rev_mix   = 0.25f;
    meta_scenes[0].lfo_rate  = 1.0f;
    meta_scenes[0].lfo_depth = 0.0f;
    meta_scenes[0].lfo_target = 0.0f;
    meta_scenes[0].cho_rate  = 0.5f;
    meta_scenes[0].cho_depth = 0.0f;
    meta_scenes[0].cho_delay = 0.5f;
    meta_scenes[0].cho_mix   = 0.0f;
    meta_scenes[0].mst_vol   = 0.8f;

    // Scene 1: Sawtooth-like (full harmonics, bright lead)
    meta_scenes[1].stored    = true;
    meta_scenes[1].rolloff   = 1.0f;
    meta_scenes[1].even_lv   = 0.8f;
    meta_scenes[1].odd_lv    = 0.8f;
    meta_scenes[1].atk_s     = 0.005f;
    meta_scenes[1].dec_s     = 0.15f;
    meta_scenes[1].sus_lv    = 0.6f;
    meta_scenes[1].rel_s     = 0.4f;
    meta_scenes[1].rev_mix   = 0.4f;
    meta_scenes[1].lfo_rate  = 3.0f;
    meta_scenes[1].lfo_depth = 0.1f;
    meta_scenes[1].lfo_target = 0.0f;
    meta_scenes[1].cho_rate  = 1.0f;
    meta_scenes[1].cho_depth = 0.5f;
    meta_scenes[1].cho_delay = 0.5f;
    meta_scenes[1].cho_mix   = 0.3f;
    meta_scenes[1].mst_vol   = 0.8f;
    // Scenes 2 and 3: empty — user stores via long-press B3/B4
}

void StoreCurrentParamsAsScene(int idx) {
    MetaScene& sc  = meta_scenes[idx];
    sc.stored      = true;
    sc.rolloff     = rolloff;
    sc.even_lv     = even_level;
    sc.odd_lv      = odd_level;
    sc.atk_s       = atk_s;
    sc.dec_s       = dec_s;
    sc.sus_lv      = sus_lv;
    sc.rel_s       = rel_s;
    sc.rev_mix     = reverb_mix;
    sc.lfo_rate    = lfo_rate;
    sc.lfo_depth   = lfo_depth;
    sc.lfo_target  = lfo_target;
    sc.cho_rate    = cho_rate;
    sc.cho_depth   = cho_depth;
    sc.cho_delay   = cho_delay;
    sc.cho_mix     = cho_mix;
    sc.mst_vol     = master_vol;
}

void ApplyMetaMorph() {
    // Collect stored scene indices in ascending order
    int sidx[kNumMetaScenes];
    int ns = 0;
    for(int i = 0; i < kNumMetaScenes; i++)
        if(meta_scenes[i].stored) sidx[ns++] = i;
    if(ns == 0) return;

    MetaScene *sa, *sb;
    float t;
    if(ns == 1) {
        sa = sb = &meta_scenes[sidx[0]];
        t = 0.f;
    } else {
        float pos = meta_position * (float)(ns - 1);
        int   ia  = (int)pos;
        if(ia >= ns - 1) ia = ns - 2;
        int   ib  = ia + 1;
        t  = pos - (float)ia;
        sa = &meta_scenes[sidx[ia]];
        sb = &meta_scenes[sidx[ib]];
    }

    // Linear interpolation across all params (Superknob: y = mx + b, Ch. 13)
    #define LERP(f) (sa->f + t * (sb->f - sa->f))
    rolloff    = LERP(rolloff);
    even_level = LERP(even_lv);
    odd_level  = LERP(odd_lv);
    atk_s      = LERP(atk_s);
    dec_s      = LERP(dec_s);
    sus_lv     = LERP(sus_lv);
    rel_s      = LERP(rel_s);
    reverb_mix = LERP(rev_mix);
    lfo_rate   = LERP(lfo_rate);
    lfo_depth  = LERP(lfo_depth);
    lfo_target = LERP(lfo_target);
    cho_rate   = LERP(cho_rate);
    cho_depth  = LERP(cho_depth);
    cho_delay  = LERP(cho_delay);
    cho_mix    = LERP(cho_mix);
    master_vol = LERP(mst_vol);
    #undef LERP

    // Apply morphed values to DSP modules
    RecalcPartials();
    voice_mgr.SetEnvAll(atk_s, dec_s, sus_lv, rel_s);
    lfo.SetFreq(lfo_rate);
    chorus.SetLfoFreq(cho_rate);
    chorus.SetLfoDepth(cho_depth);
    chorus.SetDelay(cho_delay);
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
    if(meta_mode) {
        // Knob LEDs: all on (no lock indication in meta-mode)
        for(int k = 0; k < 8; k++)
            hw.led_driver.SetLed(knob_leds[k], 1.0f);

        // SW LEDs: both dim — signals meta-mode active
        hw.led_driver.SetLed(DaisyField::LED_SW_1, 0.25f);
        hw.led_driver.SetLed(DaisyField::LED_SW_2, 0.25f);

        // A-row: all off in meta-mode
        for(int i = 0; i < 8; i++)
            hw.led_driver.SetLed(key_a_leds[i], 0.0f);

        // B1-B4: scene state (bright=selected, dim=stored, off=empty)
        for(int i = 0; i < 4; i++) {
            float br = 0.0f;
            if(i == meta_active_scene)     br = 1.0f;
            else if(meta_scenes[i].stored) br = 0.35f;
            hw.led_driver.SetLed(key_b_leds[i], br);
        }
        // B5-B8: performance toggles unchanged
        hw.led_driver.SetLed(key_b_leds[4], lfo_sync      ? 1.0f : 0.0f);
        hw.led_driver.SetLed(key_b_leds[5], reverb_bypass ? 1.0f : 0.0f);
        hw.led_driver.SetLed(key_b_leds[6], chorus_bypass ? 1.0f : 0.0f);
        hw.led_driver.SetLed(key_b_leds[7], sustain       ? 1.0f : 0.0f);
    } else {
        // Normal mode
        for(int k = 0; k < 8; k++) {
            float br = pages[active_page].locked[k] ? 0.05f : 1.0f;
            hw.led_driver.SetLed(knob_leds[k], br);
        }
        hw.led_driver.SetLed(DaisyField::LED_SW_1, active_page == 0 ? 1.0f : 0.1f);
        hw.led_driver.SetLed(DaisyField::LED_SW_2, active_page == 1 ? 1.0f : 0.1f);
        for(int i = 0; i < 8; i++)
            hw.led_driver.SetLed(key_a_leds[i], (preset_idx == i) ? 1.0f : 0.0f);
        for(int i = 0; i < 4; i++)
            hw.led_driver.SetLed(key_b_leds[i], (lfo_wave == i) ? 1.0f : 0.0f);
        hw.led_driver.SetLed(key_b_leds[4], lfo_sync      ? 1.0f : 0.0f);
        hw.led_driver.SetLed(key_b_leds[5], reverb_bypass ? 1.0f : 0.0f);
        hw.led_driver.SetLed(key_b_leds[6], chorus_bypass ? 1.0f : 0.0f);
        hw.led_driver.SetLed(key_b_leds[7], sustain       ? 1.0f : 0.0f);
    }
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
    bool sw1p = hw.GetSwitch(DaisyField::SW_1)->Pressed();
    bool sw2p = hw.GetSwitch(DaisyField::SW_2)->Pressed();

    // Hold SW1+SW2 together for 200–800ms → toggle meta_mode
    static bool     both_arm = false;
    static uint32_t both_t   = 0;
    if(sw1p && sw2p && !both_arm) { both_arm = true; both_t = System::GetNow(); }
    if(both_arm && !(sw1p && sw2p)) {
        uint32_t dt = System::GetNow() - both_t;
        if(dt >= 200 && dt <= 800) meta_mode = !meta_mode;
        both_arm = false;
    }

    // Individual presses: chord guard — suppress if other switch is also held.
    // Pressing SW1 or SW2 alone exits meta_mode and selects that page.
    if(hw.GetSwitch(DaisyField::SW_1)->RisingEdge() && !sw2p) {
        meta_mode = false;
        SwitchToPage(0);
    }
    if(hw.GetSwitch(DaisyField::SW_2)->RisingEdge() && !sw1p) {
        meta_mode = false;
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

// ─── Meta-Controller Controls ─────────────────────────────────────────────────
static uint32_t meta_scene_hold_t[4] = {};
static bool     meta_scene_armed[4]  = {};

void ProcessMetaSceneKeys() {
    // B1-B4 = keyboard indices 8-11
    // Short press: select anchor scene for editing
    // Long press (>=500ms): store current synthesis params into that scene
    for(int i = 0; i < 4; i++) {
        if(hw.KeyboardRisingEdge(8 + i)) {
            meta_active_scene    = i;
            meta_scene_hold_t[i] = System::GetNow();
            meta_scene_armed[i]  = true;
        }
        if(meta_scene_armed[i] &&
           (System::GetNow() - meta_scene_hold_t[i]) >= 500) {
            StoreCurrentParamsAsScene(i);
            meta_scene_armed[i] = false;
        }
    }
    // B5-B8: performance toggles remain active in meta-mode
    if(hw.KeyboardRisingEdge(12)) lfo_sync      = !lfo_sync;
    if(hw.KeyboardRisingEdge(13)) reverb_bypass = !reverb_bypass;
    if(hw.KeyboardRisingEdge(14)) chorus_bypass = !chorus_bypass;
    if(hw.KeyboardRisingEdge(15)) sustain       = !sustain;
}

void ProcessMetaKnobs() {
    // K1: morph position across stored scenes [0,1]
    meta_position = hw.GetKnobValue(0);

    // K2-K8: directly edit Page A params of the active anchor scene
    // Note: no pickup/catch in meta-mode — direct control is intentional here
    MetaScene& sc = meta_scenes[meta_active_scene];
    sc.rolloff    = hw.GetKnobValue(1) * 2.0f;             // K2 → 0..2
    sc.even_lv    = hw.GetKnobValue(2);                    // K3 → 0..1
    sc.odd_lv     = hw.GetKnobValue(3);                    // K4 → 0..1
    sc.atk_s      = LogTimeS(hw.GetKnobValue(4), 2000.f);  // K5
    sc.dec_s      = LogTimeS(hw.GetKnobValue(5), 2000.f);  // K6
    sc.sus_lv     = hw.GetKnobValue(6);                    // K7 → 0..1
    sc.rel_s      = LogTimeS(hw.GetKnobValue(7), 4000.f);  // K8
    // rev_mix, LFO, chorus, master_vol only set via long-press store
}

void ProcessMetaAKeys() {
    // A-row (indices 0-7): load spectral preset tonal params into current scene
    for(int i = 0; i < 8; i++) {
        if(hw.KeyboardRisingEdge(i)) {
            MetaScene& sc = meta_scenes[meta_active_scene];
            sc.rolloff    = PRESETS[i].rolloff;
            sc.even_lv    = PRESETS[i].even;
            sc.odd_lv     = PRESETS[i].odd;
            sc.stored     = true;
        }
    }
}

void ProcessMetaMode() {
    ProcessMetaSceneKeys();
    ProcessMetaAKeys();
    ProcessMetaKnobs();
    ApplyMetaMorph();
}

// ─── Meta-Controller Display ───────────────────────────────────────────────────
void UpdateMetaDisplay() {
    hw.display.Fill(false);

    // Row 0: "META" + scene slot indicators ([N]=selected, (N)=stored, .=empty)
    char top[32] = "META ";
    for(int i = 0; i < kNumMetaScenes; i++) {
        char s[5];
        if(i == meta_active_scene)
            snprintf(s, sizeof(s), "[%d]", i + 1);
        else if(meta_scenes[i].stored)
            snprintf(s, sizeof(s), "(%d)", i + 1);
        else
            snprintf(s, sizeof(s), ". ");
        strncat(top, s, sizeof(top) - strlen(top) - 1);
    }
    hw.display.SetCursor(0, 0);
    hw.display.WriteString(top, Font_6x8, true);

    // Row 1: morph position bar (y=10..18)
    hw.display.DrawRect(0, 10, 127, 18, false, false);   // border
    int barW = (int)(meta_position * 127.f);
    if(barW > 0) hw.display.DrawRect(0, 10, barW, 18, true, true);

    // Row 2: which scenes are being interpolated + t value
    int sidx[kNumMetaScenes]; int ns = 0;
    for(int i = 0; i < kNumMetaScenes; i++)
        if(meta_scenes[i].stored) sidx[ns++] = i;

    char mid[64] = "";
    if(ns >= 2) {
        float pos = meta_position * (float)(ns - 1);
        int   ia  = (int)pos;
        if(ia >= ns - 1) ia = ns - 2;
        float t   = pos - (float)ia;
        // Use local vars so compiler can see these are small integers (1-4)
        int sa = sidx[ia] + 1, sb = sidx[ia + 1] + 1;
        int ti = (int)(t * 100.f);
        snprintf(mid, sizeof(mid), "S%d->S%d t=0.%02d", sa, sb, ti);
    } else if(ns == 1) {
        snprintf(mid, sizeof(mid), "Scene %d only", sidx[0] + 1);
    } else {
        snprintf(mid, sizeof(mid), "No scenes!");
    }
    hw.display.SetCursor(0, 22);
    hw.display.WriteString(mid, Font_6x8, true);

    // Row 3: current tonal param preview
    char tone[32];
    snprintf(tone, sizeof(tone), "R:%.1f E:%d%% O:%d%%",
             rolloff,
             (int)(even_level * 100.f),
             (int)(odd_level  * 100.f));
    hw.display.SetCursor(0, 36);
    hw.display.WriteString(tone, Font_6x8, true);

    // Row 4: editing guide
    char guide[32];
    snprintf(guide, sizeof(guide), "S%d K2-8=edit LP=store",
             meta_active_scene + 1);
    hw.display.SetCursor(0, 48);
    hw.display.WriteString(guide, Font_6x8, true);

    hw.display.Update();
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

    // Pre-populate meta-controller with two default scenes (Sine + Sawtooth)
    InitMetaScenes();

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
        if(meta_mode) {
            ProcessMetaMode();
            UpdateMetaDisplay();
        } else {
            ProcessKnobs();
            ProcessKeys();
            UpdateDisplay();
        }

        // LEDs
        UpdateLeds();

        System::Delay(1);
    }
}
