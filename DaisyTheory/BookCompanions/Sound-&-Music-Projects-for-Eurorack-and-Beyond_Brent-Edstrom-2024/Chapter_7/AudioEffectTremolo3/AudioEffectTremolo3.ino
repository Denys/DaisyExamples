/*
 *  AudioEffectTremolo
 *  Brent Edstrom, 2020
 */

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Bounce.h>
#include "effect_tremolo.h"

// GUItool: begin automatically generated code
AudioInputUSB            usb1;           //xy=99,115
//AudioEffectFlange        EightBit;        //xy=274,109
AudioEffectTremolo       tremolo;
AudioOutputI2S           i2s1;           //xy=513,114
AudioConnection          patchCord1(usb1, 0, tremolo, 0);
AudioConnection          patchCord2(tremolo, 0, i2s1, 0);
AudioConnection          patchCord3(tremolo, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=499,226
// GUItool: end automatically generated code

bool passthrough = false;

Bounce passthroughBtn = Bounce(1, 15);
float freq = 5.0;

void setup() 
{
    pinMode(1,INPUT_PULLUP);

    AudioMemory(20);
    sgtl5000_1.enable();
    sgtl5000_1.volume(0.6);
    delay(1000);

    tremolo.passthrough(passthrough);
}

void loop() 
{     
    //Handle potentiometers
    static int last_freq_value = 0;
    int pot = analogRead(A7);
    if(pot != last_freq_value)
    {
        last_freq_value = pot;
        freq = (pot /1023.0) * 10.0;
        tremolo.setFrequency(freq);
    }

    static int last_amp_value = 0;
    pot = analogRead(A6);
    if(pot != last_amp_value)
    {
        last_amp_value = pot;
        float amp = (pot /1023.0);;
        tremolo.setAmplitude(amp);
    }
    
    passthroughBtn.update();
    if(passthroughBtn.fallingEdge())
    {
        if(passthrough == true)
        {
           passthrough = false;
           tremolo.passthrough(passthrough);
        }else{
            passthrough = true;
            tremolo.passthrough(passthrough);
        }
    }
}
