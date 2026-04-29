/*
 *   SimpleSynth.h
 *   Brent Edstrom, 2020
 */

#include "SynthVoice.h"

 class SimpleSynth
 {
    enum{numVoices = 4};
    
    public:
    SynthVoice voices[numVoices];

    enum{oldest_note, highest_note, lowest_note};
    int notePrioritization = oldest_note;

    void setAttack(int attack)
    {
       for(int i = 0; i < numVoices; i++)
       {
          voices[i].pEnvelope->attack(attack);
       }
    }

    void setRelease(int release)
    {
        for(int i = 0; i < numVoices; i++)
        {
          voices[i].pEnvelope->release(release);
        }
    }

    void noteOn(byte channel, byte note, byte velocity) 
    {
        //Variables for index tracking
        int lowest_index = 0;
        int highest_index = 0;
        int oldest_index = 0;
        
        //Set starting values for lowest, highest, and oldest notes
        int low_note = voices[0].last_note;
        int high_note = voices[0].last_note;
        long oldest_timestamp = voices[0].timestamp;
        
        for(int i = 0; i < numVoices; i++)
        {
            if(voices[i].isActive() != true)
            {
                //An "empty" note: output and return
                voices[i].noteOn(note, velocity);
                return;
            }
            
            byte note = voices[i].last_note;

            //Check for lowest note
            if(note < low_note)
            {
                low_note = note;
                lowest_index = i;
            }

            //Check for highest note
            if(note > high_note)
            {
                high_note = note;
                highest_index = i;
            }

            //Check for oldest note
            long timestamp = voices[i].timestamp;
            if(timestamp < oldest_timestamp)
            {       
                oldest_timestamp = timestamp;
                oldest_index = i;
            }
        }

        switch(notePrioritization)
        {
            case oldest_note:  voices[oldest_index].noteOn(note, velocity); break;
            case highest_note: voices[highest_index].noteOn(note, velocity); break;
            case lowest_note:  voices[lowest_index].noteOn(note, velocity); break;
        }
        
    }
    
    void noteOff(byte channel, byte note, byte velocity) 
    {
        for(int i = 0; i < numVoices; i++)
        {
            voices[i].noteOff(note);
        }
    }
    
 };
