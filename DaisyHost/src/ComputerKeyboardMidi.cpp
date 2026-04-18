#include "daisyhost/ComputerKeyboardMidi.h"

#include <array>
#include <cctype>

namespace daisyhost
{
namespace
{
struct KeyOffset
{
    char key;
    int  semitoneOffset;
};

constexpr std::array<KeyOffset, 13> kPianoStyleOffsets = {{{'a', 0},
                                                           {'w', 1},
                                                           {'s', 2},
                                                           {'e', 3},
                                                           {'d', 4},
                                                           {'f', 5},
                                                           {'t', 6},
                                                           {'g', 7},
                                                           {'y', 8},
                                                           {'h', 9},
                                                           {'u', 10},
                                                           {'j', 11},
                                                           {'k', 12}}};
} // namespace

int ComputerKeyboardMidi::KeyToMidiNote(char key, int octave)
{
    const char normalized = NormalizeKey(key);
    for(const auto& mapping : kPianoStyleOffsets)
    {
        if(mapping.key == normalized)
        {
            return ((ClampOctave(octave) + 1) * 12) + mapping.semitoneOffset;
        }
    }
    return -1;
}

int ComputerKeyboardMidi::ClampOctave(int octave)
{
    if(octave < kMinOctave)
    {
        return kMinOctave;
    }
    if(octave > kMaxOctave)
    {
        return kMaxOctave;
    }
    return octave;
}

bool ComputerKeyboardMidi::IsOctaveDownKey(char key)
{
    return NormalizeKey(key) == 'z';
}

bool ComputerKeyboardMidi::IsOctaveUpKey(char key)
{
    return NormalizeKey(key) == 'x';
}

char ComputerKeyboardMidi::NormalizeKey(char key)
{
    return static_cast<char>(std::tolower(static_cast<unsigned char>(key)));
}
} // namespace daisyhost
