/*
  ArpMachine v1.10
  Brent Edstrom, 2023

  Tested with TeensyLC microcontroller (set USB type to MIDI)
  Teensy 4.x or 3.x should work fine without modification.
  
  Pin and pot assignments:
  dec/inc pattern buttons (2, 3)
  dec/inc rate buttons (4, 9)
  dec/inc duration buttons (10, 11)
  dec/inc rep buttons (12, 14)
  
  Tempo pot: A5  
  active button: 16
  hold button: 15
  clock source button: 17
  restart button: 18
  LED: 13

  Optional UART MIDI receive/send (0, 1)
 
 */

#include <Bounce.h>
#include <TimerOne.h>

#include "ArpEngine.h"

const int CLOCK = 0xF8;
const int START = 0xFA;
const int CONTINUE = 0xFB;
const int STOP = 0xFC;
const unsigned long MICROSECONDS_PER_BEAT = 1000000 * 60;

//Enumerate playback types
enum{up, down, wrap, random_order};

//Primary arp. parameters
int currentPattern = up;
int rate = 6;
int duration = 6;
int reps = 1;

int clock_ticks = 0;
bool holding = false;
bool midiThru = false;
bool alignIndex = false;
bool isActive  = true;  
bool internalClock = false;
int  tempo;

const int LED_PIN =13;
const int TEMPO_PIN = A5;


//Set up buttons
Bounce decPatternBtn =  Bounce(2, 10);  //10 ms debounce
Bounce incPatternBtn =  Bounce(3, 10);
Bounce decRateBtn =     Bounce(4, 10);
Bounce incRateBtn =     Bounce(9, 10);
Bounce decDurationBtn = Bounce(10, 10);
Bounce incDurationBtn = Bounce(11, 10);
Bounce decRepsBtn =     Bounce(12, 10);
Bounce incReps =        Bounce(14, 10);


Bounce holdBtn = Bounce(15, 50);  
Bounce activeBtn = Bounce(16, 50);
Bounce clockSourceBtn = Bounce(17, 50);
Bounce restartBtn = Bounce(18, 10);

ArpEngine arpEngine;

void setup() 
{  
    //Set up button pins
    pinMode(2, INPUT_PULLUP);
    pinMode(3, INPUT_PULLUP);
    pinMode(4, INPUT_PULLUP);
    pinMode(9, INPUT_PULLUP);
    pinMode(10, INPUT_PULLUP);
    pinMode(11, INPUT_PULLUP);
    pinMode(12, INPUT_PULLUP);
    pinMode(14, INPUT_PULLUP);

    pinMode(15, INPUT_PULLUP);
    pinMode(16, INPUT_PULLUP);
    pinMode(17, INPUT_PULLUP);
    pinMode(18, INPUT_PULLUP);

    
    pinMode(LED_PIN, OUTPUT);  
    
    usbMIDI.setHandleNoteOn(noteOnHandler);
    usbMIDI.setHandleNoteOff(noteOffHandler);

    //Set up timer:
    Timer1.initialize(1000000/24);// Tempo = 60bpm (1000000)/24 
    Timer1.attachInterrupt(timerCallback); 

   //Handlers for MIDI clock
   usbMIDI.setHandleRealTimeSystem(handleRealTimeSystem);
   usbMIDI.setHandleStart(handleStart);
   //usbMIDI.setHandleContinue(handleContinue);
   usbMIDI.setHandleStop(handleStop);
   usbMIDI.setHandleSongPosition(handleSongPosition);  

}

void loop() 
{
    usbMIDI.read();
    
    trackRateButtons();
    trackPatternButtons();
    trackDurationButtons();
    trackRepeatButtons();

    if(internalClock)
    {
        trackTempoPot();
    }
    
    trackActiveSwitch();
    trackHoldButton();
    trackClockSourceButton();
    trackRestartButton();
}

  
void timerCallback()
{
    if(isActive && internalClock == true)
    {
        doClock(CLOCK);
    }
}

void noteOnHandler(byte channel, byte note, byte velocity)
{
    if(midiThru)
    {
        usbMIDI.sendNoteOn(note, velocity, channel);
    }

    if(holding)
    {
       //Pressing a note a second time will remove note from array
       if(arpEngine.removeNote(note))
       {
          //Exit if we just removed the note from the array
          return; 
       }
    }

    arpEngine.addNote(note, velocity, channel);
}


void noteOffHandler(byte channel, byte note, byte velocity)
{
    if(midiThru)
    {
        usbMIDI.sendNoteOff(note, velocity, channel);
    }
    if(holding)
    {
        //Ignore note-off messages while holding
        return;
    }
    //Remove note from the array
    arpEngine.removeNote(note); 
}

void handleRealTimeSystem(byte b)
{
  if(!isActive || internalClock == true)
  {
      //Ignore if not active
      return; 
  }
  doClock(b);
  
}

