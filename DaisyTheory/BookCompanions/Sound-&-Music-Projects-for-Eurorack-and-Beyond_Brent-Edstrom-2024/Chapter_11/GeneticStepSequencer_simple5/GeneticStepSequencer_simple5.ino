/*
  Genetic Step Sequencer
  Brent Edstom, 2023

  Tested with TeensyLC microcontroller (set USB type to MIDI)
  
  Pin and pot assignments:
  Play/stop button: 0
  Ignore rhythm button: 1
  Internal/external clock button: 2
  Record on/off button: 3
  Tempo/rate encoder: 4,5   
 */

#include <Bounce.h>
#include <Encoder.h>
#include <Metro.h>

#include "Genotype.h"

Bounce playBtn = Bounce(0, 15);
Bounce ignoreRhythmBtn = Bounce(1, 15);
Bounce internalBtn = Bounce(2, 15);
Bounce recordBtn = Bounce(3, 15);

Encoder tempoEncoder(4, 5);

//Constants for MIDI clock
const int CLOCK = 0xF8;
const int START = 0xFA;
const int CONTINUE = 0xFB;
const int STOP = 0xFC;

//Constants for sequencer state
bool playing = false;
bool ignore_rhythm = true;
bool recording = false;
bool internal_clock = false;
int  bpm = 240;
int  rate = 6;
int  clock_ticks = 0;
byte last_note = 0;

//Instantiate global objects
Genotype genotype;
MIDINote note;
Metro metro = Metro(60000/bpm);

void setup() 
{
    //Set up pins for pushbuttons
    pinMode(0, INPUT_PULLUP);
    pinMode(1, INPUT_PULLUP);
    pinMode(2, INPUT_PULLUP);
    pinMode(3, INPUT_PULLUP);

   //Configure MIDI callback functions
   usbMIDI.setHandleNoteOn(onNoteOn);
   usbMIDI.setHandleRealTimeSystem(handleRealTimeSystem);
   usbMIDI.setHandleStart(handleStart);
   usbMIDI.setHandleStop(handleStop);
   usbMIDI.setHandleSongPosition(handleSongPosition); 

   //Seed the random number generator
   randomSeed(analogRead(10));
}

void loop() 
{
    while (usbMIDI.read()) {
    }

    if(metro.check() == 1 && internal_clock == true)
    {
        tick();
        metro.reset();
    }
    
    trackPlayBtn();
    trackIgnoreRhythmBtn();
    trackRecordBtn();
    handleInternalBtn();
    handleTempoEncoder();
}

void tick()
{
    static int counter = 0;
    
    if(playing)
    {
        note = genotype.nextNote(ignore_rhythm);
        if(note.velocity != 0)
        {
          usbMIDI.sendNoteOff(last_note, note.velocity, note.channel);
          usbMIDI.sendNoteOn(note.note, note.velocity, note.channel);
          last_note = note.note;

          /*TEST
            if(counter <1000){
            Serial.println(note.note);
            }
            counter++;
          */
        }
    }
}

void handleInternalBtn()
{
    internalBtn.update();
    if(internalBtn.fallingEdge())
    {
        if(internal_clock)
        {
            internal_clock = false;
        }else{
            internal_clock = true; 
        }
    }
}

void trackPlayBtn()
{
    playBtn.update();
    if(playBtn.fallingEdge())
    {
        if(playing)
        {
          playing = false;
          usbMIDI.sendNoteOff(note.note, note.velocity, note.channel);
        }else{
          playing = true;
        }
    }
}

void trackIgnoreRhythmBtn()
{
    ignoreRhythmBtn.update();
    if(ignoreRhythmBtn.fallingEdge())
    {
        if(ignore_rhythm == true)
        {
            ignore_rhythm = false;
        }else{
            ignore_rhythm = true;
        }
        
    }
}

void trackRecordBtn()
{
    recordBtn.update();
    if(recordBtn.fallingEdge())
    {
        if(recording)
        {
            recording = false;
        }else{
            recording = true;
        }
    }
}

void handleTempoEncoder()
{
    static long last_position = 0;

    long pos = tempoEncoder.read();

    //Don't continue if the encoder position hasn't changed
    if(pos == last_position)
    {
         return;
    }

    //Calculate the difference between new and old position
    long difference = pos - last_position;
    last_position = pos;

    if(internal_clock)
    {
        bpm += difference;
        metro.interval(60000/bpm);
    }else{
        rate += difference;
        if(rate <= 0)
        {
            rate = 1;
        }else if(rate > 24)
        {
            rate = 24;
        }
    }   
}

void handleRealTimeSystem(byte b)
{
  //Ignore if sequencer is not active or running on internal clock
  if(!playing || internal_clock == true)
  {
      return; 
  }
  doClock(b);
  
}

void handleStart(void)
{
     clock_ticks = 0;
}

void handleStop(void)
{
    //usbMIDI.sendNoteOn(last_note, note.velocity, note.channel);
    allNotesOff();
}

void handleSongPosition(uint16_t pos)
{
    //Convert position to ticks (6 MIDI clocks per beat)
    clock_ticks = pos * 6;
} 

void doClock(byte b)
{
  if(b == CLOCK)
  { 
      if(clock_ticks % rate == 0)
      {
          tick();
      }
      clock_ticks++;
  }

 if(b == START)
  {
      clock_ticks = 0;
  }
  
  if(b == CONTINUE)
  {
      //Use this for LED on or other functionality
  }
  
  if(b == STOP)
  {      
     //usbMIDI.sendNoteOff(last_note, note.velocity, note.channel);
     allNotesOff();
  }
}

void onNoteOn(byte channel, byte n, byte velocity) 
{
    static int index = 0;
    if(recording && velocity != 0)
    {
       genotype.noteArray[index] = (uint8_t) n;
       index++;
       if(index >=8)
       {
          index = 0;
       }
    }
}

//Call this function to prevent stuck notes
void allNotesOff()
{
    for(int n = 0; n < 127; n++)
    {
        usbMIDI.sendNoteOff(n, 0, 1);
    }
}

