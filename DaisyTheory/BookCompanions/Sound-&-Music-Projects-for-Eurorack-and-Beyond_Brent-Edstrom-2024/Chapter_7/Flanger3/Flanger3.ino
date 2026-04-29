/*
 *  Flange demo
 *  Brent Edstrom, 2020
 *  Adapted from a PJRC demo
 */

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Bounce.h>

// GUItool: begin automatically generated code
AudioInputUSB            usb1;           //xy=117,134
AudioEffectFlange        flange_R;       //xy=315,289
AudioEffectFlange        flange_L;       //xy=330,67
AudioMixer4              mixer_l;         //xy=537,137
AudioMixer4              mixer_r;         //xy=537,221
AudioOutputI2S           i2s1;           //xy=702,153
AudioConnection          patchCord1(usb1, 0, flange_L, 0);
AudioConnection          patchCord2(usb1, 0, mixer_l, 1);
AudioConnection          patchCord3(usb1, 1, flange_R, 0);
AudioConnection          patchCord4(usb1, 1, mixer_r, 1);
AudioConnection          patchCord5(flange_R, 0, mixer_r, 0);
AudioConnection          patchCord6(flange_L, 0, mixer_l, 0);
AudioConnection          patchCord7(mixer_l, 0, i2s1, 0);
AudioConnection          patchCord8(mixer_r, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=711,246
// GUItool: end automatically generated code


// Number of samples in each delay line
#define FLANGE_DELAY_LENGTH (6*AUDIO_BLOCK_SAMPLES)
// Allocate the delay lines for left and right channels
short l_delayline[FLANGE_DELAY_LENGTH];
short r_delayline[FLANGE_DELAY_LENGTH];

bool passthrough = false;

Bounce passthroughBtn = Bounce(1, 15);

int s_offset = FLANGE_DELAY_LENGTH/4;
int s_depth = FLANGE_DELAY_LENGTH/4;

//A range of 0.05 to 5.0 works well
double s_freq = 2.0;
float wetness = 0.9;

void setup() 
{
    pinMode(1,INPUT_PULLUP);

    AudioMemory(20);
    sgtl5000_1.enable();
    sgtl5000_1.volume(0.6);

    //Configure flange effect: 
    //buffer, length, offset, depth, delay rate
    flange_L.begin(l_delayline,FLANGE_DELAY_LENGTH,s_offset,s_depth,s_freq);
    flange_R.begin(r_delayline,FLANGE_DELAY_LENGTH,s_offset,s_depth,s_freq);
}

void loop() 
{   
    int wet_pot;
    int freq_pot;
    static int last_wet_pot = 0;
    static int last_freq_pot = 0;
    
    if(checkPot(A7, last_wet_pot, wet_pot, 25))
    {
        wetness = (float) wet_pot / 1023.0;
        mixer_l.gain(0, wetness);
        mixer_l.gain(1, 1.0 - wetness);
        mixer_r.gain(0, wetness);
        mixer_r.gain(1, 1.0 - wetness);
    }else if(checkPot(A6, last_freq_pot, freq_pot, 25))
    {
         s_freq = ((float)freq_pot / 1023.0) * 5.0;
         if(s_freq < 0.05)
         {
            s_freq = 0.05;
         }
         if(!passthrough)
        {
            AudioNoInterrupts();
            flange_L.voices(s_offset,s_depth,s_freq);
            flange_R.voices(s_offset,s_depth,s_freq);
            AudioInterrupts();
        }
    }
     
    //Track passthrough button
    passthroughBtn.update();
    if(passthroughBtn.fallingEdge())
    {
        if(passthrough == true)
        {
             passthrough = false;
             AudioNoInterrupts();
             flange_L.voices(s_offset,s_depth,s_freq);
             flange_R.voices(s_offset,s_depth,s_freq);
             AudioInterrupts();
        }else{
            passthrough = true;
            flange_L.voices(FLANGE_DELAY_PASSTHRU,0,0);
            flange_R.voices(FLANGE_DELAY_PASSTHRU,0,0);    
        }
    }
}


bool checkPot(int pot_pin, int &last_value, int &new_value, int sensitivity)
{
    int val = analogRead(pot_pin);
    if(val < last_value - sensitivity || val > last_value + sensitivity)
    {
        last_value = val;
        new_value = val;
        return true; //value changed
    }
    return false; //no value change
}