void doClock(byte b)
{
  if(b == CLOCK)
  { 
     clock_ticks++;

      //Handle note-off
      if(clock_ticks % duration == 0)
      {
         arpEngine.sendLastNoteOff();
      }
      
      //Handle note-on
      if(clock_ticks % rate == 0 && arpEngine.getNumNotes() > 0)
      {
          switch(currentPattern)
          {
              case up:            arpEngine.doUpwardArp(reps); break;
              case down:          arpEngine.doDownwardArp(reps); break;
              case random_order:  arpEngine.doRandomArp(reps); break;
              case wrap:          arpEngine.doWrap(reps, 2); break;
          }
       }
 
      //Turn LED on on beats:
      if(clock_ticks % 24 == 0)
      {
          clock_ticks = 0;
          if(alignIndex)
          {
            //current_index = 0;
            arpEngine.restartArp();
            alignIndex = false;
          }
          digitalWrite(LED_PIN, HIGH);
      }
      //Turn LED off on 8ths
      if(clock_ticks == 12)
      {
          digitalWrite(LED_PIN, LOW);
      }

  }

 if(b == START)
  {
      clock_ticks = 0;
      digitalWrite(LED_PIN, HIGH);
  }
  
  if(b == CONTINUE)
  {
      digitalWrite(LED_PIN, HIGH);
  }
  
  if(b == STOP)
  {      
      digitalWrite(LED_PIN, LOW);
      arpEngine.allNotesOff();
  }
}

void handleStart(void)
{
     clock_ticks = 0;
     //Comment out this line for floating starts
     arpEngine.restartArp();
}

void handleStop(void)
{
    arpEngine.allNotesOff();
}

void handleSongPosition(uint16_t pos)
{
    //Convert position to ticks (6 MIDI clocks per beat)
    clock_ticks = pos * 6;
} 

void trackRateButtons()
{
    //==============Track rate buttons
    incRateBtn.update();
    if(incRateBtn.fallingEdge())
    {
        //Increase rate (fewer ticks = faster)
        rate--;
        alignIndex = true;
        
        if(rate < 1)
        {
           rate = 1;
        }
        //update duration to new default
        duration = rate;
    }
    decRateBtn.update();
    if(decRateBtn.fallingEdge())
    {
        //Decrease rate (more tickes = slower)
        rate++;
        alignIndex = true;
        if(rate >24)
        {
           rate = 24;
        }
        //update duration to new default
        duration = rate;
    }

}

void trackDurationButtons()
{
    //Track duration decrease switch
    decDurationBtn.update();
    if(decDurationBtn.fallingEdge())
    {
        duration--;
        if(duration <1)
        {
          duration = 1;
        }
    }

    //Track duration increase switch
    incDurationBtn.update();
    if(incDurationBtn.fallingEdge())
    {
        duration++;
        if(duration > rate)
        {
          duration = rate;
        }
    }
}

void trackPatternButtons()
{
    //==============Track duration buttons
    decPatternBtn.update();
    if(decPatternBtn.fallingEdge())
    {
        currentPattern--;
        if(currentPattern < up)
        {
          currentPattern = up;
        }
    }
    incPatternBtn.update();
    if(incPatternBtn.fallingEdge())
    {
        currentPattern++;
        if(currentPattern > random_order)
        {
          currentPattern = random_order;
        }
    }
}


void trackActiveSwitch()
{
    activeBtn.update();
    if(activeBtn.fallingEdge())
    {
        if(isActive)
        {
            //Clean up if we are currently running
            isActive = false;
            arpEngine.allNotesOff();
            digitalWrite(LED_PIN, LOW);
        }else{
            isActive = true;
        }
    }
    
}

void trackHoldButton()
{
    holdBtn.update();
    if(holdBtn.fallingEdge())
    {
        //Toggle hold status
        if(holding)
        {
            holding = false;
            arpEngine.allNotesOff();
            arpEngine.clearAllNotes();
        }else{
            holding = true;
        }
    }
}

void trackRestartButton()
{
    restartBtn.update();
    if(restartBtn.fallingEdge())
    {
        //Reset playback index
        arpEngine.restartArp();
    }
}

void trackClockSourceButton()
{
   clockSourceBtn.update();
   if(clockSourceBtn.fallingEdge())
   {
       if(internalClock == true)
       {
          internalClock = false;
       }else{
          internalClock = true;
       }
   }
}
void trackRepeatButtons()
{
    //==============Track duration buttons
    decRepsBtn.update();
    if(decRepsBtn.fallingEdge())
    {
        reps--;
        if(reps < 1)
        {
          reps = 1;
        }
    }
    incReps.update();
    if(incReps.fallingEdge())
    {
        reps++;
        //TO DO: Add upper limit for repeat count if desired
    }
}

void trackTempoPot()
{
    /*static int last_value = 0;
    int val = analogRead(TEMPO_PIN);

    if(val > last_value + 4 || val < last_value - 4)
    {
        last_value = val;
        //Calculate a new tempo and update the timer
        tempo = map(val, 0, 1023, 30, 400);
        unsigned long microSecDelay = (MICROSECONDS_PER_BEAT / tempo) /24;
        Timer1.setPeriod(microSecDelay);
    }  */
}
