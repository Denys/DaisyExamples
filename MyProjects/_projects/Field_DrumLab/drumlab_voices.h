// drumlab_voices.h — compile-time voice configuration for Field_DrumLab
// All functions are constexpr/inline so they can be tested with static_assert.
// Pattern mirrors noisetoaster_modes.h.

#pragma once
#include <stdint.h>

namespace drumlab
{

// ── Voice identifiers ────────────────────────────────────────────────────────
enum class VoiceId
{
    Kick  = 0, // AnalogBassDrum
    Snare = 1, // SyntheticSnareDrum
    HiHat = 2, // HiHat<SquareNoise, LinearVCA, true>
    Clap  = 3, // AnalogSnareDrum
    Perc  = 4, // SyntheticBassDrum
    Count = 5,
};

// ── Page identifiers (SW1 toggles) ───────────────────────────────────────────
enum class PageId
{
    Synth = 0, // K1-K7 = Pitch, Decay, Tone, Timbre, Accent, Pan, Level
    Mix
    = 1, // K1-K5 = individual voice levels, K6 = master decay, K7 = master accent
};

// ── A-row pad assignments ──────────────────────────────────────────────────────
// A1-A6: trigger voice 0-5 (A5=HiHat open, A3=HiHat closed)
// A7: trigger all voices simultaneously (crash accent)
// A8: reserved
constexpr int kNumPads = 8;

// ── Safety: LED indices for Daisy Field ───────────────────────────────────────
// A-row LEDs are REVERSED:  A1→15, A2→14 … A8→8
// B-row LEDs are sequential: B1→0,  B2→1  … B8→7
constexpr int KeyALedId(int key_index)
{
    // key_index 0..7 → A1..A8
    return 15 - key_index;
}

constexpr int KeyBLedId(int key_index)
{
    // key_index 0..7 → B1..B8
    return key_index;
}

// ── B-row: B1-B5 are focus selectors ─────────────────────────────────────────
// B-row keyboard indices are 8..15 (hw.KeyboardRisingEdge(8..15))
constexpr int FocusKeyIndex(VoiceId v)
{
    return 8 + static_cast<int>(v); // B1=8, B2=9, ... B5=12
}

// B-row LED index for a given voice focus button
constexpr int FocusLedIndex(VoiceId v)
{
    return KeyBLedId(static_cast<int>(v)); // 0..4
}

// ── B-row cycling ─────────────────────────────────────────────────────────────
constexpr VoiceId AdvanceFocus(VoiceId v)
{
    switch(v)
    {
        case VoiceId::Kick: return VoiceId::Snare;
        case VoiceId::Snare: return VoiceId::HiHat;
        case VoiceId::HiHat: return VoiceId::Clap;
        case VoiceId::Clap: return VoiceId::Perc;
        case VoiceId::Perc:
        default: return VoiceId::Kick;
    }
}

// ── Voice display names ───────────────────────────────────────────────────────
inline const char* VoiceShortName(VoiceId v)
{
    switch(v)
    {
        case VoiceId::Kick: return "KICK";
        case VoiceId::Snare: return "SNR";
        case VoiceId::HiHat: return "HAT";
        case VoiceId::Clap: return "CLAP";
        case VoiceId::Perc: return "PERC";
        default: return "---";
    }
}

inline const char* VoiceFullName(VoiceId v)
{
    switch(v)
    {
        case VoiceId::Kick: return "KICK";
        case VoiceId::Snare: return "SNARE";
        case VoiceId::HiHat: return "HIHAT";
        case VoiceId::Clap: return "CLAP";
        case VoiceId::Perc: return "PERC";
        default: return "---";
    }
}

// ── Synth page: parameter names for OLED focus zoom ─────────────────────────
// K1-K7 on Synth page:
constexpr const char* kSynthParamNames[7] = {
    "Pitch",  // K1
    "Decay",  // K2
    "Tone",   // K3
    "Timbre", // K4
    "Accent", // K5
    "Pan",    // K6
    "Level",  // K7
};

// Mix page: parameter names
constexpr const char* kMixParamNames[7] = {
    "Kick Lvl", // K1
    "Snr  Lvl", // K2
    "Hat  Lvl", // K3
    "Clap Lvl", // K4
    "Perc Lvl", // K5
    "M.Decay",  // K6
    "M.Accent", // K7
};

// ── MIDI note → voice mapping (GM percussion, channel 10) ────────────────────
// Returns -1 if note is not mapped.
constexpr int MidiNoteToVoice(int note)
{
    switch(note)
    {
        case 36: return static_cast<int>(VoiceId::Kick);  // C2  Bass Drum 1
        case 35: return static_cast<int>(VoiceId::Kick);  // B1  Bass Drum 2
        case 38: return static_cast<int>(VoiceId::Snare); // D2  Acoustic Snare
        case 40: return static_cast<int>(VoiceId::Snare); // E2  Electric Snare
        case 42: return static_cast<int>(VoiceId::HiHat); // F#2 Closed Hi-Hat
        case 44: return static_cast<int>(VoiceId::HiHat); // G#2 Pedal Hi-Hat
        case 46: return static_cast<int>(VoiceId::HiHat); // A#2 Open Hi-Hat
        case 39: return static_cast<int>(VoiceId::Clap);  // D#2 Hand Clap
        case 37: return static_cast<int>(VoiceId::Perc);  // C#2 Side Stick
        case 56: return static_cast<int>(VoiceId::Perc);  // G#3 Cowbell
        default: return -1;
    }
}

// Returns true if the MIDI note should trigger the HiHat in open mode
constexpr bool MidiNoteIsOpenHat(int note)
{
    return note == 46;
}

// ── Startup default knob values (0.0 - 1.0) ──────────────────────────────────
// Used to prime knob shadow values on boot (same pattern as kDefaultKnobValues[8])
constexpr float kDefaultKnobValues[8] = {
    0.35f, // K1 Pitch     (mid-range)
    0.40f, // K2 Decay     (medium)
    0.50f, // K3 Tone      (neutral)
    0.50f, // K4 Timbre    (neutral)
    0.60f, // K5 Accent    (moderate)
    0.50f, // K6 Pan       (centre)
    0.80f, // K7 Level     (full)
    0.80f, // K8 Master Vol
};

// ── Default per-voice parameter sets ─────────────────────────────────────────
// These match the DrumLab spec startup defaults.

// Pitch defaults (as raw frequency in Hz, or 0-1 ratio for engines without SetFreq range)
constexpr float kDefaultFreq[5]   = {55.0f, 200.0f, 3000.0f, 300.0f, 400.0f};
constexpr float kDefaultDecay[5]  = {0.40f, 0.40f, 0.20f, 0.30f, 0.20f};
constexpr float kDefaultTone[5]   = {0.50f, 0.50f, 0.50f, 0.60f, 0.50f};
constexpr float kDefaultTimbre[5] = {0.50f, 0.50f, 0.80f, 0.85f, 0.30f};
constexpr float kDefaultAccent[5] = {0.60f, 0.60f, 0.80f, 0.60f, 0.60f};
constexpr float kDefaultPan[5]    = {0.50f, 0.60f, 0.70f, 0.40f, 0.35f};
constexpr float kDefaultLevel[5]  = {0.80f, 0.80f, 0.80f, 0.80f, 0.80f};

// ── Knob focus routing helpers ────────────────────────────────────────────────
// On Synth page, K1-K7 map to:
// 0=Pitch, 1=Decay, 2=Tone, 3=Timbre, 4=Accent, 5=Pan, 6=Level
// These names are informational; actual routing is done in UpdateControls()

// ── LED brightness for focus indicator ───────────────────────────────────────
constexpr float LedForFocus(VoiceId focused, VoiceId led_voice)
{
    return (focused == led_voice) ? 1.0f : 0.08f; // dim = not focused
}

// ── Trigger flash constants ───────────────────────────────────────────────────
constexpr uint32_t kTriggerFlashMs
    = 80; // how long A-row LED flashes on trigger
constexpr uint32_t kFocusTimeoutMs
    = 1400; // OLED zoom timeout (matches Noise Toaster)
constexpr float kKnobMoveThresh = 0.015f;

// ── Fixed internal settings ───────────────────────────────────────────────────
// Stereo pan table (0=full L, 0.5=centre, 1=full R)
constexpr float kFixedPan[5] = {0.50f, 0.60f, 0.70f, 0.40f, 0.35f};

// Fixed AnalogBassDrum self-FM and attack-FM amounts
constexpr float kKickSelfFm   = 0.20f;
constexpr float kKickAttackFm = 0.30f;

// Fixed SyntheticSnareDrum FM amount
constexpr float kSnareFmAmount = 0.40f;

// Fixed HiHat frequency in Hz (baseline; focus Pitch knob shifts it)
constexpr float kHatBaseFreqHz = 3000.0f;

// Fixed clap (AnalogSnareDrum) frequency
constexpr float kClapBaseFreqHz = 300.0f;

// Fixed perc SyntheticBassDrum FM envelope
constexpr float kPercFmEnvAmount = 0.50f;
constexpr float kPercFmEnvDecay  = 0.30f;

// Output limiter ceiling
constexpr float kOutputCeiling = 0.95f;

// Number of active voices
constexpr int kNumVoices = static_cast<int>(VoiceId::Count);

} // namespace drumlab
