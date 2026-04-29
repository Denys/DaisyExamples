/*One-Finger Chord

  Note: Be sure to select Tools...USB Type: "MIDI"
*/

#include <Bounce.h>

Bounce button0 = Bounce(0, 15);
const int channel = 1;
const int velocity = 100;

void setup() 
{
  pinMode(0, INPUT_PULLUP);

}

void loop() 
{
    button0.update();
    if (button0.fallingEdge()) {
        //Output a C-major triad
        usbMIDI.sendNoteOn(60, velocity, channel);  // 60 = C4
        usbMIDI.sendNoteOn(64, velocity, channel); 
        usbMIDI.sendNoteOn(67, velocity, channel); 
    }

    if (button0.risingEdge()) {
        //Turn off the C-major triad
        usbMIDI.sendNoteOff(60, 0, channel);  
        usbMIDI.sendNoteOff(64, 0, channel); 
        usbMIDI.sendNoteOff(67, 0, channel); 
    }

    //Controllers should discard incoming MIDI messages 
    while (usbMIDI.read()) {
      // ignore incoming messages
    }


}
