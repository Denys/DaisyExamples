/*
 * ArpEnging.h
 * Brent Edstrom, 2023
 */

 #ifndef __ARPENGINE
 #define __ARPENGINE

 #include <Arduino.h>

struct MidiNote{
  byte note;
  byte velocity;
  byte channel;
  MidiNote(): note(0), velocity(0), channel(0){};
};


class ArpEngine
{
   static const int MAX_NOTES = 24;
   MidiNote notes[MAX_NOTES];
   int current_index;
   int num_notes;
   MidiNote last_note;

   public:
   //Constructor
   ArpEngine():current_index(0), num_notes(0){};

   //Utility functions
   void addNote(byte note, byte velocity, byte channel);
   bool removeNote(byte note);
   void sortNotes();
   int getNumNotes(){return num_notes;};
   void clearAllNotes(){num_notes = 0;};

   //MIDI functions
   virtual void sendNoteOn(byte note, byte velocity, byte channel);
   virtual void sendNoteOff(byte note, byte velociy, byte channel);
   void sendLastNoteOff();
   void allNotesOff();

   //Playback functions
   void restartArp(){current_index = 0;};
   void doUpwardArp(int reps);
   void doDownwardArp(int reps);
   void doRandomArp(int reps);
   void doWrap(int reps, int inc_value);
   
   //void doCommandSequence();
    
};

 #endif
