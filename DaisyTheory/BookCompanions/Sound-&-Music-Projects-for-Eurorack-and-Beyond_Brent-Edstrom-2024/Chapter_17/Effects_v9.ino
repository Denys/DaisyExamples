/*  
 *  Eurorack Effects v1.8
 *  Brent Edstrom, 5/30/2024
 *  The unit contains four effects: reverb, flanger, bit crusher, and delay
 *  
 *  One switch is provided for effect selection, and four potentiometers 
 *  control effect parameters:
 *    
 *    volume_pot:   output volume
 *    wetness_pot:  ratio of dry and wet signal (dry is the same as passthrough)
 *    freq_pot:     sets flanger frequency, reverb room size, and delay amount
 *    depth_pot:    sets flanger depth, reverb damping, and bitcrusher bits
 */

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioInputI2S            i2s2;           //xy=74.19999694824219,249
AudioEffectDelay         delayEffect;    //xy=275.1999969482422,378
AudioEffectFreeverb      freeverb;       //xy=295.20001220703125,131
AudioEffectFlange        flanger;        //xy=307.2000274658203,187.1999969482422
AudioEffectBitcrusher    bitcrusher;    //xy=310.1999969482422,261.1999969482422
AudioMixer4              drySignal;      //xy=337.1999969482422,59
AudioMixer4              delayMixer;     //xy=457.1999969482422,351
AudioMixer4              effects;        //xy=510.1999969482422,163
AudioMixer4              effectsMixer;   //xy=780.1999969482422,181
AudioMixer4              main_mix;       //xy=977.1999969482422,199
AudioOutputI2S           i2s1;           //xy=1177.1999969482422,284
AudioConnection          patchCord1(i2s2, 0, drySignal, 0);
AudioConnection          patchCord2(i2s2, 0, freeverb, 0);
AudioConnection          patchCord3(i2s2, 0, delayEffect, 0);
AudioConnection          patchCord4(i2s2, 0, flanger, 0);
AudioConnection          patchCord5(i2s2, 0, bitcrusher, 0);
AudioConnection          patchCord6(delayEffect, 0, delayMixer, 0);
AudioConnection          patchCord7(delayEffect, 1, delayMixer, 1);
AudioConnection          patchCord8(delayEffect, 2, delayMixer, 2);
AudioConnection          patchCord9(freeverb, 0, effects, 0);
AudioConnection          patchCord10(flanger, 0, effects, 1);
AudioConnection          patchCord11(bitcrusher, 0, effects, 2);
AudioConnection          patchCord12(drySignal, 0, effectsMixer, 0);
AudioConnection          patchCord13(delayMixer, 0, effects, 3);
AudioConnection          patchCord14(effects, 0, effectsMixer, 1);
AudioConnection          patchCord15(effectsMixer, 0, main_mix, 0);
AudioConnection          patchCord16(main_mix, 0, i2s1, 0);
AudioControlSGTL5000     sgtl5000_1;     //xy=1178.1999969482422,94
// GUItool: end automatically generated code




#include <Bounce.h>

//Enumeration to handle effect selection
enum{reverb_effect, flanger_effect, bitcrusher_effect, delay_effect};
int current_effect = reverb_effect;

//============FLANGER============
//Set up sample buffer for the flanger
#define FLANGE_DELAY_LENGTH (6*AUDIO_BLOCK_SAMPLES)
short l_delayline[FLANGE_DELAY_LENGTH];

//Sample offset and depth
int s_offset = FLANGE_DELAY_LENGTH/4;
float s_depth = FLANGE_DELAY_LENGTH/4;

//Set defaults for frequency, wetness, and level
//(useful for testing without pots)
double s_freq = 2.0;
float wetness = 0.9;
float level = 0.5;

//============REVERB============
float room_size = 1.0;
float damping = 0.25;

//============DELAY============
float first_delay = 100.0;

//============BIT CRUSHER============
int crusher_bits = 16;

//============POTS============
static int volume_pot = A3;
static int wetness_pot = A0; 
static int freq_pot = A1;
static int depth_pot = A2;
static int pot_sensitivity = 10;

//============SELECTION SWITCH============
static int selection_switch = 2;
Bounce selectionSwitch(selection_switch, 15);


void setup() 
{
  //Serial.begin(9600);
  pinMode(selection_switch, INPUT_PULLUP);
  
  AudioMemory(150);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.6);

  delay(100);

  //Configure flange effect: 
  //buffer, length, offset, depth, delay rate
  flanger.begin(l_delayline,FLANGE_DELAY_LENGTH,s_offset,s_depth,s_freq);
    
  //Configure reverb
  freeverb.roomsize(room_size);
  freeverb.damping(damping);

  //Configure delayMixer for two taps
  delayMixer.gain(0, 0.75);
  delayMixer.gain(1, 0.25);

  //Delay defaults
  delayEffect.delay(0, 200);
  delayEffect.delay(1, 400);

  //Bitcrusher
  bitcrusher.bits(16);
  
  //Select the active effect
  selectEffect(current_effect);

}

