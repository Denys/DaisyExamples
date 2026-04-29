
/*
 * ArpEngine.cpp
 * Brent Edstrom, 2023
 */

 #include "ArpEngine.h"

//==========Utility Member Functions==========
void ArpEngine::addNote(byte note, byte velocity, byte channel)
{
    num_notes++;
    if(num_notes >= MAX_NOTES)
    {
        num_notes = MAX_NOTES-1;
    }
    notes[num_notes -1].note = note;
    notes[num_notes -1].velocity = velocity;
    notes[num_notes -1].channel = channel;

    //Do a simple bubble sort based on note number
    sortNotes();
}


bool ArpEngine::removeNote(byte note)
{
  bool note_found = false;
 
  //Loop through the note array
  for(int i = 0; i < num_notes; i++)
  {
       //See if the note is in the array
      if(notes[i].note == note)
      {
          note_found = true;
          //Remove the note by moving higher notes
          //down one slot in the array
          for(int c = i; c < num_notes; c++)
          {
             notes[c] = notes[c+1];
          }
          
          //Update the note counter
          num_notes--;
          if(num_notes < 0)
          {
            num_notes = 0;
          }
          //Break out of the loop
          break;
      }

  }
  //Return note_found status
  return note_found;

}

//See notes at: https://medium.com/karuna-sehgal/an-introduction-to-bubble-sort-d85273acfcd8
void ArpEngine::sortNotes()
{
    for (int i = 0; i < num_notes; i++)
    {
      for (int j = 0; j < (num_notes - i - 1); j++)
      {
          if (notes[j].note > notes[j + 1].note)
          {
              MidiNote temp = notes[j];
              notes[j] = notes[j + 1];
              notes[j + 1] = temp;
          }
      }
    }
}

//==========MIDI Member Functions==========
void ArpEngine::sendNoteOn(byte note, byte velocity, byte channel)
{
    usbMIDI.sendNoteOn(note, velocity, channel);
}

void ArpEngine::sendNoteOff(byte note, byte velocity, byte channel)
{
    usbMIDI.sendNoteOff(note, velocity, channel);
}

void ArpEngine::sendLastNoteOff()
{
   if(last_note.note != 0)
   {
      sendNoteOff(last_note.note, last_note.velocity, last_note.channel);
   }
}

void ArpEngine::allNotesOff()
{
  for(int i = 0; i < num_notes; i++)
  {
      sendNoteOff(notes[i].note, 0, notes[i].channel);
  }
}

//==========Playback Member Functions==========
void ArpEngine::doUpwardArp(int reps)
{
    static int repeats = 0;
    sendNoteOn(notes[current_index].note, notes[current_index].velocity, 
              notes[current_index].channel);
    last_note = notes[current_index];
    repeats++;
    if(repeats >= reps)
    {
        repeats = 0;
        current_index++;
        if(current_index >= num_notes)
        {
            current_index = 0;
        } 
    }
}

void ArpEngine::doDownwardArp(int reps)
{
    static int repeats = 0;
    sendNoteOn(notes[current_index].note, notes[current_index].velocity, notes[current_index].channel);
    last_note = notes[current_index];
    repeats++;
    if(repeats >= reps)
    {
        repeats = 0;
        current_index--;
        if(current_index <0)
        {
            current_index = num_notes -1;
        }
    }
}

void ArpEngine::doRandomArp(int reps)
{
    static int repeats = 0;
    sendNoteOn(notes[current_index].note, notes[current_index].velocity, 
              notes[current_index].channel);
    last_note = notes[current_index];
    repeats++;
    if(repeats >= reps)
    {
        repeats = 0;
        current_index = random(0, num_notes);
    }
}

//NOTE: use negative inc_value for downward arpeggiation
void ArpEngine::doWrap(int reps, int inc_value)
{
    static int repeats = 0;
    static int calculated_index = 0;
    sendNoteOn(notes[current_index].note, notes[current_index].velocity, 
              notes[current_index].channel);
    last_note = notes[current_index];
    repeats++;
    if(repeats >= reps)
    {
        repeats = 0;
        //Use modulus to constrain index to array bounds
        calculated_index = calculated_index + inc_value;
        current_index = calculated_index % num_notes;
    }
}
