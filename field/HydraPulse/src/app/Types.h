#pragma once
#include "src/core/Pattern16.h"
#include "src/dsp/Voices.h"
#include <array>
#include <cstdint>
namespace hydrapulse
{
constexpr std::size_t kTracks = 4, kSteps = 16, kBanks = 3, kPresets = 8;
enum class Bank : std::uint8_t
{
    A = 0,
    B = 1,
    Fill = 2
};
struct BankPattern
{
    core::Pattern16<kTracks> notes, accents;
};
struct Preset
{
    const char *name;
    float bpm, swing, drive;
    std::array<dsp::VoiceParameters, kTracks> voices;
    // masks[bank][track], least significant bit = step 1.
    std::array<std::array<std::uint16_t, kTracks>, kBanks> notes, accents;
};
struct StereoFrame
{
    float left{0}, right{0};
};
struct InputFrame
{
    std::array<float, 8> knobs{};
    std::uint16_t keys{0}; // stable logical held-key states
    bool play{false}, shift{false};
};
enum class MidiCommandType : std::uint8_t
{
    Trigger,
    Start,
    Stop,
    Panic
};
struct MidiCommand
{
    MidiCommandType type{};
    std::uint8_t track{0}, velocity{0};
};
struct UiSnapshot
{
    std::array<float, 4> params{};
    std::uint16_t notes{0}, accents{0}, pickup_mask{0};
    std::uint8_t selected{0}, step{0}, edit_bank{0}, play_bank{0}, queued_bank{0}, preset{0};
    std::uint8_t pending_preset{0}, mutes{0};
    bool running{false}, shift{false}, fill{false}, preset_armed{false}, loading{false};
    float bpm{120}, swing{0}, drive{0}, master{0};
    std::uint32_t callback_count{0}, snapshot_drops{0}, midi_drops{0}, faults{0};
    std::uint32_t load_max_per_mille{0}, overruns{0}, late_starts{0};
};
} // namespace hydrapulse