void loop() 
{
  handleSwitch();
  handleVolumePot(pot_sensitivity);
  handleWetnessPot(pot_sensitivity);
  handleFreqPot(pot_sensitivity);
  handleDepthPot(pot_sensitivity);
}

void handleSwitch()
{
  selectionSwitch.update();
  if(selectionSwitch.fallingEdge())
  {
      current_effect++;
      if(current_effect > delay_effect)
      {
          current_effect = reverb_effect;
      }
      selectEffect(current_effect);
      //Serial.print("Effect "); Serial.println(current_effect);
  }
}

void selectEffect(int effect)
{
  //Turn on the appropriate effect channel
  for(int i = 0; i < 4; i++)
  {
      if(effect == i)
      {
          //Turn active effect on
          effects.gain(i, level);
      }else{
          //Turn other effects off
          effects.gain(i, 0);
      }
  }
}

void handleVolumePot(int sensitivity)
{
    static int last_val = 0;
    //Read the value of the volume pot:
    int val = analogRead(volume_pot);

    //Update gain if the position of the pot has changed.
    if(val < last_val - sensitivity || val > last_val + sensitivity)
    {
      last_val = val;
      level = (float) val / 1023.0;
      main_mix.gain(0, level);
      //Serial.print("Volume pot: "); Serial.println(val);
    }
}

void handleWetnessPot(int sensitivity)
{
    //Set ratio of wet and dry signals:
    //wet = percentage of 0-1, dry = 1-wet
    static int last_val = 0;
    int val = analogRead(wetness_pot);
    
    if(val < last_val - sensitivity || val > last_val + sensitivity)
    {
        last_val = val;
        float wetness =  float(val) / 1023.0;
        effectsMixer.gain(1, wetness);
        effectsMixer.gain(0, 1.0 - wetness);
        //Serial.print("Wetness: ");Serial.println((int)val);
    }
}

void handleFreqPot(int sensitivity)
{
    //This pot sets flanger frequency, reverb room size, 
    //and delay amount
  
    static int last_val = 0;
    int val = analogRead(freq_pot);
    float percent = (float) val / 1023.0;
  
    if(val < last_val - sensitivity || 
       val > last_val + sensitivity)
    {
        last_val = val;
        //Serial.print("Frequency: "); Serial.println(val);
        //=====Update flanger frequency=====
        if(current_effect == flanger_effect)
        {
            s_freq = 10.0 * percent;
            if(s_freq < 0.05)
            {
              s_freq = 0.05;
            }
       
            AudioNoInterrupts();
            flanger.voices(s_offset,s_depth,s_freq);
            AudioInterrupts();
            last_val = val;
        }
        
        //=====Update reverb room size=====
        if (current_effect == reverb_effect)
        {
            AudioNoInterrupts();
            freeverb.roomsize(room_size);
            AudioInterrupts();
        }

        //=====Update delay amount=====
        if(current_effect == delay_effect)
        {
            AudioNoInterrupts();
            first_delay = 200.0 * percent;  //Max delay = 200, 400
            delayEffect.delay(0, first_delay);
            delayEffect.delay(1, first_delay * 2.0);
            AudioInterrupts();
        }
    }
    
}

void handleDepthPot(int sensitivity)
{
    //This pot sets flanger depth, reverb damping, and bitcrusher bits
    static int last_val = 0;
    int val = analogRead(depth_pot);
    float percent = (float) val / 1023.0;
    if(val < last_val - sensitivity || val > last_val + sensitivity)
    {
        last_val = val; 
        //Serial.print("Depth: "); Serial.println(val);
        //=====Update flanger depth=====
        if(current_effect == flanger_effect)
        {
            s_depth = percent * (float) FLANGE_DELAY_LENGTH;
            AudioNoInterrupts();
            flanger.voices(s_offset,s_depth,s_freq);
            AudioInterrupts();
        }
        
        //=====Update reverb damping=====
        if (current_effect == reverb_effect)
        {
            AudioNoInterrupts();
            freeverb.damping(damping);
            AudioInterrupts();
            
        }

        //=====Update bitcrusher bits=====
        if(current_effect == bitcrusher_effect)
        {
            //Constrain from 1-16 bits: 
            int num_bits = (percent * 16) +1;
            //Adjust so higher depth = greater effect
            num_bits = 17 - num_bits;
            AudioNoInterrupts();
            bitcrusher.bits(num_bits);
            AudioInterrupts();
            //Serial.print("Crusher bits: "); Serial.println(num_bits);
        }
    }
}
