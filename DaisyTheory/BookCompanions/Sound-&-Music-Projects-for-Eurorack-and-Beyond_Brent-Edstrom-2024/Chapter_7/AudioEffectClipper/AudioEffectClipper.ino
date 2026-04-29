/*
 *  AudioEffectClipper
 *  Brent Edstrom, 2020
 */

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Bounce.h>
#include "effect_clipper.h"

// GUItool: begin automatically generated code
AudioInputUSB            usb1;           //xy=99,115
//AudioEffectFlange        EightBit;        //xy=274,109
AudioEffectClipper       clipper;
AudioOutputI2S           i2s1;           //xy=513,114
AudioConnection          patchCord1(usb1, 0, clipper, 0);
AudioConnection          patchCord2(clipper, 0, i2s1, 0);
AudioConnection          patchCord3(clipper, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=499,226
// GUItool: end automatically generated code

bool passthrough = false;

Bounce passthroughBtn = Bounce(1, 15);

void setup() 
{
    pinMode(1,INPUT_PULLUP);

    AudioMemory(20);
    sgtl5000_1.enable();
    sgtl5000_1.volume(0.6);
    delay(1000);

    clipper.passthrough(passthrough);
}

void loop() 
{   
    //Handle potentiometer
    static int last_value = 0;
    int pot = analogRead(A7);
    if(pot != last_value)
    {
        last_value = pot;
        //Invert the input (larger values = more distortion)
        int inverted_pot = 1023 - pot; 
        float clip_point = ((float) inverted_pot) /1023.0;
        clipper.setClipPoint(clip_point);
    }
    
    //Handle pass-through switch
    passthroughBtn.update();
    if(passthroughBtn.fallingEdge())
    {
        if(passthrough == true)
        {
           passthrough = false;
           clipper.passthrough(passthrough);
        }else{
            passthrough = true;
            clipper.passthrough(passthrough);
        }
    }
}
