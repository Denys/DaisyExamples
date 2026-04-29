/*
 *   Simple Polysynth
 *   Brent Edstrom, 2020
 */

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       waveform1;      //xy=85,93
AudioSynthWaveform       waveform3;      //xy=85,243
AudioSynthWaveform       waveform2;      //xy=86,167
AudioSynthWaveform       waveform4;      //xy=86,325
AudioEffectEnvelope      envelope1;      //xy=284,93
AudioEffectEnvelope      envelope2;      //xy=288,168
AudioEffectEnvelope      envelope3;      //xy=288,243
AudioEffectEnvelope      envelope4;      //xy=292,325
AudioMixer4              mixer;          //xy=497,112
AudioOutputI2S           i2s1;           //xy=706,107
AudioConnection          patchCord1(waveform1, envelope1);
AudioConnection          patchCord2(waveform3, envelope3);
AudioConnection          patchCord3(waveform2, envelope2);
AudioConnection          patchCord4(waveform4, envelope4);
AudioConnection          patchCord5(envelope1, 0, mixer, 0);
AudioConnection          patchCord6(envelope2, 0, mixer, 1);
AudioConnection          patchCord7(envelope3, 0, mixer, 2);
AudioConnection          patchCord8(envelope4, 0, mixer, 3);
AudioConnection          patchCord9(mixer, 0, i2s1, 0);
AudioConnection          patchCord10(mixer, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=702,195
// GUItool: end automatically generated code

#include "SimpleSynth.h"

SimpleSynth simpleSynth;

void setup() 
{
  AudioMemory(20);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.8);

  //Set voice levels
  mixer.gain(0, 0.25);
  mixer.gain(1, 0.25);
  mixer.gain(2, 0.25);
  mixer.gain(3, 0.25);

  //Set note prioritization
  simpleSynth.notePrioritization = SimpleSynth::oldest_note;

  //Attach audio objects
  simpleSynth.voices[0].attachAudioObjects(&waveform1, &envelope1);
  simpleSynth.voices[1].attachAudioObjects(&waveform2, &envelope2);
  simpleSynth.voices[2].attachAudioObjects(&waveform3, &envelope3);
  simpleSynth.voices[3].attachAudioObjects(&waveform4, &envelope4);

  //MIDI Callback setup
  usbMIDI.setHandleNoteOff(onNoteOff);
  usbMIDI.setHandleNoteOn(onNoteOn);
  
}

void loop() 
{
    while (usbMIDI.read()) {
    }

    int attackKnob = analogRead(A0);
    int releaseKnob = analogRead(A1);

    int attack = map(attackKnob, 0, 1023, 0, 1000);
    float release = map(releaseKnob, 0, 1023, 0, 1000);
    simpleSynth.setAttack(attack);
    simpleSynth.setRelease(release);

}

void onNoteOn(byte channel, byte note, byte velocity) 
{
    simpleSynth.noteOn(channel, note, velocity);
}

void onNoteOff(byte channel, byte note, byte velocity) 
{
    simpleSynth.noteOff(channel, note, velocity);
}
