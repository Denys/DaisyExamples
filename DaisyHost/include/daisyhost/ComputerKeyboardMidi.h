#pragma once

namespace daisyhost
{
class ComputerKeyboardMidi
{
  public:
    static constexpr int kMinOctave = 0;
    static constexpr int kMaxOctave = 8;

    static int KeyToMidiNote(char key, int octave);
    static int ClampOctave(int octave);
    static bool IsOctaveDownKey(char key);
    static bool IsOctaveUpKey(char key);

  private:
    static char NormalizeKey(char key);
};
} // namespace daisyhost
