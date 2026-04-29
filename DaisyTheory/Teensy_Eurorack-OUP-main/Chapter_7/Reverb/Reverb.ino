/*
 *  Reverb demo
 *  Brent Edstrom, 2023
 *  
 *  Notes:
 *  Connect three potentiometers to A5, A6, and A7 respectively:
 *  -one outer lead connects to 3.3V and the other outer lead connects to GND
 *  -the center pot pin connects to each of the analog inputs listed above
 */

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioInputUSB            usb1;           //xy=75,155
AudioMixer4              mixer1;         //xy=238,166
AudioEffectFreeverb      freeverb1;      //xy=403,104
AudioMixer4              mixer2;         //xy=556,189
AudioOutputI2S           i2s1;           //xy=753,121
AudioConnection          patchCord1(usb1, 0, mixer1, 0);
AudioConnection          patchCord2(usb1, 1, mixer1, 1);
AudioConnection          patchCord3(mixer1, freeverb1);
AudioConnection          patchCord4(mixer1, 0, mixer2, 1);
AudioConnection          patchCord5(freeverb1, 0, mixer2, 0);
AudioConnection          patchCord6(mixer2, 0, i2s1, 0);
AudioConnection          patchCord7(mixer2, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=763,287
// GUItool: end automatically generated code

void setup() 
{
    AudioMemory(10);
    sgtl5000_1.enable();
    sgtl5000_1.volume(0.5);
    
    mixer1.gain(0, 0.5); //Left and Right channels each at 50% 
    mixer1.gain(1, 0.5);
}

void loop() 
{
    float wetness = (float)analogRead(A7) / 1023.0;
    float size = (float)analogRead(A6) / 1023.0;
    float damping = (float)analogRead(A5) / 1023.0;

    mixer2.gain(0, wetness);
    mixer2.gain(1, 1.0 - wetness);
    freeverb1.roomsize(size);
    freeverb1.damping(damping);
}
