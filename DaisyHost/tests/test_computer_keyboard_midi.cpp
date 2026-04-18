#include <gtest/gtest.h>

#include "daisyhost/ComputerKeyboardMidi.h"

namespace
{
TEST(ComputerKeyboardMidiTest, MapsPianoStyleKeysToChromaticNotes)
{
    EXPECT_EQ(daisyhost::ComputerKeyboardMidi::KeyToMidiNote('a', 4), 60);
    EXPECT_EQ(daisyhost::ComputerKeyboardMidi::KeyToMidiNote('w', 4), 61);
    EXPECT_EQ(daisyhost::ComputerKeyboardMidi::KeyToMidiNote('s', 4), 62);
    EXPECT_EQ(daisyhost::ComputerKeyboardMidi::KeyToMidiNote('j', 4), 71);
    EXPECT_EQ(daisyhost::ComputerKeyboardMidi::KeyToMidiNote('k', 4), 72);
}

TEST(ComputerKeyboardMidiTest, RejectsUnsupportedKeysAndClampsOctave)
{
    EXPECT_EQ(daisyhost::ComputerKeyboardMidi::KeyToMidiNote('p', 4), -1);
    EXPECT_EQ(daisyhost::ComputerKeyboardMidi::ClampOctave(-2), 0);
    EXPECT_EQ(daisyhost::ComputerKeyboardMidi::ClampOctave(12), 8);
}

TEST(ComputerKeyboardMidiTest, DetectsOctaveShiftKeys)
{
    EXPECT_TRUE(daisyhost::ComputerKeyboardMidi::IsOctaveDownKey('z'));
    EXPECT_TRUE(daisyhost::ComputerKeyboardMidi::IsOctaveUpKey('x'));
    EXPECT_FALSE(daisyhost::ComputerKeyboardMidi::IsOctaveDownKey('a'));
    EXPECT_FALSE(daisyhost::ComputerKeyboardMidi::IsOctaveUpKey('a'));
}
} // namespace
