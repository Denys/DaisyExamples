

/*
  Chapter 8: UART MIDI input
*/

#include <MIDI.h>

MIDI_CREATE_DEFAULT_INSTANCE();

void setup()
{
  //Start MIDI and listen to all channels
  MIDI.begin(MIDI_CHANNEL_OMNI);
}

void loop()
{
    if(MIDI.read())
    {
        switch(MIDI.getType())
        {
            case midi::NoteOn:
              usbMIDI.sendNoteOn(MIDI.getData1(),MIDI.getData2(), MIDI.getChannel());
              break;
            case midi::NoteOff:
              usbMIDI.sendNoteOff(MIDI.getData1(), MIDI.getData2(), MIDI.getChannel());
              break;
            default: 
              break;
   
        }
    }
}



