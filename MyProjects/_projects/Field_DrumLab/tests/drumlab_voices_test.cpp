// tests/drumlab_voices_test.cpp — compile-time verification of drumlab_voices.h
// Same pattern as noisetoaster_modes_test.cpp
// Compile with: g++ -std=c++17 -I.. drumlab_voices_test.cpp -o drumlab_voices_test

#include "drumlab_voices.h"

using namespace drumlab;

// ── KeyALedId ─────────────────────────────────────────────────────────────────
static_assert(KeyALedId(0) == 15); // A1 → LED 15
static_assert(KeyALedId(1) == 14); // A2 → LED 14
static_assert(KeyALedId(7) == 8);  // A8 → LED 8

// ── KeyBLedId ─────────────────────────────────────────────────────────────────
static_assert(KeyBLedId(0) == 0); // B1 → LED 0
static_assert(KeyBLedId(4) == 4); // B5 → LED 4

// ── FocusKeyIndex ─────────────────────────────────────────────────────────────
static_assert(FocusKeyIndex(VoiceId::Kick) == 8); // B1 rising edge index
static_assert(FocusKeyIndex(VoiceId::Snare) == 9);
static_assert(FocusKeyIndex(VoiceId::HiHat) == 10);
static_assert(FocusKeyIndex(VoiceId::Clap) == 11);
static_assert(FocusKeyIndex(VoiceId::Perc) == 12);

// ── FocusLedIndex ─────────────────────────────────────────────────────────────
static_assert(FocusLedIndex(VoiceId::Kick) == 0);
static_assert(FocusLedIndex(VoiceId::Snare) == 1);
static_assert(FocusLedIndex(VoiceId::HiHat) == 2);
static_assert(FocusLedIndex(VoiceId::Clap) == 3);
static_assert(FocusLedIndex(VoiceId::Perc) == 4);

// ── AdvanceFocus ─────────────────────────────────────────────────────────────
static_assert(AdvanceFocus(VoiceId::Kick) == VoiceId::Snare);
static_assert(AdvanceFocus(VoiceId::Snare) == VoiceId::HiHat);
static_assert(AdvanceFocus(VoiceId::HiHat) == VoiceId::Clap);
static_assert(AdvanceFocus(VoiceId::Clap) == VoiceId::Perc);
static_assert(AdvanceFocus(VoiceId::Perc) == VoiceId::Kick);

// ── LedForFocus ──────────────────────────────────────────────────────────────
static_assert(LedForFocus(VoiceId::Kick, VoiceId::Kick) == 1.0f);
static_assert(LedForFocus(VoiceId::Kick, VoiceId::Snare) == 0.08f);
static_assert(LedForFocus(VoiceId::Perc, VoiceId::Perc) == 1.0f);

// ── MidiNoteToVoice ───────────────────────────────────────────────────────────
static_assert(MidiNoteToVoice(36) == static_cast<int>(VoiceId::Kick));
static_assert(MidiNoteToVoice(35) == static_cast<int>(VoiceId::Kick));
static_assert(MidiNoteToVoice(38) == static_cast<int>(VoiceId::Snare));
static_assert(MidiNoteToVoice(40) == static_cast<int>(VoiceId::Snare));
static_assert(MidiNoteToVoice(42) == static_cast<int>(VoiceId::HiHat));
static_assert(MidiNoteToVoice(46) == static_cast<int>(VoiceId::HiHat));
static_assert(MidiNoteToVoice(39) == static_cast<int>(VoiceId::Clap));
static_assert(MidiNoteToVoice(37) == static_cast<int>(VoiceId::Perc));
static_assert(MidiNoteToVoice(99) == -1); // unmapped note

// ── MidiNoteIsOpenHat ────────────────────────────────────────────────────────
static_assert(MidiNoteIsOpenHat(46) == true);
static_assert(MidiNoteIsOpenHat(42) == false);

// ── Default values sanity ────────────────────────────────────────────────────
static_assert(kDefaultFreq[0] == 55.0f);   // Kick default 55 Hz
static_assert(kDefaultDecay[2] == 0.20f);  // HiHat closed default short decay
static_assert(kDefaultAccent[2] == 0.80f); // HiHat high accent
static_assert(kNumVoices == 5);
static_assert(kFixedPan[0] == 0.50f); // Kick centre

// ── kOutputCeiling in range ───────────────────────────────────────────────────
static_assert(kOutputCeiling > 0.9f && kOutputCeiling < 1.0f);

int main()
{
    return 0;
}
