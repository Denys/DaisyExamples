/*
  4-button patch changer demo
*/

#include <Bounce.h>

Bounce button0 = Bounce(0, 15);
Bounce button1 = Bounce(1, 15);
Bounce button2 = Bounce(2, 15);
Bounce button3 = Bounce(3, 15);

const int channel = 1;
const int velocity = 100;
const int patch1 = 0;
const int patch2 = 1;
const int patch3 = 2;
const int patch4 = 3;

void setup()
{
  pinMode(0, INPUT_PULLUP);
  pinMode(1, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);

}

void loop()
{
    button0.update();
    if (button0.fallingEdge()) {
        usbMIDI.sendProgramChange(patch1, channel);
    }

    button1.update();
    if (button1.fallingEdge()) {
        usbMIDI.sendProgramChange(patch2, channel);
    }

    button2.update();
    if (button2.fallingEdge()) {
        usbMIDI.sendProgramChange(patch3, channel);
    }

    button3.update();
    if (button3.fallingEdge()) {
        usbMIDI.sendProgramChange(patch4, channel);
    }

    //Controllers should discard incoming MIDI messages
    while (usbMIDI.read()) {

    }
}
