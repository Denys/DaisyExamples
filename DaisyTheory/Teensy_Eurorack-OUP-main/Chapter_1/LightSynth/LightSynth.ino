/*
 *  Light Synth
 *  Brent Edstrom, 2023
 */

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       waveform1;      //xy=105,339
AudioSynthWaveform       waveform2;      //xy=107,402
AudioMixer4              mixer1;         //xy=283,359
AudioFilterStateVariable filter1;        //xy=450,366
AudioOutputI2S           i2s1;           //xy=711,357
AudioConnection          patchCord1(waveform1, 0, mixer1, 0);
AudioConnection          patchCord2(waveform2, 0, mixer1, 1);
AudioConnection          patchCord3(mixer1, 0, filter1, 0);
AudioConnection          patchCord4(filter1, 0, i2s1, 0);
AudioConnection          patchCord5(filter1, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=711,501
// GUItool: end automatically generated code


elapsedMillis elapsedMS; 
const float detune = 1.01;

void setup() 
{
    AudioMemory(20); 
    sgtl5000_1.enable();
    sgtl5000_1.volume(0.60);
    mixer1.gain(0, 0.40);
    mixer1.gain(1, 0.40);

    //Configure filter resonance
    filter1.resonance(4.8); //range is from 0.7 to 5.0
    filter1.octaveControl(1);

    waveform1.begin(0.40, 80, WAVEFORM_SQUARE);
    waveform2.begin(0.40, 82, WAVEFORM_SQUARE);
}

void loop() 
{
    //Sample LDRs 20 times per second
    if(elapsedMS >= 50)
    {
        elapsedMS = 0;
        float filter_cutoff = analogRead(A0) + 1500;
        float freq = analogRead(A1) + 50;

        //Update filter and oscillator with new values
        filter1.frequency(filter_cutoff);
        waveform1.frequency(freq);
        waveform2.frequency(freq * detune);
    }
}
