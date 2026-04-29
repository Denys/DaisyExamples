/* Teensy 4.0 with Audio Shield
   FM-1KTest
   1Khz carrier modulated by 100 Hz modulator
   Brent Edstrom, 2020
 */

//Added by Audio System Design Tool
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveformModulated modulatorOsc;           //xy=128.8235626220703,245.00006103515625
AudioSynthWaveformModulated carrierOsc;           //xy=339.82359313964844,251.00003051757812
AudioMixer4              mainMixer;      //xy=543.8235626220703,271.0000457763672
AudioOutputI2S           i2s1;           //xy=763.8235626220703,275.0000457763672
AudioConnection          patchCord1(modulatorOsc, 0, carrierOsc, 0);
AudioConnection          patchCord2(carrierOsc, 0, mainMixer, 0);
AudioConnection          patchCord3(mainMixer, 0, i2s1, 0);
AudioConnection          patchCord4(mainMixer, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=774.8235626220703,365.0000762939453
// GUItool: end automatically generated code

float freq = 100;
float amp = 0.3;

void setup() 
{
    AudioMemory(12);
    sgtl5000_1.enable();
    sgtl5000_1.volume(0.5);

    carrierOsc.begin(0.3, 1000, WAVEFORM_SINE);
    modulatorOsc.begin(amp, freq, WAVEFORM_SINE);
}

void loop() 
{
  
}
