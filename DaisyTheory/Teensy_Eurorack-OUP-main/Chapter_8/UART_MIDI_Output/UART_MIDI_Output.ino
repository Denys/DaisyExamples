/*
    Chapter 8: UART MIDI output
*/


#include <Bounce.h>
#include <MIDI.h>

Bounce button0 = Bounce(0, 15);
const int channel = 1;
const int velocity = 100;

MIDI_CREATE_DEFAULT_INSTANCE();

void setup()
{
  pinMode(0, INPUT_PULLUP);
  MIDI.begin(MIDI_CHANNEL_OMNI);

}

void loop()
{
    button0.update();
    if (button0.fallingEdge()) {
        //Output a C-major triad
        MIDI.sendNoteOn(60, velocity, channel);  // 60 = C4
        MIDI.sendNoteOn(64, velocity, channel);
        MIDI.sendNoteOn(67, velocity, channel);
}

    if (button0.risingEdge()) {
        //Turn off the C-major triad
        MIDI.sendNoteOff(60, 0, channel);
        MIDI.sendNoteOff(64, 0, channel);
        MIDI.sendNoteOff(67, 0, channel);
}

    //Controllers should discard incoming MIDI messages
    while (MIDI.read()) {
      // ignore incoming messages
    }
}
