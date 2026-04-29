/*
 *  Wavetable Synth: monophonic
 *  Brent Edstrom, 2023
 */

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWavetable      wavetable;     //xy=100,71
AudioEffectEnvelope      envelope;      //xy=295,72
AudioMixer4              mixer;          //xy=508,91
AudioOutputI2S           i2s1;           //xy=717,86
AudioConnection          patchCord1(wavetable, envelope);
AudioConnection          patchCord2(envelope, 0, mixer, 0);
AudioConnection          patchCord3(mixer, 0, i2s1, 0);
AudioConnection          patchCord4(mixer, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=713,174
// GUItool: end automatically generated code
#include "strings_samples.h"

byte last_note = 0;


void setup() 
{
    
  AudioMemory(20);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.8);
  mixer.gain(0, 0.7);

  wavetable.setInstrument(strings);
 
  //MIDI Callback setup
  usbMIDI.setHandleNoteOff(onNoteOff);
  usbMIDI.setHandleNoteOn(onNoteOn);

}

void loop() 
{
    while (usbMIDI.read()) {
    }

    //Read potentiometers
    int knob1 = analogRead(A0);
    int knob2 = analogRead(A1);

    //Scale potentiomer values (0-1023) to duration (0-4 seconds)
    float attack = map(knob1, 0, 1023, 0, 4000);
    float release = map(knob2, 0, 1023, 0, 4000);

    //Update the envelope
    envelope.attack(attack);
    envelope.release(release);

}

void onNoteOn(byte channel, byte note, byte velocity) 
{
    wavetable.playNote(note, velocity);
    envelope.noteOn();
    last_note = note;
}

void onNoteOff(byte channel, byte note, byte velocity) 
{
    if(note == last_note)
    {
      envelope.noteOff();
    }
}
